---
title: 适配器模式
order: 6
---

**适配器模式** 把 **现有接口** 转换成 **调用方期望的接口**，使 **原本因接口不兼容而无法协作的组件可以一起工作**——不改第三方或遗留代码，由适配器在中间做「翻译」。

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

## 意图

用一句话说：**将一个类的接口转换成客户希望的另一个接口，使原本因接口不兼容而不能一起工作的类可以一起工作。**

在目标接口（如 `PaymentProcessor`）与被适配者（如遗留银行网关、Stripe SDK）之间插入适配器：适配器实现目标接口，持有被适配者，在 `Pay` 里把调用翻译成对方能懂的方法——不改第三方代码，也不改业务接口。

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

## 组装实践

> **阅读提示**：先掌握「Adapter 实现 `PaymentProcessor` + 持有 Adaptee + 组装注入」即可。本节是工程中的常见变体；初学可先跳过。

### 配置在组装层注入

支付账号、客户 ID、币种等 **支付渠道级配置** 应在 `main` 或 `NewServiceFromConfig` 里传给 Adapter，不要写死在 Adapter 内部：

```go
func NewServiceFromConfig(cfg Config) (*Service, error) {
    var processor PaymentProcessor
    switch cfg.PaymentProvider {
    case "legacy_bank":
        processor = NewLegacyBankAdapter(LegacyBankClient{}, cfg.BankAccount)
    case "stripe":
        processor = NewStripeAdapter(StripeClient{}, cfg.StripeCustomerID)
    default:
        return nil, fmt.Errorf("unknown payment provider: %q", cfg.PaymentProvider)
    }
    return &Service{processor: processor}, nil
}
```

这与 [工厂方法 · 按配置注入产品](/cs-fundamentals/design-patterns/factory#按配置注入产品) 相同：**选型 + 构造** 留在组装层；`Service` 仍只调 `Pay`。

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

### 与外观的区别

| | 适配器 | 外观（Facade） |
| :--- | :--- | :--- |
| 目的 | 让 **现有** 接口符合 **客户端已定的** Target | **简化** 多个子系统，提供新的粗粒度入口 |
| 接口 | 必须实现 Client 已有的 Target（如 `PaymentProcessor`） | 常定义全新 API（如 `CheckoutOrder(req)`） |
| 典型场景 | Stripe SDK → `PaymentProcessor` | 支付 + 库存 + 发票 + 审计日志 → 一个 `CheckoutFacade`（见 [外观模式](/cs-fundamentals/design-patterns/facade)） |

一个类可以同时「像适配器又像外观」——判断时看 **动机**：是为 **兼容旧接口**，还是为 **降低子系统使用复杂度**。

### 与装饰器的区别

| | 适配器 | 装饰器（Decorator） |
| :--- | :--- | :--- |
| 接口 | Target 与 Adaptee **不同**，需要翻译 | 装饰器与组件 **同一接口** |
| 目的 | 让不能协作的类能协作 | 在不改原对象前提下 **增强行为**（重试、日志） |
| 例子 | `StripeAdapter` 把 `CreatePayment` 变成 `Pay` | `RetryPaymentProcessor` 包装任意 `PaymentProcessor` 并在 `Pay` 里重试 |

二者可 **叠加**：`Service` 注入 `RetryPaymentProcessor{inner: NewStripeAdapter(...)}`。

### 指针接收者与 Adaptee 生命周期

Adaptee 若是有状态客户端（连接池、HTTP Client），Adapter 应 **持有指针**，并在 `NewXxxAdapter` 里注入 **共享实例** 或 **每服务一个实例**，与工厂方法里产品的生命周期策略一致：

```go
type StripeAdapter struct {
    client *StripeClient // 共享连接
    customerID string
}
```

避免每个 `Pay` 临时 `new` 重量级 SDK 客户端。

## 小结

记住这四点即可：

1. **接口对不上、又改不了 Adaptee → 用适配器**：实现 Target，持有 Adaptee，在 Target 方法里翻译调用。
2. **Client 只依赖 Target**：`Service` 继续用 `PaymentProcessor.Pay`，不出现 `ChargeCard` / `CreatePayment`。
3. **与工厂方法组合**：组装层 `NewLegacyBankAdapter(...)` 注入 `Service`；选型与翻译分层清晰。
4. **Go 只有对象适配器**：struct 组合 Adaptee；别与外观、装饰器混淆——适配器解决 **接口兼容**，不是简化子系统或增强行为。

创建型模式解决 **怎么造对象**；结构型模式解决 **类与类如何拼在一起**。本篇是结构型的第一篇：**把现成的、接口不合的组件接进你的抽象里**。当接口已经能接上，但支付请求形态和支付后端都要独立扩展时，就进入下一篇 [桥接模式](/cs-fundamentals/design-patterns/bridge)。

## 参考阅读

- [x] [工厂方法模式](/cs-fundamentals/design-patterns/factory) — 电商订单系统前置，`PaymentProcessor` 与组装注入
- [x] [Refactoring.Guru - 适配器模式](https://refactoringguru.cn/design-patterns/adapter) (2026-06-18)
- [x] [菜鸟教程 - 适配器模式](https://www.runoob.com/design-pattern/adapter-pattern.html) (2026-06-18)
