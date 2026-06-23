---
title: 责任链模式
order: 13
---

**责任链模式** 使多个对象都有机会处理请求，从而避免请求的发送者与接收者之间的耦合；将这些对象连成一条链，并沿着这条链传递该请求，直到有一个对象处理它为止。

通俗地说，一连串规则各自负责一环，请求从链头进去、逐个往后传，直到有人处理或全部检查完毕；发起方只交给链头，加规则、调顺序改链的组装就行，不必改调用方代码。

## 问题

下单前要依次做购物车校验、库存检查、优惠券验证、风控；退款要按金额分流到自动批、主管批、财务批。最直接的做法是在 `CheckoutFacade` 和 `RefundService` 里 **硬编码一长串 if**。

规则少时还能应付；每加一条风控或合规检查，问题就会一起暴露：

1. **难扩展**：加一条规则就要改 Facade 源码；不同渠道想关掉某一环，只能再加 `if cfg.X`。
2. **职责混杂**：Facade 既管 **编排**（预占→支付→落库），又管 **十余条校验**，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。
3. **调用方仍知道太多**：B 端想跳过 App 专属风控、跨境站要多一环关税估算——各入口复制不同 if 顺序。
4. **审批规则难维护**：「100 元以下自动退」改成「VIP 自动退、普通 50 元以下」时，`switch amount` 又长又脆。

本质矛盾是：**完成一次业务** 往往需要 **多步检查或分级审批**，但 **链有多长、哪一步处理** 应能 **独立配置**；发送方只应 **把请求交给链头**。典型写法如下：

```go
func (f *CheckoutFacade) PlaceOrder(ctx context.Context, req PlaceOrderRequest) error {
    if err := validateCart(req); err != nil { return err }
    if err := f.checkInventory(ctx, req); err != nil { return err }
    if err := validateCoupon(req); err != nil { return err }
    if f.cfg.EnableFraud {
        if err := f.fraud.Check(ctx, req); err != nil { return err }
    }
    // … 真正的下单编排
    return f.placeOrderCore(ctx, req)
}
```

## 解决方案

定义 **处理者** 接口；各 **具体处理者** 实现单一规则；用 **基类或组合** 持有 `next` 并转发。发送方只依赖 **链头接口**。

### 处理者（Handler）接口与链基类

```go
type PlaceOrderContext struct {
    Req     PlaceOrderRequest
    User    User
    Channel string // "app" | "b2b" | "cross_border"
}

type OrderHandler interface {
    Handle(ctx context.Context, order *PlaceOrderContext) error
}

type BaseHandler struct {
    next OrderHandler
}

func (b *BaseHandler) SetNext(h OrderHandler) OrderHandler {
    b.next = h
    return h
}

func (b *BaseHandler) callNext(ctx context.Context, order *PlaceOrderContext) error {
    if b.next == nil {
        return nil
    }
    return b.next.Handle(ctx, order)
}
```

`SetNext` 返回传入的 handler，便于 **流畅组装**：`h1.SetNext(h2).SetNext(h3)`。

### 管道式：下单前校验链（每环必须通过）

```go
type CartValidator struct{ BaseHandler }

func (h *CartValidator) Handle(ctx context.Context, order *PlaceOrderContext) error {
    if len(order.Req.Lines) == 0 {
        return ErrEmptyCart
    }
    for _, line := range order.Req.Lines {
        if line.Quantity <= 0 {
            return ErrInvalidQuantity
        }
    }
    return h.callNext(ctx, order)
}

type InventoryValidator struct {
    BaseHandler
    inventory InventoryService
}

func (h *InventoryValidator) Handle(ctx context.Context, order *PlaceOrderContext) error {
    for _, line := range order.Req.Lines {
        if err := h.inventory.CheckStock(ctx, line.SKU, line.Quantity); err != nil {
            return err
        }
    }
    return h.callNext(ctx, order)
}

type CouponValidator struct {
    BaseHandler
    coupons CouponService
}

func (h *CouponValidator) Handle(ctx context.Context, order *PlaceOrderContext) error {
    if order.Req.CouponCode == "" {
        return h.callNext(ctx, order)
    }
    if err := h.coupons.Validate(ctx, order.User.ID, order.Req.CouponCode); err != nil {
        return err
    }
    return h.callNext(ctx, order)
}

type FraudValidator struct {
    BaseHandler
    fraud FraudService
}

func (h *FraudValidator) Handle(ctx context.Context, order *PlaceOrderContext) error {
    if order.Channel == "b2b" {
        return h.callNext(ctx, order) // B 端跳过 C 端风控
    }
    if err := h.fraud.Score(ctx, order); err != nil {
        return err
    }
    return h.callNext(ctx, order)
}
```

任一环返回 `error` → **链条短路**，不会进入 [外观](/cs-fundamentals/design-patterns/facade) 的 `placeOrderCore`；`next == nil` 时 **管道成功结束**。

### 单处理者：退款审批链（仅一环真正处理）

```go
type RefundContext struct {
    Req    RefundRequest
    User   User
    Result ApprovalResult
}

type RefundHandler interface {
    Handle(ctx context.Context, r *RefundContext) error
}

type AutoRefundHandler struct {
    BaseRefundHandler
    limit int64 // 分
}

func (h *AutoRefundHandler) Handle(ctx context.Context, r *RefundContext) error {
    if r.Req.Amount <= h.limit && !r.User.IsVIP {
        r.Result = ApprovalResult{Approved: true, By: "auto"}
        return nil // 处理完毕，不 callNext
    }
    return h.callNext(ctx, r)
}

type SupervisorHandler struct {
    BaseRefundHandler
    limit int64
    sup   SupervisorService
}

func (h *SupervisorHandler) Handle(ctx context.Context, r *RefundContext) error {
    if r.Req.Amount <= h.limit {
        res, err := h.sup.Approve(ctx, r.Req)
        if err != nil {
            return err
        }
        r.Result = res
        return nil
    }
    return h.callNext(ctx, r)
}

type FinanceHandler struct {
    BaseRefundHandler
    finance FinanceService
}

func (h *FinanceHandler) Handle(ctx context.Context, r *RefundContext) error {
    res, err := h.finance.Approve(ctx, r.Req)
    if err != nil {
        return err
    }
    r.Result = res
    return nil // 链尾必须处理，否则应返回 ErrNoHandler
}
```

100 元以下在 `AutoRefundHandler` **终结**；更大金额 **穿透** 到主管、财务。发送方 `RefundService` **不知道** 最终是哪一环批的。

### 组装链（按渠道不同链长）

```go
func NewAppPreCheckChain(inv InventoryService, coupons CouponService, fraud FraudService) OrderHandler {
    cart := &CartValidator{}
    stock := &InventoryValidator{inventory: inv}
    coupon := &CouponValidator{coupons: coupons}
    risk := &FraudValidator{fraud: fraud}
    cart.SetNext(stock).SetNext(coupon).SetNext(risk)
    return cart
}

func NewB2BPreCheckChain(inv InventoryService) OrderHandler {
    cart := &CartValidator{}
    stock := &InventoryValidator{inventory: inv}
    cart.SetNext(stock) // 无券、无 C 端风控
    return cart
}

func NewRefundApprovalChain(autoLimit, supervisorLimit int64, sup SupervisorService, fin FinanceService) RefundHandler {
    auto := &AutoRefundHandler{limit: autoLimit}
    supervisor := &SupervisorHandler{limit: supervisorLimit, sup: sup}
    finance := &FinanceHandler{finance: fin}
    auto.SetNext(supervisor).SetNext(finance)
    return auto
}
```

### 客户端（Client）——Facade 与 RefundService

```go
type CheckoutFacade struct {
    preCheck OrderHandler
    // … inventory, payment, orders 等
}

func (f *CheckoutFacade) PlaceOrder(ctx context.Context, req PlaceOrderRequest) error {
    oc := &PlaceOrderContext{Req: req, User: userFrom(ctx), Channel: channelFrom(ctx)}
    if err := f.preCheck.Handle(ctx, oc); err != nil {
        return err
    }
    return f.placeOrderCore(ctx, req)
}

type RefundService struct {
    approval RefundHandler
    payout   PayoutService
}

func (s *RefundService) RequestRefund(ctx context.Context, r RefundRequest) error {
    rc := &RefundContext{Req: r, User: userFrom(ctx)}
    if err := s.approval.Handle(ctx, &rc); err != nil {
        return err
    }
    if !rc.Result.Approved {
        return ErrRejected
    }
    return s.payout.Execute(ctx, r)
}
```

测试时注入 **只含两环的短链** 或 **mock 某一环**，无需启动完整 Facade。


## 适用场景

1. **多个对象可能处理同一请求，且处理者在运行时未确定**：退款分级、客服工单升级、日志级别过滤（只由某一环写盘）。
2. **想动态指定处理者集合或顺序**：App / B 端 / 跨境 **不同链**；大促临时 **加一环** 限流校验。
3. **发送方不应依赖具体处理者类型**：Facade、Controller 只认 `OrderHandler`。
4. **规则可独立测试、独立发布**：新风控环 **新 struct + 改组装**，不动 `placeOrderCore`。
5. **管道式前置检查**：下单、提现、入驻审核——**全会签** 用责任链比 Facade 内 `if` 堆叠更清晰。

**不必强行使用**：

- 只有 **固定两步、永不变** 的校验——直接两个函数调用更简单。
- 需要的是 **有副作用的固定编排**（预占→支付→落库）——用 [外观](/cs-fundamentals/design-patterns/facade)，不是责任链替代 Facade。
- 每一层都要 **包装同一接口并叠加计价**——用 [装饰器](/cs-fundamentals/design-patterns/decorator)。
- 只有 **一个** 接收者——不需要链。
- 处理者之间 **强顺序依赖且共享大量中间状态**——可能更适合 **显式状态机** 或工作流引擎（责任链仍可作 **轻量** 替代）。

常见例子：Servlet `Filter` 链、Netty `ChannelPipeline`、Kubernetes **准入控制器**（Admission Webhook 链）、前端事件冒泡、日志 appender 链。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新校验 / 新审批档 **加 ConcreteHandler + 改组装** |
| **单一职责** | 每环一个文件一种规则；Facade 专注编排 |
| **发送方解耦** | Client 不 import 十个 Validator |
| **可组合** | 按渠道拼不同链；测试用短链 |
| **与 Facade 正交** | 前置链 + 核心编排 **分层清晰** |

| 缺点 | 说明 |
| :--- | :--- |
| **链未命中** | 单处理者链若 **无人处理**，需链尾兜底或明确 `ErrNoHandler` |
| **调试路径** | 请求在哪一环失败，要靠 **日志 / 链环命名**；过长链难 trace |
| **顺序敏感** | 管道式：先风控还是先库存，业务上要 **文档化** |
| **性能** | 环过多时 **每次请求遍历**；极热路径可合并或编译期链 |
| **与循环依赖** | Handler 互相引用子系统时，注意 **依赖方向** 仍指向领域服务 |

## 实践

> **阅读提示**：先掌握「**链头进入、每环处理或转发、发送方不知具体环**」即可。本节是工程变体；初学可先跳过。

### 指针链 vs 切片链

```go
// 切片管道：顺序即链，无 SetNext
type Pipeline struct {
    handlers []OrderHandler
}

func (p *Pipeline) Handle(ctx context.Context, order *PlaceOrderContext) error {
    for _, h := range p.handlers {
        if err := h.Handle(ctx, order); err != nil {
            return err
        }
    }
    return nil
}

// 注意：此时每个 Handler 不应再 callNext，否则双重遍历
```

**指针链**：灵活 `SetNext`、经典 GoF 图。**切片链**：顺序一目了然、易单测「去掉中间某一环」。**勿混用**：切片模式下 Handler **只 return err 或 nil**，不再 `callNext`。

### 管道链顺序建议（下单前）

| 顺序（前 → 后） | 原因 |
| :--- | :--- |
| **廉价、无副作用** 校验靠前 | 空购物车、非法数量 **尽早失败** |
| **依赖外部的读检查** | 库存、券有效性 |
| **重 / 慢 / 第三方** 靠后 | 风控评分、跨境合规 **少打无效单** |
| **进入 Facade 核心** 最后 | 预占、扣款 **只在全会签后** |

与 [代理模式](/cs-fundamentals/design-patterns/proxy) 组合：`InventoryValidator` 注入的 `InventoryService` 可以是 **带缓存的远程代理**，责任链 **不关心** 库存是本地还是 gRPC。

### 单处理者链尾兜底

```go
type RejectHandler struct{ BaseRefundHandler }

func (h *RejectHandler) Handle(ctx context.Context, r *RefundContext) error {
    r.Result = ApprovalResult{Approved: false, Reason: "no approver matched"}
    return nil
}
// finance.SetNext(&RejectHandler{})
```

避免请求 **穿透链尾无人处理** 却 **静默成功**。

### 与外观、装饰器一起用

```go
func NewCheckoutFacade(preCheck OrderHandler, /* 子系统 */) *CheckoutFacade {
    return &CheckoutFacade{preCheck: preCheck, /* … */}
}

// 行级计价仍在装饰器；链只管订单级前置条件
facade := NewCheckoutFacade(
    NewAppPreCheckChain(cachingInventoryProxy, coupons, fraud),
    /* payment, orders, … */
)
```

[装饰器](/cs-fundamentals/design-patterns/decorator) 作用在 `OrderLine.Total()`；责任链作用在 `PlaceOrderRequest` **整体**——层次不同，可 **同时存在**。

### 可观测性

```go
func (h *CartValidator) Handle(ctx context.Context, order *PlaceOrderContext) error {
    ctx = context.WithValue(ctx, handlerKey{}, "cart")
    start := time.Now()
    err := h.handle(ctx, order)
    metrics.Record("precheck", "cart", time.Since(start), err)
    return err
}
```

每一环打 **name + latency**，排障时可见 **卡在哪一环**。

### 测试策略

```go
func TestPreCheckChain_StopsOnEmptyCart(t *testing.T) {
    chain := NewAppPreCheckChain(fakeInv{}, fakeCoupons{}, fakeFraud{})
    err := chain.Handle(context.Background(), &PlaceOrderContext{Req: PlaceOrderRequest{}})
    if !errors.Is(err, ErrEmptyCart) {
        t.Fatal(err)
    }
}

func TestRefundChain_AutoApprovesSmall(t *testing.T) {
    chain := NewRefundApprovalChain(10000, 100000, nil, nil)
    rc := &RefundContext{Req: RefundRequest{Amount: 5000}}
    _ = chain.Handle(context.Background(), rc)
    if rc.Result.By != "auto" {
        t.Fatal("expected auto")
    }
}

func TestRefundChain_SkipsAutoForLarge(t *testing.T) {
    sup := &recordingSupervisor{}
    chain := NewRefundApprovalChain(10000, 100000, sup, nil)
    rc := &RefundContext{Req: RefundRequest{Amount: 50000}}
    _ = chain.Handle(context.Background(), rc)
    if sup.calls != 1 {
        t.Fatal("supervisor should approve")
    }
}
```

## 小结

记住这四点即可：

1. **链上传递，发送方只认链头**：`CheckoutFacade` / `RefundService` 把 `PlaceOrderContext`、`RefundContext` 交给 **第一个 Handler**，不必知道哪一环最终处理。
2. **两种形态分清**：**管道式**（下单前校验，每环必须通过）；**单处理者**（退款审批，一环接单即结束）。
3. **与 Facade 分工**：责任链做 **可插拔检查 / 分级审批**；[外观](/cs-fundamentals/design-patterns/facade) 做 **有副作用的核心编排**——`Handle` 通过后再 `placeOrderCore`。
4. **组装在边界**：按渠道 `NewAppPreCheckChain` / `NewB2BPreCheckChain`；新增规则 **新 Handler + 改链**，少改 Facade 内部。

[外观模式](/cs-fundamentals/design-patterns/facade) 解决了 **「客户端不要认识六个子系统」**；责任链解决 **「客户端也不要认识十二条校验与三档审批」**——把 **谁有权处理这类请求** 从发送方剥到 **可配置的处理者链** 上，让下单与退款在规则频繁变化时仍保持 [开闭](/cs-fundamentals/design-patterns#设计原则)。

## 参考阅读

- [x] [Refactoring.Guru - 责任链模式](https://refactoringguru.cn/design-patterns/chain-of-responsibility) (2026-06-22)
- [x] [菜鸟教程 - 责任链模式](https://www.runoob.com/design-pattern/chain-of-responsibility-pattern.html) (2026-06-22)
