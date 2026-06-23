---
title: 模板方法模式
order: 21
---

**模板方法模式** 在 **抽象类** 中 **固定算法骨架**（步骤顺序），把 **可变步骤** 延迟到 **子类或钩子** 实现——子类 **只覆写某几步**，不重写整条流程。

## 问题

App、B 端、跨境站都要 **预占→计价→支付→落库**，骨架相同，但 **前置校验、收款方式、下单后通知** 各不同。最直接的做法是为每个渠道 **各写一遍完整 PlaceOrder**。

渠道少时还能应付；每多一个渠道，问题就会一起暴露：

1. **骨架重复**：预占 → 计价 → 收款 → 落库 → 失败补偿 **复制多份**——改「Save 失败要 Refund」容易漏改 B2B。
2. **顺序难统一**：跨境把合规日志放 Pay 前，App 放 Save 后——没有单一真相。
3. **改一步要改多处**：加「落库后发领域事件」要改每个 Checkout 实现。
4. **容易跳过步骤**：子类自己拼流程时，可能 **先 Pay 后 Reserve** 或 **漏 Release**。

本质矛盾是：**PlaceOrder 的阶段划分稳定**，但 **若干阶段的实现因渠道变化**；不应 **为每个渠道重写整条算法**。典型写法如下：

```go
func (s *AppCheckout) PlaceOrder(ctx context.Context, req PlaceOrderRequest) error {
    // 预占 → 计价 → 在线支付 → 落库 → 发短信
    // B2BCheckout、CrossBorderCheckout 各复制一遍，只改中间几行
}
```

## 意图

用一句话说：**在一个方法中定义一个算法的骨架，而将一些步骤延迟到子类中。模板方法使得子类可以不改变一个算法的结构即可重定义该算法的某些特定步骤。**

引入 **抽象模板** `PlaceOrderTemplate`，**模板方法** `PlaceOrder` **final 顺序**（Go 用 **非导出或文档约定** 不覆写）；**钩子** `prePlace`、`collectPayment`、`afterPlace` 由 **具体子类** 实现：

```go
// 骨架在基类；AppCheckout 只覆写 Hook
return appCheckout.PlaceOrder(ctx, req)
```

GoF 从 **结构** 角度的定义：

> 在一个操作中定义算法的骨架，而将一些步骤延迟到子类中。模板方法使得子类可以不改变算法的结构即可重定义该算法的某些特定步骤。

### 和外观、策略、责任链有啥不同

| | 模板方法 | 外观 | 策略 | 责任链 |
| :--- | :--- | :--- | :--- | :--- |
| **动机** | **复用算法骨架** | **简化外部使用** | **替换整段算法** | **传递请求** |
| **结构** | 继承/嵌入 + 钩子 | 组合子系统 | 组合 Strategy | 链式 Handler |
| **步骤顺序** | **模板固定** | Facade 内固定 | 策略内自定 | 链上 **可变** |
| **电商例子** | App/B2B 下单骨架 | 对外 PlaceOrder | 会员计价 | 下单前校验链 |

#### 模板方法像「Facade 里的 if channel 吗？」

**部分像，但模板更清晰。** `if channel == "b2b"` 堆在 **一个 Facade** 里会 **膨胀**；模板 **每渠道一个子类/嵌入**，**骨架只维护一处**——**开闭** 更好。

#### Go 没有继承怎么办？

**结构体嵌入 + 覆写钩子方法**（同名方法 **shadow**），或 **Hooks struct 注入**——思想仍是 **模板方法**。

## 解决方案

定义 **PlaceOrderTemplate**（抽象类/基 struct）；**模板方法** `PlaceOrder` 按序调 **primitive + hook**；**AppCheckout**、**B2BCheckout** **只覆写钩子**。

### 钩子与上下文

```go
type PlaceOrderRequest struct {
    User      User
    Lines     []OrderLine
    Channel   string
    AccountID string // B2B
    Currency  string // 跨境
}

type CheckoutDeps struct {
    Inventory InventoryService
    Pricing   *PricingEngine
    Payment   PaymentProcessor
    Credit    CreditService
    Orders    OrderRepository
    Notify    NotificationService
    PreCheck  OrderHandler // 责任链链头
}

type placeCtx struct {
    req     PlaceOrderRequest
    amount  int64
    orderID string
}
```

### 抽象模板与模板方法

```go
type CheckoutTemplate struct {
    deps CheckoutDeps
}

// 模板方法：顺序固定，子类不应覆写（Go：导出但文档禁止 override）
func (t *CheckoutTemplate) PlaceOrder(ctx context.Context, req PlaceOrderRequest) (string, error) {
    pc := &placeCtx{req: req}

    if err := t.prePlace(ctx, pc); err != nil {
        return "", err
    }
    if err := t.deps.Inventory.Reserve(ctx, req.Lines); err != nil {
        return "", err
    }
    pc.amount = t.deps.Pricing.Total(ctx, PricingContext{
        Order: Order{Lines: req.Lines}, User: req.User, Channel: req.Channel,
    })

    if err := t.collectPayment(ctx, pc); err != nil {
        _ = t.deps.Inventory.Release(ctx, req.Lines)
        return "", err
    }
    orderID, err := t.deps.Orders.Save(ctx, req)
    if err != nil {
        t.rollbackPayment(ctx, pc)
        _ = t.deps.Inventory.Release(ctx, req.Lines)
        return "", err
    }
    pc.orderID = orderID

    if err := t.afterPlace(ctx, pc); err != nil {
        return orderID, err // 订单已落库：after 失败常记录日志，不 rollback 主单
    }
    return orderID, nil
}

// —— 钩子与可覆写原语 —— //

func (t *CheckoutTemplate) prePlace(ctx context.Context, pc *placeCtx) error {
    return nil // 默认无额外前置
}

func (t *CheckoutTemplate) collectPayment(ctx context.Context, pc *placeCtx) error {
    return t.deps.Payment.Pay(ctx, pc.req.User.ID, pc.amount)
}

func (t *CheckoutTemplate) rollbackPayment(ctx context.Context, pc *placeCtx) {
    _ = t.deps.Payment.Refund(ctx, pc.req.User.ID, pc.amount)
}

func (t *CheckoutTemplate) afterPlace(ctx context.Context, pc *placeCtx) error {
    return nil
}
```

**补偿** `Release` / `Refund` **写在骨架里**——子类 **不能忘**。

### 具体类：App 下单

```go
type AppCheckout struct {
    CheckoutTemplate
}

func NewAppCheckout(deps CheckoutDeps) *AppCheckout {
    return &AppCheckout{CheckoutTemplate: CheckoutTemplate{deps: deps}}
}

func (c *AppCheckout) prePlace(ctx context.Context, pc *placeCtx) error {
    return c.deps.PreCheck.Handle(ctx, &PlaceOrderContext{Req: pc.req})
}

func (c *AppCheckout) afterPlace(ctx context.Context, pc *placeCtx) error {
    return c.deps.Notify.SendSMS(ctx, pc.req.User.ID, pc.orderID)
}
```

嵌入 `CheckoutTemplate` 后，`AppCheckout.PlaceOrder` **提升为外层方法**——实际 Go 中 **显式委托** 更清晰：

```go
func (c *AppCheckout) PlaceOrder(ctx context.Context, req PlaceOrderRequest) (string, error) {
    return c.CheckoutTemplate.PlaceOrder(ctx, req) // 钩子多态靠嵌入 shadow
}
```

Go 嵌入 **不自动多态覆写** 父 struct 方法内的 `t.prePlace`——**模板内应调接口** 或 **Hooks 字段**。工程上常用 **Hooks 接口**：

```go
type CheckoutHooks interface {
    PrePlace(ctx context.Context, pc *placeCtx) error
    CollectPayment(ctx context.Context, pc *placeCtx) error
    AfterPlace(ctx context.Context, pc *placeCtx) error
}

type CheckoutTemplate struct {
    deps  CheckoutDeps
    hooks CheckoutHooks
}

func (t *CheckoutTemplate) PlaceOrder(ctx context.Context, req PlaceOrderRequest) (string, error) {
    pc := &placeCtx{req: req}
    if err := t.hooks.PrePlace(ctx, pc); err != nil {
        return "", err
    }
    // … Reserve …
    if err := t.hooks.CollectPayment(ctx, pc); err != nil {
        _ = t.deps.Inventory.Release(ctx, req.Lines)
        return "", err
    }
    // … Save、rollback、AfterPlace …
    return pc.orderID, nil
}
```

下文 **B2B** 用 **Hooks 实现**（Go **地道** 写法）。

### 具体 Hooks：B2B 代客下单

```go
type B2BHooks struct {
    deps CheckoutDeps
}

func (h B2BHooks) PrePlace(ctx context.Context, pc *placeCtx) error {
    return validatePO(ctx, pc.req)
}

func (h B2BHooks) CollectPayment(ctx context.Context, pc *placeCtx) error {
    return h.deps.Credit.ChargeAccount(ctx, pc.req.AccountID, pc.amount)
}

func (h B2BHooks) AfterPlace(ctx context.Context, pc *placeCtx) error {
    return h.deps.Notify.SendEmail(ctx, pc.req.AccountID, pc.orderID)
}

func NewB2BCheckout(deps CheckoutDeps) *CheckoutTemplate {
    return &CheckoutTemplate{deps: deps, hooks: B2BHooks{deps: deps}}
}
```

### 具体 Hooks：跨境站

```go
type CrossBorderHooks struct {
    deps       CheckoutDeps
    compliance ComplianceService
}

func (h CrossBorderHooks) PrePlace(ctx context.Context, pc *placeCtx) error {
    if err := h.deps.PreCheck.Handle(ctx, &PlaceOrderContext{Req: pc.req}); err != nil {
        return err
    }
    return h.compliance.Verify(ctx, pc.req)
}

func (h CrossBorderHooks) CollectPayment(ctx context.Context, pc *placeCtx) error {
    return h.deps.Payment.PayWithFX(ctx, pc.req.User.ID, pc.amount, pc.req.Currency)
}

func (h CrossBorderHooks) AfterPlace(ctx context.Context, pc *placeCtx) error {
    _ = h.compliance.LogExport(ctx, pc.orderID)
    return h.deps.Notify.SendEmail(ctx, pc.req.User.ID, pc.orderID)
}
```

### 客户端——Facade 选择模板

```go
type CheckoutFacade struct {
    app         *CheckoutTemplate
    b2b         *CheckoutTemplate
    crossBorder *CheckoutTemplate
}

func (f *CheckoutFacade) PlaceOrder(ctx context.Context, req PlaceOrderRequest) (string, error) {
    switch req.Channel {
    case "b2b":
        return f.b2b.PlaceOrder(ctx, req)
    case "cross_border":
        return f.crossBorder.PlaceOrder(ctx, req)
    default:
        return f.app.PlaceOrder(ctx, req)
    }
}
```

[外观](/cs-fundamentals/design-patterns/facade) **对外一个入口**；内部 **按渠道选 Template**——**Hollywood**：Controller **只调 Facade**，Facade **调模板**，模板 **调 Hooks**。


## 适用场景

1. **多实现共享算法骨架**：下单、对账导出、数据同步流水线。
2. **要固定顺序与补偿**：骨架 **统一 rollback**，子类 **不能乱序**。
3. **开闭**：新渠道 **新 Hooks**，**不改** `PlaceOrder` 顺序。
4. **Hollywood 控制反转**：框架 **调扩展点**，扩展 **不调用框架内部步骤**。
5. **与策略组合**：骨架里 `Pricing.Total` **用** [策略](/cs-fundamentals/design-patterns/strategy)；**骨架本身** 用模板。

**不必强行使用**：

- **仅一个渠道、流程永不变**——一个 Facade 函数够用。
- **步骤顺序也要变**——模板 **不适合**；考虑 **策略** 或 **不同 Facade**。
- **每步是可选处理者**——[责任链](/cs-fundamentals/design-patterns/chain-of-responsibility)。
- **整段算法完全替换**——策略，不是模板。

常见例子：`HttpServlet.service`、JUnit `setUp/test/tearDown`、Spring `JdbcTemplate`、ETL 框架、游戏主循环。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **复用骨架** | 补偿、顺序 **一处维护** |
| **开闭** | 新渠道 **新 Hooks** |
| **控制反转** | 子类 **只填钩子** |
| **减少复制粘贴** | 三份 PlaceOrder → **一份模板** |
| **与 Facade 叠加** | 对外简、对内 **分渠道模板** |

| 缺点 | 说明 |
| :--- | :--- |
| **继承/钩子限制** | Go **无真正多态嵌入**——需 **Hooks 接口** |
| **骨架僵化** | 要 **中间加一步** 仍要 **改模板** |
| **层级增多** | 简单流程 **可能过度** |
| **与 Facade 边界** | 团队需约定 **谁是对外 Facade、谁是内部模板** |
| **after 失败语义** | 落库后钩子失败 **是否补偿** 要 **文档化** |

## 组装实践

> **阅读提示**：先掌握「**PlaceOrder 固定顺序，Hooks 可变步骤**」即可。本节是工程变体；初学可先跳过。

### 与责任链、策略、观察者

```text
PlaceOrderTemplate.PlaceOrder
  → hooks.PrePlace → preCheckChain（责任链）
  → Pricing.Total（策略）
  → hooks.CollectPayment
  → Save
  → hooks.AfterPlace → publish(OrderPlaced)（观察者，可选放 after）
```

[责任链](/cs-fundamentals/design-patterns/chain-of-responsibility) 在 **PrePlace 钩子**；[策略](/cs-fundamentals/design-patterns/strategy) 在 **计价步**；[观察者](/cs-fundamentals/design-patterns/observer) 在 **AfterPlace**——**骨架不变**。

### 钩子类型

| 类型 | 行为 | 例子 |
| :--- | :--- | :--- |
| **abstract** | 子类 **必须** 实现 | `CollectPayment`（B2B vs App 必不同） |
| **hook（空默认）** | 可 **不覆写** | 默认 `AfterPlace` no-op |
| **conditional hook** | 模板问 `shouldSendSMS()` | App true，B2B false |

```go
func (t *CheckoutTemplate) PlaceOrder(...) {
    // …
    if t.hooks.ShouldNotify() {
        _ = t.hooks.AfterPlace(ctx, pc)
    }
}
```

### 模板方法 vs 生成器

| | 模板方法 | [生成器](/cs-fundamentals/design-patterns/builder) |
| :--- | :--- | :--- |
| **目的** | **过程** 骨架复用 | **对象** 分步构建 |
| **电商** | PlaceOrder 流水线 | 构建复杂 `PlaceOrderRequest` |

可 **Builder 构建 req** → **Template PlaceOrder(req)**。

### 测试：只测钩子

```go
func TestPlaceOrderTemplate_RollbackOnPayFail(t *testing.T) {
    inv := &fakeInventory{}
    hooks := &stubHooks{payErr: ErrDeclined}
    tpl := &CheckoutTemplate{deps: CheckoutDeps{Inventory: inv, /* … */}, hooks: hooks}
    _, err := tpl.PlaceOrder(context.Background(), PlaceOrderRequest{Lines: lines})
    if err == nil || !inv.released {
        t.Fatal("expected release on pay fail")
    }
}

func TestB2BHooks_ChargesCredit(t *testing.T) {
    credit := &fakeCredit{}
    h := B2BHooks{deps: CheckoutDeps{Credit: credit}}
    pc := &placeCtx{req: PlaceOrderRequest{AccountID: "a1"}, amount: 1000}
    _ = h.CollectPayment(context.Background(), pc)
    if credit.charged != 1000 {
        t.Fatal()
    }
}
```

### 反模式：子类重写 PlaceOrder

```go
// 反模式：B2BCheckout 重写整条 PlaceOrder，又复制骨架
func (b *B2BCheckout) PlaceOrder(ctx context.Context, req PlaceOrderRequest) (string, error) {
    // 自己拼 Reserve/Pay/Save——回到「问题」一节
}
```

**应只实现 Hooks**。

## 小结

记住这四点即可：

1. **骨架在一处**：`PlaceOrder` 顺序与 **失败补偿** 只在 `CheckoutTemplate`。
2. **变化在钩子**：`PrePlace`、`CollectPayment`、`AfterPlace` **按渠道实现**。
3. **Hollywood**：渠道代码 **不自己拼 Reserve→Pay**；**模板调用 Hooks**。
4. **与 Facade 叠加**：对外 [外观](/cs-fundamentals/design-patterns/facade) `PlaceOrder`；对内 **选 Template/Hooks**。

[外观模式](/cs-fundamentals/design-patterns/facade) 解决了 **「客户端如何一次用完子系统」**；模板方法解决了 **「多个渠道共享同一条用例流水线，又不必复制粘贴」**——把 **不变算法结构** 留在 **模板方法**，把 **易变步骤** 延迟到 **钩子**，在 [开闭](/cs-fundamentals/design-patterns#设计原则) 下扩展 App / B2B / 跨境下单。

## 参考阅读

- [x] [外观模式](/cs-fundamentals/design-patterns/facade) — 对外统一入口；内部可用模板
- [x] [策略模式](/cs-fundamentals/design-patterns/strategy) — 骨架内计价；整段算法替换对比
- [x] [责任链模式](/cs-fundamentals/design-patterns/chain-of-responsibility) — PrePlace 钩子内校验链
- [x] [观察者模式](/cs-fundamentals/design-patterns/observer) — AfterPlace 发领域事件
- [x] [Refactoring.Guru - 模板方法模式](https://refactoringguru.cn/design-patterns/template-method) (2026-06-22)
- [x] [菜鸟教程 - 模板方法模式](https://www.runoob.com/design-pattern/template-method-pattern.html) (2026-06-22)
