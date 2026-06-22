---
title: 工厂方法模式
order: 1
---

**工厂方法模式** 提供一种创建对象的方式，使 **创建过程与使用过程分离**：业务代码只依赖统一接口来使用对象，不必自己 `new` 具体类型；造哪一种、怎么造，由工厂方法在程序 **启动时**（`main` 或组装层）决定。

在电商订单系统里，业务代码只关心调用统一的 `PaymentProcessor` 完成支付；用支付宝、微信支付还是信用卡，在部署/组装阶段定好，而不是每次 `Checkout` 都重新选择支付渠道。

## 问题

业务代码经常需要 **创建对象**，最直接的做法是在用到的地方直接构造具体类型：

```go
func checkout(method, order Order) {
    if method == "alipay" {
        processor := &AlipayProcessor{}
        processor.Pay(order)
    } else if method == "wechat_pay" {
        processor := &WeChatPayProcessor{}
        processor.Pay(order)
    } else if method == "credit_card" {
        processor := &CreditCardProcessor{}
        processor.Pay(order)
    }
}
```

这种写法在类型少、逻辑简单时还能应付，但对象种类一多，问题就会暴露：

1. **调用方与具体类紧耦合**：`checkout` 必须知道 `AlipayProcessor`、`WeChatPayProcessor` 等每一个实现类的名字，并直接依赖它们的构造方式。新增一种支付渠道，就要改所有创建对象的代码。
2. **创建逻辑分散、重复**：具体类型构造和 `if/else` 散落在各处，同一套「按类型选实现」的规则被复制多遍，维护成本高。
3. **违反开闭原则**（对扩展开放、对修改关闭——加新功能尽量写新代码，少改旧代码）：扩展新类型意味着修改已有分支，而不是只增加新代码；容易引入回归。
4. **不利于测试与替换**：单元测试时难以注入 **mock**（测试用的假实现）；运行时切换实现（例如从自研网关换成第三方支付 API）也要改调用方。

本质矛盾是：**使用方只关心「拿来用」**，却不得不同时处理 **「怎么造」** 的细节。

## 意图

用一句话说：**提供一种创建对象的方式，使创建过程与使用过程分离。**

业务只依赖统一接口（如 `PaymentProcessor`）并使用对象；具体造哪一种，在程序启动时（**组装** 阶段）由对应的工厂方法决定。

在 Go 里通常这样做：

- 每种产品提供各自的 **构造函数**（如 `NewAlipayProcessor()`），由它决定如何构造具体类型；
- **组装阶段**（`main` 或依赖注入工具里）选定调用哪个构造函数，把得到的 **产品** **注入**（从外部传进来，而不是在内部自己 `new`）业务代码；
- 业务代码只依赖 `PaymentProcessor` 等产品 **接口**（一组方法的约定；谁实现了这些方法，谁就算这种类型），不出现具体类型名和 `switch`；新增产品时 **新增构造函数**，在组装处换一行。

GoF（《设计模式》一书）从 **实现结构** 角度的定义是：

> 定义一个创建对象的接口，让子类决定实例化哪一个类；工厂方法把类的实例化延迟到子类。

这是经典 OOP 里的写法。Java/C++ 里常体现为 Creator 子类；**Go 没有继承**，用 `NewXxx()` 函数 + 注入达到同样效果。

> **命名说明**
>
> - **简单工厂**：一个函数 + `switch` 选类型——常见写法，**不是** GoF 23 种模式之一。
> - **工厂方法**（本文）：每种产品一个 `NewXxx()`，选型挪到组装层——GoF 意义上的工厂方法模式。

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
| **解耦** | 业务代码只依赖 `PaymentProcessor` 接口，不知道具体类型名 |
| **集中创建逻辑** | 每种产品的构造规则落在各自的 `NewXxx()` 里，不散落在各处 |
| **易扩展** | 加新产品 = 加一个 `NewXxx()`，不必改 `Service` 和已有构造函数 |
| **易测试** | 组装处可换成 mock 产品，或注入返回 mock 的构造函数 |

| 缺点 | 说明 |
| :--- | :--- |
| **构造函数变多** | 每种产品通常对应一个 `NewXxx()` |
| **间接层变多** | 读代码时要跳构造函数 → 产品，比直接 `new` 多一层 |
| **选型仍在某处发生** | `switch` 从业务里挪到了 `main` 或组装函数，并没有消失，只是换了个位置 |

## 组装实践

> **阅读提示**：先掌握上一节「在 `main` 里选 `NewXxx()` 并注入 `Service`」即可。本节是真实项目里的常见变体；若觉得信息量偏大，可先跳过，回头再读。

前文 `main` 里写死了 `NewAlipayProcessor()`。实际项目里，组装根常见两种写法：**注入已创建好的产品**，或 **注入构造函数、用时再创建**。选型逻辑（包括配置里的 `switch`）都应停在这一层，不进入 `Service` 的业务方法。

### 按配置注入产品

选型来自配置或环境变量时，组装函数里仍可能出现 `switch`——这很正常：

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
> 确实像——若只把 `NewPaymentProcessor(method)` 拆成多个 `NewXxx()`，组装处仍留一个 `switch`，**形式上**和简单工厂很接近。价值不在于「消灭 `switch`」，而在于 **`switch` 挪到了哪里、动的是哪份代码**：
>
> | | 简单工厂 | 工厂方法（本文） |
> | :--- | :--- | :--- |
> | `switch` 在哪 | `NewPaymentProcessor` 内部 | 组装函数 `NewServiceFromConfig` |
> | 何时执行 | 每次 `checkout(method, …)` 都可能走到 | 启动 / 组装时执行一次 |
> | 业务代码 | 仍要传 `method`，参与选型 | `Checkout(order)`，不再传 `method` |
> | 加 PayPal 渠道 | 改 `NewPaymentProcessor`，**动已有工厂函数** | 加 `NewPayPalProcessor()` + 组装处加一个 `case`，**不动** `Service` 和已有 `NewXxx()` |
>
> 开闭原则保护的是 **`Service` 和各产品的构造函数**——新增支付渠道不必改它们；组装处的 `switch` 仍要加一个分支，这是 **配置映射** 的成本，两种写法都逃不掉。
>
> 若各支付渠道构造参数不同（支付宝要 appID 和密钥、微信支付要商户号和 API Key），简单工厂的 `NewPaymentProcessor(method string)` 会迅速膨胀成一堆可选参数；拆成 `NewAlipayProcessor(appID, secret string)`、`NewWeChatPayProcessor(apiKey string)` 才划得来——这时「多个构造函数」不只是换皮，而是真的在分担各自的构造逻辑。
>
> 若连组装处的 `switch` 也想省掉，可以进一步用 **注册表**（`map[string]func() PaymentProcessor`，各包在 `init` 里注册），或 **按部署拆分**（支付宝版、微信支付版各自独立的 `main`，配置文件里写死支付渠道）。那是工厂方法之上的工程选择，不是模式本身的必选项。

### 注入构造函数

若每次调用都需要 **新的产品实例**（例如每次订单都要新建一个带请求 ID、超时设置的对象），组装时只注入构造函数，在业务方法里再创建。

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

按配置选型时，组装处选定构造函数再注入即可——`switch` 仍在组装层，逻辑与上一小节相同：

```go
func NewCheckoutServiceFromConfig(cfg Config) (*CheckoutService, error) {
    switch cfg.PaymentProcessorType {
    case "alipay":
        return &CheckoutService{newPaymentProcessor: NewAlipayProcessor}, nil
    case "wechat_pay":
        return &CheckoutService{newPaymentProcessor: NewWeChatPayProcessor}, nil
    default:
        return nil, fmt.Errorf("unknown processor: %q", cfg.PaymentProcessorType)
    }
}
```

### 两种注入方式对比

| | 注入产品（`Service`） | 注入构造函数（`CheckoutService`） |
| :--- | :--- | :--- |
| 组装处 | `processor: NewAlipayProcessor()` | `newPaymentProcessor: NewAlipayProcessor` |
| 何时创建 | 组装时一次 | 每次 `Checkout` 调用时 |
| 适用 | 产品无状态、可复用同一实例 | 每次需要新实例（带请求上下文等） |
| 更接近 GoF | Client 与工厂方法分离 | 业务方法内调工厂方法 |

两种写法都是合法的工厂方法，按产品是否有状态、是否每次需要新实例来选。

## 小结

记住这三点即可：

1. **业务不传 `method`**：`Service` 只调 `Checkout(order)`，不参与选型。
2. **每种产品一个 `NewXxx()`**：新增支付渠道 = 加产品 + 加构造函数，不必改 `Service`。
3. **`switch` 可以留在组装层**：从业务里挪走就有价值；不必强求「彻底消灭 `switch`」。

在电商订单系统这条主线上，工厂方法回答的是 **「这次部署要用哪个支付渠道实现」**。当问题从一个支付渠道扩展为「支付渠道、日志、订单模板必须成套出现」时，就进入下一篇 [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory)。

## 参考阅读

- [x] [Refactoring.Guru - 工厂方法模式](https://refactoringguru.cn/design-patterns/factory-method) (2026-06-17) — 与本文同名，Java 示例为主
- [x] [菜鸟教程 - 工厂模式](https://www.runoob.com/design-pattern/factory-pattern.html) (2026-06-17) — 标题写「工厂模式」，内容偏 **简单工厂**；读时注意与本文 **工厂方法** 区分
