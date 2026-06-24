---
title: 策略模式
order: 20
---

**策略模式** 亦称 **Strategy**，定义一系列的算法，把它们一个个封装起来，并且使它们可相互替换；策略模式使得算法可独立于使用它的客户而变化。

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

定义 **PricingStrategy**、**ShippingStrategy**；**PricingEngine** 委托；各 **ConcreteStrategy** 实现一种规则；**Resolver**（或 [工厂方法](/cs-fundamentals/design-patterns/factory)）按上下文 **选择策略**。

### 策略（Strategy）接口

```go
type PricingContext struct {
    Order   Order
    User    User
    Channel string
}

type PricingStrategy interface {
    LineUnitPrice(ctx context.Context, pc PricingContext, line OrderLine) int64
    OrderDiscount(ctx context.Context, pc PricingContext, subtotal int64) int64
    Name() string
}

type ShippingStrategy interface {
    Fee(ctx context.Context, pc PricingContext, subtotal int64) int64
    Name() string
}
```

拆 **行单价** 与 **整单满减** 可在 **同一 Strategy** 内保持 **规则一致**（避免 live_flash 行价与满减 **分两处 switch**）。

### 具体策略：标准标价

```go
type StandardPricingStrategy struct{}

func (StandardPricingStrategy) Name() string { return "standard" }

func (StandardPricingStrategy) LineUnitPrice(_ context.Context, _ PricingContext, line OrderLine) int64 {
    return line.UnitPrice
}

func (StandardPricingStrategy) OrderDiscount(_ context.Context, _ PricingContext, _ int64) int64 {
    return 0
}
```

### 具体策略：会员价

```go
type MemberPricingStrategy struct {
    discount float64 // gold: 0.88
}

func (s MemberPricingStrategy) Name() string { return "member" }

func (s MemberPricingStrategy) LineUnitPrice(_ context.Context, _ PricingContext, line OrderLine) int64 {
    return int64(float64(line.UnitPrice) * s.discount)
}

func (s MemberPricingStrategy) OrderDiscount(context.Context, PricingContext, int64) int64 {
    return 0
}
```

### 具体策略：直播间大促

```go
type FlashSalePricingStrategy struct {
    flashSKUs  map[string]int64
    orderOff   int64
}

func (FlashSalePricingStrategy) Name() string { return "flash_sale" }

func (s FlashSalePricingStrategy) LineUnitPrice(_ context.Context, pc PricingContext, line OrderLine) int64 {
    if p, ok := s.flashSKUs[line.SKU]; ok {
        return p
    }
    return line.UnitPrice
}

func (s FlashSalePricingStrategy) OrderDiscount(_ context.Context, _ PricingContext, subtotal int64) int64 {
    if subtotal >= 10000 {
        return s.orderOff
    }
    return 0
}
```

### 具体策略：B2B 合约价 + 免运

```go
type B2BPricingStrategy struct {
    catalog ContractCatalog
}

func (B2BPricingStrategy) Name() string { return "b2b" }

func (s B2BPricingStrategy) LineUnitPrice(ctx context.Context, pc PricingContext, line OrderLine) int64 {
    return s.catalog.Price(ctx, pc.Order.B2BContractID, line.SKU)
}

func (B2BPricingStrategy) OrderDiscount(context.Context, PricingContext, int64) int64 { return 0 }

type B2BShippingStrategy struct{}

func (B2BShippingStrategy) Fee(context.Context, PricingContext, int64) int64 { return 0 }
func (B2BShippingStrategy) Name() string                                       { return "b2b_free" }
```

### 上下文（Context）——PricingEngine

```go
type PricingEngine struct {
    pricing  PricingStrategy
    shipping ShippingStrategy
}

func NewPricingEngine(pricing PricingStrategy, shipping ShippingStrategy) *PricingEngine {
    return &PricingEngine{pricing: pricing, shipping: shipping}
}

func (e *PricingEngine) SetPricingStrategy(s PricingStrategy)  { e.pricing = s }
func (e *PricingEngine) SetShippingStrategy(s ShippingStrategy) { e.shipping = s }

func (e *PricingEngine) Total(ctx context.Context, pc PricingContext) int64 {
    var sub int64
    for _, line := range pc.Order.Lines {
        unit := e.pricing.LineUnitPrice(ctx, pc, line)
        sub += unit * int64(line.Quantity)
    }
    off := e.pricing.OrderDiscount(ctx, pc, sub)
    if off > sub {
        off = sub
    }
    return sub - off
}

func (e *PricingEngine) ShippingFee(ctx context.Context, pc PricingContext) int64 {
    sub := e.Total(ctx, pc)
    return e.shipping.Fee(ctx, pc, sub)
}
```

**CheckoutService、Report、Mediator** 只调 **`engine.Total(pc)`**——**无 channel switch**。

### 策略解析（Client / 组装层）

```go
func ResolvePricingStrategy(user User, channel string, order Order) PricingStrategy {
    switch {
    case order.B2BContractID != "":
        return B2BPricingStrategy{catalog: contractCatalog}
    case channel == "live_flash":
        return FlashSalePricingStrategy{flashSKUs: flashTable, orderOff: 5000}
    case user.Tier == "gold" || user.Tier == "silver":
        return MemberPricingStrategy{discount: tierDiscount(user.Tier)}
    default:
        return StandardPricingStrategy{}
    }
}

func ResolveShippingStrategy(channel string, order Order) ShippingStrategy {
    if order.B2BContractID != "" || channel == "b2b" {
        return B2BShippingStrategy{}
    }
    if order.CrossBorder {
        return WeightBasedShippingStrategy{rates: crossBorderRates}
    }
    return ThresholdFreeShippingStrategy{threshold: 9900, baseFee: 800}
}
```

解析逻辑 **集中在 Resolver**；Engine **只算**——符合 [依赖倒置](/cs-fundamentals/design-patterns#设计原则)。


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

> **阅读提示**：先掌握「**Resolver 选 Strategy，Engine 委托 Calculate**」即可。本节是工程变体；初学可先跳过。

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

### 与装饰器、组合一起用

```text
PricingEngine.Total（Strategy：会员行价）
  → 对每行 line.Total() 若已套 Decorator（CouponLine、GiftWrapLine）
  → 或 Strategy 只算 base unit，Decorator 在行上调整

Composite 树（BundleLine）
  → Strategy 可按 SKU 询价；Bundle 的 Total() 内部递归
```

**约定**：**Strategy 管「基准价从哪来」**；**Decorator 管「行上可选叠加」**——避免 **同一折扣既在 Strategy 又在 CouponLine**。

### 与外观、中介者

```go
// Facade.PlaceOrder 内
pc := PricingContext{Order: req.Order, User: req.User, Channel: req.Channel}
engine := NewPricingEngine(
    ResolvePricingStrategy(req.User, req.Channel, req.Order),
    ResolveShippingStrategy(req.Channel, req.Order),
)
amount := engine.Total(ctx, pc)

// CheckoutMediator 刷新合计
m.pricing = engine
m.state.Total = engine.Total(ctx, m.buildPC())
```

[外观](/cs-fundamentals/design-patterns/facade) **用 Engine 算应付**；[中介者](/cs-fundamentals/design-patterns/mediator) **引用同一 Engine** 刷新 UI——**规则只在一处**。

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

### 策略 vs 责任链

| | 策略 | 责任链 |
| :--- | :--- | :--- |
| **语义** | **择一算法** 完成计算 | **多处理者** 依次处理请求 |
| **电商** | 选 B2B 计价 | 下单前风控链 |

「先过风控链，再用 MemberStrategy 计价」——**链在前，策略在后**。

### 测试策略

```go
func TestMemberPricingStrategy_Gold88(t *testing.T) {
    s := MemberPricingStrategy{discount: 0.88}
    pc := PricingContext{}
    line := OrderLine{UnitPrice: 10000, Quantity: 1}
    got := s.LineUnitPrice(context.Background(), pc, line)
    if got != 8800 {
        t.Fatal(got)
    }
}

func TestPricingEngine_UsesInjectedStrategy(t *testing.T) {
    stub := MemberPricingStrategy{discount: 0.5}
    e := NewPricingEngine(stub, B2BShippingStrategy{})
    pc := PricingContext{Order: Order{Lines: []OrderLine{{UnitPrice: 100, Quantity: 2}}}}
    if e.Total(context.Background(), pc) != 100 {
        t.Fatal()
    }
}
```

## 小结

记住这四点即可：

1. **算法即对象**：会员价、大促价、B2B 价各是一个 `PricingStrategy`。
2. **Context 只委托**：`PricingEngine.Total` **不认 channel**，只调 `strategy.LineUnitPrice`。
3. **Resolver 负责选择**：组装层 `ResolvePricingStrategy(user, channel, order)`——与 [状态](/cs-fundamentals/design-patterns/state) **自动迁移** 区分。
4. **与装饰器分层**：Strategy **整单/基准算法**；Decorator **行级可叠加**。

[装饰器模式](/cs-fundamentals/design-patterns/decorator) 解决了 **「同一行上多种增强如何套娃」**；策略模式解决了 **「整单计价/运费等算法族如何可替换」**——把 **条件分支** 收到 **可独立测试的策略类**，让渠道与会员规则在 [开闭](/cs-fundamentals/design-patterns#设计原则) 下扩展。

## 参考阅读

- [x] [Refactoring.Guru - 策略模式](https://refactoringguru.cn/design-patterns/strategy) (2026-06-22)
- [x] [菜鸟教程 - 策略模式](https://www.runoob.com/design-pattern/strategy-pattern.html) (2026-06-22)
