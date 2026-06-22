---
title: 观察者模式
order: 18
---

**观察者模式**（Observer）在 **主题（Subject）** 与 **多个观察者（Observer）** 之间建立 **一对多依赖**：主题 **状态或领域事实发生变化** 时，**自动通知** 所有已注册观察者——主题 **只负责发布**，不必认识 **短信、搜索索引、积分** 各自怎么实现，观察者 **只订阅感兴趣的事件**，不必 **反向依赖** `OrderService` 的写方法。

与 [外观模式](/cs-fundamentals/design-patterns/facade) 的 **分工** 很常被问到：[外观](/cs-fundamentals/design-patterns/facade) **主动编排** 下单用例（预占 → 支付 → 落库）；观察者处理 **「事实已发生」之后的扇出**（`OrderPaid` → 发邮件、更新 ES、加积分）——Facade **同步、有顺序**；Observer **常异步、订阅者彼此独立**。与 [中介者模式](/cs-fundamentals/design-patterns/mediator) 也不同：中介 **协调对等同事的双向联动**（改地址 → 刷运费）；观察者 **主题单向广播**，观察者 **不应再指挥主题改状态**。与 [命令模式](/cs-fundamentals/design-patterns/command) 可配合：`PlaceOrder` 或 `AdjustQuantityCommand.Execute` **成功后** **再 `Publish(OrderPaid)`**——命令管 **写与 Undo**；观察者管 **只读/异步副作用**。

下文继续用「电商订单系统」：[外观](/cs-fundamentals/design-patterns/facade) 已完成 `PlaceOrder`；[命令](/cs-fundamentals/design-patterns/command) 已支持运营改单。当 **订单支付、发货、取消** 后要 **通知用户、更新搜索、写审计、加会员积分、推 WMS**，且 **每加一种下游就要改 `OrderService`** 时，若写操作里 **硬编码** 六个 `if err := xxx.Notify()`，会出现 **开闭困难、测试要 mock 全家、同步路径变慢**——观察者把 **「订单状态变了」** 与 **「谁要响应」** 拆开，组装层 **注册 Observer**，主题 **只 Publish 领域事件**。

## 问题

`OrderService` 在每次状态变更后 **直接调用所有下游**：

```go
func (s *OrderService) MarkPaid(ctx context.Context, orderID string) error {
    order, err := s.repo.Get(ctx, orderID)
    if err != nil {
        return err
    }
    order.Status = StatusPaid
    if err := s.repo.Save(ctx, order); err != nil {
        return err
    }
    // 写库成功后——扇出散落在此
    _ = s.email.SendPaidConfirmation(order)
    _ = s.sms.SendPaid(order.UserID)
    _ = s.search.IndexOrder(order)
    _ = s.loyalty.AccruePoints(order)
    _ = s.analytics.Track("order_paid", order.ID)
    _ = s.audit.Log("order.paid", orderID)
    // 产品又要「大促直播间订单推弹幕」——再改 OrderService
    return nil
}

func (s *OrderService) MarkShipped(ctx context.Context, orderID, tracking string) error {
    // … Save …
    _ = s.email.SendShipped(order)
    _ = s.sms.SendShipped(order.UserID)
    _ = s.search.IndexOrder(order)
    // 重复：每种状态一套 Notify 列表
    return nil
}
```

1. **违反开闭**：每加 **直播推送、发票、风控回写**，都要 **改 OrderService** 三个方法。
2. **职责混杂**：领域服务既 **改状态**，又 **管通知、索引、埋点**——违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。
3. **同步路径膨胀**：`MarkPaid` **等齐** 邮件、ES、积分 **才返回**——慢依赖 **拖垮写接口**；某观察者 panic **可能拖死主流程**。
4. **测试组合爆炸**：单测 `MarkPaid` 必须 mock **六个副作用服务**。
5. **与 Facade / 命令的分工错位**：[外观](/cs-fundamentals/design-patterns/facade) 管 **用例编排**；[命令](/cs-fundamentals/design-patterns/command) 管 **可撤销写**——这里要解决 **状态变更后的多订阅者通知**，不是 **再写一遍 PlaceOrder**。

本质矛盾是：**一个领域事实**（订单已支付）会 **触发 N 个独立下游**，且 N **持续增加**；写状态的核心逻辑 **不应** 认识每一个下游类型。

### 观察者 vs 直接调用 vs 全局 EventBus

| 方式 | 耦合 | 典型形态 | 何时够用 |
| :--- | :--- | :--- | :--- |
| **OrderService 内硬调** | 主题 **依赖** 所有下游 | 同步、难扩展 | 永远只有 1 个副作用 |
| **观察者（经典）** | 主题 **只知 Observer 接口** | Subject 维护订阅列表 | 单进程、需类型化事件 |
| **领域 EventBus** | 发布者 **只知 Bus** | `bus.Publish(evt)` | 跨模块、观察者很多 |
| **消息队列** | 发 **集成事件** 到 MQ | 异步、跨服务 | 微服务扇出 |

EventBus 常是观察者 **工程化变体**；GoF 观察者强调 **Subject–Observer 抽象** 与 **可替换的订阅表**。

## 意图

用一句话说：**定义对象间的一种一对多的依赖关系，使得每当一个对象的状态发生改变时，所有依赖于它的对象都得到通知并自动更新。**

在电商里「状态改变」常落实为 **领域事件**（`OrderPaid`、`OrderShipped`）：主题 **Publish** 不可变事件；观察者 **Handle(event)**：

```go
if err := orders.MarkPaid(ctx, orderID); err != nil {
    return err
}
// MarkPaid 内部 Save 成功后：
publisher.Publish(ctx, OrderPaid{OrderID: orderID, UserID: order.UserID, Amount: order.Total})
```

GoF 从 **结构** 角度的定义：

> 定义对象间的一种一对多的依赖关系，使得当一个对象的状态发生改变时，所有依赖于它的对象都得到通知并自动更新。

### 和外观、中介者、命令有啥不同

| | 观察者 | 外观 | 中介者 | 命令 |
| :--- | :--- | :--- | :--- | :--- |
| **动机** | **状态变更 → 通知依赖者** | **简化用例编排** | **封装同事间交互** | **封装操作为对象** |
| **方向** | 主题 → 观察者 **单向** | 客户端 → 子系统 | 同事 ↔ 中介 **双向协调** | Invoker → Command → Receiver |
| **时机** | **事后** 扇出 | **事中** 编排 | **交互过程中** 联动 | **执行/撤销** 操作 |
| **能否 Undo** | **否**（已发生事实） | 编排内可补偿 | 不强调 Undo | **可 Undo** |
| **电商例子** | 支付后发邮件/索引 | PlaceOrder | 结算页改地址刷运费 | AdjustQuantity Undo |

#### 观察者和领域事件是一回事吗？

**工程上常是。** `OrderPaid` 事件 + `Subscribe(handler)` **就是** 观察者；DDD 强调 **「已发生事实、不可变、可持久化」**。经典 Observer 偏 **内存订阅表**；EventBus/MQ 偏 **跨边界**——**模式思想相同**。

#### 观察者能替代 Facade 编排吗？

**不能。** 支付失败要 **Release 库存** 必须在 [外观](/cs-fundamentals/design-patterns/facade) **同步编排** 内完成；**支付成功后的邮件** 适合 Observer **异步**。**关键路径编排** vs **副作用扇出** 分层。

## 解决方案

定义 **观察者** 接口；**主题** 维护订阅列表并在变更后 **Notify**；各 **具体观察者** 实现单一副作用；写状态逻辑 **只 Publish**。

### 领域事件与观察者接口

```go
type DomainEvent interface {
    EventName() string
    OccurredAt() time.Time
}

type OrderPaid struct {
    OrderID   string
    UserID    string
    Amount    int64
    occurred  time.Time
}

func (e OrderPaid) EventName() string    { return "order.paid" }
func (e OrderPaid) OccurredAt() time.Time { return e.occurred }

type OrderShipped struct {
    OrderID    string
    TrackingNo string
    occurred   time.Time
}

func (e OrderShipped) EventName() string    { return "order.shipped" }
func (e OrderShipped) OccurredAt() time.Time { return e.occurred }

type OrderCancelled struct {
    OrderID  string
    occurred time.Time
}

func (e OrderCancelled) EventName() string    { return "order.cancelled" }
func (e OrderCancelled) OccurredAt() time.Time { return e.occurred }

type Observer interface {
    // 返回支持的 event 名；空切片表示订阅全部（慎用）
    InterestedIn() []string
    Handle(ctx context.Context, ev DomainEvent) error
}
```

可按 **事件名路由**，避免每个 Observer 里 **长 switch**——或用 **泛型** `Observer[OrderPaid]`（见 [组装实践](#组装实践)）。

### 主题（Subject）——EventPublisher

```go
type EventPublisher struct {
    observers []Observer
}

func (p *EventPublisher) Subscribe(o Observer) {
    p.observers = append(p.observers, o)
}

func (p *EventPublisher) Unsubscribe(o Observer) {
    for i, x := range p.observers {
        if x == o {
            p.observers = append(p.observers[:i], p.observers[i+1:]...)
            return
        }
    }
}

func (p *EventPublisher) Publish(ctx context.Context, ev DomainEvent) error {
    for _, o := range p.observers {
        if !interested(o, ev.EventName()) {
            continue
        }
        if err := o.Handle(ctx, ev); err != nil {
            return err // 同步模式：任一失败可中断；异步见下文
        }
    }
    return nil
}

func interested(o Observer, name string) bool {
    for _, n := range o.InterestedIn() {
        if n == name {
            return true
        }
    }
    return false
}
```

`OrderService` **注入** `EventPublisher`，**不注入** `EmailService` + `SearchIndexer` + …

### 具体观察者

```go
type EmailNotifier struct {
    mail EmailService
}

func (n *EmailNotifier) InterestedIn() []string { return []string{"order.paid", "order.shipped"} }

func (n *EmailNotifier) Handle(ctx context.Context, ev DomainEvent) error {
    switch e := ev.(type) {
    case OrderPaid:
        return n.mail.SendPaidConfirmation(ctx, e.OrderID)
    case OrderShipped:
        return n.mail.SendShipped(ctx, e.OrderID, e.TrackingNo)
    default:
        return nil
    }
}

type SearchIndexer struct {
    search SearchService
}

func (n *SearchIndexer) InterestedIn() []string { return []string{"order.paid", "order.shipped", "order.cancelled"} }

func (n *SearchIndexer) Handle(ctx context.Context, ev DomainEvent) error {
    var id string
    switch e := ev.(type) {
    case OrderPaid:
        id = e.OrderID
    case OrderShipped:
        id = e.OrderID
    case OrderCancelled:
        id = e.OrderID
    default:
        return nil
    }
    return n.search.IndexOrder(ctx, id)
}

type LoyaltyAccruer struct {
    loyalty LoyaltyService
}

func (n *LoyaltyAccruer) InterestedIn() []string { return []string{"order.paid"} }

func (n *LoyaltyAccruer) Handle(ctx context.Context, ev DomainEvent) error {
    e := ev.(OrderPaid)
    return n.loyalty.AccruePoints(ctx, e.UserID, e.Amount)
}
```

新增 **直播弹幕推送** = **新 Observer + Subscribe**，**不改** `OrderService.MarkPaid`。

### 发起人（Originator）——OrderService 只 Publish

```go
type OrderService struct {
    repo      OrderRepository
    publisher *EventPublisher
}

func (s *OrderService) MarkPaid(ctx context.Context, orderID string) error {
    order, err := s.repo.Get(ctx, orderID)
    if err != nil {
        return err
    }
    if order.Status != StatusPending {
        return ErrInvalidTransition
    }
    order.Status = StatusPaid
    if err := s.repo.Save(ctx, order); err != nil {
        return err
    }
    return s.publisher.Publish(ctx, OrderPaid{
        OrderID: order.ID, UserID: order.UserID, Amount: order.Total,
        occurred: time.Now(),
    })
}
```

**事务边界**：Save **成功后再 Publish**；若 Publish 失败，需 **Outbox / 重试**（见 [组装实践](#组装实践)）——避免 **库已改、下游未通知**。

### 组装（Client）

```go
func NewOrderModule(/* deps */) *OrderService {
    pub := &EventPublisher{}
    pub.Subscribe(&EmailNotifier{mail: emailSvc})
    pub.Subscribe(&SearchIndexer{search: searchSvc})
    pub.Subscribe(&LoyaltyAccruer{loyalty: loyaltySvc})
    pub.Subscribe(&AuditLogger{audit: auditSvc})
    return &OrderService{repo: repo, publisher: pub}
}
```

[外观](/cs-fundamentals/design-patterns/facade) `PlaceOrderCore` 在落库后 **同样 Publish** `OrderPlaced` / `OrderPaid`——**一处注册 Observer**，HTTP 与 MQ 入口 **共享**。


## 适用场景

1. **一变更、多反应**：订单状态、库存水位、购物车变更。
2. **开闭扇出**：新下游 **新 Observer**，少改主题。
3. **解耦 UI**：MVC 中 Model 通知 View（经典 GoF 例子）。
4. **领域事件架构**：单体内 Publisher；跨服务 **MQ 仍是 Observer 思想**。
5. **只读副作用**：通知、索引、埋点、缓存失效——**非** 写路径关键编排。

**不必强行使用**：

- **仅一个固定下游**——直接调一个 Service。
- **必须同步编排与补偿**——用 [外观](/cs-fundamentals/design-patterns/facade) / Saga，不是 Observer。
- **对等 UI 联动有严格顺序**——用 [中介者](/cs-fundamentals/design-patterns/mediator)。
- **要撤销操作**——用 [命令](/cs-fundamentals/design-patterns/command)，不是「通知已发生」。

常见例子：GUI 按钮-列表联动、`addEventListener`、Reactive Streams、Kafka 消费者、Spring `@EventListener`、GitHub Webhooks 订阅。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新 Observer **Subscribe** 即可 |
| **解耦** | Subject **不依赖** 具体 Notifier |
| **单一职责** | 写状态 vs 发邮件 **分离** |
| **可组合** | 同一事件 **多观察者** 独立失败策略 |
| **易测** | `MarkPaid` 测 **Publish 是否调用**；Observer **单测** |

| 缺点 | 说明 |
| :--- | :--- |
| **通知顺序不确定** | 除非 Publisher **显式排序** |
| **循环依赖风险** | Observer 内 **再改 Subject** 要禁止或事件化 |
| **调试链路过长** | 「谁处理了 OrderPaid」需 **日志/trace** |
| **一致性与投递** | Save 与 Notify **非原子**——要 Outbox |
| **订阅泄漏** | 忘记 Unsubscribe **内存泄漏**（长生命周期 UI） |

## 组装实践

> **阅读提示**：先掌握「**Save 成功后 Publish，Observer Handle**」即可。本节是工程变体；初学可先跳过。

### 同步 vs 异步通知

```go
func (p *EventPublisher) PublishAsync(ctx context.Context, ev DomainEvent) {
    for _, o := range p.observers {
        if !interested(o, ev.EventName()) {
            continue
        }
        o := o
        go func() {
            _ = o.Handle(context.WithoutCancel(ctx), ev)
        }()
    }
}
```

**写接口** 用 **Async** 避免邮件拖慢 `MarkPaid`；失败 **打日志 + 死信**，不 **return err 给调用方**（除非同步强一致）。

### Transactional Outbox

```text
MarkPaid:
  BEGIN TX
    UPDATE orders SET status=paid
    INSERT outbox(event=OrderPaid, payload=…)
  COMMIT
OutboxWorker:
  READ outbox → Publish → DELETE
```

保证 **至少一次** 投递；Observer **幂等**（按 `OrderID+EventName` 去重）。

### 与命令、外观一起用

```text
PlaceOrder（Facade）
  → Reserve / Pay / Save
  → publisher.Publish(OrderPlaced)

AdjustQuantityCommand.Execute
  → SetLineQuantity
  → publisher.Publish(OrderLineChanged)

OrderPaid 观察者
  → Email / Search / Loyalty（只读扇出）
```

[命令](/cs-fundamentals/design-patterns/command) **Execute 后发事件**；[外观](/cs-fundamentals/design-patterns/facade) **用例成功后再 Publish**——**写** 与 **通知** 边界清晰。

### 类型安全订阅（Go 泛型）

```go
type Handler[T DomainEvent] interface {
    Handle(ctx context.Context, ev T) error
}

type typedPublisher[T DomainEvent] struct {
    handlers []Handler[T]
}

func (p *typedPublisher[T]) Publish(ctx context.Context, ev T) error {
    for _, h := range p.handlers {
        if err := h.Handle(ctx, ev); err != nil {
            return err
        }
    }
    return nil
}

// OrderPaid 专用 bus，编译期类型安全
var paidBus typedPublisher[OrderPaid]
```

多事件类型可用 **注册表** `map[string][]Observer` 或 **独立 Publisher per event**。

### 推模型 vs 拉模型

| 模型 | 行为 | 适用 |
| :--- | :--- | :--- |
| **推** | Notify 时 **把 event 传给** Observer | 领域事件、payload 小 |
| **拉** | Notify 只给 **提示**，Observer 调 `subject.GetState()` | 状态大、观察者 **选择性读** |

电商 **OrderPaid 带 OrderID** 通常 **推**；Observer **自己查读模型** 拿详情。

### 与中介者的边界

| 场景 | 选用 |
| :--- | :--- |
| 结算页 **改地址 → 刷运费**（页内、有顺序） | [中介者](/cs-fundamentals/design-patterns/mediator) |
| **订单已支付 → 六个下游**（跨模块、事后） | **观察者** |
| 跨模块 **弱类型** 广播 | EventBus（观察者变体） |

### 测试策略

```go
func TestMarkPaid_PublishesOrderPaid(t *testing.T) {
    pub := &EventPublisher{}
    rec := &recordingObserver{names: []string{"order.paid"}}
    pub.Subscribe(rec)
    svc := &OrderService{repo: fakeRepo{}, publisher: pub}
    _ = svc.MarkPaid(context.Background(), "o1")
    if len(rec.events) != 1 {
        t.Fatal("expected publish")
    }
}

func TestEmailNotifier_OnlyPaidAndShipped(t *testing.T) {
    mail := &fakeMail{}
    n := &EmailNotifier{mail: mail}
    _ = n.Handle(context.Background(), OrderCancelled{OrderID: "x"})
    if mail.sent != 0 {
        t.Fatal("should ignore cancelled")
    }
}

type recordingObserver struct {
    names   []string
    events  []DomainEvent
}

func (r *recordingObserver) InterestedIn() []string { return r.names }
func (r *recordingObserver) Handle(_ context.Context, ev DomainEvent) error {
    r.events = append(r.events, ev)
    return nil
}
```

## 小结

记住这四点即可：

1. **一对多通知**：主题 **Publish** 领域事件；观察者 **Subscribe + Handle**。
2. **写与扇出分离**：`OrderService` **Save 后 Publish**；邮件/索引 **在 Observer**。
3. **与 Facade / 命令分层**：Facade **同步编排**；Command **可 Undo 的写**；Observer **事后、常异步** 副作用。
4. **投递要可靠**：异步 + **Outbox**；Observer **幂等**。

[外观模式](/cs-fundamentals/design-patterns/facade) 解决了 **「客户端如何一次走完下单」**；观察者解决了 **「一个事实发生后，如何让多个依赖方自动跟上，而主题不必认识它们」**——把 **扇出** 从写路径 **剥到可扩展的订阅表**，符合 [开闭](/cs-fundamentals/design-patterns#设计原则) 与 **单一职责**。

## 参考阅读

- [x] [外观模式](/cs-fundamentals/design-patterns/facade) — 同步用例编排；与事后 Observer 分层
- [x] [命令模式](/cs-fundamentals/design-patterns/command) — 写操作；Execute 后 Publish 事件
- [x] [中介者模式](/cs-fundamentals/design-patterns/mediator) — 页内对等联动；非事后扇出
- [x] [备忘录模式](/cs-fundamentals/design-patterns/memento) — 状态快照；与不可变领域事件对比
- [x] [Refactoring.Guru - 观察者模式](https://refactoringguru.cn/design-patterns/observer) (2026-06-22)
- [x] [菜鸟教程 - 观察者模式](https://www.runoob.com/design-pattern/observer-pattern.html) (2026-06-22)
