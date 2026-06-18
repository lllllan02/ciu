---
title: 生成器模式
order: 3
---

**生成器模式**（GoF 称 **建造者模式**，Builder）提供一种 **分步骤构建复杂对象** 的方式，使 **构建过程与最终表示分离**：调用方按需设置各字段，最后统一 `Build()` 得到不可变或校验完备的产品；同一套构建步骤也可以产出不同配置的对象。

在电商订单系统里，每条 `Order` 有买家、商品列表、优惠券、支付方式、收货地址、发票抬头、预约配送时间、元数据等字段，且大多可选——可以分步骤逐项设置，最后 `Build()` 得到完整订单；不必在构造函数里一次性塞十几个参数，也不必为每种组合写一个 `NewXxx()`。

下文继续使用电商订单系统：支付渠道和套装已在组装层选好，本文关注 **单条订单如何一步步构建完整**，而不是选择哪种支付渠道或哪套产品族。

## 问题

业务里经常要构造 **字段多、可选参数多** 的对象。最直接的做法是 **巨型构造函数** 或 **参数列表很长的函数**：

```go
type Order struct {
    Buyer         []string
    InvoiceTitle  []string
    OrderTitle    string
    Items         string
    PaymentMethod string
    Priority      int
    DeliverAt     time.Time
    Items         []string
    Metadata      map[string]string
}

func NewOrder(
    buyer []string,
    invoiceTitle []string,
    orderTitle, items, paymentMethod string,
    priority int,
    deliverAt time.Time,
    items []string,
    metadata map[string]string,
) *Order {
    return &Order{ /* … */ }
}
```

调用方往往要传大量 `nil`、空字符串或零值占位：

```go
n := NewOrder(
    []string{"user@example.com"},
    nil, "", "", "alipay",
    0, time.Time{}, nil, nil,
)
```

字段一多，问题就会暴露：

1. **可读性差**：十几个 positional 参数，调用处看不出哪个是 `InvoiceTitle`、哪个是 `Priority`。
2. **组合爆炸**：为常见场景写 `NewUrgentOrder()`、`NewScheduledDeliveryOrder()` 等专用构造函数，数量随可选字段指数增长。
3. **校验分散**：必填项（如 `Buyer`、`Items`）要在每个 `NewXxx()` 或调用方重复检查，容易漏。
4. **构建过程与表示混在一起**：同一对象有时要「先设支付渠道再填订单明细」，有时要「从订单模板渲染再设买家」——全塞进一个构造函数，顺序和分支都挤在一起。
5. **部分构建不安全**：对象构造到一半就被拿去用（缺字段），编译期发现不了。

本质矛盾是：**对象最终长什么样**（表示）和 **怎么一步步填好各字段**（构建过程）绑死在同一个构造函数里。

## 意图

用一句话说：**将一个复杂对象的构建与它的表示分离，使同样的构建过程可以创建不同的表示。**

调用方通过 **生成器** 按步骤设置字段；**构建**（校验、默认值、不可变拷贝）集中在 `Build()`；业务拿到的是完整、合法的产品。GoF 从 **实现结构** 角度的定义是：

> 将一个复杂对象的构建与它的表示分离，使得同样的构建过程可以创建不同的表示。

与 [工厂方法模式](/cs-fundamentals/design-patterns/factory) 的关系：

| | 工厂方法 | 生成器 |
| :--- | :--- | :--- |
| 解决什么 | **创建哪一种** 产品（选型、解耦 `new`） | **如何一步步拼出** 一个复杂产品（多字段、多步骤） |
| 典型入口 | `NewAlipayProcessor()` | `NewOrderBuilder().Buyer(...).Items(...).Build()` |
| 产品复杂度 | 构造相对简单，类型种类多 | 往往 **一种** 产品，但字段多、可选组合多 |

二者可以 **组合**：工厂方法决定造哪种 `PaymentProcessor`，生成器负责拼出要提交的 `Order` 请求体。

> **命名说明**
>
> - **生成器 / 建造者**（本文，GoF Builder）：分步设置 + `Build()`，构建与表示分离。
> - **函数式选项**（Functional Options，Go 惯用法）：`NewOrder(WithBuyer(...), WithPriority(...))`——解决同类问题，见下文 [组装实践 · 函数式选项](#函数式选项)。
> - **简单工厂 / 工厂方法**：管「造哪类对象」，不管「对象里十几个字段怎么填」。

## 解决方案

把「填字段」拆成链式（或分步）方法，把「校验并产出最终对象」收进 `Build()`。

### 产品

订单——字段多，且部分必填：

```go
type Order struct {
    buyer         []string
    invoiceTitle  []string
    orderTitle    string
    items         string
    paymentMethod string
    priority      int
    deliverAt     time.Time
    items         []string
    metadata      map[string]string
}

// 只暴露 getter，避免 Build 之后被随意改字段（可选但推荐）
func (n *Order) Buyer() []string        { return n.buyer }
func (n *Order) Items() string       { return n.items }
func (n *Order) PaymentMethod() string    { return n.paymentMethod }
// …
```

### 生成器

生成器持有 **构建中的状态**；每个 setter 返回 `*OrderBuilder` 以支持链式调用：

```go
type OrderBuilder struct {
    buyer         []string
    invoiceTitle  []string
    orderTitle    string
    items         string
    paymentMethod string
    priority      int
    deliverAt     time.Time
    items         []string
    metadata      map[string]string
}

func NewOrderBuilder() *OrderBuilder {
    return &OrderBuilder{
        paymentMethod:  "alipay", // 合理默认值
        priority: 0,
    }
}

func (b *OrderBuilder) Buyer(buyers ...string) *OrderBuilder {
    b.buyer = append(b.buyer, buyers...)
    return b
}

func (b *OrderBuilder) InvoiceTitle(buyers ...string) *OrderBuilder {
    b.invoiceTitle = append(b.invoiceTitle, buyers...)
    return b
}

func (b *OrderBuilder) OrderTitle(s string) *OrderBuilder {
    b.orderTitle = s
    return b
}

func (b *OrderBuilder) Items(s string) *OrderBuilder {
    b.items = s
    return b
}

func (b *OrderBuilder) PaymentMethod(c string) *OrderBuilder {
    b.paymentMethod = c
    return b
}

func (b *OrderBuilder) Priority(p int) *OrderBuilder {
    b.priority = p
    return b
}

func (b *OrderBuilder) DeliverAt(t time.Time) *OrderBuilder {
    b.deliverAt = t
    return b
}

func (b *OrderBuilder) Item(skus ...string) *OrderBuilder {
    b.items = append(b.items, skus...)
    return b
}

func (b *OrderBuilder) Metadata(key, value string) *OrderBuilder {
    if b.metadata == nil {
        b.metadata = make(map[string]string)
    }
    b.metadata[key] = value
    return b
}

func (b *OrderBuilder) Build() (*Order, error) {
    if len(b.buyer) == 0 {
        return nil, fmt.Errorf("order: buyer required")
    }
    if b.items == "" {
        return nil, fmt.Errorf("order: items required")
    }
    // 拷贝切片 / map，避免外部修改构建器内部状态
    buyer := append([]string(nil), b.buyer...)
    invoiceTitle := append([]string(nil), b.invoiceTitle...)
    items := append([]string(nil), b.items...)
    meta := make(map[string]string, len(b.metadata))
    for k, v := range b.metadata {
        meta[k] = v
    }
    return &Order{
        buyer: buyer, invoiceTitle: invoiceTitle, orderTitle: b.orderTitle, items: b.items,
        paymentMethod: b.paymentMethod, priority: b.priority, deliverAt: b.deliverAt,
        items: items, metadata: meta,
    }, nil
}
```

### 使用者

业务代码 **只调生成器 + `Build()`**，不再碰巨型构造函数：

```go
func submitOrder(b *OrderBuilder) error {
    n, err := b.Build()
    if err != nil {
        return err
    }
    // 交给已注入的 PaymentProcessor 提交…
    return submit(n)
}

// 调用方：只写关心的字段
err := submitOrder(
    NewOrderBuilder().
        Buyer("user@example.com").
        OrderTitle("订单已发货").
        Items("您的包裹正在途中…").
        Priority(1),
)
```

与巨型构造函数对比：

| | 巨型构造函数 | 生成器 |
| :--- | :--- | :--- |
| 可读性 | 大量 `nil` 占位 | 具名方法，链式自解释 |
| 必填校验 | 分散在各处 | 集中在 `Build()` |
| 扩展字段 | 改签名，所有调用点受影响 | 加 setter，旧调用不动 |
| 部分构建 | 容易造出半完成对象 | 未 `Build()` 前不暴露产品 |

## 结构

| 角色 | 代码里是谁 | 管什么 |
| :--- | :--- | :--- |
| **产品** | `Order` | 最终要用的复杂对象 |
| **生成器** | `OrderBuilder` | 分步设置字段，持有构建中状态 |
| **指导者**（可选） | `OrderDirector` 等 | 封装 **固定构建顺序** 的预设流程 |
| **使用者** | `submitOrder` / `Service` | 调生成器或直接调指导者 |

构建与使用分两个阶段：

```mermaid
flowchart LR
    A["调用方\n链式 setter"] --> B["生成器\nOrderBuilder"]
    B --> C["Build()\n校验 + 组装"]
    C --> D["产品\nOrder"]
    D --> E["业务\n提交 / 持久化"]
```

**构建时**：逐步填字段，尚未得到合法产品：

```go
b := NewOrderBuilder().
    Buyer("a@example.com").
    Items("hello")
// 此时还不能提交——可能还缺校验
```

**`Build()` 后**：得到完整产品，或明确错误：

```go
n, err := b.OrderTitle("Hi").Build()
```

### 指导者（选读）

当多种场景共享 **同一套构建步骤、只是个别字段不同** 时，可抽 **指导者**（Director）封装流程，生成器只负责「怎么设字段」：

```go
type OrderDirector struct {
    builder *OrderBuilder
}

func NewOrderDirector(b *OrderBuilder) *OrderDirector {
    return &OrderDirector{builder: b}
}

func (d *OrderDirector) BuildOrderShipped(buyer, orderID string) (*Order, error) {
    return d.builder.
        Buyer(buyer).
        OrderTitle("订单已发货").
        Items(fmt.Sprintf("订单 %s 已发出，请注意查收。", orderID)).
        PaymentMethod("alipay").
        Priority(1).
        Build()
}

func (d *OrderDirector) BuildPromoPush(buyer, title, link string) (*Order, error) {
    return d.builder.
        Buyer(buyer).
        OrderTitle(title).
        Items(link).
        PaymentMethod("credit_card").
        Priority(0).
        Build()
}
```

Go 里指导者 **不是必选项**——几个包级函数 `BuildOrderShippedOrder(...)` 往往同样清晰。只有构建步骤长、且要在多处复用 **同一顺序** 时，再引入 Director struct。

### 和 GoF 术语的对应（选读）

| GoF 叫法 | 本文代码 | 一句话 |
| :--- | :--- | :--- |
| Product | `Order` | 复杂产品 |
| Builder | `OrderBuilder` | 分步设置 + `Build()` |
| ConcreteBuilder | 同上（Go 里通常只有一个） | 具体生成器 |
| Director | `OrderDirector`（可选） | 固定构建流程 |
| Client | 调用 `Build()` 的代码 | 发起构建的一方 |

## 适用场景

1. **对象字段多、可选参数多**：HTTP 请求、支付宝/订单、报表配置、SQL 查询构建器等。
2. **构建步骤有顺序或依赖**：先设 `PaymentMethod` 再校验 `Items` 长度、先 `Render` 订单模板再填 `OrderTitle`——步骤留在生成器方法里，比构造函数清晰。
3. **需要不同表示、同一构建过程**：同一套 setter 流程，`Build()` 里根据 `paymentMethod` 产出不同校验规则或不同内部表示。
4. **希望构建期与使用期分离**：未 `Build()` 的不算合法产品；`Build()` 内集中校验，符合 [单一职责](/cs-fundamentals/design-patterns#设计原则)。

常见例子：电商结算里的 `Order` 构建、ORM 的 query builder、测试数据构造器、带 many optional 的配置 struct。

**不必强行使用**：字段少（三五个以内）、几乎无可选参数、构造一次定终身——直接 struct literal 或小型 `NewXxx(opts...)` 更简单。为两个字段的对象写十行链式 API 属于过度设计。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **可读性好** | 具名 setter 代替 positional 参数和满屏 `nil` |
| **校验集中** | 必填、互斥字段在 `Build()` 一处处理 |
| **易扩展字段** | 新增 `InvoiceTitle()` 等方法，旧调用不受影响 |
| **构建过程可复用** | 指导者或预设函数封装常见流程 |
| **步骤清晰** | 构建中与构建后分离，减少半完成对象 |

| 缺点 | 说明 |
| :--- | :--- |
| **代码量变多** | 每个字段一个方法 + 一个 `Build()`，比单个 struct 重 |
| **间接层增加** | 读代码要跟踪链式调用 |
| **可变构建器需注意** | 若复用同一 `Builder` 实例多次 `Build()`，要防止状态污染（见下文） |
| **与函数式选项重叠** | Go 社区更常选 Functional Options；团队需统一风格 |

## 组装实践

> **阅读提示**：先掌握「`NewOrderBuilder()` + 链式 setter + `Build()`」即可。本节是 Go 项目里的常见变体；初学可先跳过。

### 函数式选项

Go 里解决「多可选参数」的另一惯用法是 **Functional Options**：

```go
type Option func(*Order)

func WithBuyer(buyers ...string) Option {
    return func(n *Order) { n.buyer = append(n.buyer, buyers...) }
}

func WithItems(items string) Option {
    return func(n *Order) { n.items = items }
}

func NewOrder(opts ...Option) (*Order, error) {
    n := &Order{paymentMethod: "alipay"}
    for _, opt := range opts {
        opt(n)
    }
    if len(n.buyer) == 0 {
        return nil, fmt.Errorf("order: buyer required")
    }
    if n.items == "" {
        return nil, fmt.Errorf("order: items required")
    }
    return n, nil
}

// 调用
n, err := NewOrder(
    WithBuyer("user@example.com"),
    WithItems("hello"),
)
```

与生成器对比：

| | 生成器（链式） | 函数式选项 |
| :--- | :--- | :--- |
| 写法 | `b.Buyer(...).Items(...).Build()` | `NewXxx(WithBuyer(...), WithItems(...))` |
| 状态 | 显式 `Builder` struct | 闭包改 `Order` |
| 步骤感 | 强，适合逐项填写订单字段 | 弱，更像一次性传入配置项 |
| Go 社区 | 常见，尤其库对外 API | **更常见**（`grpc.DialOption` 等） |
| 复用构建流程 | 指导者 / 同一 builder 实例 | 组合 `Option` 变量或 `Options()` 辅助函数 |

两者都优于巨型构造函数。团队选一种并保持一致即可；**不必** 为同一类型同时维护两套 API。

### 复用生成器实例

同一 `OrderBuilder` 若连续 `Build()` 两次，第二次会 **带着第一次的字段**（除非手动重置）。常见做法：

1. **每次 `NewOrderBuilder()` 新建**（最简单，推荐默认）。
2. **`Build()` 后返回新 builder**，或提供 `Reset()`。
3. **指导者每次注入新 builder**，不在 Director 内长期持有可变状态。

```go
func (d *OrderDirector) BuildOrderShipped(buyer, orderID string) (*Order, error) {
    return NewOrderBuilder(). // 每次新建，避免污染
        Buyer(buyer).
        OrderTitle("订单已发货").
        Items(fmt.Sprintf("订单 %s 已发出。", orderID)).
        Build()
}
```

### 与工厂方法组合

[工厂方法](/cs-fundamentals/design-patterns/factory) 在 **组装层** 选定 `PaymentProcessor`；生成器在 **运行时** 拼出每条订单——职责不同，常一起出现：

```go
type Service struct {
    processor PaymentProcessor // 组装时注入：造哪种支付渠道
}

func (s *Service) Checkout(builder *OrderBuilder) error {
    n, err := builder.Build()
    if err != nil {
        return err
    }
    return s.processor.Pay(n) // PaymentProcessor 只关心已构建好的 Order
}
```

选型（支付宝 / 微信支付 / 信用卡）留在 `main`；单条订单长什么样，由调用方或 Director 通过生成器决定。

### 不可变产品与 `Build()`

若希望 **产品一经构建就不可变**（便于并发、缓存）：

- 字段小写 + 只读 getter（上文 `Order`）。
- `Build()` 内做 **深拷贝**（切片、map）。
- 生成器本身可变无妨——变的是「草稿」，不是「定稿」。

这与 [开闭原则](/cs-fundamentals/design-patterns#设计原则) 不直接冲突，但有助于避免「发出去之后又被改内容」类 bug。

## 小结

记住这四点即可：

1. **字段多、可选多 → 考虑生成器**：用 setter 链代替巨型构造函数和满屏 `nil`。
2. **`Build()` 集中校验**：必填、互斥、默认值在构建终点处理，而不是散落在调用方。
3. **和工厂方法分工不同**：工厂管「造哪类对象」；生成器管「一个复杂对象怎么一步步拼好」。
4. **Go 里也可选函数式选项**：与链式生成器二选一，勿两套并行维护。

上一篇的 [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory) 管 **一族产品成套创建**；本篇管 **单个复杂产品分步构建**。对象种类多但构造简单 → 工厂；一种对象字段爆炸 → 生成器。若某些订单模板已经用生成器构建好，后续下单只是复制模板并替换少量字段，就进入下一篇 [原型模式](/cs-fundamentals/design-patterns/prototype)。

## 参考阅读

- [x] [工厂方法模式](/cs-fundamentals/design-patterns/factory) — 创建型模式前置，与本文可组合使用
- [x] [Refactoring.Guru - 生成器模式](https://refactoringguru.cn/design-patterns/builder) (2026-06-17)
- [x] [菜鸟教程 - 建造者模式](https://www.runoob.com/design-pattern/builder-pattern.html) (2026-06-17)
