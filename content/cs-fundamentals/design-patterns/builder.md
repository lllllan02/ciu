---
title: 生成器模式
order: 3
---

**生成器模式** 亦称 **建造者模式**（Builder），将一个复杂对象的构建与其表示分离，使得同样的构建过程可以创建不同的表示。

通俗地说，把一个复杂对象拆成一步步来填——想设哪项就设哪项，其余不必硬凑占位；哪些必填、默认值是什么、最后在哪里校验，都集中在构建流程里处理，调用处清晰可读。

## 问题

下单、发通知、拼 HTTP 请求——这类业务对象往往 **字段很多，且大半是可选的**：买家、商品、支付方式、优先级、定时发送…… 真正必填的通常只有其中几项。

遇到这种情况，大家第一反应往往是写一个 **参数很多的构造函数**，调用时一次把所有字段传进去。只关心买家、正文和支付方式时，调用大概长这样：

```go
order, err := NewOrder(
    []string{"user@example.com"}, // 买家
    nil,                          // 发票抬头
    "",                           // 标题
    "您的包裹正在途中…",              // 正文
    "alipay",                     // 支付方式
    0,                            // 优先级
    time.Time{},                  // 定时发送
    nil,                          // 附加商品
    nil,                          // 扩展字段
)
```

光从这一行调用，很难一眼看出哪个是买家、哪个是优先级；注释一多，反而更乱。字段再多一些，麻烦会一起冒出来：

1. **看不清在填什么**：参数全靠 **位置** 区分，调用处要对着函数签名数「第几个才是 Priority」。
2. **专用构造函数越写越多**：为了让「加急单」「定时单」好写一点，有人再加 `NewUrgentOrder()`、`NewScheduledOrder()`…… 可选字段一多，这种组合会 **越积越多**。
3. **校验到处重复**：买家、商品明细等必填项，要在每个 `NewXxx` 或每个调用方自己检查一遍，漏一处就出 bug。
4. **「怎么填」和「填完长什么样」搅在一起**：有时先定支付方式再填明细，有时从模板渲染再改标题——全塞进一个构造函数，顺序和分支都挤在一处，很难维护。
5. **半完成对象也能被用**：对象还没填齐字段就能传出去，编译器不会拦，运行时才发现缺数据。

本质矛盾是：**对象最终长什么样**（表示），和 **怎么一步步把它填好**（构建过程），被绑死在同一个构造函数里了。

## 解决方案

下面演示 **一种常见拆法**：`New()` 接收少量必填项并设默认值，其余字段链式追加。全部字段都走链式 setter，或改用 GoF 式生成器 + `Build()`，同样成立。

```go
type Order struct {
    buyer         []string
    invoiceTitle  []string
    orderTitle    string
    items         string
    paymentMethod string
    priority      int
    deliverAt     time.Time
    metadata      map[string]string
}

func NewOrder(buyer []string, items string) (*Order, error) {
    if len(buyer) == 0 {
        return nil, fmt.Errorf("order: buyer required")
    }
    if items == "" {
        return nil, fmt.Errorf("order: items required")
    }
    return &Order{
        buyer:         buyer,
        items:         items,
        paymentMethod: "alipay",
        priority:      0,
    }, nil
}

func (o *Order) OrderTitle(s string) *Order {
    o.orderTitle = s
    return o
}

// Priority、PaymentMethod、DeliverAt、InvoiceTitle 等同理，均 return o 以支持链式调用
```

### 使用者

本示例中 `NewOrder` 已在构造时校验必填项，业务可直接提交：

```go
func submitOrder(o *Order) error {
    return submit(o)
}

// 本示例：必填走 New，可选链式追加
order, err := NewOrder(
    []string{"user@example.com"},
    "您的包裹正在途中…",
)
if err != nil {
    return err
}
err = submitOrder(
    order.OrderTitle("订单已发货").Priority(1),
)
```

## 适用场景

1. **对象字段多、可选参数多**：HTTP 请求、支付宝/订单、报表配置、SQL 查询构建器等。
2. **构建步骤有顺序或依赖**：先设 `PaymentMethod` 再校验 `Items` 长度、先 `Render` 订单模板再填 `OrderTitle`——步骤留在 setter 方法里，比构造函数清晰。
3. **需要封装常见组合**：`OrderShipped(buyer, orderID)` 这类预设函数内部链式填字段，比为每种组合写构造函数省事。
4. **希望校验有固定落点**：在 `New()`、`Build()` 或提交前统一检查，避免每个调用方各自判空。

常见例子：电商结算里的 `Order` 构建、ORM 的 query builder、测试数据构造器、带 many optional 的配置 struct。

**不必强行使用**：字段少（三五个以内）、几乎无可选参数、构造一次定终身——直接 struct literal 或小型 `NewXxx(opts...)` 更简单。为两个字段的对象写十行链式 API 属于过度设计。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **可读性好** | 具名 setter 代替 positional 参数和满屏 `nil` |
| **校验可集中** | 必填、互斥规则可在 `New()`、`Build()` 等节点统一处理 |
| **易扩展字段** | 新增 `InvoiceTitle()` 等方法，旧调用不受影响 |
| **构建过程可复用** | 预设函数封装常见填字段流程 |
| **步骤清晰** | 链式调用比 positional 参数好读 |

| 缺点 | 说明 |
| :--- | :--- |
| **代码量变多** | 每个字段一个方法，比单个 struct 重 |
| **间接层增加** | 读代码要跟踪链式调用 |
| **与函数式选项重叠** | Go 社区更常选 Functional Options；团队需统一风格 |

## 实践

### 函数式选项

Go 里解决「多可选参数」的另一惯用法是 **Functional Options**（与上文链式写法 **二选一**，同包不必并存）：

```go
type Option func(*Order)

func WithBuyer(buyers ...string) Option {
    return func(n *Order) { n.buyer = append(n.buyer, buyers...) }
}

func WithItems(items string) Option {
    return func(n *Order) { n.items = items }
}

func NewOrderFromOptions(opts ...Option) (*Order, error) {
    n := &Order{paymentMethod: "alipay", priority: 0}
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
n, err := NewOrderFromOptions(
    WithBuyer("user@example.com"),
    WithItems("hello"),
)
```

与链式写法对比：

| | 链式 setter | 函数式选项 |
| :--- | :--- | :--- |
| 写法 | `NewOrder(buyer, items).OrderTitle(...)` | `NewOrderFromOptions(WithBuyer(...), ...)` |
| 步骤感 | 强，适合逐项填写 | 弱，更像一次性传入配置项 |
| Go 社区 | 常见 | **更常见**（`grpc.DialOption` 等） |
| 复用构建流程 | 预设函数内部链式调用 | 组合 `Option` 变量 |

两者都优于巨型构造函数。团队选一种并保持一致即可；**不必** 为同一类型同时维护两套 API。

### 预设组合

常见订单模板可以封装成函数，内部链式填字段：

```go
func OrderShipped(buyer, orderID string) (*Order, error) {
    o, err := NewOrder(
        []string{buyer},
        fmt.Sprintf("订单 %s 已发出。", orderID),
    )
    if err != nil {
        return nil, err
    }
    return o.OrderTitle("订单已发货"), nil
}
```


## 关联

- 许多设计在初期会先用 [工厂方法模式](/cs-fundamentals/design-patterns/factory)（较简单，也便于通过子类定制），随后再演化为 [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory)、[原型模式](/cs-fundamentals/design-patterns/prototype) 或 [生成器模式](/cs-fundamentals/design-patterns/builder)（更灵活，也更复杂）。
- [生成器模式](/cs-fundamentals/design-patterns/builder) 关注如何 **分步** 拼出一个复杂对象；[抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory) 则专门生产 **一系列相关对象**。抽象工厂拿到手就是成品，生成器则允许你在取回产品前再执行若干构造步骤（`Build()`、链式 setter 填完再提交等）。
- 构造复杂的 [组合模式](/cs-fundamentals/design-patterns/composite) 树时，生成器的构建步骤可以 **递归** 执行——父节点和子节点用同一套分步 API 逐层填好。
- 生成器可与 [桥接模式](/cs-fundamentals/design-patterns/bridge) 搭配：**指导者**（Director，封装固定构建流程的类或函数）负责抽象侧的组装步骤，不同 **生成器** 负责具体实现侧的填字段细节。
- [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory)、生成器与 [原型模式](/cs-fundamentals/design-patterns/prototype) 的实现类，都可以用 [单例模式](/cs-fundamentals/design-patterns/singleton) 来提供全局唯一的工厂或生成器实例。

## 参考阅读

- [x] [Refactoring.Guru - 生成器模式](https://refactoringguru.cn/design-patterns/builder) (2026-06-17)
- [x] [菜鸟教程 - 建造者模式](https://www.runoob.com/design-pattern/builder-pattern.html) (2026-06-17)
