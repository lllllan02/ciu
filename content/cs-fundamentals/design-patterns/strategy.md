---
title: 策略模式
order: 20
---

**策略模式**（Strategy）定义一系列的算法，把它们一个个封装起来，并且使它们可相互替换；策略模式使得算法可独立于使用它的客户而变化。

通俗地说，同一类计算或决策有多种算法可选，每种算法单独封装；需要换规则时换掉策略即可，使用方不必堆满分支判断。

## 问题

整单计价、运费、优惠券分摊会随 **渠道、会员等级、大促、B2B 合约** 切换不同规则。最直接的做法是在 `PricingEngine` 里写 **巨型 switch**——`if 直播间`、`if 金卡会员`、`if B2B 合约`……

规则少时还能应付；每加一种渠道或等级，问题就会一起暴露：

1. **难扩展**：加「直播间专享价」要改 Total 和 ShippingFee 多处 switch。
2. **规则重复**：报表、购物车预览、结算页 **复制同一套 if**。
3. **职责混杂**：`PricingEngine` 同时管标价、会员、大促、B2B、运费，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。
4. **难以单测**：测「金卡 88 折」必须构造带 live_flash 分支的 Engine。

本质矛盾是：**同一入口**（`Calculate(order)`）背后有 **多种互斥或择一的规则**，且规则随业务扩展；Context **不应** 认识每一种 if 分支。典型写法如下：

```go
func (e *PricingEngine) Total(ctx context.Context, order Order, user User, channel string) int64 {
    for _, line := range order.Lines {
        switch {
        case channel == "live_flash" && e.flashSKUs[line.SKU]:
            unit = e.flashPrice(line.SKU)
        case user.Tier == "gold":
            unit = int64(float64(line.UnitPrice) * 0.88)
        case order.B2BContractID != "":
            unit = e.contractPrice(order.B2BContractID, line.SKU)
        // …
        }
    }
}
```

## 解决方案

定义 **Strategy** 接口；**Context**（`PricingEngine`）持有策略并 **委托** 计算；各 **ConcreteStrategy** 封装一种规则；**Resolver** 按上下文 **选择** 策略。

### 策略与上下文

```go
type PricingContext struct {
    Order   Order
    User    User
    Channel string
}

type PricingStrategy interface {
    LineUnitPrice(ctx context.Context, pc PricingContext, line OrderLine) int64
    Name() string
}

type PricingEngine struct {
    pricing PricingStrategy
}

func NewPricingEngine(p PricingStrategy) *PricingEngine {
    return &PricingEngine{pricing: p}
}

func (e *PricingEngine) Total(ctx context.Context, pc PricingContext) int64 {
    var sum int64
    for _, line := range pc.Order.Lines {
        unit := e.pricing.LineUnitPrice(ctx, pc, line)
        sum += unit * int64(line.Quantity)
    }
    return sum
}
```

`CheckoutService`、报表、Mediator **只调** `engine.Total(pc)`，**无 channel switch**。

### 具体策略

```go
type StandardPricingStrategy struct{}

func (StandardPricingStrategy) LineUnitPrice(_ context.Context, _ PricingContext, line OrderLine) int64 {
    return line.UnitPrice
}

type MemberPricingStrategy struct {
    discount float64 // gold: 0.88
}

func (s MemberPricingStrategy) LineUnitPrice(_ context.Context, _ PricingContext, line OrderLine) int64 {
    return int64(float64(line.UnitPrice) * s.discount)
}
```

直播间大促、B2B 合约价、满减、运费等 **各一个 ConcreteStrategy**，套路相同。

### 选择与组装

```go
func ResolvePricingStrategy(user User, channel string, order Order) PricingStrategy {
    switch {
    case order.B2BContractID != "":
        return B2BPricingStrategy{catalog: contractCatalog}
    case channel == "live_flash":
        return FlashSalePricingStrategy{flashSKUs: flashTable}
    case user.Tier == "gold":
        return MemberPricingStrategy{discount: 0.88}
    default:
        return StandardPricingStrategy{}
    }
}

engine := NewPricingEngine(ResolvePricingStrategy(user, channel, order))
```

解析逻辑 **集中在 Resolver**；Engine **只算**。`ShippingStrategy`、函数式策略、注册表见 **实践** 一节。


## 适用场景

1. **多种互斥算法**：计价、运费、税、积分分摊、风控评分模型。
2. **运行时切换**：A/B 价、渠道差异、用户等级——**Resolver 换 Strategy**。
3. **消除条件分支**：同一 **Context** 方法 **不应** 堆 `if channel`.
4. **开闭扩展**：新大促 = **新 ConcreteStrategy** + Resolver 一条分支。
5. **可测**：`FlashSalePricingStrategy` **单测** 不碰其他渠道。

**不必强行使用**：

- **只有一种算法、永不变**——内联计算。
- **增强要层层叠加**——[装饰器](/cs-fundamentals/design-patterns/decorator)。
- **行为随订单状态自动变**——[状态](/cs-fundamentals/design-patterns/state)。
- **两维独立扩展（形态×后端）**——[桥接](/cs-fundamentals/design-patterns/bridge)。
- **择一处理者沿链传递**——[责任链](/cs-fundamentals/design-patterns/chain-of-responsibility)。

常见例子：排序算法、压缩格式、路由算法、支付风控模型、Go `io.Reader` 族、Java `Comparator`。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新算法 **新 Strategy** |
| **消除 switch** | Context **只委托** |
| **算法可复用** | 报表与结算 **同一 Engine** |
| **依赖倒置** | 高层依赖 **Strategy 接口** |
| **易单测** | 每个 Strategy **表驱动** |

| 缺点 | 说明 |
| :--- | :--- |
| **类/文件增多** | 简单规则 **可能过度** |
| **Client 要选策略** | Resolver 本身可能 **变复杂**——可配置化 |
| **策略间隐式依赖** | 行价 Strategy 与分摊 Strategy **需文档约定** |
| **与装饰边界** | 团队需约定 **何者 Strategy、何者 Decorator** |
| **运行时切换错配** | 注入错 Strategy **难编译期发现**——集成测覆盖 |

## 实践

### 函数式 Strategy（Go 惯用）

```go
type LinePricer func(ctx context.Context, pc PricingContext, line OrderLine) int64

func (f LinePricer) LineUnitPrice(ctx context.Context, pc PricingContext, line OrderLine) int64 {
    return f(ctx, pc, line)
}

// 匿名策略
gold := LinePricer(func(_ context.Context, _ PricingContext, line OrderLine) int64 {
    return int64(float64(line.UnitPrice) * 0.88)
})
```

**无状态、短规则** 用函数；**要注入 Catalog** 用 struct。

### 策略注册表（运营配置）

```go
var pricingRegistry = map[string]func() PricingStrategy{
    "standard": func() PricingStrategy { return StandardPricingStrategy{} },
    "flash":    func() PricingStrategy { return FlashSalePricingStrategy{/* from config */} },
}

func FromConfig(name string) PricingStrategy {
    if f, ok := pricingRegistry[name]; ok {
        return f()
    }
    return StandardPricingStrategy{}
}
```

运营改 **活动价** 改 **配置 + 注册**，不必发版 **改 switch**（仍要 **测试 Strategy**）。

## 关联

- [桥接模式](/cs-fundamentals/design-patterns/bridge)、[状态模式](/cs-fundamentals/design-patterns/state)、策略模式（以及在一定程度上 [适配器模式](/cs-fundamentals/design-patterns/adapter)）的接口结构很相似——都基于 [组合模式](/cs-fundamentals/design-patterns/composite) 式的委托，但各自要解决的问题不同。模式不仅是代码组织方式，也是与同伴讨论 **如何解题** 的共同语言。
- [命令模式](/cs-fundamentals/design-patterns/command) 和策略模式看上去很像，因为两者均能用某些行为来参数化对象。但是，它们的意图完全不同。
  - 使用命令模式，你可以将任何操作转换为对象，该对象中的操作参数则成为对象的成员变量。你可以延迟执行该操作、将其放入队列、记录操作历史或者向远程服务发送对象等。
  - 使用策略模式，你通常可以描述实现同一目标的不同方式，使你在同一个上下文类中切换不同的算法。
- [装饰模式](/cs-fundamentals/design-patterns/decorator) 可让你改变对象的外壳，策略模式则让你能够改变对象的内核。
- [模板方法模式](/cs-fundamentals/design-patterns/template-method) 基于继承机制：它允许你通过扩展子类中的部分内容来修改算法的某些步骤。同时，策略模式基于组合机制：你可以通过对相应行为提供不同的策略来替换对象的默认行为。模板方法模式将算法分解在类层次上，策略模式将算法分解在对象层次上。
- [状态模式](/cs-fundamentals/design-patterns/state) 是策略模式的扩展。两者都基于组合机制：它们都通过将部分工作委派给「帮手」对象来在运行时改变行为。还有一个相似之处在于——对客户端而言，它们都是透明的。
  - 策略模式会让各个策略对象相互完全独立，彼此之间没有任何联系。
  - 状态模式不会限制具体状态之间的依赖，并允许它们自行改变在不同状态间进行切换。

## 参考阅读

- [x] [Refactoring.Guru - 策略模式](https://refactoringguru.cn/design-patterns/strategy) (2026-06-22)
- [x] [菜鸟教程 - 策略模式](https://www.runoob.com/design-pattern/strategy-pattern.html) (2026-06-22)
