---
title: 单例模式
order: 5
---

**单例模式** 保证一个类仅有一个实例，并提供一个访问它的全局访问点。

通俗地说，整个进程里只需要一份的那种组件，把「怎么拿到那唯一一份」收进类型自身；各处调用都走同一个入口取实例，限流、连接池、全局状态由此真正共享，而不是走到哪儿各自新建一份。

## 问题

有些组件 **天然在进程里只应有一份**：结算中心、连接池、全局限流器、指标收集器。最直接的做法是在每个用到的地方各自 `New` 一份。

调用点少时看不出问题；入口一多，麻烦就会一起冒出来：

1. **状态分裂**：限流器按实例各自计数，「全局每秒 1000 条」变成 N×1000，容易打爆下游。
2. **资源浪费**：每个实例各持一套连接池，连接数和内存随调用点线性增长。
3. **指标失真**：监控数据分散在多个实例里，看不到真实的结算量。
4. **初始化重复**：支付客户端、TLS 握手、凭证加载在每条路径上重复执行。

本质矛盾是：**系统语义上只需要一个协调者**，代码却允许 **随处 `New` 出多个**。典型写法如下：

```go
func submitOrder(...) error {
    hub := NewCheckoutHub(...) // 各自 new 限流器、metrics
    return hub.Checkout(...)
}

func submitRefund(...) error {
    hub := NewCheckoutHub(...) // 又一个 hub——配额与指标互不共享
    return hub.Refund(...)
}
```

## 解决方案

把「全局只应有一份」的 `CheckoutHub` 收进单例；提交路径只 **取用**，不 **新建**。

### 经典结构（概念）

```go
type CheckoutHub struct {
    limiter *RateLimiter
}

var (
    hubInstance *CheckoutHub
    hubOnce     sync.Once
)

// Instance 懒加载：首次调用时初始化，之后始终返回同一指针
func Instance() *CheckoutHub {
    hubOnce.Do(func() {
        hubInstance = &CheckoutHub{limiter: NewRateLimiter(1000)}
    })
    return hubInstance
}

func (h *CheckoutHub) Checkout(n *Order) error {
    if err := h.limiter.Wait(context.Background()); err != nil {
        return err
    }
    // 路由支付、记录指标…
    return pay(n)
}
```

### 使用者

业务入口只 **取用** 单例，不各自 `New`：

```go
func submitOrder(order *Order) error {
    return checkouthub.Instance().Checkout(order)
}

func submitRefund(order *Order) error {
    return checkouthub.Instance().Refund(order)
}
```

## 适用场景

1. **必须全局唯一的状态**：限流器、计数器、配置快照、任务调度器令牌。
2. **昂贵资源的单一入口**：数据库连接池、订单队列 producer、到第三方 API 的长连接客户端。
3. **协调多子系统**：本文的 `CheckoutHub` 统一路由支付宝 / 微信支付 / 信用卡。
4. **日志、追踪、插件注册表**：全进程一份注册表，避免重复注册或状态不一致。
5. **与 [原型](/cs-fundamentals/design-patterns/prototype) 分工**：原型负责 **复制订单模板**；单例 hub 负责 **用同一份基础设施发出去**。

常见例子：`database/sql` 的 `DB` 往往以单例方式在进程内复用；应用级 `Logger`；硬件驱动访问层。

**不必强行使用**：无共享状态、每次调用需要独立实例（带请求上下文的产品对象）——用 [工厂方法](/cs-fundamentals/design-patterns/factory) 每次 `New` 更合适。单例滥用会导致 **隐式全局状态**、测试困难；若「唯一」只需在 `main` 组装层保证，**依赖注入** 往往比 `Instance()` 更清晰。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **状态一致** | 限流、指标、连接池全进程一份，语义与运维预期一致 |
| **节省资源** | 昂贵初始化只做一次 |
| **访问简单** | `Instance()` 随处可取，不必层层传参 |
| **与创建型模式互补** | 工厂造支付渠道、原型造订单体、单例管派发枢纽 |

| 缺点 | 说明 |
| :--- | :--- |
| **隐式全局依赖** | 调用链上看不出依赖 hub，耦合藏在 `Instance()` 里 |
| **测试不便** | 需 `Configure`、接口抽象或重置钩子，否则难以 mock |
| **违反单一职责** | 类型既管业务又管「如何成为唯一」 |
| **并发与生命周期** | 懒初始化要考虑 `sync.Once`；进程内唯一 ≠ 多副本部署时的分布式唯一 |
| **过度使用** | 把本可注入的普通服务做成单例，扩大全局状态面 |

## 实践

> **阅读提示**：先掌握「`sync.Once` + `Instance()`」即可。本节是 Go 项目里的实现细节与替代方案；初学可先跳过。

### `sync.Once` 与饿汉 / 懒汉

| 方式 | 写法 | 特点 |
| :--- | :--- | :--- |
| **饿汉** | `var hub = NewCheckoutHub(...)` 包初始化 | 简单、天然并发安全；若 `New` 很重且未必用到，浪费启动时间 |
| **懒汉（Once）** | `hubOnce.Do(func() { … })` | 首次访问才初始化；Go 推荐写法 |
| **懒汉（无 Once，反例）** | `if hub == nil { hub = New… }` | 竞态，**不要**在并发下这样写 |

```go
// 饿汉：依赖简单、必定会用时
var defaultLimiter = NewRateLimiter(1000)

// 懒汉：依赖重或可能不用时
func Instance() *CheckoutHub {
    hubOnce.Do(initHub)
    return hubInstance
}
```

### 依赖注入与单例的取舍

单例要约束的是 **进程内只有一份实例**；至于这份实例 **怎么交到业务手里**，常见有两种写法，别混为一谈。

**`Instance()` 全局访问点**：类型自己保管唯一实例，调用方随处 `checkouthub.Instance()` 取用。好处是调用简单，库和 SDK 边界也常见——用户不必从 `main` 一路传参。代价是依赖藏在静态入口里，读代码时看不出 `CheckoutService` 其实依赖 hub，测试时也难直接换成 mock。

**组装层注入同一指针**：`main` 里 `hub := NewCheckoutHub()` 只执行一次，再把 **同一个** `*CheckoutHub` 传给 `CheckoutService`、`RefundHandler` 等。业务代码用的是字段 `s.hub`，不是 `Instance()`。实例数量仍是 **一份**，限流与指标照样共享；「唯一性」由组装层保证，而不是类型内部的隐藏全局。

两种写法 **语义等价**（同一指针、同一份状态），差别在依赖是否可见、测试是否顺手：

```go
// main：只构造一次，多处注入同一指针
hub := NewCheckoutHub()
svc := NewCheckoutService(hub)

type CheckoutService struct {
    hub *CheckoutHub
}

func NewCheckoutService(hub *CheckoutHub) *CheckoutService {
    return &CheckoutService{hub: hub}
}

func (s *CheckoutService) Submit(order *Order) error {
    return s.hub.Checkout(order)
}
```

| | `Instance()` 单例 | 组装层注入同一指针 |
| :--- | :--- | :--- |
| 实例数量 | 进程内唯一 | 进程内唯一 |
| 依赖可见性 | 隐式，调用链上看不出 | 显式，构造参数即文档 |
| 测试 | 常需 `Configure`、reset 或接口抽象 | 直接 `NewCheckoutService(mockHub)` |
| 常见场景 | 库、SDK、必须随处可取的全局设施 | **应用内部** 更常见 |

**结论**：需要「随处可取的全局入口」时用 `Instance()`；应用内部更推荐 **组装层造一份、注入传递**——仍是单例语义，但不引入隐藏全局，也更符合 Go 的显式依赖习惯。

### 与枚举式单例

Go 无 enum，常用 **包级变量 + 不导出类型** 表达「仅此一份」：

```go
var Hub = newCheckoutHub() // 包内初始化，对外只读使用 Hub.Checkout

type checkoutHub struct { /* 小写，包外无法 New */ }
```

与 `Instance()` 等价，风格更「Go idiom」。

## 关联

- [外观模式](/cs-fundamentals/design-patterns/facade) 类通常可以转换为 [单例模式](/cs-fundamentals/design-patterns/singleton) 类——在大部分情况下，一个外观对象就足够了。
- 如果你能将对象的所有共享状态简化为一个享元对象，[享元模式](/cs-fundamentals/design-patterns/flyweight) 就和单例有些相似；但二者有两个根本区别：进程内 **只有一个** 单例实例，享元类却可以有多份实体、各份内在状态也可不同；单例对象 **可以是可变的**，享元对象 **应当是不可变的**。
- [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory)、[生成器模式](/cs-fundamentals/design-patterns/builder) 和 [原型模式](/cs-fundamentals/design-patterns/prototype) 都可以用 [单例模式](/cs-fundamentals/design-patterns/singleton) 来实现。

## 参考阅读

- [x] [Refactoring.Guru - 单例模式](https://refactoringguru.cn/design-patterns/singleton) (2026-06-17)
- [x] [菜鸟教程 - 单例模式](https://www.runoob.com/design-pattern/singleton-pattern.html) (2026-06-17)
