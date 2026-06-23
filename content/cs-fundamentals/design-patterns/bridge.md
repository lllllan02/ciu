---
title: 桥接模式
order: 7
---

**桥接模式** 把 **抽象与实现分离**，使 **两个独立变化的维度各自扩展、互不相牵**：抽象通过 **组合** 引用实现接口，而不是用 **继承** 为每一种组合写一个类。

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

## 意图

用一句话说：**把「对外的用法」和「底层的干法」拆成两层，各自独立扩展，再用组合把它们接在一起。**

### 先别被名字吓到

GoF 把这两层叫 **抽象部分**（Abstraction）和 **实现部分**（Implementor）。这里的「抽象 / 实现」**不是** 编程语言里的 `interface`、`abstract class`，也 **不是** 日常说的「越抽象越 vague、说不清楚」——它只是 **模式里的两个角色名**。

可以先这样记：

| 模式里的叫法 | 实际是什么 | 打个比方 |
| :--- | :--- | :--- |
| **抽象部分** | 调用方直接用的那一层 | 收银台：负责「收什么款、怎么填单」 |
| **实现部分** | 真正去扣款、调网关的那一层 | 支付通道：负责「走支付宝还是微信、怎么发请求」 |

两层 **都会变**——以后可能加「退款入口」「PayPal 通道」——但 **不应该** 每加一种组合就写一个新类（上一节 `AlipayDirectPaymentProcessor`、`WeChatPayInstallmentPaymentProcessor` 那种）。

### 电商订单里谁管什么

继续用本文的订单支付例子，把两件事 **分开设计**：

**抽象部分（对调用方可见）**

- **管**：调用方怎么提交——直接支付、分期支付、退款；提交前要不要再套一层订单模板
- **不管**：最后走支付宝还是微信

例如：`CheckoutAPI` 负责普通提交，`InstallmentCheckoutAPI` 负责先渲染分期模板再提交。它们都是「提交订单」的不同入口，**不是**「越写越模糊的意思」。

**实现部分（底层真正干活）**

- **管**：调哪个支付网关、怎么扣款
- **不管**：调用方是普通提交还是分期提交

例如：`AlipayBackend`、`WeChatPayBackend` 各自对接银行网关、微信网关。

**怎么接在一起**

抽象部分 **组合持有** 实现部分——Go 里通常是 struct 里放 `PaymentBackend` 字段。一次分期支付的调用链大致是：

```
InstallmentCheckoutAPI 先渲染订单模板
    → 再调用 CheckoutAPI
    → 再调用 backend.Charge(...)
    → 底层支付宝 / 微信真正扣款
```

这样一来：

- 加 **退款入口** → 只动抽象部分（例如新增 `RefundCheckoutAPI`）
- 接 **PayPal** → 只动实现部分（新增 `PayPalBackend`）
- **不必** 再写 `AlipayRefundPaymentProcessor`、`WeChatPayRefundPaymentProcessor` 这类组合类

GoF 从 **结构** 角度的定义：

> 将抽象部分与它的实现部分分离，使它们都可以独立地变化。

### 和适配器模式有啥不同

两者结构上都像「A 持有 B」，但 **动机不同**：

| | 桥接 | 适配器 |
| :--- | :--- | :--- |
| **你在解决什么问题** | 设计时就知道有两个方向都会变，**主动拆开** | 已有接口对不上，**事后翻译成** 能用的接口 |
| **典型场景** | 「提交方式 × 支付后端」都会不断增加 | 第三方 SDK 的方法名、参数和自家 `PaymentProcessor` 不一致 |
| **会不会改第三方代码** | 实现层通常是你自己写的，随设计演进 | 往往 **不动** 第三方，只在外面包一层 |

两者还 **可以一起用**：`AlipayBackend` 内部用 [适配器](/cs-fundamentals/design-patterns/adapter) 包装 `LegacyBankClient`；`InstallmentCheckoutAPI` 在组装时桥接任意一个 `PaymentBackend`。

> **命名说明**
>
> - **桥接 vs 策略**：策略一般是「同一种事，换一种算法」；桥接是「整层实现（支付后端、渲染引擎）和整层用法（提交 API）分开，**两侧都可能各自长出一族类型**」。详见后文 [组装实践 · 与策略的区别](#与策略的区别)。
> - **桥接 vs 适配器**：长得像（都是 A 持有 B），区别在于 **你是为两个变化维度做设计（桥接），还是为兼容旧接口做翻译（适配器）**。

## 解决方案

下文按 **模式角色** 命名，避免和日常用语混淆：

| 角色 | 本文类型名 | 为何不用别的叫法 |
| :--- | :--- | :--- |
| 抽象部分 | `CheckoutAPI`、`InstallmentCheckoutAPI` | 强调 **调用方入口**，不是 `PaymentProcessor` 那种「听起来包办支付」的名字 |
| 实现部分 | `PaymentBackend`、`AlipayBackend` | 强调 **底层支付**，不叫 `PaymentChannel`——「Channel」在电商订单系统里通常指支付宝/微信支付等 **业务支付后端**，易和 Implementor 接口混淆 |

把 **底层怎么支付** 抽成 `PaymentBackend`（Implementor），把 **调用方怎么提交** 放在 `CheckoutAPI` 及其 refined 类型（Abstraction，Go 里用 **嵌入** 扩展，不是继承子类）；`Checkout` 处理完本层逻辑后，委托 `backend.Charge(...)`。

### 实现部分（Implementor）

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
    client WeChatPayClient
}

func (b WeChatPayBackend) Charge(orderID, request string) error {
    _, err := b.client.CreatePayment(orderID, parseAmount(request))
    return err
}

type StripeBackend struct {
    client StripeClient
}

func (b StripeBackend) Charge(orderID, request string) error {
    _, err := b.client.CreatePayment(orderID, parseAmount(request))
    return err
}
```

### 抽象部分（Abstraction）

```go
type CheckoutAPI struct {
    backend PaymentBackend
}

func NewCheckoutAPI(backend PaymentBackend) CheckoutAPI {
    return CheckoutAPI{backend: backend}
}

func (api CheckoutAPI) Checkout(orderID, request string) error {
    return api.backend.Charge(orderID, request)
}
```

### refined 抽象部分（Refined Abstraction）

```go
type InstallmentCheckoutAPI struct {
    CheckoutAPI
    installment string
}

func NewInstallmentCheckoutAPI(backend PaymentBackend, tmpl string) InstallmentCheckoutAPI {
    return InstallmentCheckoutAPI{
        CheckoutAPI:  NewCheckoutAPI(backend),
        installment: tmpl,
    }
}

func (api InstallmentCheckoutAPI) CheckoutWithVars(orderID string, vars map[string]string) error {
    request := buildInstallmentRequest(api.installment, vars)
    return api.CheckoutAPI.Checkout(orderID, request)
}

type RefundCheckoutAPI struct {
    CheckoutAPI
}

func NewRefundCheckoutAPI(backend PaymentBackend) RefundCheckoutAPI {
    return RefundCheckoutAPI{CheckoutAPI: NewCheckoutAPI(backend)}
}

func (api RefundCheckoutAPI) Refund(orderID, refundReason string) error {
    request := buildRefundRequest(refundReason)
    return api.CheckoutAPI.Checkout(orderID, request)
}
```

### 客户端与组装

```go
type CheckoutService struct {
    sender interface {
        Checkout(orderID, request string) error
    }
}

func (svc *CheckoutService) CheckoutDirect(orderID, request string) error {
    return svc.sender.Checkout(orderID, request)
}

// 组装层：任意「抽象 × 实现」组合，不必新建组合类
alipayDirect := NewCheckoutAPI(AlipayBackend{bank: LegacyBankClient{}, account: "merchant-001"})
wechatInstallment := NewInstallmentCheckoutAPI(WeChatPayBackend{client: WeChatPayClient{}}, "order={{.OrderID}}, periods={{.Periods}}")
stripeRefund := NewRefundCheckoutAPI(StripeBackend{client: StripeClient{}})

svc := &CheckoutService{sender: alipayDirect}
_ = wechatInstallment.CheckoutWithVars("order-001", map[string]string{"OrderID": "order-001", "Periods": "3"})
_ = stripeRefund.Refund("order-001", "用户取消订单")
```

新增 **支付后端** → 只加 `PaymentBackend` 实现；新增 **支付请求形态** → 只加 refined `CheckoutAPI`；**不必** 为每种组合写 `AlipayRefundPaymentProcessor`。


## 适用场景

1. **两个维度独立变化**：支付请求形态 × 支付后端、UI 控件 × 渲染后端（矢量/光栅）、业务 API × 存储引擎（MySQL/Redis）。
2. **想避免组合类爆炸**：若用继承要 `M×N` 个类，桥接后约 `M+N`。
3. **实现可能在运行时切换**：同一 `InstallmentCheckoutAPI` 换注入的 `PaymentBackend`（如 A/B 支付后端、故障转移）。
4. **抽象与实现都应面向接口编程**：高层测订单模板逻辑时注入 `fakeBackend`；低层测 银行网关 时不碰 refund 包装。
5. **实现细节应对客户端隐藏**：Client 只调 `Send` / `SendWithVars`，不知道 微信支付 还是 Stripe。

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
| **间接层增加** | 读代码需跳 Abstraction → Implementor |
| **设计 upfront 成本** | 要事先识别「哪两个维度该拆」；拆错维度后期仍痛苦 |
| **小项目显繁琐** | 固定组合少时，几个 PaymentProcessor 类比桥接更直观 |
| **接口设计要稳** | Implementor 接口过窄要频繁改；过宽则实现类臃肿 |

## 组装实践

> **阅读提示**：先掌握「Abstraction 持有 PaymentBackend + refined 类型扩展格式 + 组装层自由组合」即可。本节是工程变体；初学可先跳过。

### 在组装层完成「抽象 × 实现」绑定

与 [工厂方法 · 按配置注入产品](/cs-fundamentals/design-patterns/factory#按配置注入产品) 类似，**选型** 留在 `main`：

```go
func NewCheckoutStack(cfg Config) (*CheckoutService, error) {
    var backend PaymentBackend
    switch cfg.PaymentBackend {
    case "alipay":
        backend = AlipayBackend{bank: LegacyBankClient{}, account: cfg.MerchantAccount}
    case "wechat_pay":
        backend = WeChatPayBackend{client: WeChatPayClient{}}
    default:
        return nil, fmt.Errorf("unknown payment backend: %q", cfg.PaymentBackend)
    }

    var sender CheckoutAPI
    switch cfg.RequestKind {
    case "direct":
        sender = NewCheckoutAPI(backend)
    case "installment":
        return &CheckoutService{
            sender: NewInstallmentCheckoutAPI(backend, cfg.InstallmentTemplate),
        }, nil
    default:
        return nil, fmt.Errorf("unknown request kind: %q", cfg.RequestKind)
    }
    return &CheckoutService{sender: sender}, nil
}
```

两个 `switch` 在 **组装层相乘**，而不是在业务方法里；新增维度只扩展对应分支。

### 与适配器叠加

Implementor 内部可包装遗留 SDK——桥接管 **维度拆分**，适配器管 **接口翻译**：

```go
type AlipayBackend struct {
    adapter PaymentBackend // LegacyBankAdapter 等，已实现 Charge 或再包一层
}

func (b AlipayBackend) Charge(orderID, request string) error {
    return b.adapter.Charge(orderID, request) // 账户等在 Adapter 组装时注入
}
```

### 与策略的区别

| | 桥接 | 策略 |
| :--- | :--- | :--- |
| 意图 | **抽象层 + 实现层** 两套层次都可扩展 | 替换 **一种行为/算法** |
| 结构 | Abstraction **HAS-A** Implementor，常有 refined 抽象 | Context **HAS-A** Strategy |
| 例子 | `InstallmentCheckoutAPI` × `WeChatPayBackend` | 排序算法、压缩算法二选一 |
| 判断 | 两侧是否都会 **成族地** 增加类型/实现 | 是否只是 **换一种做法** |

若只有「选哪种支付后端」、支付请求形态不变，[工厂方法](/cs-fundamentals/design-patterns/factory) + 单一 `PaymentProcessor` 足够；若支付请求形态与支付后端 **都会成族扩展**，用桥接。

### 与装饰器的区别

| | 桥接 | 装饰器 |
| :--- | :--- | :--- |
| 目的 | 拆 **两个变化维度** | **增强** 同一接口上的行为（重试、日志） |
| 关系 | 抽象 **拥有** 实现 | 装饰器 **包装** 同接口组件 |
| 叠加 | `RetryBackend` 包装 `PaymentBackend` 仍属实现层装饰 | `RetryPaymentProcessor` 包装 `PaymentProcessor` |

可在 Implementor 外包装饰：`CheckoutAPI{backend: RetryBackend{inner: WeChatPayBackend{...}}}`。

### Implementor 接口粒度

`Charge(orderID, request string)` 保持 **最小可用**；支付后端特有参数（商户号、币种、网关地址）在 **具体 Implementor 构造时** 注入，不要泄漏到 `InstallmentCheckoutAPI`：

```go
func NewAlipayBackend(bank LegacyBankClient, merchantAccount string) AlipayBackend {
    return AlipayBackend{bank: bank, account: merchantAccount}
}
```

若多种 Implementor 共享重试、指标，可抽 **装饰器** 或 **中间抽象**（`BaseBackend`），而不是把横切逻辑写进每个 `Charge`。

### 指针、生命周期与共享 backend

重量级客户端（HTTP、连接池）在组装层 **共享一个** `WeChatPayBackend` 实例，注入多个 `CheckoutAPI` / `InstallmentCheckoutAPI`——与 [适配器 · 指针接收者与 Adaptee 生命周期](/cs-fundamentals/design-patterns/adapter#指针接收者与-adaptee-生命周期) 相同：

```go
sharedWeChatPay := WeChatPayBackend{client: weChatPayClient}
direct := NewCheckoutAPI(sharedWeChatPay)
tmpl := NewInstallmentCheckoutAPI(sharedWeChatPay, cfg.AlertTemplate)
```

## 小结

记住这四点即可：

1. **两个独立变化维度 → 桥接**：抽象（支付请求形态）与实现（支付后端）分开，用组合连接，避免 `M×N` 子类。
2. **Abstraction 持有 Implementor**：`CheckoutAPI` 调 `PaymentBackend.Charge`，refined 类型只管格式化。
3. **组装层做笛卡尔积**：`NewInstallmentCheckoutAPI(WeChatPayBackend{...}, tmpl)`，不必写 `WeChatPayInstallmentPaymentProcessor`。
4. **别与适配器、策略混淆**：桥接是 **设计上的分层**；适配器是 **集成时的翻译**；策略是 **单一行为替换**。

[适配器模式](/cs-fundamentals/design-patterns/adapter) 把 **现成的、接口不合** 的组件接进抽象；桥接则在 **设计之初** 就把 **会独立演化的两层** 拆开。放回电商订单系统这条主线：适配器解决「外部支付后端怎么接进来」，桥接解决「支付请求形态与支付后端如何避免组合爆炸」。当订单明细从扁平 SKU 长成嵌套套餐、礼盒时，下一篇 [组合模式](/cs-fundamentals/design-patterns/composite) 解决「如何让结算与库存对整棵明细树使用同一套接口」。

## 参考阅读

- [x] [工厂方法模式](/cs-fundamentals/design-patterns/factory) — 电商订单系统与组装注入
- [x] [适配器模式](/cs-fundamentals/design-patterns/adapter) — 支付后端集成与 `PaymentProcessor` 翻译
- [x] [Refactoring.Guru - 桥接模式](https://refactoringguru.cn/design-patterns/bridge) (2026-06-18)
- [x] [菜鸟教程 - 桥接模式](https://www.runoob.com/design-pattern/bridge-pattern.refund) (2026-06-18)
