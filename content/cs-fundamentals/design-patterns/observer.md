---
title: 观察者模式
order: 18
---

**观察者模式**（Observer）亦称 **事件订阅者**、**监听者**，定义对象间的一种一对多的依赖关系，使得每当一个对象的状态发生改变时，所有依赖于它的对象都得到通知并自动更新。

通俗地说，状态一变，所有关心这件事的下游自动收到通知；发布方只负责宣布「发生了什么」，具体发邮件、写索引、记日志等副作用各自订阅处理，新增下游不必改发布方的核心逻辑。

## 问题

订单支付、发货、取消之后，要 **通知用户、更新搜索、写审计、加积分、推 WMS**…… 每加一种下游，就要在 `OrderService` 里 **多调一个方法**。

下游少时还能应付；副作用一多，问题就会一起暴露：

1. **难扩展**：加直播推送、发票、风控回写，都要改 `OrderService` 的多个方法。
2. **职责混杂**：领域服务既 **改状态**，又 **管通知、索引、埋点**，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。
3. **同步路径变慢**：`MarkPaid` 等齐邮件、ES、积分才返回——慢依赖拖垮写接口。
4. **测试组合爆炸**：单测 `MarkPaid` 必须 mock 六个副作用服务。

本质矛盾是：**一个领域事实**（订单已支付）会 **触发 N 个独立下游**，且 N 持续增加；写状态的核心逻辑 **不应** 认识每一个下游类型。典型写法如下：

```go
func (s *OrderService) MarkPaid(ctx context.Context, orderID string) error {
    // 写库成功后——扇出散落在此
    _ = s.email.SendPaidConfirmation(order)
    _ = s.sms.SendPaid(order.UserID)
    _ = s.search.IndexOrder(order)
    _ = s.loyalty.AccruePoints(order)
    // 产品又要「大促直播间推弹幕」——再改 OrderService
    return nil
}
```

## 解决方案

定义 **观察者** 接口；**主题** 维护订阅列表并在变更后 **Publish**；各 **具体观察者** 处理单一副作用；写状态逻辑 **只发事件**。

### 事件与主题

```go
type DomainEvent interface {
    EventName() string
}

type OrderPaid struct {
    OrderID, UserID string
    Amount          int64
}

func (e OrderPaid) EventName() string { return "order.paid" }

type Observer interface {
    InterestedIn() []string
    Handle(ctx context.Context, ev DomainEvent) error
}

type EventPublisher struct {
    observers []Observer
}

func (p *EventPublisher) Subscribe(o Observer) {
    p.observers = append(p.observers, o)
}

func (p *EventPublisher) Publish(ctx context.Context, ev DomainEvent) error {
    for _, o := range p.observers {
        for _, name := range o.InterestedIn() {
            if name == ev.EventName() {
                if err := o.Handle(ctx, ev); err != nil {
                    return err
                }
                break
            }
        }
    }
    return nil
}
```

### 观察者与 OrderService

```go
type EmailNotifier struct{ mail EmailService }

func (n *EmailNotifier) InterestedIn() []string { return []string{"order.paid"} }

func (n *EmailNotifier) Handle(ctx context.Context, ev DomainEvent) error {
    e := ev.(OrderPaid)
    return n.mail.SendPaidConfirmation(ctx, e.OrderID)
}

type OrderService struct {
    repo      OrderRepository
    publisher *EventPublisher
}

func (s *OrderService) MarkPaid(ctx context.Context, orderID string) error {
    order, err := s.repo.Get(ctx, orderID)
    if err != nil {
        return err
    }
    order.Status = StatusPaid
    if err := s.repo.Save(ctx, order); err != nil {
        return err
    }
    return s.publisher.Publish(ctx, OrderPaid{
        OrderID: order.ID, UserID: order.UserID, Amount: order.Total,
    })
}
```

搜索索引、积分、审计等 **各一个 Observer**；新增下游 = `Subscribe`，**不改** `MarkPaid`。组装示例：

```go
pub := &EventPublisher{}
pub.Subscribe(&EmailNotifier{mail: emailSvc})
pub.Subscribe(&SearchIndexer{search: searchSvc})
svc := &OrderService{repo: repo, publisher: pub}
```

`Save` **成功后再 Publish**；异步投递、Outbox、更多事件类型见 **实践** 一节。


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

## 实践

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

## 关联

- [责任链模式](/cs-fundamentals/design-patterns/chain-of-responsibility)、[命令模式](/cs-fundamentals/design-patterns/command)、[中介者模式](/cs-fundamentals/design-patterns/mediator) 和观察者模式均用于在不同对象之间传递请求，但各自采用不同的方法。责任链模式按顺序传递请求，直到有一个接收者处理它；命令模式在发送者和请求者之间建立单向连接；中介者模式让发送者和请求者完全消除相互引用，只能通过中介对象间接通信；观察者模式允许接收者动态订阅或取消订阅接收请求。
- [中介者模式](/cs-fundamentals/design-patterns/mediator) 和观察者模式之间的区别往往很难记住。在大部分情况下，你可以同时使用这两种模式；有时你甚至可以使用其中任意一种。它们之间的主要区别是意图上有所不同。
  - 中介者的主要目标是消除一组系统组件之间的相互依赖。这些组件转而依赖于单独的中介者对象。该对象会将所有的交互协调起来，从而保持各组件间松耦合。
  - 观察者的目标是在对象之间建立动态的单向连接，使部分对象可作为其他对象的下属。
  - 实现中介者模式的一种流行方法是通过依赖观察者模式。中介者对象担当发布者的角色，其余组件则作为订阅者，可以订阅和取消订阅发布者对象上的事件。中介者看上去会非常像观察者，当你只能在一个程序中使用一种模式的话，通常可以使用观察者来替代中介者。但是，你可以在一个程序中同时使用这两种模式：使用观察者来将组件和中介者连接起来。
  - 还有另外一种实现中介者模式的方法。其中对象所持有的是对中介者对象的永久引用。这样实现方式和观察者并不相同，但这仍然是中介者模式。
  - 如果将所有组件都变为发布者，且它们之间建立动态连接，使得系统中没有中心化的中介者对象，则最终整个系统会变成一组「分布式观察者」。

## 参考阅读

- [x] [Refactoring.Guru - 观察者模式](https://refactoringguru.cn/design-patterns/observer) (2026-06-22)
- [x] [菜鸟教程 - 观察者模式](https://www.runoob.com/design-pattern/observer-pattern.html) (2026-06-22)
