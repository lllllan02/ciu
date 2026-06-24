---
title: 中介者模式
order: 16
---

**中介者模式** 亦称 **调解人**、**控制器**、**Intermediary**、**Controller**、**Mediator**，用一个中介对象来封装一系列的对象交互；中介者使各对象不需要显式地相互引用，从而使其耦合松散，而且可以独立地改变它们之间的交互。

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

定义 **中介者** 接口；**具体中介** 持有 **共享上下文** 与同事引用；各 **同事** 只依赖 `Mediator`。

### 共享上下文（CheckoutContext）

```go
type CheckoutContext struct {
    Lines    []OrderLine
    Address  Address
    Coupon   string
    Shipping ShippingQuote
    Total    int64
    Region   string
}

type EventType string

const (
    LinesChanged    EventType = "lines_changed"
    AddressChanged  EventType = "address_changed"
    CouponApplied   EventType = "coupon_applied"
    ShippingChanged EventType = "shipping_changed"
)

type Event struct {
    Type    EventType
    Payload any
}
```

上下文 **由中介维护**；同事 **读快照** 或通过中介 **查询**，避免 **直接改** 他人状态。

### 中介者（Mediator）接口与同事标识

```go
type ColleagueID string

const (
    ColleagueLines    ColleagueID = "lines"
    ColleagueAddress  ColleagueID = "address"
    ColleagueCoupon   ColleagueID = "coupon"
    ColleagueShipping ColleagueID = "shipping"
    ColleaguePayment  ColleagueID = "payment"
    ColleagueSummary  ColleagueID = "summary"
)

type Colleague interface {
    ID() ColleagueID
    Refresh(ctx context.Context, state *CheckoutContext) error
}

type Mediator interface {
    Register(c Colleague)
    Notify(from ColleagueID, ev Event) error
    State() *CheckoutContext
}
```

`Notify` **由触发变更的同事调用**；`Refresh` **由中介回调** 需要更新的同事。

### 具体中介：CheckoutMediator

```go
type CheckoutMediator struct {
    state      CheckoutContext
    colleagues map[ColleagueID]Colleague
    pricing    PricingEngine
    shipping   ShippingService
}

func (m *CheckoutMediator) Register(c Colleague) {
    if m.colleagues == nil {
        m.colleagues = make(map[ColleagueID]Colleague)
    }
    m.colleagues[c.ID()] = c
}

func (m *CheckoutMediator) State() *CheckoutContext {
    return &m.state
}

func (m *CheckoutMediator) Notify(from ColleagueID, ev Event) error {
    ctx := context.Background()
    switch ev.Type {
    case LinesChanged:
        lines := ev.Payload.([]OrderLine)
        m.state.Lines = lines
        return m.recalcAll(ctx)

    case AddressChanged:
        addr := ev.Payload.(Address)
        m.state.Address = addr
        m.state.Region = addr.Country
        quote, err := m.shipping.Quote(ctx, addr, m.state.Lines)
        if err != nil {
            return err
        }
        m.state.Shipping = quote
        if err := m.refresh(ColleagueShipping, ColleaguePayment, ColleagueSummary); err != nil {
            return err
        }
        return m.recalcTotal(ctx)

    case CouponApplied:
        m.state.Coupon = ev.Payload.(string)
        if err := m.recalcTotal(ctx); err != nil {
            return err
        }
        if m.pricing.FreeShipping(m.state.Coupon, m.state.Total) {
            m.state.Shipping.Fee = 0
            _ = m.colleagues[ColleagueShipping].Refresh(ctx, &m.state)
        }
        return m.refresh(ColleagueSummary, ColleaguePayment)

    default:
        return nil
    }
}

func (m *CheckoutMediator) recalcAll(ctx context.Context) error {
    quote, err := m.shipping.Quote(ctx, m.state.Address, m.state.Lines)
    if err != nil {
        return err
    }
    m.state.Shipping = quote
    if err := m.recalcTotal(ctx); err != nil {
        return err
    }
    return m.refresh(ColleagueShipping, ColleagueSummary, ColleaguePayment, ColleagueCoupon)
}

func (m *CheckoutMediator) recalcTotal(ctx context.Context) error {
    m.state.Total = m.pricing.Total(ctx, m.state.Lines, m.state.Coupon, m.state.Shipping)
    return nil
}

func (m *CheckoutMediator) refresh(ctx context.Context, ids ...ColleagueID) error {
    for _, id := range ids {
        if c, ok := m.colleagues[id]; ok {
            if err := c.Refresh(ctx, &m.state); err != nil {
                return err
            }
        }
    }
    return nil
}
```

**联动顺序**（先运费再合计再支付）**只写在一处**；新增「发票抬头」= **新 Colleague + 在对应 `case` 里 `refresh`**。

### 具体同事：地址表单（只认中介）

```go
type AddressForm struct {
    mediator Mediator
    addr     Address
}

func (f *AddressForm) SetMediator(m Mediator) { f.mediator = m }

func (f *AddressForm) ID() ColleagueID { return ColleagueAddress }

func (f *AddressForm) OnUserSelect(addr Address) error {
    f.addr = addr
    return f.mediator.Notify(ColleagueAddress, Event{Type: AddressChanged, Payload: addr})
}

func (f *AddressForm) Refresh(ctx context.Context, state *CheckoutContext) error {
    // 只更新自身展示，例如从 state 同步校验提示
    return nil
}
```

### 具体同事：合计面板

```go
type SummaryPanel struct {
    mediator Mediator
    display  int64
}

func (p *SummaryPanel) ID() ColleagueID { return ColleagueSummary }

func (p *SummaryPanel) Refresh(ctx context.Context, state *CheckoutContext) error {
    p.display = state.Total + state.Shipping.Fee
    return nil
}
```

### 具体同事：支付面板（示例）

```go
type PaymentPanel struct {
    mediator       Mediator
    regionFiltered bool
}

func (p *PaymentPanel) SetMediator(m Mediator) { p.mediator = m }

func (p *PaymentPanel) ID() ColleagueID { return ColleaguePayment }

func (p *PaymentPanel) Refresh(ctx context.Context, state *CheckoutContext) error {
    p.regionFiltered = state.Region != "" // 按地区过滤 COD 等
    return nil
}
```

### 组装与客户端

```go
func NewCheckoutPage(pricing PricingEngine, ship ShippingService) *CheckoutMediator {
    m := &CheckoutMediator{pricing: pricing, shipping: ship}
    lines := &LineItemsPanel{}
    addr := &AddressForm{}
    coupon := &CouponWidget{}
    shipping := &ShippingPanel{}
    payment := &PaymentPanel{}
    summary := &SummaryPanel{}

    for _, c := range []Colleague{lines, addr, coupon, shipping, payment, summary} {
        m.Register(c)
        if setter, ok := c.(interface{ SetMediator(Mediator) }); ok {
            setter.SetMediator(m)
        }
    }
    return m
}

// 用户点「提交」——仍走外观，不走 Mediator 替代 PlaceOrder
func (h *CheckoutHandler) Submit(w http.ResponseWriter, r *http.Request) {
    state := h.page.State()
    req := PlaceOrderRequest{Lines: state.Lines, Address: state.Address, /* … */}
    if err := h.facade.PlaceOrder(r.Context(), req); err != nil {
        http.Error(w, err.Error(), 500)
    }
}
```

**页内协同** 用 Mediator；**落库提交** 用 [外观](/cs-fundamentals/design-patterns/facade)——职责清晰。


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

> **阅读提示**：先掌握「**同事 Notify，中介 Refresh**」即可。本节是工程变体；初学可先跳过。

### 中介者 vs 全局 EventBus

```go
// 跨服务：订单已支付 → 通知、积分、搜索
bus.Publish(OrderPaid{ID: orderID})

// 结算页内：必须先 recalcTotal 再 refresh Payment
mediator.Notify(ColleagueCoupon, Event{Type: CouponApplied, Payload: code})
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

### 与命令、责任链一起用

- **页内**：Mediator 协调展示；用户点「改数量」可 **`invoker.Run(AdjustQuantityCommand)`**（[命令](/cs-fundamentals/design-patterns/command)）成功后 **`mediator.Notify(LinesChanged)`**。
- **提交前**：`PlaceOrder` 前 **`preCheckChain.Handle`**（[责任链](/cs-fundamentals/design-patterns/chain-of-responsibility)）在 Facade 内；Mediator **不参与** 服务端校验链。

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

### 测试策略

```go
func TestCheckoutMediator_AddressChangeRefreshesPayment(t *testing.T) {
    payment := &PaymentPanel{}
    m := NewCheckoutPage(fakePricing{}, fakeShipping{})
    m.Register(payment) // 覆盖默认注册，便于断言
    addr := &AddressForm{mediator: m}
    m.Register(addr)

    if err := addr.OnUserSelect(Address{Country: "US"}); err != nil {
        t.Fatal(err)
    }
    if !payment.regionFiltered {
        t.Fatal("payment should refresh on address change")
    }
}

func TestAddressForm_DoesNotReferenceSummary(t *testing.T) {
    med := &recordingMediator{lastFrom: ColleagueAddress}
    f := &AddressForm{mediator: med}
    _ = f.OnUserSelect(Address{City: "Shanghai"})
    if med.lastEvent.Type != AddressChanged {
        t.Fatal("should notify mediator only")
    }
}

type recordingMediator struct {
    lastFrom  ColleagueID
    lastEvent Event
}

func (r *recordingMediator) Register(Colleague) {}
func (r *recordingMediator) State() *CheckoutContext { return &CheckoutContext{} }
func (r *recordingMediator) Notify(from ColleagueID, ev Event) error {
    r.lastFrom, r.lastEvent = from, ev
    return nil
}
```

## 小结

记住这四点即可：

1. **星型而非网状**：同事 **只 Notify Mediator**，不互持引用。
2. **联动规则集中**：改地址 → 运费 → 合计 → 支付过滤 **写在中介**（或 `CheckoutRules`）。
3. **与 Facade 分层**：Mediator 管 **页内/会话内协同**；Facade 管 **提交用例 PlaceOrder**。
4. **开闭靠 Register**：新面板 **实现 Colleague + Register**；改 **Notify 分支** 而非改所有同事。

[外观模式](/cs-fundamentals/design-patterns/facade) 解决了 **「外部客户端如何一次用完子系统」**；中介者解决 **「内部多个对等组件如何联动而不织成网」**——把 **对象间的交互** 从分散的 **双向依赖** 收到 **可测试、可演进的中介**，符合 [迪米特法则](/cs-fundamentals/design-patterns#设计原则) 与 **单一职责**。

## 参考阅读

- [x] [Refactoring.Guru - 中介者模式](https://refactoringguru.cn/design-patterns/mediator) (2026-06-22)
- [x] [菜鸟教程 - 中介者模式](https://www.runoob.com/design-pattern/mediator-pattern.html) (2026-06-22)
