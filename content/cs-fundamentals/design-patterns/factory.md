---
title: 工厂方法模式
order: 1
---

**工厂方法模式**（Factory Method）亦称 **虚拟构造器**，定义一个用于创建对象的接口，让子类决定实例化哪一个类；工厂方法使一个类的实例化延迟到其子类。

通俗地说，使用者只管「拿来用」，不必关心对象是哪里创建的、具体是哪一种实现；具体选哪个、怎么造，在程序启动或组装阶段一次性定好，再交给业务代码。

## 问题

业务代码经常需要 **根据不同情况，选用不同的处理方式**。以订单结算为例：用户选了支付宝、微信还是信用卡，程序就得走各自对应的那套支付流程。最直接的做法，是在每一处需要支付的地方 **自己判断、自己创建**——先看用户选了哪种方式，再当场创建对应的支付组件来用。

类型少、逻辑简单时，这样写还能应付。一旦支付渠道、退款通道、对账适配器等 **创建点变多**，问题就会一起暴露：

1. **调用方与具体实现紧耦合**：结算、退款、对账等模块都要认识支付宝、微信、信用卡等每一种支付方式，并知道各自怎么创建。新增一种渠道，就要改所有相关代码。
2. **创建逻辑分散、重复**：同一套「按类型选实现」的 `if/else` 和构造语句散落在多处，规则被复制多遍，改一处容易漏另一处。
3. **违反开闭原则**：扩展新类型意味着修改已有分支，而不是只增加新代码；容易引入回归。
4. **不利于测试与替换**：单元测试时难以注入 **mock**（测试用的假实现）；运行时从自研网关切换为第三方 API，也要改调用方。

本质矛盾是：**使用方只关心「拿来用」**，却不得不同时处理 **「怎么造、造哪一种」** 的细节。典型写法如下——结算、退款、对账等处往往各写一遍：

```go
if method == "alipay" {
    processor = &AlipayProcessor{}
} else if method == "wechat_pay" {
    processor = &WeChatPayProcessor{}
} else if method == "credit_card" {
    processor = &CreditCardProcessor{}
}
```

## 解决方案

沿用电商订单系统的例子：先说明 **简单工厂** 及其局限，再给出 **工厂方法** 的完整解法。

### 前置：简单工厂

把 `if/else` 和构造逻辑收进一个工厂函数，是迈向工厂方法的第一步：

```go
// 产品接口：只要实现了 Pay，就算是一种 PaymentProcessor
type PaymentProcessor interface {
    Pay(order Order)
}

type AlipayProcessor struct{}

func (AlipayProcessor) Pay(order Order) {
    // 调用支付宝通道…
}

type WeChatPayProcessor struct{}

func (WeChatPayProcessor) Pay(order Order) {
    // 调用微信支付通道…
}

type CreditCardProcessor struct{}

func (CreditCardProcessor) Pay(order Order) {
    // 调用信用卡通道…
}

func NewAlipayProcessor() PaymentProcessor { return &AlipayProcessor{} }
func NewWeChatPayProcessor() PaymentProcessor { return &WeChatPayProcessor{} }
func NewCreditCardProcessor() PaymentProcessor { return &CreditCardProcessor{} }

func NewPaymentProcessor(method string) (PaymentProcessor, error) {
    switch method {
    case "alipay":
        return NewAlipayProcessor(), nil
    case "wechat_pay":
        return NewWeChatPayProcessor(), nil
    case "credit_card":
        return NewCreditCardProcessor(), nil
    default:
        return nil, fmt.Errorf("unknown method: %s", method)
    }
}

func checkout(method, order Order) error {
    processor, err := NewPaymentProcessor(method)
    if err != nil {
        return err
    }
    processor.Pay(order)
    return nil
}
```

这解决了调用方直接 `new` 的问题，但 **新增支付渠道仍要改 `NewPaymentProcessor` 的 `switch`**，工厂本身不满足 [开闭原则](/cs-fundamentals/design-patterns#设计原则)。工厂方法要做的，就是把这段 `switch` 从业务路径里拆掉，并把选型挪到 **组装层**（程序启动、`main` 里把各模块拼在一起的那一层）。

### 工厂方法

**核心变化**：每种支付渠道对应一个 **构造函数**（如 `NewAlipayProcessor()`），在 **组装时** 调用一次，把得到的 `PaymentProcessor` 注入 `Service`。运行时 `Service` 只使用产品，不再传 `method`。

```go
type Service struct {
    processor PaymentProcessor // 组装时注入已创建好的产品
}

func (s *Service) Checkout(order Order) error {
    s.processor.Pay(order)
    return nil
}
```

**组装根**（程序入口，通常是 `main`）决定支付渠道——构造函数在这里完成创建，选型发生在启动阶段：

```go
alipaySvc := &Service{processor: NewAlipayProcessor()}
wechatPaySvc := &Service{processor: NewWeChatPayProcessor()}
creditCardSvc := &Service{processor: NewCreditCardProcessor()}
```

与简单工厂对比：

| | 简单工厂 | 工厂方法 |
| :--- | :--- | :--- |
| 谁决定支付渠道 | 每次调用传 `method`，`NewPaymentProcessor` 内 `switch` | `main` 选用哪个 `NewXxx()` |
| 调用签名 | `checkout(method, order)` | `Checkout(order)` |
| 新增 PayPal 渠道 | 改 `NewPaymentProcessor`，加 `case "paypal"` | 只加 `NewPayPalProcessor()`，组装处换一行 |

新增支付渠道时，工厂方法只需扩展，不必修改已有代码：

```go
type PayPalProcessor struct{}

func (PayPalProcessor) Pay(order Order) { /* 调用 PayPal 通道… */ }

func NewPayPalProcessor() PaymentProcessor { return &PayPalProcessor{} }

paypalSvc := &Service{processor: NewPayPalProcessor()}
```


## 适用场景

以下几类情况适合用工厂方法，而不是在业务里直接 `new` 或堆 `switch`：

1. **实现会随环境变化**：例如日志写本地文件还是远程服务、支付走自研网关还是第三方支付 API、数据库驱动随部署切换——具体类型在启动时确定，运行时不变。
2. **需要扩展而不改旧代码**：新增一种支付渠道或后端，只加「产品 + `NewXxx()`」，符合 [开闭原则](/cs-fundamentals/design-patterns#设计原则)。
3. **创建过程有一定复杂度**：构造时需要读配置、建立连接、注入依赖，值得从业务逻辑里拆出去。
4. **测试要替换实现**：组装阶段注入 mock 的 `PaymentProcessor`，`Service` 本身不用改。

常见例子：多后端日志、可切换的数据库访问层、支持多种协议（HTTP / gRPC / WebSocket）的客户端框架。

**不必强行使用**：对象构造非常简单（无参数、无分支）、类型几乎不变、团队规模很小——直接 `&AlipayProcessor{}` 往往更清晰。多个 `NewXxx()` 构造函数在简单场景下也属于过度设计。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **解耦** | 业务代码只依赖统一抽象（如「能支付」），不必知道支付宝、微信等具体实现 |
| **单一职责** | 「怎么创建」与「怎么使用」分开：每种实现各自管构造，创建代码集中在一处，不再与结算、退款等业务逻辑搅在一起 |
| **开闭原则** | 新增一种支付方式时，只需增加新的创建逻辑，不必改动已有业务模块和构造函数 |
| **易测试** | 组装处可换成测试用的假实现，业务代码本身不用改 |

| 缺点 | 说明 |
| :--- | :--- |
| **样板代码增多** | 每种实现通常多一个创建函数及配套类型，产品越多，要维护的代码面越大 |
| **间接层加深** | 追代码时要经组装层、创建函数才能抵达具体实现，比直接创建多跳几层 |
| **分支只是搬家** | 「选哪一种再创建」的逻辑从业务模块挪到了组装阶段，并没有真正消失 |
| **简单场景过重** | 种类少、创建又简单时，多出的抽象层比直接创建更绕，往往得不偿失 |

## 实践

前文 `main` 里直接写死 `NewAlipayProcessor()`，只是为了把模式讲清楚。落到真实项目，组装层至少还要回答两个问题：

1. **支付渠道从哪来？** 不同部署环境用的支付方式往往写在配置文件或环境变量里，启动时读入再决定——「选哪一种再创建」的逻辑并不会消失，该放在哪、长什么样？
2. **创建一次，还是每次调用都新建？** 无状态的支付组件组装时创建一次、全程复用即可；若每次订单都要带请求 ID、超时设置等新实例，又该注入什么？

下面分别看这两种常见情况。**选型逻辑应停在组装层**，不要重新漏回业务模块。

### 按配置注入产品

**问题**：支付渠道来自配置，不能在 `main` 里写死某一种。

**做法**：在组装函数里读取配置、判断类型、创建对应组件，再注入业务模块。这里仍可能出现 `switch`，但只出现在组装层，业务代码不参与选型：

```go
func NewServiceFromConfig(cfg Config) (*Service, error) {
    var processor PaymentProcessor
    switch cfg.PaymentProcessorType {
    case "alipay":
        processor = NewAlipayProcessor()
    case "wechat_pay":
        processor = NewWeChatPayProcessor()
    case "credit_card":
        processor = NewCreditCardProcessor()
    default:
        return nil, fmt.Errorf("unknown processor: %q", cfg.PaymentProcessorType)
    }
    return &Service{processor: processor}, nil
}

// main
svc, err := NewServiceFromConfig(loadConfig())
```

> [!tip]- 看到这里很容易问：**这不还是 `switch` 吗？和简单工厂有什么区别？**
>
> 确实像。区别不在「有没有分支」，而在 **分支在哪、加新渠道时要改哪份代码**：
>
> - **简单工厂**：分支在工厂函数里，业务每次调用都可能参与选型；加渠道要改共用的工厂。
> - **工厂方法**：分支留在组装层，启动时定一次；加渠道只增创建逻辑和组装处分支，**业务模块和已有创建逻辑不用动**。

### 注入构造函数

**问题**：每次业务调用都需要 **新的组件实例**（例如每次订单要带不同的请求 ID、超时设置），组装时只创建一次就不够。

**做法**：组装时不注入已创建好的组件，而是注入 **创建函数**，在业务方法里用时再创建。

Go 里没有继承，可以 **注入 `func() PaymentProcessor`、在业务方法里调用**：

```go
type CheckoutService struct {
    newPaymentProcessor func() PaymentProcessor // 注入构造函数，而非已创建好的产品
}

func (s *CheckoutService) Checkout(order Order) error {
    processor := s.newPaymentProcessor() // 业务方法里创建
    processor.Pay(order)
    return nil
}

// 组装：传函数值，不要加括号
svc := &CheckoutService{newPaymentProcessor: NewAlipayProcessor}
```

| | 注入产品（`Service`） | 注入构造函数（`CheckoutService`） |
| :--- | :--- | :--- |
| 组装处 | `processor: NewAlipayProcessor()` | `newPaymentProcessor: NewAlipayProcessor` |
| 何时创建 | 组装时一次 | 每次 `Checkout` 调用时 |
| 适用 | 产品无状态、可复用同一实例 | 每次需要新实例（带请求上下文等） |
| 更接近 GoF | Client 与工厂方法分离 | 业务方法内调工厂方法 |

两种写法都是合法的工厂方法，按产品是否有状态、是否每次需要新实例来选。

## 关联

- 许多设计在初期会先用工厂方法模式（较简单，也便于通过子类定制），随后再演化为 [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory)、[原型模式](/cs-fundamentals/design-patterns/prototype) 或 [生成器模式](/cs-fundamentals/design-patterns/builder)（更灵活，也更复杂）。
- [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory) 通常基于一组工厂方法，但你也可以使用 [原型模式](/cs-fundamentals/design-patterns/prototype) 来生成这些产品。
- 你可以同时使用工厂方法和 [迭代器模式](/cs-fundamentals/design-patterns/iterator)，让子类集合返回不同类型的迭代器，并使得迭代器与集合相匹配。
- 原型并不基于继承，因此没有继承的缺点；另一方面，原型需要对被复制对象进行复杂的初始化。工厂方法基于继承，但不需要这些初始化步骤。
- 工厂方法是 [模板方法模式](/cs-fundamentals/design-patterns/template-method) 的一种特殊形式；同时，工厂方法也可以作为一个大型模板方法中的一个步骤。

## 参考阅读

- [x] [Refactoring.Guru - 工厂方法模式](https://refactoringguru.cn/design-patterns/factory-method) (2026-06-17)
- [x] [菜鸟教程 - 工厂模式](https://www.runoob.com/design-pattern/factory-pattern.html) (2026-06-17)
