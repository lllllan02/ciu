---
title: 桥接模式
order: 7
---

**桥接模式** 将抽象部分与实现部分分离，使它们都可以独立地变化。

通俗地说，两个本来会各自扩展的维度拆开，再用组合接在一起；新增一种形态或换一种后端，都不必为每一种搭配单独写一个类。GoF 里的「抽象部分 / 实现部分」**不是**日常说的「抽象类 vs 具体类」——下文会专门说明。

## 问题

支付系统里往往有 **两个独立会变的维度**：支付后端（支付宝、微信、Stripe…）和支付形态（直接支付、分期、退款…）。最直接的做法是为 **每一种组合** 写一个类——`AlipayDirectProcessor`、`WeChatInstallmentProcessor`……

维度少时还能应付；两边都要扩展时，类数会 **相乘爆炸**：

1. **组合爆炸**：每加一种后端或一种形态，就要新增多个类，且大量复制扣款代码。
2. **两维绑死**：改分期逻辑要改每个后端的子类；换微信网关时，每种形态的类都要动。
3. **难以独立测试**：测「分期」绕不开某个后端，测「支付宝」又绕不开某种形态。
4. **职责混杂**：一个类同时管 **请求怎么组织** 和 **怎么调网关**，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。

本质矛盾是：**有两个独立变化的维度**，却用 **继承穷举每一种组合** 来表达。典型写法如下：

```go
// 后端 × 形态 → 类数相乘
type AlipayDirectProcessor struct{}
type AlipayInstallmentProcessor struct{}
type WeChatDirectProcessor struct{}
type WeChatInstallmentProcessor struct{}
// 再加 Stripe、退款… 继续乘
```

## 解决方案

两个维度拆开：**形态**（直接 / 分期 / 退款）管怎么组织请求，**后端**（支付宝 / 微信 / Stripe）管怎么调网关。组装层把两者配对后，业务层只依赖 `CheckoutSender`。

### 三个角色：别和「抽象类」混淆

GoF 的 **Abstraction / Implementor** 指 **两个独立变化的维度**，不是「抽象类 vs 具体类」。划分看 **委托方向**：形态侧持有 `PaymentBackend` 并调 `Charge`；后端侧被委托调网关；`main` 里配对两维的是 **Client（组装层）**，不是第三条变化维。

| GoF | 本文 | 类型 | 职责 |
| :--- | :--- | :--- | :--- |
| **Abstraction** | 控制维（形态） | `CheckoutSender` 及其实现 | 格式化 `request`，委托 `Charge` |
| **Implementor** | 平台维（后端） | `PaymentBackend` 及其实现 | 接收 `request`，调网关 |
| **Client** | 组装层 | `NewCheckoutStack` 等 | 形态 × 后端配对 |

**怎么认**：扩展时 **会调另一维** → 控制维（如新增分期仍调 `Charge`）；**只接收另一维输出** → 平台维（如新增 Stripe）；**只换配对** → 组装层。

3 种形态 × 3 种后端 = 9 种配对，只需 **3 + 3 = 6** 个类型。

### 平台维（Implementor）：支付后端

```go
type PaymentBackend interface {
    Charge(orderID, request string) error
}

type AlipayBackend struct {
    bank    LegacyBankClient // 可与适配器组合
    account string
}

func (b AlipayBackend) Charge(orderID, request string) error {
    return b.bank.ChargeCard(b.account, "CNY", parseAmount(request))
}

type WeChatPayBackend struct {
    appID  string
    client WeChatPayClient
}

func (b WeChatPayBackend) Charge(orderID, request string) error {
    return b.client.UnifiedOrder(b.appID, orderID, parseAmount(request))
}

type StripeBackend struct {
    customerID string
    client     StripeClient
}

func (b StripeBackend) Charge(orderID, request string) error {
    return b.client.ChargeCustomer(b.customerID, orderID, parseAmount(request))
}
```

### 控制维（Abstraction）：支付形态

三种形态 **共用 `CheckoutSender` 接口**；每种形态 **持有** 一个 `PaymentBackend`，在 `Submit` 里格式化后 **委托** `Charge`：

```go
type CheckoutInput struct {
    Amount  int64
    Periods int    // 分期用
    Reason  string // 退款用
}

type CheckoutSender interface {
    Submit(orderID string, in CheckoutInput) error
}

type CheckoutAPI struct {
    backend PaymentBackend
}

func NewCheckoutAPI(backend PaymentBackend) CheckoutSender {
    return CheckoutAPI{backend: backend}
}

func (api CheckoutAPI) Submit(orderID string, in CheckoutInput) error {
    request := fmt.Sprintf("amount=%d", in.Amount)
    return api.backend.Charge(orderID, request)
}

type InstallmentCheckoutAPI struct {
    backend PaymentBackend
    tmpl    string
}

func NewInstallmentCheckoutAPI(backend PaymentBackend, tmpl string) CheckoutSender {
    return InstallmentCheckoutAPI{backend: backend, tmpl: tmpl}
}

func (api InstallmentCheckoutAPI) Submit(orderID string, in CheckoutInput) error {
    request := buildInstallmentRequest(api.tmpl, orderID, in.Periods, in.Amount)
    return api.backend.Charge(orderID, request)
}

type RefundCheckoutAPI struct {
    backend PaymentBackend
}

func NewRefundCheckoutAPI(backend PaymentBackend) CheckoutSender {
    return RefundCheckoutAPI{backend: backend}
}

func (api RefundCheckoutAPI) Submit(orderID string, in CheckoutInput) error {
    request := buildRefundRequest(in.Reason, in.Amount)
    return api.backend.Charge(orderID, request)
}
```

### 组装层（Client）与业务层

`CheckoutService` 是 **Client**：只消费组装好的 `CheckoutSender`，不参与扩展形态或后端。配对发生在注入之前：

```go
type CheckoutService struct {
    sender CheckoutSender
}

func (svc *CheckoutService) Pay(orderID string, in CheckoutInput) error {
    return svc.sender.Submit(orderID, in)
}

// Client：形态 × 后端配对，再注入 CheckoutService
alipay := AlipayBackend{bank: legacyBank, account: "merchant-001"}
wechat := WeChatPayBackend{appID: "wx-app", client: wechatClient}
stripe := StripeBackend{customerID: "cus_xxx", client: stripeClient}

// 直接 × 支付宝 | 分期 × 微信 | 退款 × Stripe —— 不必为每种组合写新类
svcDirectAlipay      := &CheckoutService{sender: NewCheckoutAPI(alipay)}
svcInstallmentWechat := &CheckoutService{sender: NewInstallmentCheckoutAPI(wechat, "periods={{.Periods}}")}
svcRefundStripe      := &CheckoutService{sender: NewRefundCheckoutAPI(stripe)}

_ = svcDirectAlipay.Pay("order-001", CheckoutInput{Amount: 9900})
_ = svcInstallmentWechat.Pay("order-002", CheckoutInput{Amount: 9900, Periods: 3})
_ = svcRefundStripe.Pay("order-003", CheckoutInput{Amount: 9900, Reason: "用户取消"})
```

新增 **支付后端** → 只加 `PaymentBackend` 实现（平台维）；新增 **支付形态** → 只加一种 `CheckoutSender` 实现（控制维）；**换配对** → 只改组装层，不必写 `AlipayRefundProcessor`。


## 适用场景

1. **两个维度独立变化**：支付请求形态 × 支付后端、UI 控件 × 渲染后端（矢量/光栅）、业务 API × 存储引擎（MySQL/Redis）。
2. **想避免组合类爆炸**：若用继承要 `M×N` 个类，桥接后约 `M+N`。
3. **实现可能在运行时切换**：同一 `InstallmentCheckoutAPI` 换注入的 `PaymentBackend`（如 A/B 支付后端、故障转移）。
4. **控制维与平台维都应面向接口编程**：高层测形态逻辑时注入 `fakeBackend`；测支付宝网关时不碰分期包装。
5. **实现细节应对 Client 隐藏**：`CheckoutService` 只调 `Submit`，不知道背后是微信支付还是 Stripe。

**不必强行使用**：

- 只有一个变化维度、且不会增长——直接一个接口 + 几个实现（策略或简单多态）即可。
- 组合永远固定（例如 **只有** 支付宝直接支付）——桥接多一层 indirection，收益不大。
- 问题是 **已有第三方接口不合**——优先 [适配器](/cs-fundamentals/design-patterns/adapter)，不是桥接。
- 变化维度 **三个以上**——桥接通常只桥 **一对** 层次；更复杂的用组合多个桥或重新划边界。

常见例子：跨平台 GUI（Widget × Renderer）、 JDBC 式驱动桥（DAO × Driver）、日志（Appender 抽象 × Console/File 实现）、订单队列生产者（序列化格式 × Broker 客户端）。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新支付后端 / 新支付请求形态各加一类，不修改对方 |
| **单一职责** | 请求编排与支付执行分离，职责清晰 |
| **可测试** | `fakePaymentBackend` 单测抽象；支付后端单测不碰订单模板 |
| **运行时绑定** | 组装层或配置决定 `CheckoutAPI` 桥接哪个 `PaymentBackend` |
| **合成复用** | 用组合替代继承穷举，符合 [设计原则](/cs-fundamentals/design-patterns#设计原则) |

| 缺点 | 说明 |
| :--- | :--- |
| **间接层增加** | 读代码需跳 控制维 → 平台维（Abstraction → Implementor） |
| **设计 upfront 成本** | 要事先识别「哪两个维度该拆」；拆错维度后期仍痛苦 |
| **小项目显繁琐** | 固定组合少时，几个 PaymentProcessor 类比桥接更直观 |
| **接口设计要稳** | Implementor 接口过窄要频繁改；过宽则实现类臃肿 |

## 关联

- 桥接模式通常在开发前期就设计好，用来把程序拆成可独立演进的两部分；[适配器模式](/cs-fundamentals/design-patterns/adapter) 则多用于已有代码，让原本不兼容的类能一起工作。
- 桥接模式、[状态模式](/cs-fundamentals/design-patterns/state)、[策略模式](/cs-fundamentals/design-patterns/strategy) 的接口结构很相似——都基于 [组合模式](/cs-fundamentals/design-patterns/composite) 式的委托，但各自要解决的问题不同。模式不仅是代码组织方式，也是与同伴讨论 **如何解题** 的共同语言。
- 若桥接定义的控制维只能与特定平台维配合，可以用 [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory) 封装这些配对关系，并对 Client 隐藏复杂性。
- [生成器模式](/cs-fundamentals/design-patterns/builder) 可与桥接搭配：**指导者**（Director，封装固定构建流程）承担控制维角色，各 **生成器**（Builder）承担平台维的具体实现工作。

## 参考阅读

- [x] [Refactoring.Guru - 桥接模式](https://refactoringguru.cn/design-patterns/bridge) (2026-06-18)
- [x] [菜鸟教程 - 桥接模式](https://www.runoob.com/design-pattern/bridge-pattern.refund) (2026-06-18)
