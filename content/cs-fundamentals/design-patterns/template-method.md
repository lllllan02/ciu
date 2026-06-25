---
title: 模板方法模式
order: 21
---

**模板方法模式**（Template Method）定义一个操作中的算法的骨架，而将一些步骤延迟到子类中；模板方法使得子类可以不改变一个算法的结构即可重定义该算法的某些特定步骤。

通俗地说，流程的大步骤和顺序固定不变，其中若干环节留给子类或钩子去填；新增一种变体只改写不同的那几步，整条流程不用重写，也不易漏步骤。

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

## 解决方案

定义 **CheckoutTemplate**；**模板方法** `PlaceOrder` 固定 **预占→计价→收款→落库** 顺序与失败补偿；可变步骤通过 **CheckoutHooks** 注入——Go 无 Java 式子类多态覆写，**Hooks 接口** 是地道写法（嵌入 struct 的局限见 **实践**）。

### 钩子与模板

```go
type PlaceOrderRequest struct {
    User      User
    Lines     []OrderLine
    Channel   string
    AccountID string // B2B
}

type CheckoutDeps struct {
    Inventory InventoryService
    Pricing   *PricingEngine
    Payment   PaymentProcessor
    Credit    CreditService
    Orders    OrderRepository
    Notify    NotificationService
}

type placeCtx struct {
    req     PlaceOrderRequest
    amount  int64
    orderID string
}

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
    if err := t.deps.Inventory.Reserve(ctx, req.Lines); err != nil {
        return "", err
    }
    pc.amount = t.deps.Pricing.Total(ctx, PricingContext{
        Order: Order{Lines: req.Lines}, User: req.User, Channel: req.Channel,
    })
    if err := t.hooks.CollectPayment(ctx, pc); err != nil {
        _ = t.deps.Inventory.Release(ctx, req.Lines)
        return "", err
    }
    orderID, err := t.deps.Orders.Save(ctx, req)
    if err != nil {
        _ = t.deps.Payment.Refund(ctx, pc.req.User.ID, pc.amount)
        _ = t.deps.Inventory.Release(ctx, req.Lines)
        return "", err
    }
    pc.orderID = orderID
    if err := t.hooks.AfterPlace(ctx, pc); err != nil {
        return orderID, err // 已落库：after 失败常记日志，不 rollback 主单
    }
    return orderID, nil
}
```

**Release / Refund 补偿写在骨架里**——子类 **不能忘**。

### 具体钩子

```go
type AppHooks struct{ deps CheckoutDeps }

func (AppHooks) PrePlace(context.Context, *placeCtx) error { return nil }

func (h AppHooks) CollectPayment(ctx context.Context, pc *placeCtx) error {
    return h.deps.Payment.Pay(ctx, pc.req.User.ID, pc.amount)
}

func (h AppHooks) AfterPlace(ctx context.Context, pc *placeCtx) error {
    return h.deps.Notify.SendSMS(ctx, pc.req.User.ID, pc.orderID)
}

type B2BHooks struct{ deps CheckoutDeps }

func (h B2BHooks) PrePlace(ctx context.Context, pc *placeCtx) error {
    return validatePO(ctx, pc.req)
}

func (h B2BHooks) CollectPayment(ctx context.Context, pc *placeCtx) error {
    return h.deps.Credit.ChargeAccount(ctx, pc.req.AccountID, pc.amount)
}

func (h B2BHooks) AfterPlace(ctx context.Context, pc *placeCtx) error {
    return h.deps.Notify.SendEmail(ctx, pc.req.AccountID, pc.orderID)
}
```

跨境合规校验、FX 支付、落库后审计日志等 **再实现一套 Hooks**，套路相同。

### 选择与组装

```go
func NewCheckout(deps CheckoutDeps, channel string) *CheckoutTemplate {
    switch channel {
    case "b2b":
        return &CheckoutTemplate{deps: deps, hooks: B2BHooks{deps: deps}}
    default:
        return &CheckoutTemplate{deps: deps, hooks: AppHooks{deps: deps}}
    }
}
```

对外可由 [外观](/cs-fundamentals/design-patterns/facade) `CheckoutFacade.PlaceOrder` 按 `req.Channel` 选模板——**Hollywood**：Controller **只调 Facade**，Facade **调模板**，模板 **调 Hooks**。conditional hook、责任链 PrePlace 见 **实践**。

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


## 关联

- [工厂方法模式](/cs-fundamentals/design-patterns/factory) 是 [模板方法模式](/cs-fundamentals/design-patterns/template-method) 的一种特殊形式。同时，工厂方法可以作为一个大型模板方法中的一个步骤。
- [模板方法模式](/cs-fundamentals/design-patterns/template-method) 基于继承机制：它允许你通过扩展子类中的部分内容来改变部分算法。[策略模式](/cs-fundamentals/design-patterns/strategy) 基于组合机制：你可以通过对相应行为提供不同的策略来改变对象的部分行为。模板方法在类层次上运作，因此它是静态的。策略在对象层次上运作，因此允许在运行时切换行为。

## 参考阅读

- [x] [Refactoring.Guru - 模板方法模式](https://refactoringguru.cn/design-patterns/template-method) (2026-06-22)
- [x] [菜鸟教程 - 模板方法模式](https://www.runoob.com/design-pattern/template-method-pattern.html) (2026-06-22)
