---
title: 抽象工厂模式
order: 2
---

**抽象工厂模式**（Abstract Factory）提供一个接口，用于创建相关或依赖对象的家族，而不需要指定具体类。

通俗地说，业务只需按统一接口使用「一整套」配套对象，不必逐个挑选、逐个创建族内成员，更不必操心成员之间是否匹配；整套配置在程序启动时一次配齐，再整体交给业务使用。

## 问题

[工厂方法模式](/cs-fundamentals/design-patterns/factory) 解决的是「创建 **一种** 产品」的解耦问题。实际项目里，很多对象不是单独用的，而是 **成套出现**，且组内成员必须风格一致——就像买家具：沙发、茶几、电视柜要同一套配色，不能混搭。

继续用订单结算举例。国内版和跨境版各自是一套完整流程：

- **国内版**：支付宝支付 + 审计日志 + 正式 HTML 订单模板
- **跨境版**：信用卡支付 + 埋点日志 + 短文案订单模板

业务代码真正需要的，是「按版本拿一整套来用」。最直接的做法，是在每个用到的地方分别判断、分别 `new` 三个具体类型。类型少时还能应付；一旦创建点变多，问题就会一起暴露：

1. **容易配错组合**：三个组件分散创建，改一处、漏一处，就可能出现「国内支付宝 + 跨境短模板」这类混搭，运行时才发现不兼容。
2. **调用方与多套具体类紧耦合**：业务代码必须认识每一族里的所有具体类型名，违反 [依赖倒置](/cs-fundamentals/design-patterns#设计原则)（依赖抽象，而非具体实现）。
3. **扩展与切换成本高**：新增「国内微信套装」要在多处同步改 `if/else`；从国内版切到跨境版，本应是「一键换族」，分散的 `new` 却变成全局搜索替换。

本质矛盾是：**使用方只关心「拿来用一整套」**，却不得不同时维护 **「族内有哪些产品、怎么配对」** 的细节。典型写法如下——结算、退款、对账等处往往各写一遍：

```go
if suite == "domestic" {
    processor = &AlipayProcessor{}
    logger = &AuditReceiptLogger{}
    invoiceTpl = &VATInvoiceTemplate{}
} else if suite == "cross_border" {
    processor = &CreditCardProcessor{}
    logger = &AnalyticsReceiptLogger{}
    invoiceTpl = &MobileReceiptTemplate{}
}
// 三行若来自不同文件、不同人维护，极易混搭不同族的产品
```

## 解决方案

把「一族产品」的创建收拢到一个 **套装** 里；每种套装在组装层一次性配好三个接口字段。业务只依赖套装里的抽象接口，不直接 `new` 具体类型。

### 产品接口

三个产品各自定义接口（与工厂方法篇相同，`PaymentProcessor` 可直接复用）：

```go
type PaymentProcessor interface {
    Pay(order Order)
}

type ReceiptLogger interface {
    Record(channel string, body string)
}

type InvoiceTemplate interface {
    Render(order Order) string
}
```

### 具体产品（两族示例）

**国内套装**：

```go
type AlipayProcessor struct{}
func (AlipayProcessor) Pay(order Order) { /* 调支付宝网关… */ }

type AuditReceiptLogger struct{}
func (AuditReceiptLogger) Record(channel, body string) { /* 写审计日志… */ }

type VATInvoiceTemplate struct{}
func (VATInvoiceTemplate) Render(order Order) string {
    return "<html><body>" + order.Summary + "</body></html>"
}
```

**跨境套装**：

```go
type CreditCardProcessor struct{}
func (CreditCardProcessor) Pay(order Order) { /* 调信用卡收单网关… */ }

type AnalyticsReceiptLogger struct{}
func (AnalyticsReceiptLogger) Record(channel, body string) { /* 埋点… */ }

type MobileReceiptTemplate struct{}
func (MobileReceiptTemplate) Render(order Order) string {
    if len(order.Summary) > 140 {
        return order.Summary[:137] + "..."
    }
    return order.Summary
}
```

### 套装（抽象工厂）

Go 里最常见、也最顺手的写法：**不必** 再包一层带 `CreateXxx()` 的 factory interface——直接把一族产品收成 **一个 struct，里面放三个产品接口**：

```go
type CheckoutSuite struct {
    PaymentProcessor PaymentProcessor
    ReceiptLogger   ReceiptLogger
    InvoiceTemplate InvoiceTemplate
}
```

每种套装用一个 **组装函数** 配好族内三个字段——配对写在一处，不会混搭：

```go
func NewDomesticCheckoutSuite() CheckoutSuite {
    return CheckoutSuite{
        PaymentProcessor: &AlipayProcessor{},
        ReceiptLogger:   &AuditReceiptLogger{},
        InvoiceTemplate: &VATInvoiceTemplate{},
    }
}

func NewCrossBorderCheckoutSuite() CheckoutSuite {
    return CheckoutSuite{
        PaymentProcessor: &CreditCardProcessor{},
        ReceiptLogger:   &AnalyticsReceiptLogger{},
        InvoiceTemplate: &MobileReceiptTemplate{},
    }
}
```

这就是 Go 里的「抽象工厂」：`CheckoutSuite` 是 **一族产品的抽象容器**，`NewDomesticCheckoutSuite()` 是 **具体工厂**（在组装层决定注入哪些实现）。

### 使用者

`Service` 在 **组装时** 注入整套 `CheckoutSuite`，运行时只使用其中的接口：

```go
type Service struct {
    suite CheckoutSuite
}

func NewService(suite CheckoutSuite) *Service {
    return &Service{suite: suite}
}

func (s *Service) Checkout(order Order) error {
    body := s.suite.InvoiceTemplate.Render(order)
    s.suite.PaymentProcessor.Pay(order)
    s.suite.ReceiptLogger.Record("checkout", body)
    return nil
}
```

**组装根** 决定用哪一套：

```go
domesticSvc := NewService(NewDomesticCheckoutSuite())
cross_borderSvc := NewService(NewCrossBorderCheckoutSuite())
```

## 适用场景

1. **产品必须成套、不能混搭**：UI 主题（按钮 + 对话框 + 滚动条）、数据库访问（连接 + 命令 + 事务）、跨平台渲染套件等。
2. **要在运行时切换整套实现**：国内版 / 跨境版、Windows / macOS 皮肤、MySQL / PostgreSQL 驱动族。
3. **想隐藏具体类的构造与组合**：调用方只知道「结算套装」，不知道族内每个类叫什么。
4. **扩展新套装而不改业务**：符合 [开闭原则](/cs-fundamentals/design-patterns#设计原则)——加 `NewCrossBorderCheckoutSuite()` 即可。

常见例子：跨平台 GUI 控件库、主题化的文档导出（PDF 字体 + 页眉 + 页脚一套）、云厂商 SDK 族（对象存储 + 队列 + 监控同一套认证方式）。

**不必强行使用**：只有一两个独立对象、不存在「族」或「配套」约束时，工厂方法或直接 `new` 更简单。为两三个毫无关联的类硬凑一个「抽象工厂」，只会增加间接层。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **保证族内一致** | 组装函数封装配对逻辑，避免混搭 |
| **切换套装简单** | 组装处换一个 `NewXxxSuite()`，业务与已有产品类不动 |
| **隔离具体类** | 使用者只依赖抽象接口，符合依赖倒置 |
| **易扩展新族** | 新增组装函数 + 具体产品，不必改 `Service`；Go 里不必为每族多写一个 struct |

| 缺点 | 说明 |
| :--- | :--- |
| **向套装加新产品种类成本高** | 若要在所有套装里加「第四类产品」，要改 `CheckoutSuite` 的字段及 **每一个** `NewXxxSuite()` |
| **间接层更深** | 比工厂方法多一层「工厂的组合」，阅读和调试成本更高 |
| **容易过度设计** | 产品之间本无配套关系时，抽象工厂是负担 |

最后一点值得单独说明：GoF 原文也指出，抽象工厂最大的麻烦是 **向套装体系新增一种产品类型**（例如给所有套装都加 `Metrics` 字段）——这比新增一个 **新套装** 更痛，因为要动 struct 和所有组装函数。若产品种类经常变、套装相对稳定，要权衡是否值得。

## 关联

- 许多设计在初期会先用 [工厂方法模式](/cs-fundamentals/design-patterns/factory)（较简单，也便于通过子类定制），随后再演化为 [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory)、[原型模式](/cs-fundamentals/design-patterns/prototype) 或 [生成器模式](/cs-fundamentals/design-patterns/builder)（更灵活，也更复杂）。
- [生成器模式](/cs-fundamentals/design-patterns/builder) 重点关注如何 **分步** 生成复杂对象；抽象工厂专门用于生产 **一系列相关对象**。抽象工厂会马上返回产品，生成器则允许你在获取产品前执行一些额外构造步骤。
- 抽象工厂模式通常基于一组工厂方法，但你也可以使用原型模式来生成这些产品。
- 当只需对客户端代码 **隐藏子系统创建对象的方式** 时，你可以用抽象工厂来代替 [外观模式](/cs-fundamentals/design-patterns/facade)。
- 你可以将抽象工厂和 [桥接模式](/cs-fundamentals/design-patterns/bridge) 搭配使用。若桥接定义的抽象只能与特定实现合作，抽象工厂可以封装这些配对关系，并对客户端隐藏其复杂性。
- 抽象工厂、生成器和原型都可以用 [单例模式](/cs-fundamentals/design-patterns/singleton) 来实现。

## 参考阅读

- [x] [Refactoring.Guru - 抽象工厂模式](https://refactoringguru.cn/design-patterns/abstract-factory) (2026-06-17)
- [x] [菜鸟教程 - 抽象工厂模式](https://www.runoob.com/design-pattern/abstract-factory-pattern.html) (2026-06-17)
