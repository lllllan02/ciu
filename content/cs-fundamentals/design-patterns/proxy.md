---
title: 代理模式
order: 12
---

**代理模式** 为其他对象提供一种代理以控制对这个对象的访问。

通俗地说，在真正干活的实现外面包一层替身，对外接口不变；访问前后可以在这一层统一做鉴权、延迟加载、缓存、远程重试等控制，调用方不必处处重复这些逻辑。

## 问题

运营后台、移动端、对账 Worker 都依赖 `OrderRepository` 查订单。真实实现 **直连数据库**，但调用方越来越多，各自 **裸调** 真实对象——没有懒加载、没有鉴权、没有缓存。

调用点少时还能应付；订单体量变大后，问题就会一起暴露：

1. **加载过重**：列表页只要订单摘要，`GetOrder` 却 **总是** 拉全量明细——DB 和堆压力随 QPS 涨。
2. **鉴权散落**：Admin、客服、买家三个入口各写一遍「是否本人 / 是否有权限」，漏一处就 **越权读单**。
3. **远程细节泄漏**：超时、重试、连接池摊在每个 Handler 里，改 gRPC 地址要改 N 处。
4. **缓存不一致**：有的入口缓存库存预检 5 秒、有的不缓存——超卖和脏读各算各的。

本质矛盾是：**主题接口稳定**，但 **何时加载、谁有权访问、是否走缓存** 应在 **一处** 统一控制，却 **不应** 写进 SQL 实现，也不该在每个 Controller 复制。典型写法如下：

```go
func (h *AdminHandler) ShowOrder(w http.ResponseWriter, r *http.Request) {
    orderID := r.URL.Query().Get("id")
    // 无鉴权；GetOrder 内部 JOIN 10 万行明细——列表页其实只要 header
    order, _ := h.repo.GetOrder(r.Context(), orderID)
    json.NewEncoder(w).Encode(order)
}
```

## 解决方案

定义与 **主题** 相同的接口 `OrderRepository`；**真实主题** `SQLOrderRepository` 负责 SQL；**代理** 实现同一接口，持有 `real`，在委托前后插入控制。客户端 **只注入接口**。

### 主题（Subject）与真实主题（Real Subject）

```go
type Order struct {
    ID     string
    UserID string
    Status string
    Amount int64
    lines  []OrderLine // 小写：由仓库填充，列表页可不加载
}

func (o *Order) Lines() []OrderLine { return o.lines }

type OrderRepository interface {
    GetOrder(ctx context.Context, orderID string) (*Order, error)
    LoadLines(ctx context.Context, orderID string) ([]OrderLine, error)
    Save(ctx context.Context, order *Order) error
}

type SQLOrderRepository struct {
    db *sql.DB
}

func (r *SQLOrderRepository) GetOrder(ctx context.Context, orderID string) (*Order, error) {
    // SELECT id, user_id, status, amount FROM orders WHERE id = ?
    // 不 JOIN lines
}

func (r *SQLOrderRepository) LoadLines(ctx context.Context, orderID string) ([]OrderLine, error) {
    // SELECT … FROM order_lines WHERE order_id = ?
}
```

### 虚拟代理（Virtual Proxy）——延迟加载明细

```go
type LazyOrderProxy struct {
    real OrderRepository
}

func (p *LazyOrderProxy) GetOrder(ctx context.Context, orderID string) (*Order, error) {
    order, err := p.real.GetOrder(ctx, orderID)
    if err != nil {
        return nil, err
    }
    return &lazyOrder{
        header: order,
        real:   p.real,
        ctx:    ctx,
    }, nil
}

type lazyOrder struct {
    header *Order
    real   OrderRepository
    ctx    context.Context
    lines  []OrderLine
    loaded bool
}

func (o *lazyOrder) Lines() []OrderLine {
    if !o.loaded {
        o.lines, _ = o.real.LoadLines(o.ctx, o.header.ID)
        o.loaded = true
    }
    return o.lines
}
```

列表页只读 `Amount`、`Status` ** never 触发** `LoadLines`；详情页第一次 `Lines()` 才查 DB。可与 [享元](/cs-fundamentals/design-patterns/flyweight) 衔接：`LoadLines` 返回 `(sku, ctx)`，由工厂 `Get(sku)` 绑定元数据。

### 保护代理（Protection Proxy）——访问控制

```go
type AuthChecker interface {
    CanReadOrder(ctx context.Context, orderID string) bool
    CanWriteOrder(ctx context.Context, orderID string) bool
}

type ProtectionProxy struct {
    real OrderRepository
    auth AuthChecker
}

func (p *ProtectionProxy) GetOrder(ctx context.Context, orderID string) (*Order, error) {
    if !p.auth.CanReadOrder(ctx, orderID) {
        return nil, ErrForbidden
    }
    return p.real.GetOrder(ctx, orderID)
}

func (p *ProtectionProxy) Save(ctx context.Context, order *Order) error {
    if !p.auth.CanWriteOrder(ctx, order.ID) {
        return ErrForbidden
    }
    return p.real.Save(ctx, order)
}
```

鉴权 **集中在代理**；`SQLOrderRepository` **不 import** `http` 或 JWT——符合 [单一职责](/cs-fundamentals/design-patterns#设计原则)。[外观](/cs-fundamentals/design-patterns/facade) 的 `PlaceOrder` 仍调 **已注入权限上下文** 的 repo；后台代客下单可走 **另一套** `AuthChecker` 实现，代理层不变。

### 远程代理（Remote Proxy）——封装跨区库存

```go
type InventoryService interface {
    CheckStock(ctx context.Context, sku string, qty int) error
    Reserve(ctx context.Context, lines []OrderLine) error
}

type RemoteInventoryProxy struct {
    client pb.InventoryClient
    timeout time.Duration
}

func (p *RemoteInventoryProxy) CheckStock(ctx context.Context, sku string, qty int) error {
    ctx, cancel := context.WithTimeout(ctx, p.timeout)
    defer cancel()
    _, err := p.client.CheckStock(ctx, &pb.CheckStockRequest{Sku: sku, Qty: int32(qty)})
    return err
}
```

Checkout、[外观](/cs-fundamentals/design-patterns/facade) 内 `inventory` 字段类型仍是 `InventoryService`——**本地实现** 与 **gRPC 代理** 可替换；与 [适配器](/cs-fundamentals/design-patterns/adapter) 组合：Adapter 把 SDK 类型 **译成** `InventoryService`，Remote Proxy 管 **网络与控制**。

### 缓存代理（Caching Proxy）——短 TTL 库存预检

```go
type CachingInventoryProxy struct {
    inner InventoryService
    cache *ttlcache.Cache[string, error] // key: sku+qty
    ttl   time.Duration
}

func (p *CachingInventoryProxy) CheckStock(ctx context.Context, sku string, qty int) error {
    key := sku + ":" + strconv.Itoa(qty)
    if v, ok := p.cache.Get(key); ok {
        return v
    }
    err := p.inner.CheckStock(ctx, sku, qty)
    p.cache.Set(key, err, p.ttl)
    return err
}
```

`Reserve` **必须** 直通 `inner`（写操作不缓存）；`CheckStock` 只读可缓存——**智能引用** 可在 `Reserve` 成功后 **Invalidate** 相关 sku 键。

### 客户端（Client）——组装链

```go
realRepo := &SQLOrderRepository{db: db}
repo := &ProtectionProxy{
    real: &LazyOrderProxy{real: realRepo},
    auth: authChecker,
}

adminHandler := &AdminHandler{repo: repo}
```

客户端 **只依赖** `OrderRepository`；测试时 `repo` 换为 **内存 fake**，无需 DB、gRPC、Redis。


## 适用场景

1. **访问需要控制**：权限、租户隔离、审计前置条件——**保护代理**。
2. **创建或加载成本高**：大对象、大 JOIN、远程连接——**虚拟 / 远程代理** 延迟或本地化。
3. **读多写少且可容忍短暂 stale**：库存预检、商品详情——**缓存代理**（写路径 **失效** 缓存）。
4. **想在不改 Client 的前提下换实现**：本地 dev 用内存 repo，生产用 SQL + 代理链——**依赖倒置** + 代理。
5. **横切控制应 reusable**：同一套 `ProtectionProxy` 供 HTTP、MQ、CLI 共用——别只在 Controller 写 middleware。

**不必强行使用**：

- 对象 **总是** 全量加载、**无** 权限差异、调用 **纯本地且廉价**——多一层 indirection 只增复杂度。
- 需要的是 **改接口形状**（Stripe SDK → `PaymentProcessor`）——用 [适配器](/cs-fundamentals/design-patterns/adapter)，不是代理。
- 需要的是 **给 `Total()` 叠折扣**——用 [装饰器](/cs-fundamentals/design-patterns/decorator)。
- 需要的是 **编排库存+支付+落库**——用 [外观](/cs-fundamentals/design-patterns/facade)。
- 全局 **唯一** 协调者——用 [单例](/cs-fundamentals/design-patterns/singleton)，不是按资源的访问代理。

常见例子：Hibernate lazy association、Spring `@Cacheable` + 接口、Kubernetes **Service** 作 Pod 的 **网络代理**、RPC stub、智能指针 / ARC（语言级 **引用控制**）。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新增缓存 / 鉴权 **加一层代理**，少改 `SQLOrderRepository` |
| **职责清晰** | 主题管领域；代理管 **访问策略** |
| **客户端稳定** | Handler 只认 `OrderRepository`；环境切换在组装层 |
| **可组合** | 保护 → 懒加载 → SQL；缓存 → 远程 gRPC |
| **可测** | 单测代理 **顺序与分支**；主题 **mock DB** |

| 缺点 | 说明 |
| :--- | :--- |
| **间接层数** | 链过长时 **调试栈** 变深；需命名清晰、日志带 `proxy=` 标签 |
| **与装饰混淆** | 团队纪律：**控制访问 vs 增强行为** 分文件分类型 |
| **缓存代理一致性** | TTL、Invalidate 漏做 → 超卖或 stale 展示 |
| **虚拟代理陷阱** | `lazyOrder` 若 **逃逸** 出 `ctx` 生命周期，异步读 `Lines()` 会踩坑 |
| **重复包装** | 每个接口都手写代理 **啰嗦**——可接受时用 **中间件 / 代码生成** |

## 组装实践

> **阅读提示**：先掌握「**同接口替身 + 控制访问 + 委托 real**」即可。本节是工程变体；初学可先跳过。

### 代理链顺序建议

| 顺序（外 → 内） | 原因 |
| :--- | :--- |
| **保护** 最外 | 无权限 **不打 DB、不打远程** |
| **缓存** 在远程外、保护内 | 避免 **缓存未授权数据**（key 含 tenant/user） |
| **虚拟 / 懒加载** 靠近 real | 已通过鉴权再延迟加载 |
| **real** 最内 | 纯持久化 / 纯 RPC |

```go
repo := &ProtectionProxy{
    auth: auth,
    real: &LazyOrderProxy{real: sqlRepo},
}
// 缓存订单 header 时：key 必须含 userID/tenant，且写后 Invalidate
```

### 与装饰器、外观一起用

```go
// 组装层
innerRepo := &SQLOrderRepository{db: db}
lazy := &LazyOrderProxy{real: innerRepo}
protected := &ProtectionProxy{real: lazy, auth: auth}
audited := &AuditOrderRepoDecorator{inner: protected, audit: auditLog} // 装饰：读后打日志

facade := NewCheckoutFacade(
    cachingInventoryProxy,
    pricingEngine,
    paymentProcessor,
    audited, // Facade 用的仍是 OrderRepository 接口
    /* … */
)
```

**Facade 不感知** 代理层数——只调 `orders.Save`；**享元** 在 `LoadLines` 之后绑定 SKU，与代理 **正交**。

### 虚拟代理与 `context`

`lazyOrder` 应 **存 orderID**，在 `Lines()` 内 **接收 `ctx context.Context` 参数**，而不是闭包持有 **请求已结束的 ctx**：

```go
func (o *lazyOrder) Lines(ctx context.Context) ([]OrderLine, error) {
    if !o.loaded {
        lines, err := o.real.LoadLines(ctx, o.orderID)
        // …
    }
    return o.lines, nil
}
```

若 Subject 接口 **不能改**，虚拟代理返回的 **仍是** `*Order`，则 lazy 字段应在 **同请求内** 消费——文档化 **线程 / 生命周期** 约束。

### 测试策略

```go
func TestProtectionProxy_Forbidden(t *testing.T) {
    real := &fakeOrderRepo{order: &Order{ID: "o1"}}
    proxy := &ProtectionProxy{
        real: real,
        auth: fakeAuth{allow: false},
    }
    _, err := proxy.GetOrder(context.Background(), "o1")
    if !errors.Is(err, ErrForbidden) {
        t.Fatal(err)
    }
    if real.getCalls != 0 {
        t.Fatal("real should not be called")
    }
}

func TestLazyOrderProxy_SkipsLinesUntilAccess(t *testing.T) {
    real := &fakeOrderRepo{}
    proxy := &LazyOrderProxy{real: real}
    order, _ := proxy.GetOrder(context.Background(), "o1")
    _ = order.Amount // header only
    if real.loadLinesCalls != 0 {
        t.Fatal("lines not loaded yet")
    }
    _ = order.Lines()
    if real.loadLinesCalls != 1 {
        t.Fatal("expected one load")
    }
}
```

### 动态代理与 Go

Java `InvocationHandler`、C# `DispatchProxy` 可 **运行时** 生成代理；Go 倾向 **编译期显式类型**。接口方法 **很多** 时：

- **按 concern 拆接口**（`OrderReader` / `OrderWriter`）减少样板；
- 或 **代码生成**（`go generate`）——**不要** 为省文件把鉴权+缓存+日志塞进一个「上帝 Proxy」。

## 小结

记住这四点即可：

1. **同接口替身，控制访问**：代理与 **Real Subject** 实现同一 `OrderRepository` / `InventoryService`；在 **委托前后** 做懒加载、鉴权、远程、缓存。
2. **动机区别于装饰器**：代理管 **能否 / 何时触达**；装饰器管 **触达后多做什么**（折扣、审计可装饰，鉴权宜代理）。
3. **客户端只依赖 Subject**：组装层 **链式** `Protection → Lazy → SQL`；[外观](/cs-fundamentals/design-patterns/facade) 与 Handler **不 import** gRPC/JWT 细节。
4. **写操作与缓存**：`Reserve`、`Save` **直通** real 并 **失效** 缓存；只读 `CheckStock`、`GetOrder` header 才适合缓存 / 虚拟加载。

[享元模式](/cs-fundamentals/design-patterns/flyweight) 压缩了 **SKU 元数据在内存中的重复**；代理模式压缩的是 **「每次访问都全量加载、裸连远程、无统一鉴权」** 的混乱——把 **对订单与库存资源的访问控制** 收进 **与主题同型** 的一层，主题继续专注 SQL 与领域规则。

## 参考阅读

- [x] [装饰器模式](/cs-fundamentals/design-patterns/decorator) — 同接口增强；与代理可组合、勿混动机
- [x] [外观模式](/cs-fundamentals/design-patterns/facade) — 多子系统编排；内部可注入带代理的 repo
- [x] [适配器模式](/cs-fundamentals/design-patterns/adapter) — 接口转换；常与远程代理叠用
- [x] [享元模式](/cs-fundamentals/design-patterns/flyweight) — 明细元数据共享；与虚拟代理懒加载 Lines 衔接
- [x] [Refactoring.Guru - 代理模式](https://refactoringguru.cn/design-patterns/proxy) (2026-06-22)
- [x] [菜鸟教程 - 代理模式](https://www.runoob.com/design-pattern/proxy-pattern.html) (2026-06-22)
