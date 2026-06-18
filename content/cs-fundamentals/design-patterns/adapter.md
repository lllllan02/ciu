---
title: 适配器模式
order: 6
---

**适配器模式**（Adapter）提供一种 **把现有接口转换成调用方期望接口** 的方式，使 **原本因接口不兼容而无法协作的组件可以一起工作**：业务代码仍依赖统一的目标接口（如 `Notifier`），第三方 SDK 或遗留系统的 API 不必改写，由适配器在中间做「翻译」。

打个比方：出国充电像「带转换插头」——墙上的插座（第三方 API）和设备的插头（你的 `Notifier`）规格不同；适配器不改变插座也不改设备，只负责把两种规格接起来。

下文延续 [工厂方法模式](/cs-fundamentals/design-patterns/factory) 的「通知模块」场景：组装层已选定 `Notifier` 接口，但具体渠道可能来自 **自研实现**、**遗留 SMTP 库** 或 **第三方短信 SDK**——后两者的方法签名与 `Notifier` 不一致，需要适配器。

## 问题

[工厂方法](/cs-fundamentals/design-patterns/factory) 让业务只依赖 `Notifier` 接口：

```go
type Notifier interface {
    Send(message string) error
}

type Service struct {
    notifier Notifier
}

func (s *Service) NotifyUser(message string) error {
    return s.notifier.Send(message)
}
```

实际接入渠道时，常遇到 **接口对不上** 的情况：

```go
// 遗留邮件库：三个参数，方法名也不同
type LegacySMTP struct{}

func (LegacySMTP) MailTo(addr, subject, body string) error {
    // 走旧 SMTP 协议…
    return nil
}

// 第三方短信 SDK：返回消息 ID，参数是手机号 + 正文
type TwilioClient struct{}

func (TwilioClient) CreateMessage(to, body string) (msgID string, err error) {
    // 调 Twilio HTTP API…
    return "SMxxx", nil
}
```

若直接在 `Service` 里分支调用，问题立刻暴露：

```go
func (s *Service) NotifyUser(message string) error {
    switch n := s.notifier.(type) {
    case LegacySMTP:
        return n.MailTo("user@example.com", "Notification", message)
    case TwilioClient:
        _, err := n.CreateMessage("+8613800138000", message)
        return err
    default:
        // …
    }
    return nil
}
```

1. **调用方被迫认识每一种第三方 API**：`Service` 要知道 `MailTo` 还是 `CreateMessage`，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)——业务逻辑与集成细节缠在一起。
2. **无法统一依赖抽象**：`Service` 的字段类型若写成 `interface{}` 或具体第三方类型，就 **不能** 再声明为 `Notifier`，测试注入 mock 也困难。
3. **修改第三方库代价高**：Twilio SDK 升级改方法名、遗留库不能动——改 `Service` 牵一发而动全身。
4. **违反开闭原则**：每接一个渠道，就要改 `Service` 的分支，而不是只加新代码。
5. **与工厂方法的分工被打乱**：工厂本应在组装层选好实现；选型之后仍要在业务里写 `switch`，等于 **创建解耦了、使用没解耦**。

本质矛盾是：**你期望的接口**（`Send(message)`）和 **现有组件的接口**（`MailTo` / `CreateMessage`）不一致，却又 **不能或不愿** 改第三方/遗留代码。

## 意图

用一句话说：**将一个类的接口转换成客户希望的另一个接口，使原本因接口不兼容而不能一起工作的类可以一起工作。**

在目标接口（`Notifier`）与 **被适配者**（Adaptee，如 `LegacySMTP`、`TwilioClient`）之间插入 **适配器**（Adapter）：适配器 **实现** 目标接口，**持有** 被适配者，在 `Send` 里把调用 **翻译** 成对方能懂的方法。GoF 从 **实现结构** 角度的定义是：

> 将一个类的接口转换成客户希望的另外一个接口。适配器模式使得原本由于接口不兼容而不能一起工作的类可以一起工作。

与 [工厂方法](/cs-fundamentals/design-patterns/factory) 的关系：

| | 工厂方法 | 适配器 |
| :--- | :--- | :--- |
| 解决什么 | **创建哪一种** 产品（解耦 `new`、选型） | **接口不匹配** 时让已有组件能当目标接口用 |
| 典型动机 | 渠道种类多、构造要集中 | 第三方 / 遗留 API 与内部接口不一致 |
| 典型入口 | `NewEmailNotifier()` | `NewLegacySMTPAdapter(smtp, addr)` |
| 是否改 Adaptee | — | **不改** 第三方或遗留代码 |

二者常 **组合**：工厂（或组装函数）决定注入哪个适配器；`Service` 始终只看到 `Notifier`。

> **命名说明**
>
> - **对象适配器**（本文，Go 唯一常用形态）：适配器 **组合**（has-a）持有 Adaptee，在目标方法里转发——Go 无多继承，一律用这种。
> - **类适配器**（GoF，Java/C++）：适配器 **继承** Target 又 **继承** Adaptee——Go **不支持**，了解即可。
> - **适配器 vs 外观（Facade）**：适配器 **改接口** 以符合现有客户端；外观 **简化** 一整组子系统的调用，接口可以仍是新的。见下文 [组装实践 · 与外观的区别](#与外观的区别)。

## 解决方案

为每种不兼容的 Adaptee 写一个适配器，实现 `Notifier`，在 `Send` 里完成参数映射与调用转发。

### 目标接口与被适配者

```go
type Notifier interface {
    Send(message string) error
}

type LegacySMTP struct{}

func (LegacySMTP) MailTo(addr, subject, body string) error {
    // …
    return nil
}

type TwilioClient struct{}

func (TwilioClient) CreateMessage(to, body string) (string, error) {
    // …
    return "SMxxx", nil
}
```

### 对象适配器

适配器 **持有** Adaptee 及组装时需要的配置（收件地址、手机号等），**实现** `Notifier`：

```go
type LegacySMTPAdapter struct {
    smtp    LegacySMTP
    address string
}

func NewLegacySMTPAdapter(smtp LegacySMTP, address string) Notifier {
    return &LegacySMTPAdapter{smtp: smtp, address: address}
}

func (a *LegacySMTPAdapter) Send(message string) error {
    return a.smtp.MailTo(a.address, "Notification", message)
}

type TwilioAdapter struct {
    client TwilioClient
    to     string
}

func NewTwilioAdapter(client TwilioClient, to string) Notifier {
    return &TwilioAdapter{client: client, to: to}
}

func (a *TwilioAdapter) Send(message string) error {
    _, err := a.client.CreateMessage(a.to, message)
    return err
}
```

### 使用者不变

`Service` **完全不知道** 背后是遗留 SMTP 还是 Twilio——与 [工厂方法](/cs-fundamentals/design-patterns/factory) 里注入 `EmailNotifier` 的写法一致：

```go
type Service struct {
    notifier Notifier
}

func (s *Service) NotifyUser(message string) error {
    return s.notifier.Send(message)
}

// 组装层
legacySvc := &Service{
    notifier: NewLegacySMTPAdapter(LegacySMTP{}, "user@example.com"),
}
smsSvc := &Service{
    notifier: NewTwilioAdapter(TwilioClient{}, "+8613800138000"),
}
```

新增渠道时：**加 Adaptee 的适配器 + 组装处换一行**，不必改 `Service`。

## 结构

| 角色 | 代码里是谁 | 管什么 |
| :--- | :--- | :--- |
| **目标**（Target） | `Notifier` | 客户端期望的接口 |
| **被适配者**（Adaptee） | `LegacySMTP`、`TwilioClient` | 已有、接口不兼容的组件 |
| **适配器**（Adapter） | `LegacySMTPAdapter` 等 | 实现 Target，持有 Adaptee，做调用翻译 |
| **客户端**（Client） | `Service` | 只依赖 Target，不知道 Adaptee |

```mermaid
flowchart LR
    C["Client\nService"] --> T["Target\nNotifier"]
    T --> A["Adapter\nLegacySMTPAdapter"]
    A --> AD["Adaptee\nLegacySMTP"]
    C -.->|"只看见 Target"| T
    A -.->|"翻译 Send → MailTo"| AD
```

**运行时** 调用链：

```go
svc.NotifyUser("hello")
// → notifier.Send("hello")          // Target
// → adapter.smtp.MailTo(...)        // 适配器内部翻译
```

与工厂方法组合时的 **组装阶段**：

```mermaid
flowchart LR
    M["main / 组装层"] --> N["NewLegacySMTPAdapter(...)"]
    N --> B["Notifier 实例"]
    B --> S["Service{notifier}"]
    S --> R["运行时 NotifyUser"]
```

### 和 GoF 术语的对应（选读）

| GoF 叫法 | 本文代码 | 一句话 |
| :--- | :--- | :--- |
| Target | `Notifier` | 客户端使用的目标接口 |
| Adaptee | `LegacySMTP` | 需要被「接上来」的现有类 |
| Adapter | `LegacySMTPAdapter` | 实现 Target，包装 Adaptee |
| Client | `Service` | 只依赖 Target 的调用方 |

Go 里 Adapter 几乎都是 **对象适配器**（struct 字段持有 Adaptee）；没有 Java 式 **类适配器**（多重继承同时 IS-A Target 又 IS-A Adaptee）。

## 适用场景

1. **接入第三方库**：SDK 方法名、参数、返回值与内部抽象不一致，且 **不能改 SDK 源码**。
2. **包装遗留系统**：老模块仍用旧 API（`MailTo`、`send_sms`），新代码已统一成 `Notifier` / `Repository` 等接口。
3. **复用现有类，但接口不符合 [依赖倒置](/cs-fundamentals/design-patterns#设计原则)**：希望高层只依赖 `Notifier`，低层却是具体第三方类型。
4. **测试与生产切换**：生产注入真实 SDK 的适配器，测试注入内存版 Adaptee + 同一套 Adapter（或 mock Adaptee）。
5. **渐进式迁移**：新功能走 `Notifier`；旧实现通过适配器挂接，日后替换 Adaptee 而不动 Client。

**不必强行使用**：

- Adaptee **本来** 就实现了 Target（或稍加修改即可）——直接实现接口，不必多包一层。
- 只是为了 **少写几行转发** 而套适配器——若 Adaptee 归你维护，改 Adaptee 实现 Target 往往更直接。
- 需要的是 **简化一整组复杂子系统** 的入口——那更像 [外观模式](#与外观的区别)，不是适配器。

常见例子：日志库适配统一 `Logger` 接口、ORM 适配仓储接口、HTTP 客户端适配内部 `Fetcher`、支付渠道 SDK 适配 `PaymentGateway`。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新渠道 = 新 Adapter，Client 与已有 Adapter 不必改 |
| **复用** | 第三方、遗留代码原样复用，无需 fork 或重写 |
| **单一职责** | 翻译逻辑集中在 Adapter，`Service` 保持业务纯净 |
| **可测试** | 对 Adaptee 注入 fake/stub，单独测 Adapter 的映射是否正确 |

| 缺点 | 说明 |
| :--- | :--- |
| **多一层间接** | 读代码需跳 Adapter → Adaptee，调试多一跳 |
| **类数量增加** | 每种 Adaptee 常对应一个 Adapter（及 `NewXxxAdapter`） |
| **翻译可能复杂** | 错误码、异步回调、流式 API 映射到同步 `Send` 时，Adapter 会变厚 |
| **掩盖设计问题** | 滥用适配器堆叠，可能推迟「该不该换掉遗留模块」的决策 |

## 组装实践

> **阅读提示**：先掌握「Adapter 实现 `Notifier` + 持有 Adaptee + 组装注入」即可。本节是工程中的常见变体；初学可先跳过。

### 配置在组装层注入

手机号、邮箱地址等 **渠道级配置** 应在 `main` 或 `NewServiceFromConfig` 里传给 Adapter，不要写死在 Adapter 内部：

```go
func NewServiceFromConfig(cfg Config) (*Service, error) {
    var notifier Notifier
    switch cfg.Channel {
    case "legacy_smtp":
        notifier = NewLegacySMTPAdapter(LegacySMTP{}, cfg.EmailTo)
    case "twilio":
        notifier = NewTwilioAdapter(TwilioClient{}, cfg.Phone)
    default:
        return nil, fmt.Errorf("unknown channel: %q", cfg.Channel)
    }
    return &Service{notifier: notifier}, nil
}
```

这与 [工厂方法 · 按配置注入产品](/cs-fundamentals/design-patterns/factory#按配置注入产品) 相同：**选型 + 构造** 留在组装层；`Service` 仍只调 `Send`。

### 适配器持有接口还是具体类型

Adaptee 若能抽象成小型接口，Adapter 依赖 **接口** 更利于测试：

```go
type MessageCreator interface {
    CreateMessage(to, body string) (string, error)
}

type TwilioAdapter struct {
    client MessageCreator
    to     string
}
```

生产注入 `TwilioClient{}`，测试注入 `fakeTwilio{}`。若 SDK 无法抽接口，只能持有具体 struct，测试时对 Adapter 做集成测试或包一层 thin wrapper。

### 错误与返回值映射

Adaptee 返回 `(msgID, err)`、枚举错误码或 panic 时，Adapter 负责 **归一化** 为 Target 的 `error`：

```go
func (a *TwilioAdapter) Send(message string) error {
    _, err := a.client.CreateMessage(a.to, message)
    if err != nil {
        return fmt.Errorf("twilio send: %w", err)
    }
    return nil
}
```

避免把 SDK 特有类型泄漏到 `Service`；必要时在 Adapter 层打日志、重试、指标打点。

### 与外观的区别

| | 适配器 | 外观（Facade） |
| :--- | :--- | :--- |
| 目的 | 让 **现有** 接口符合 **客户端已定的** Target | **简化** 多个子系统，提供新的粗粒度入口 |
| 接口 | 必须实现 Client 已有的 Target（如 `Notifier`） | 常定义全新 API（如 `SendNotification(req)`） |
| 典型场景 | Twilio SDK → `Notifier` | 邮件 + 短信 + 推送 + 审计日志 → 一个 `NotificationFacade` |

一个类可以同时「像适配器又像外观」——判断时看 **动机**：是为 **兼容旧接口**，还是为 **降低子系统使用复杂度**。

### 与装饰器的区别

| | 适配器 | 装饰器（Decorator） |
| :--- | :--- | :--- |
| 接口 | Target 与 Adaptee **不同**，需要翻译 | 装饰器与组件 **同一接口** |
| 目的 | 让不能协作的类能协作 | 在不改原对象前提下 **增强行为**（重试、日志） |
| 例子 | `TwilioAdapter` 把 `CreateMessage` 变成 `Send` | `RetryNotifier` 包装任意 `Notifier` 并在 `Send` 里重试 |

二者可 **叠加**：`Service` 注入 `RetryNotifier{inner: NewTwilioAdapter(...)}`。

### 指针接收者与 Adaptee 生命周期

Adaptee 若是有状态客户端（连接池、HTTP Client），Adapter 应 **持有指针**，并在 `NewXxxAdapter` 里注入 **共享实例** 或 **每服务一个实例**，与工厂方法里产品的生命周期策略一致：

```go
type TwilioAdapter struct {
    client *TwilioClient // 共享连接
    to     string
}
```

避免每个 `Send` 临时 `new` 重量级 SDK 客户端。

## 小结

记住这四点即可：

1. **接口对不上、又改不了 Adaptee → 用适配器**：实现 Target，持有 Adaptee，在 Target 方法里翻译调用。
2. **Client 只依赖 Target**：`Service` 继续用 `Notifier.Send`，不出现 `MailTo` / `CreateMessage`。
3. **与工厂方法组合**：组装层 `NewLegacySMTPAdapter(...)` 注入 `Service`；选型与翻译分层清晰。
4. **Go 只有对象适配器**：struct 组合 Adaptee；别与外观、装饰器混淆——适配器解决 **接口兼容**，不是简化子系统或增强行为。

创建型模式解决 **怎么造对象**；结构型模式解决 **类与类如何拼在一起**。本篇是结构型的第一篇：**把现成的、接口不合的组件接进你的抽象里**。下一篇可继续学习其他结构型模式（如桥接、装饰器、外观等）。

## 参考阅读

- [x] [工厂方法模式](/cs-fundamentals/design-patterns/factory) — 通知模块前置，`Notifier` 与组装注入
- [x] [Refactoring.Guru - 适配器模式](https://refactoringguru.cn/design-patterns/adapter) (2026-06-18)
- [x] [菜鸟教程 - 适配器模式](https://www.runoob.com/design-pattern/adapter-pattern.html) (2026-06-18)
