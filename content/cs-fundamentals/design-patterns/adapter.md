---
title: 适配器模式
order: 6
---

**适配器模式**（Adapter）亦称 **封装器模式**，将一个类的接口转换成客户希望的另一个接口，使原本因接口不兼容而不能一起工作的类可以一起工作。

通俗地说，在两套对不上的接口之间插一层「翻译」：对外符合你期望的样子，对内再去调现有组件；第三方或遗留代码不动，业务侧接口也不改，两边就能对接。

## 问题

[工厂方法](/cs-fundamentals/design-patterns/factory) 已让业务只依赖统一的 `PaymentProcessor` 接口。实际接入支付渠道时，第三方 SDK 和遗留系统的方法名、参数往往 **对不上**——银行网关叫 `ChargeCard`，Stripe 叫 `CreatePayment`，返回值也不一样。

最直接的做法是在结算逻辑里 **按类型分支调用**。渠道少时还能应付；每多接一个，问题就会一起暴露：

1. **业务与集成搅在一起**：结算代码要认识每一种第三方 API，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。
2. **无法统一依赖抽象**：字段类型写成具体 SDK 类型，就 **不能** 再声明为 `PaymentProcessor`，测试也难注入 mock。
3. **改一处牵全身**：SDK 升级改方法名、遗留库不能动——都要改结算分支。
4. **违反开闭原则**：每接一个渠道就要改已有代码，而不是只加新适配器。

本质矛盾是：**你期望的接口** 和 **现有组件的接口** 不一致，却又 **不能或不愿** 改第三方代码。典型写法如下：

```go
func (s *Service) Checkout(order Order) error {
    switch n := s.processor.(type) {
    case LegacyBankClient:
        return n.ChargeCard(order.Account, order.Currency, order.Amount)
    case StripeClient:
        _, err := n.CreatePayment(order.CustomerID, order.Amount)
        return err
    }
    return nil
}
```

## 解决方案

为每种不兼容的 Adaptee 写一个适配器，实现 `PaymentProcessor`，在 `Pay` 里完成参数映射与调用转发。

### 目标接口与被适配者

```go
type PaymentProcessor interface {
    Pay(order Order) error
}

type LegacyBankClient struct{}

func (LegacyBankClient) ChargeCard(account, currency string, amount int64) error {
    // …
    return nil
}

type StripeClient struct{}

func (StripeClient) CreatePayment(customerID string, amount int64) (string, error) {
    // …
    return "pi_xxx", nil
}
```

### 对象适配器

适配器 **持有** Adaptee 及组装时需要的配置（账户、客户 ID、币种等），**实现** `PaymentProcessor`：

```go
type LegacyBankAdapter struct {
    bank    LegacyBankClient
    account string
}

func NewLegacyBankAdapter(bank LegacyBankClient, account string) PaymentProcessor {
    return &LegacyBankAdapter{bank: bank, account: account}
}

func (a *LegacyBankAdapter) Pay(order Order) error {
    return a.bank.ChargeCard(a.account, order.Currency, order.Amount)
}

type StripeAdapter struct {
    client StripeClient
    customerID string
}

func NewStripeAdapter(client StripeClient, customerID string) PaymentProcessor {
    return &StripeAdapter{client: client, customerID: customerID}
}

func (a *StripeAdapter) Pay(order Order) error {
    _, err := a.client.CreatePayment(a.customerID, order.Amount)
    return err
}
```

### 使用者不变

`Service` **完全不知道** 背后是遗留银行网关还是 Stripe——与 [工厂方法](/cs-fundamentals/design-patterns/factory) 里注入 `AlipayProcessor` 的写法一致：

```go
type Service struct {
    processor PaymentProcessor
}

func (s *Service) Checkout(order Order) error {
    return s.processor.Pay(order)
}

// 组装层
legacySvc := &Service{
    processor: NewLegacyBankAdapter(LegacyBankClient{}, "buyer-001"),
}
stripeSvc := &Service{
    processor: NewStripeAdapter(StripeClient{}, "cus_001"),
}
```

新增支付渠道时：**加 Adaptee 的适配器 + 组装处换一行**，不必改 `Service`。


## 适用场景

1. **接入第三方库**：SDK 方法名、参数、返回值与内部抽象不一致，且 **不能改 SDK 源码**。
2. **包装遗留系统**：老模块仍用旧 API（`ChargeCard`、`legacy_pay`），新代码已统一成 `PaymentProcessor` / `Repository` 等接口。
3. **复用现有类，但接口不符合 [依赖倒置](/cs-fundamentals/design-patterns#设计原则)**：希望高层只依赖 `PaymentProcessor`，低层却是具体第三方类型。
4. **测试与生产切换**：生产注入真实 SDK 的适配器，测试注入内存版 Adaptee + 同一套 Adapter（或 mock Adaptee）。
5. **渐进式迁移**：新功能走 `PaymentProcessor`；旧实现通过适配器挂接，日后替换 Adaptee 而不动 Client。

**不必强行使用**：

- Adaptee **本来** 就实现了 Target（或稍加修改即可）——直接实现接口，不必多包一层。
- 只是为了 **少写几行转发** 而套适配器——若 Adaptee 归你维护，改 Adaptee 实现 Target 往往更直接。
- 需要的是 **简化一整组复杂子系统** 的入口——那更像 [外观模式](#与外观的区别)，不是适配器。

常见例子：日志库适配统一 `Logger` 接口、ORM 适配仓储接口、HTTP 客户端适配内部 `Fetcher`、支付渠道 SDK 适配 `PaymentGateway`。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新支付渠道 = 新 Adapter，Client 与已有 Adapter 不必改 |
| **复用** | 第三方、遗留代码原样复用，无需 fork 或重写 |
| **单一职责** | 翻译逻辑集中在 Adapter，`Service` 保持业务纯净 |
| **可测试** | 对 Adaptee 注入 fake/stub，单独测 Adapter 的映射是否正确 |

| 缺点 | 说明 |
| :--- | :--- |
| **多一层间接** | 读代码需跳 Adapter → Adaptee，调试多一跳 |
| **类数量增加** | 每种 Adaptee 常对应一个 Adapter（及 `NewXxxAdapter`） |
| **翻译可能复杂** | 错误码、异步回调、流式 API 映射到同步 `Pay` 时，Adapter 会变厚 |
| **掩盖设计问题** | 滥用适配器堆叠，可能推迟「该不该换掉遗留模块」的决策 |

## 实践

### 适配器持有接口还是具体类型

Adaptee 若能抽象成小型接口，Adapter 依赖 **接口** 更利于测试：

```go
type PaymentCreator interface {
    CreatePayment(customerID string, amount int64) (string, error)
}

type StripeAdapter struct {
    client PaymentCreator
    customerID string
}
```

生产注入 `StripeClient{}`，测试注入 `fakeStripe{}`。若 SDK 无法抽接口，只能持有具体 struct，测试时对 Adapter 做集成测试或包一层 thin wrapper。

### 错误与返回值映射

Adaptee 返回 `(msgID, err)`、枚举错误码或 panic 时，Adapter 负责 **归一化** 为 Target 的 `error`：

```go
func (a *StripeAdapter) Pay(order Order) error {
    _, err := a.client.CreatePayment(a.customerID, order.Amount)
    if err != nil {
        return fmt.Errorf("stripe payment: %w", err)
    }
    return nil
}
```

避免把 SDK 特有类型泄漏到 `Service`；必要时在 Adapter 层打日志、重试、指标打点。

## 关联

- [桥接模式](/cs-fundamentals/design-patterns/bridge) 通常在开发前期就设计好，用来把程序拆成可独立演进的两部分；[适配器模式](/cs-fundamentals/design-patterns/adapter) 则多用于已有代码，让原本不兼容的类能一起工作。
- [适配器模式](/cs-fundamentals/design-patterns/adapter) 在外层 **实现 Client 期望的另一套接口**，对内包装原对象并翻译调用——被适配者的原接口不动；[装饰模式](/cs-fundamentals/design-patterns/decorator) **沿用与被包装对象相同的接口**，在委托前后增强行为，且支持递归组合——适配器不支持这种同接口叠层。
- 从接口形态看：[适配器](/cs-fundamentals/design-patterns/adapter) 提供 **不同的** 接口；[代理模式](/cs-fundamentals/design-patterns/proxy) 提供 **相同的** 接口；[装饰模式](/cs-fundamentals/design-patterns/decorator) 提供 **增强后的** 接口。
- [外观模式](/cs-fundamentals/design-patterns/facade) 为现有对象 **定义新接口**；适配器则尽量 **复用已有接口**。适配器通常只包装 **一个** 对象，外观往往面向 **整个子系统**。
- [桥接模式](/cs-fundamentals/design-patterns/bridge)、[状态模式](/cs-fundamentals/design-patterns/state)、[策略模式](/cs-fundamentals/design-patterns/strategy)（以及在一定程度上适配器）的接口结构很相似——都基于 [组合模式](/cs-fundamentals/design-patterns/composite) 式的委托，但各自要解决的问题不同。模式不仅是代码组织方式，也是与同伴讨论 **如何解题** 的共同语言。

## 参考阅读

- [x] [Refactoring.Guru - 适配器模式](https://refactoringguru.cn/design-patterns/adapter) (2026-06-18)
- [x] [菜鸟教程 - 适配器模式](https://www.runoob.com/design-pattern/adapter-pattern.html) (2026-06-18)
