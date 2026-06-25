---
title: 状态模式
order: 19
---

**状态模式**（State）允许一个对象在其内部状态改变时改变它的行为；对象看起来似乎修改了它的类。

通俗地说，对象处于不同阶段时，允许的操作和执行后的变化各不一样——把这些差异放进各自的状态里，由当前状态决定是否允许、是否切换；调用方不必到处写「如果当前是某某状态则……」。

## 问题

订单有 **待支付、已支付、已发货、已取消** 等多个阶段——`Pay`、`Ship`、`Cancel`、`AdjustLines` 在不同阶段 **能不能调用、调用后变什么状态**，规则各不相同。

最直接的做法是用 **枚举 + 巨型 switch**：每个 Service 方法里判断 `order.Status`。状态少时还能应付；生命周期变复杂后，问题就会一起暴露：

1. **难扩展**：加「部分发货」状态，要改 Pay、Ship、Cancel、AdjustLines 所有 switch。
2. **转换规则分散**：「已支付取消 = 退款 + 释库存」只在 Cancel 里写一半，Refund API 又写一遍。
3. **状态可被绕过**：`order.Status = StatusPaid` 随处可赋，非法转换拦不住。
4. **测试组合爆炸**：每种 `(status, operation)` 组合都要单独测。

本质矛盾是：**订单在不同阶段允许的操作不同**，且 **操作成功会触发状态迁移**；这些规则 **不应** 以 `if status` 重复形式散落在各 Service。典型写法如下：

```go
func (s *OrderService) Pay(ctx context.Context, orderID string) error {
    order, _ := s.repo.Get(ctx, orderID)
    switch order.Status {
    case StatusPending:
        order.Status = StatusPaid
        return s.repo.Save(ctx, order)
    case StatusCancelled:
        return ErrOrderCancelled
    // Cancel、Ship、AdjustLines 里又要写一遍 switch…
    }
}
```

## 解决方案

定义 **State** 接口；**Order**（Context）持有当前 State 并 **委托** 操作；各 **ConcreteState** 决定 **允许 / 拒绝 / 迁移**。

### 状态与上下文

```go
type OrderState interface {
    Pay(ctx context.Context, o *Order) error
    Ship(ctx context.Context, o *Order, tracking string) error
    Cancel(ctx context.Context, o *Order) error
    Name() string
}

type Order struct {
    ID, UserID  string
    Lines       []OrderLine
    TrackingNo  string
    CrossBorder bool
    state       OrderState
    repo       OrderRepository
    payment    PaymentService
    inventory  InventoryService
}

func (o *Order) Total() int64 {
    var sum int64
    for _, line := range o.Lines {
        sum += line.Amount()
    }
    return sum
}

func (o *Order) Pay(ctx context.Context) error    { return o.state.Pay(ctx, o) }
func (o *Order) Ship(ctx context.Context, t string) error { return o.state.Ship(ctx, o, t) }
func (o *Order) Cancel(ctx context.Context) error { return o.state.Cancel(ctx, o) }
func (o *Order) Status() string                   { return o.state.Name() }

func (o *Order) setState(s OrderState) { o.state = s }
func (o *Order) persist(ctx context.Context) error { return o.repo.Save(ctx, o) }
```

DB 仍存 **status 字符串**；加载时用 `stateFromName(row.Status)` 还原 State 对象。

### 具体状态

```go
type PendingState struct{}

func (PendingState) Name() string { return "pending" }

func (PendingState) Pay(ctx context.Context, o *Order) error {
    if err := o.payment.Capture(ctx, o.ID, o.Total()); err != nil {
        return err
    }
    o.setState(PaidState{})
    return o.persist(ctx)
}

func (PendingState) Ship(context.Context, *Order, string) error { return ErrMustPayFirst }

func (PendingState) Cancel(ctx context.Context, o *Order) error {
    _ = o.inventory.Release(ctx, o.Lines)
    o.setState(CancelledState{})
    return o.persist(ctx)
}

type PaidState struct{}

func (PaidState) Name() string { return "paid" }

func (PaidState) Pay(context.Context, *Order) error { return ErrAlreadyPaid }

func (PaidState) Ship(ctx context.Context, o *Order, tracking string) error {
    o.TrackingNo = tracking
    o.setState(ShippedState{})
    return o.persist(ctx)
}

func (PaidState) Cancel(ctx context.Context, o *Order) error {
    _ = o.payment.Refund(ctx, o.ID)
    _ = o.inventory.Release(ctx, o.Lines)
    o.setState(CancelledState{})
    return o.persist(ctx)
}

type ShippedState struct{}   // 已发货：Cancel 等操作返回 ErrNotAllowed
type CancelledState struct{} // 终态：所有写操作拒绝
```

`ShippedState` / `CancelledState` 等 **终态或只读态** 在各自 struct 里统一 `return ErrNotAllowed`；改明细、退款等操作同理扩展接口。

### 客户端

```go
func (s *OrderService) Pay(ctx context.Context, orderID string) error {
    o, err := s.load(ctx, orderID) // RestoreOrder + stateFromName
    if err != nil {
        return err
    }
    return o.Pay(ctx)
}
```

HTTP、MQ、CLI **统一** `order.Pay()`，**无 switch**。迁移后 `Publish` 领域事件、表驱动 FSM、`AdjustLines` 等见 **实践** 一节。


## 适用场景

1. **对象行为随状态变**：订单、工单、审批单、连接（TCP）、播放器。
2. **大量条件分支依赖状态**：`switch status` **重复** 且 **难维护**。
3. **转换规则要集中**：非法转换 **编译/测试可覆盖**。
4. **开闭**：新状态 **新 ConcreteState**，少改 Context 方法签名。
5. **与观察者组合**：State **迁移后 Publish** 领域事件。

**不必强行使用**：

- **2～3 个状态、几乎无分支**——枚举够用。
- **纯算法切换、无生命周期**——用 **策略**。
- **状态由运营配置、上百条转换**——**表驱动 FSM** 更合适。
- **仅通知下游**——[观察者](/cs-fundamentals/design-patterns/observer) 足够，不需 State 类。

常见例子：TCP 连接状态、媒体播放器、工作流引擎、Document 草稿/发布、Vending machine。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **消除巨型 switch** | 行为 **按状态分文件** |
| **开闭** | 新状态 **加类** |
| **转换内聚** | Pay/Cancel **副作用** 在 **对应 State** |
| **显式非法操作** | `ShippedState.Cancel` **统一拒绝** |
| **易单测** | `PaidState{}.Ship(mockOrder)` **无 DB** |

| 缺点 | 说明 |
| :--- | :--- |
| **类数量增加** | 每状态一类；可用 **表驱动** 减类 |
| **Context 可能膨胀** | 共享依赖 **注入 Order**；或 **StateFactory** |
| **双份「状态」** | DB enum + 内存 State——**Restore 要同步** |
| **分散的迁移图** | 需 **文档/图** 汇总全局 FSM |
| **与 DDD 聚合** | 大 Aggregate 上 State 要 **控边界** |

## 实践

### Restore 与终态

```go
func stateFromName(name string) OrderState {
    switch name {
    case "pending":
        return PendingState{}
    case "paid":
        return PaidState{}
    case "shipped":
        return ShippedState{}
    case "cancelled":
        return CancelledState{}
    default:
        return PendingState{}
    }
}

type ShippedState struct{}

func (ShippedState) Name() string { return "shipped" }
func (ShippedState) Pay(context.Context, *Order) error { return ErrAlreadyPaid }
func (ShippedState) Cancel(context.Context, *Order) error { return ErrCannotCancelShipped }
func (ShippedState) Ship(ctx context.Context, o *Order, tracking string) error {
    o.TrackingNo = tracking
    return o.persist(ctx)
}
```

迁移成功后 `Publish(OrderPaid)`；`AdjustLines`、`Refund` 按状态扩展接口。

### 表驱动 FSM（状态很多时）

```go
type transition struct {
    from, event, to string
    guard           func(*Order) bool
}

var orderTransitions = []transition{
    {from: "pending", event: "pay", to: "paid"},
    {from: "paid", event: "ship", to: "shipped"},
    {from: "pending", event: "cancel", to: "cancelled"},
}

func (o *Order) Apply(ctx context.Context, event string) error {
    for _, t := range orderTransitions {
        if o.Status() == t.from && event == t.event {
            if t.guard != nil && !t.guard(o) {
                return ErrGuardFailed
            }
            o.setState(stateFromName(t.to))
            return o.persist(ctx)
        }
    }
    return ErrInvalidTransition
}
```

**运营可配置** 时用表；**行为差异大**（Paid Cancel 要退款链）仍用 **多态 State** 或 **表 + 钩子**。

## 关联

- [桥接模式](/cs-fundamentals/design-patterns/bridge)、状态模式、[策略模式](/cs-fundamentals/design-patterns/strategy)（以及在一定程度上 [适配器模式](/cs-fundamentals/design-patterns/adapter)）的接口结构很相似——都基于 [组合模式](/cs-fundamentals/design-patterns/composite) 式的委托，但各自要解决的问题不同。模式不仅是代码组织方式，也是与同伴讨论 **如何解题** 的共同语言。
- 状态模式是 [策略模式](/cs-fundamentals/design-patterns/strategy) 的扩展。两者都基于组合机制：它们都通过将部分工作委派给「帮手」对象来在运行时改变行为。还有一个相似之处在于——对客户端而言，它们都是透明的。客户端与多种状态的「策略」对象交互时，通常不会察觉到这一点。
  - 策略模式会让各个策略对象相互完全独立，彼此之间没有任何联系。
  - 状态模式不会限制具体状态之间的依赖，并允许它们自行改变在不同状态间进行切换。

## 参考阅读

- [x] [Refactoring.Guru - 状态模式](https://refactoringguru.cn/design-patterns/state) (2026-06-22)
- [x] [菜鸟教程 - 状态模式](https://www.runoob.com/design-pattern/state-pattern.html) (2026-06-22)
