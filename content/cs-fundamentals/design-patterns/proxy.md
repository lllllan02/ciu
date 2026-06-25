---
title: 代理模式
order: 12
---

**代理模式**（Proxy）为其他对象提供一种代理以控制对这个对象的访问。

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

定义与 **主题** 相同的接口；**真实主题** 负责 SQL / RPC；**代理** 实现同一接口，持有 `real`，在委托前后插入控制。客户端 **只注入接口**。

### 主题与保护代理

```go
type OrderRepository interface {
    GetOrder(ctx context.Context, id string) (*Order, error)
    Save(ctx context.Context, order *Order) error
}

type SQLOrderRepository struct{ db *sql.DB } // 只管持久化

type ProtectionProxy struct {
    real OrderRepository
    auth AuthChecker
}

func (p *ProtectionProxy) GetOrder(ctx context.Context, id string) (*Order, error) {
    if !p.auth.CanRead(ctx, id) {
        return nil, ErrForbidden
    }
    return p.real.GetOrder(ctx, id)
}
```

`Save` 同理：鉴权通过再 `p.real.Save`。鉴权集中在代理里，`SQLOrderRepository` **不必** import JWT 或 HTTP。

### 其他代理形态（同一套路）

| 类型 | 控制什么 | 要点 |
| :--- | :--- | :--- |
| **虚拟** | 延迟加载 | `GetOrder` 只查 header；第一次访问明细再 `LoadLines` |
| **远程** | 网络访问 | 在 `CheckStock` 外包 timeout / 重试，对内仍是 `InventoryService` |
| **缓存** | 读多写少 | 只读 `CheckStock` 可短 TTL；`Reserve` / `Save` **直通** inner |

三种都是 **同接口 + 持有 inner + 委托前后加逻辑**，与保护代理结构相同；差别只在 **控制动机**（懒加载 / 远程 / 缓存 vs 鉴权）。

### 客户端

组装层可 **链式** 包装；Handler 只认 `OrderRepository`：

```go
repo := &ProtectionProxy{
    real: &SQLOrderRepository{db: db},
    auth: authChecker,
}
adminHandler := &AdminHandler{repo: repo}
```

测试时把 `repo` 换成 fake，无需 DB、gRPC。虚拟 / 缓存等细节见 **实践** 一节。


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

## 实践

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

## 关联

- 从接口形态看：[适配器模式](/cs-fundamentals/design-patterns/adapter) 为被封装对象提供 **不同的** 接口；代理模式提供 **相同的** 接口；[装饰模式](/cs-fundamentals/design-patterns/decorator) 提供 **增强后的** 接口。
- [外观模式](/cs-fundamentals/design-patterns/facade) 与代理的相似之处在于它们都缓存了一个复杂实体并自行对其进行初始化。代理与其服务对象遵循同一接口，使得自己和服务对象可以互换，在这一点上它与外观不同。
- [装饰模式](/cs-fundamentals/design-patterns/decorator) 与代理有着相似的结构，但是其意图却非常不同。这两个模式的构建都基于组合原则，也就是说一个对象应该将部分工作委派给另一个对象。两者之间的不同之处在于代理通常自行管理其服务对象的生命周期，而装饰的生成则总是由客户端进行控制。

## 参考阅读

- [x] [Refactoring.Guru - 代理模式](https://refactoringguru.cn/design-patterns/proxy) (2026-06-22)
- [x] [菜鸟教程 - 代理模式](https://www.runoob.com/design-pattern/proxy-pattern.html) (2026-06-22)
