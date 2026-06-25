---
title: 中介者模式
order: 16
---

**中介者模式**（Mediator）亦称 **调解人**、**控制器**，用一个中介对象来封装一系列的对象交互；中介者使各对象不需要显式地相互引用，从而使其耦合松散，而且可以独立地改变它们之间的交互。

通俗地说，多个组件之间的联动规则集中到一个中介里；各组件只向中介通报变化，由中介决定还要通知谁、做什么，彼此不必互相引用、互相了解。

## 问题

结算页上有明细、地址、优惠券、配送、支付、合计等多个 **对等面板**——用户改数量、改地址、用优惠券时， **多个区块都要跟着刷新**。

最直接的做法是让每个面板 **直接持有** 其他面板的引用并互调。面板少时还能应付；区块一多，问题就会一起暴露：

1. **网状耦合**：6 个面板两两引用，加「发票抬头」区块要改 5 个 struct 的字段。
2. **联动规则散落**：「用券后免运」写在优惠券组件，「改地址禁支付」写在地址组件——同一规则重复或互相遗漏。
3. **难复用与测试**：单测地址表单必须 mock 合计、运费、支付三个面板。
4. **违反迪米特法则**：面板只管自己的 UI，却 **跨层知道** 运费怎么算、分期怎么刷。

本质矛盾是：**多个对等组件** 需要 **随用户操作彼此同步**，但 **谁该响应谁** 的规则会频繁变化，不应让每个组件 **自己维护一张依赖图**。典型写法如下：

```go
func (p *LineItemsPanel) OnQtyChanged(sku string, qty int) {
    p.summary.RecalcTotal()
    p.shipping.RecalcFee() // 重量变了
    p.coupon.Revalidate()  // 满减门槛可能变
}
```

## 解决方案

核心思路：**把「谁改了什么 → 还要刷新谁」从各个面板里抽出来，集中到一个中介里**。

### 和原来有什么不同

用户改地址时，两种写法对比：

**原来：面板互调（网状）**

```text
AddressForm ──直接调用──▶ ShippingPanel.Recalc()
              ──直接调用──▶ SummaryPanel.Recalc()
              ──直接调用──▶ PaymentPanel.FilterByRegion()
```

每个面板 struct 里都要 **持有** 其他面板的引用；改地址要知道运费、合计、支付 **三个下游**，加「发票抬头」还要再改地址表单。

**现在：只找中介（星型）**

```text
AddressForm ──Notify("address_changed")──▶ CheckoutMediator
                                              │
                         ┌────────────────────┼────────────────────┐
                         ▼                    ▼                    ▼
                  ShippingPanel        SummaryPanel         PaymentPanel
                  Refresh(state)         Refresh(state)       Refresh(state)
```

地址表单 **只说一句「地址变了」**；中介更新共享 `CheckoutContext`，再决定 **refresh 哪几个面板、按什么顺序**。

| | 原来（互调） | 中介者 |
| :--- | :--- | :--- |
| **依赖形状** | N 个面板 → 网状互引 | N 个面板 → 各连 1 条到 Mediator |
| **联动规则在哪** | 散在 `OnQtyChanged`、`OnUserSelect` 各处 | 集中在 `Mediator.Notify` |
| **加新面板** | 可能要改 **多个** 旧面板 | `Register` + 改 **一处** `case` |
| **单测地址表单** | mock 合计、运费、支付 | mock **一个** Mediator，断言 Notify 了什么 |
| **面板复用** | 绑死在当前页的依赖图上 | 换页只 **换注册列表** |

优势一句话：**同事只上报事件，中介统一编排响应**——依赖从「人人认识人人」变成「人人只认识中介」。

### 中介者与同事

定义 **中介者** 接口；**具体中介** 维护共享状态与联动规则；各 **同事** 只依赖 `Mediator`，变更时 `Notify`，由中介决定 `Refresh` 谁。

```go
type CheckoutContext struct {
    Lines    []OrderLine
    Address  Address
    Coupon   string
    Shipping ShippingQuote
    Total    int64
}

type Event struct {
    Type    string
    Payload any
}

type Colleague interface {
    Refresh(ctx context.Context, state *CheckoutContext) error
}

type Mediator interface {
    Register(c Colleague)
    Notify(from string, ev Event) error
    State() *CheckoutContext
}
```

同事 **不互持引用**；只向中介 `Notify`，由中介更新 `CheckoutContext` 并回调需要刷新的面板。

### 具体中介

```go
type CheckoutMediator struct {
    state      CheckoutContext
    colleagues map[string]Colleague
    pricing    PricingEngine
    shipping   ShippingService
}

func (m *CheckoutMediator) Notify(from string, ev Event) error {
    switch ev.Type {
    case "address_changed":
        m.state.Address = ev.Payload.(Address)
        quote, _ := m.shipping.Quote(context.Background(), m.state.Address, m.state.Lines)
        m.state.Shipping = quote
        m.state.Total = m.pricing.Total(m.state.Lines, m.state.Coupon)
        return m.refresh("shipping", "summary", "payment")
    case "lines_changed":
        m.state.Lines = ev.Payload.([]OrderLine)
        m.state.Total = m.pricing.Total(m.state.Lines, m.state.Coupon)
        return m.refresh("summary", "coupon", "payment")
    }
    return nil
}

func (m *CheckoutMediator) refresh(ids ...string) error {
    for _, id := range ids {
        _ = m.colleagues[id].Refresh(context.Background(), &m.state)
    }
    return nil
}
```

「用券免运」「改地址禁 COD」等规则 **只写在这里**；新增面板 = `Register` + 在对应 `case` 里补 `refresh`。

### 同事与组装

```go
type AddressForm struct {
    mediator Mediator
}

func (f *AddressForm) OnUserSelect(addr Address) error {
    return f.mediator.Notify("address", Event{Type: "address_changed", Payload: addr})
}

func (f *AddressForm) Refresh(ctx context.Context, state *CheckoutContext) error {
    return nil // 更新自身展示
}

m := &CheckoutMediator{pricing: pricing, shipping: shipping}
addr := &AddressForm{mediator: m}
m.colleagues = map[string]Colleague{
    "address": addr,
    "summary": &SummaryPanel{},
    "payment": &PaymentPanel{},
}
```

**页内协同** 走 Mediator；**提交落库** 仍走 [外观](/cs-fundamentals/design-patterns/facade) 的 `PlaceOrder`——职责分开。多事件分支、`CheckoutRules` 拆分见 **实践** 一节。


## 适用场景

1. **多个对等对象交互复杂**：结算页、配置向导、IDE 工具窗口、聊天室（用户只经 ChatRoom 发消息）。
2. **网状依赖难维护**：加/减一个面板不想 **改 N 个 struct**。
3. **联动规则常变**：「跨境禁 COD」「满额免运」集中在 Mediator **改 case**。
4. **需独立测试同事**：mock `Mediator`，断言 **Notify 了什么事件**。
5. **复用同事到不同页面**：B 端少注册 `PaymentPanel`，中介 **refresh 列表不含 payment**。

**不必强行使用**：

- **两个控件、一条固定联动**——直接回调更简单。
- **外部一次调用多个子系统**——用 [外观](/cs-fundamentals/design-patterns/facade)，不是 Mediator。
- **单向广播、无协调顺序**——观察者 / EventBus 足够。
- **请求沿链择一处理**——用 [责任链](/cs-fundamentals/design-patterns/chain-of-responsibility)。

常见例子：对话框控件联动、航空管制、群聊、前端 Redux store + dispatch（集中状态与 reducer 类似 Mediator 思想）、WPF `Interaction.RequestNavigate`。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **降低同事间耦合** | N 条边 → **每人一条到 Mediator** |
| **集中交互逻辑** | 免运、禁支付等 **一处维护** |
| **开闭** | 新同事 **Register + 改 Notify 分支** |
| **符合迪米特法则** | 同事 **只认识 Mediator** |
| **易测** | 中介单测 **事件 → refresh 顺序** |

| 缺点 | 说明 |
| :--- | :--- |
| **中介可能膨胀** | `CheckoutMediator` 变 God Object——需 **按事件拆私有方法** 或 **分层中介** |
| **间接层** | 调试要 trace **Notify 路径** |
| **与 Observer 职责重叠** | 团队需约定：**页内 Mediator，跨模块 Bus** |
| **过度设计** | 两控件页面 **不必** 上 Mediator |
| **并发** | 共享 `CheckoutContext` 要 **锁或单线程 UI 模型** |

## 实践

### 中介者 vs 全局 EventBus

```go
// 跨服务：订单已支付 → 通知、积分、搜索
bus.Publish(OrderPaid{ID: orderID})

// 结算页内：必须先 recalcTotal 再 refresh Payment
mediator.Notify(ColleagueCoupon, Event{Type: "coupon_applied", Payload: code})
```

**页内** 用 Mediator 保证 **顺序与一致性**；**跨 bounded context** 用 Bus。**勿** 用全局 Bus 替代页内 Mediator 却 **无编排**，易出现 **Payment 读到旧 Total**。

### 分层中介

| 层级 | 职责 | 例子 |
| :--- | :--- | :--- |
| **页面中介** | UI 块联动 | `CheckoutMediator` |
| **用例外观** | 提交编排 | `CheckoutFacade.PlaceOrder` |
| **领域服务** | 纯规则 | `PricingEngine.Total` |

```text
User edits address
  → CheckoutMediator.Notify（页内）
User clicks Submit
  → CheckoutFacade.PlaceOrder（用例）
  → Inventory / Payment / Orders（子系统）
```

[装饰器](/cs-fundamentals/design-patterns/decorator) 在 **`PricingEngine.Total` 内部** 叠行级价；Mediator **不替代** 计价领域逻辑。

### 防止中介膨胀

```go
// 按领域拆私有策略，Mediator 只做 dispatch
type CheckoutRules struct {
    pricing  PricingEngine
    shipping ShippingService
}

func (r *CheckoutRules) OnAddressChanged(state *CheckoutContext, addr Address) error {
    state.Address = addr
    state.Region = addr.Country
    q, err := r.shipping.Quote(context.Background(), addr, state.Lines)
    if err != nil {
        return err
    }
    state.Shipping = q
    return r.recalcTotal(state)
}
```

`CheckoutMediator.Notify` **委托** `rules.OnXxx`；复杂规则 **可单测 Rules** 而不 mock 六个 Panel。

## 关联

- [责任链模式](/cs-fundamentals/design-patterns/chain-of-responsibility)、[命令模式](/cs-fundamentals/design-patterns/command)、[中介者模式](/cs-fundamentals/design-patterns/mediator) 和 [观察者模式](/cs-fundamentals/design-patterns/observer) 均用于在不同对象之间传递请求，但各自采用不同的方法。责任链模式按顺序传递请求，直到有一个接收者处理它；命令模式在发送者和请求者之间建立单向连接；中介者模式让发送者和请求者完全消除相互引用，只能通过中介对象间接通信；观察者模式允许接收者动态订阅或取消订阅接收请求。
- [外观模式](/cs-fundamentals/design-patterns/facade) 和 [中介者模式](/cs-fundamentals/design-patterns/mediator) 的职责类似：它们都尝试在大量紧密耦合的类中组织起合作。
    - 外观为子系统中的所有对象定义了一个简单接口，但是它不提供任何新功能。子系统本身不会意识到外观的存在。子系统中的对象可以直接进行交流。
    - 中介者将系统中组件的沟通行为中心化。各组件只知道中介者对象，无法直接相互交流。
    - **为谁简化**：外观简化 **外部调用方**（Handler、Worker）；中介者简化 **内部对等组件**（结算页各面板）。
    - **关系方向**：外观是 **单向**——Client → Facade → 子系统；中介者是 **星型**——同事 ↔ Mediator，同事之间 **不直连**。
    - **子系统是否互知**：外观 **不禁止** 库存调支付；中介者 **禁止** 地址面板直接调合计面板。
    - **典型动作**：外观 **编排用例**（`PlaceOrder`：预占→支付→落库）；中介者 **协调联动**（改地址→刷新运费→合计→支付）。
    - **电商落点**：`CheckoutFacade` 管 **提交**；`CheckoutMediator` 管 **页内刷新**——常 **同页并存**，各管一层。
- [中介者模式](/cs-fundamentals/design-patterns/mediator) 和 [观察者模式](/cs-fundamentals/design-patterns/observer) 之间的区别往往很难记住。在大部分情况下，你可以同时使用这两种模式；有时你甚至可以使用其中任意一种。它们之间的主要区别是意图上有所不同。
  - 中介者的主要目标是消除一组系统组件之间的相互依赖。这些组件转而依赖于单独的中介者对象。该对象会将所有的交互协调起来，从而保持各组件间松耦合。
  - 观察者的目标是在对象之间建立动态的单向连接，使部分对象可作为其他对象的下属。
  - 实现中介者模式的一种流行方法是通过依赖 [观察者模式](/cs-fundamentals/design-patterns/observer)。中介者对象担当发布者的角色，其余组件则作为订阅者，可以订阅和取消订阅发布者对象上的事件。中介者看上去会非常像观察者，当你只能在一个程序中使用一种模式的话，通常可以使用观察者来替代中介者。但是，你可以在一个程序中同时使用这两种模式：使用观察者来将组件和中介者连接起来。
  - 还有另外一种实现中介者模式的方法。其中对象所持有的是对中介者对象的永久引用。这样实现方式和观察者并不相同，但这仍然是中介者模式。
  - 如果将所有组件都变为发布者，且它们之间建立动态连接，使得系统中没有中心化的中介者对象，则最终整个系统会变成一组「分布式观察者」。

## 参考阅读

- [x] [Refactoring.Guru - 中介者模式](https://refactoringguru.cn/design-patterns/mediator) (2026-06-22)
- [x] [菜鸟教程 - 中介者模式](https://www.runoob.com/design-pattern/mediator-pattern.html) (2026-06-22)
