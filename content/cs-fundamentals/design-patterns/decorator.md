---
title: 装饰器模式
order: 9
---

**装饰器模式** 动态地给一个对象添加一些额外的职责；就增加功能来说，装饰器模式比生成子类更为灵活。

通俗地说，给对象叠加额外能力时，一层层包上去即可，每层只多干一件事；需要哪些增强、叠几层，运行时按需组合，不必为每一种组合预先写子类。

## 问题

同一行明细上，可能要叠加多种 **可选、可组合** 的增强：会员价、优惠券、礼品包装、运费险…… 最直接的做法是用 **子类穷举** 每一种组合——`MemberProductLine`、`MemberCouponProductLine`、`MemberCouponGiftWrapProductLine`……

增强项少时还能应付；每多一种可选能力，组合数就 **指数增长**：

1. **子类爆炸**：N 种增强 → 最多 2^N 个子类，加「运费险」又要新增一批组合类。
2. **职责混杂**：基础明细既要算单价，又要管折扣、包装费，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。
3. **运行时难灵活拆装**：大促只想开优惠券、关礼品包装——继承树在编译期就定死了，配置改不了。
4. **测试困难**：想单测「优惠券怎么改 `Total()`」，得构造带会员、礼品包装的巨型子类。

本质矛盾是：**同一接口上的多种增强** 需要 **运行时按需叠加**，却用 **静态继承** 来表达。典型写法如下：

```go
func buildLine(sku string, member, coupon, giftWrap bool) OrderLine {
    switch {
    case member && coupon && giftWrap:
        return MemberCouponGiftWrapProductLine{SKU: sku}
    case member && coupon:
        return MemberCouponProductLine{SKU: sku}
    // … 2^N 种组合
    default:
        return ProductLine{SKU: sku}
    }
}
```

## 解决方案

定义与 [组合模式](/cs-fundamentals/design-patterns/composite) 相同的 **组件** 接口 `OrderLine`；**具体组件** 实现基础逻辑；**装饰器** 持有 `inner OrderLine` 并实现同一接口，在方法里 **先委托、再增强**。

### 组件（Component）

```go
type OrderLine interface {
    Total() int64
    Validate() error
    ReserveInventory() error
}
```

`CheckoutService` 只依赖这一接口——与组合模式一致。

### 具体组件（Concrete Component）

```go
type ProductLine struct {
    SKU       string
    Name      string
    Quantity  int
    UnitPrice int64
}

func (p ProductLine) Total() int64 {
    return p.UnitPrice * int64(p.Quantity)
}

func (p ProductLine) Validate() error {
    if p.SKU == "" {
        return fmt.Errorf("product line: missing sku")
    }
    if p.Quantity <= 0 {
        return fmt.Errorf("product line %s: invalid quantity", p.SKU)
    }
    return nil
}

func (p ProductLine) ReserveInventory() error {
    return inventory.Reserve(p.SKU, p.Quantity)
}
```

### 装饰器（Decorator）

Go 无抽象装饰器基类，常用 **嵌入 `inner OrderLine` 的 struct** 表达「这是包装层」；各具体装饰器实现自己的增强逻辑：

```go
type MemberDiscountLine struct {
    Inner      OrderLine
    DiscountBP int // 万分比，例如 9500 = 95 折
}

func (d MemberDiscountLine) Total() int64 {
    base := d.Inner.Total()
    return base * int64(d.DiscountBP) / 10000
}

func (d MemberDiscountLine) Validate() error {
    return d.Inner.Validate()
}

func (d MemberDiscountLine) ReserveInventory() error {
    return d.Inner.ReserveInventory()
}

type CouponLine struct {
    Inner OrderLine
    Off   int64 // 固定减免，单位：分
}

func (c CouponLine) Total() int64 {
    total := c.Inner.Total()
    if total <= c.Off {
        return 0
    }
    return total - c.Off
}

func (c CouponLine) Validate() error {
    return c.Inner.Validate()
}

func (c CouponLine) ReserveInventory() error {
    return c.Inner.ReserveInventory()
}

type GiftWrapLine struct {
    Inner OrderLine
    Fee   int64
}

func (g GiftWrapLine) Total() int64 {
    return g.Inner.Total() + g.Fee
}

func (g GiftWrapLine) Validate() error {
    return g.Inner.Validate()
}

func (g GiftWrapLine) ReserveInventory() error {
    return g.Inner.ReserveInventory()
}
```

未增强的方法 **原样转发** 给 `inner`——装饰器只改它关心的那一两个方法（这里是 `Total()`），库存仍由最内层 `ProductLine` 决定。

### 客户端与组装

```go
type CheckoutService struct {
    lines []OrderLine
}

func (svc *CheckoutService) Checkout() error {
    var amount int64
    for _, line := range svc.lines {
        if err := line.Validate(); err != nil {
            return err
        }
        if err := line.ReserveInventory(); err != nil {
            return err
        }
        amount += line.Total()
    }
    return charge(amount)
}
```

组装层按配置 **套装饰器**，不必为每种组合建类：

```go
func buildTeaLine(cfg LineConfig) OrderLine {
    line := OrderLine(ProductLine{
        SKU: "tea-001", Name: "春茶", Quantity: 1, UnitPrice: 8800,
    })
    if cfg.MemberDiscountBP > 0 {
        line = MemberDiscountLine{Inner: line, DiscountBP: cfg.MemberDiscountBP}
    }
    if cfg.CouponOff > 0 {
        line = CouponLine{Inner: line, Off: cfg.CouponOff}
    }
    if cfg.GiftWrap {
        line = GiftWrapLine{Inner: line, Fee: 500}
    }
    return line
}

// 会员 95 折 + 满 10 减 10 + 礼品盒 5 元 → 8800*0.95 - 1000 + 500 = 7860
line := buildTeaLine(LineConfig{MemberDiscountBP: 9500, CouponOff: 1000, GiftWrap: true})
svc := &CheckoutService{lines: []OrderLine{line}}
_ = svc.Checkout()
```

新增 **积分抵扣** → 只加 `PointsLine` 装饰器，`CheckoutService` **不必改**。


## 适用场景

1. **多种可选增强可任意组合**：会员价、优惠券、包装费、税费、行级日志——用装饰器比 `2^N` 子类更可控。
2. **不能或不宜改原类**：`ProductLine` 已稳定、多处使用，新促销规则用外层装饰添加。
3. **增强可运行时拆装**：根据订单上下文在组装层决定是否 `GiftWrapLine`。
4. **符合开闭**：新增强 = 新装饰器类，不改已有 `ProductLine` 与其他装饰器。
5. **横切关注点**：支付 `PaymentProcessor` 的重试、指标、审计日志——装饰 `Pay` 前后逻辑。

**不必强行使用**：

- 增强 **固定且只有一种**（永远会员 95 折）——直接在 `ProductLine` 或组装时算一次更简单。
- 装饰层之间 **强依赖复杂状态机**（要读全局购物车才能算满减）——可能更适合 **领域服务** 或 **策略对象**，而不是一层层 `Total()` 嵌套。
- 结构是 **树形部分-整体**——用 [组合](/cs-fundamentals/design-patterns/composite)，不是装饰器。
- 两个 **独立变化维度成族扩展**——用 [桥接](/cs-fundamentals/design-patterns/bridge)。
- 接口 **根本不一致**——用 [适配器](/cs-fundamentals/design-patterns/adapter)。

常见例子：`io.Reader` / `io.Writer` 包装链（`bufio`、`gzip`、`encrypt`）、HTTP 中间件、`java.io` 流装饰、带缓存的 Repository 包装。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新增强 = 新装饰器，少改 `ProductLine` 与 Client |
| **单一职责** | 会员折扣在 `MemberDiscountLine`，SKU 计价在 `ProductLine` |
| **运行时组合** | 组装层按配置套娃，无组合子类爆炸 |
| **符合合成复用** | 用 has-a 包装替代 is-a 继承堆叠 |
| **可叠加** | 多层装饰仍对外是一个 `OrderLine` |

| 缺点 | 说明 |
| :--- | :--- |
| **顺序敏感** | 折扣与满减叠加顺序影响金额，需明确约定 |
| **调试链较长** | `Total()` 要跳进多层 inner，栈深时读代码费力 |
| **小对象过多** | 每层装饰一个 struct，极高频路径要注意分配（可用函数式或合并装饰缓解） |
| **接口变宽时痛苦** | `OrderLine` 每加一个方法，所有装饰器都要转发——与组合相同，接口胖时考虑拆分 |
| **过度设计** | 只有一层固定加价时，单独字段比装饰器直接 |

## 组装实践

> **阅读提示**：先掌握「`OrderLine` + 持有 `inner` 的装饰器 + 组装层套娃」即可。本节是工程变体；初学可先跳过。

### 与组合叠加

套餐子行可以是「带装饰的单品」：

```go
giftBox := BundleLine{
    BundleID: "gift-2026",
    Children: []OrderLine{
        GiftWrapLine{
            Fee: 500,
            Inner: CouponLine{
                Off: 800,
                Inner: ProductLine{SKU: "tea-001", Quantity: 1, UnitPrice: 8800},
            },
        },
        ProductLine{SKU: "cup-002", Quantity: 1, UnitPrice: 3200},
    },
}
```

组合管 **树怎么遍历**；装饰器管 **某一节点上的增强**。`BundleLine.Total()` 仍递归 `child.Total()`，不关心 child 是裸 `ProductLine` 还是三层包装。

### 支付侧的同类用法

对 `PaymentProcessor` 做重试、日志，结构与订单行装饰相同：

```go
type PaymentProcessor interface {
    Pay(order Order) error
}

type RetryProcessor struct {
    Inner      PaymentProcessor
    MaxRetries int
}

func (r RetryProcessor) Pay(order Order) error {
    var err error
    for i := 0; i <= r.MaxRetries; i++ {
        if err = r.Inner.Pay(order); err == nil {
            return nil
        }
    }
    return err
}
```

可与 [桥接](/cs-fundamentals/design-patterns/bridge) 叠加：`CheckoutAPI` 桥接 `RetryBackend{inner: WeChatPayBackend{...}}`——桥接拆维度，装饰器加横切。

### 函数式装饰

Go 里也可用 **高阶函数** 减少装饰器 struct 数量（适合只改一个方法的场景）：

```go
type Pricer interface {
    Total() int64
}

func WithCoupon(inner Pricer, off int64) Pricer {
    return pricedFunc(func() int64 {
        t := inner.Total()
        if t <= off {
            return 0
        }
        return t - off
    })
}

type pricedFunc func() int64

func (f pricedFunc) Total() int64 { return f() }
```

完整 `OrderLine` 三个方法时，struct 装饰器通常 **更清晰**；仅 `Total()` 可变时函数式更轻。

### 顺序与可交换性

文档化 **组装顺序** 或在装饰器内声明优先级：

```go
// 约定：先会员折扣 → 再优惠券 → 最后礼品包装费（费用不参与折扣）
func DecorateLine(base OrderLine, cfg LineConfig) OrderLine {
    line := base
    if cfg.MemberDiscountBP > 0 {
        line = MemberDiscountLine{Inner: line, DiscountBP: cfg.MemberDiscountBP}
    }
    if cfg.CouponOff > 0 {
        line = CouponLine{Inner: line, Off: cfg.CouponOff}
    }
    if cfg.GiftWrap {
        line = GiftWrapLine{Inner: line, Fee: 500}
    }
    return line
}
```

复杂满减（跨行、满 300 减 50）不适合单行装饰器——放在 `CheckoutService` 或 **价格引擎** 统一算，单行装饰只处理 **行内** 逻辑。

### 与策略的区别

| | 装饰器 | 策略 |
| :--- | :--- | :--- |
| 意图 | **叠加** 多层行为，对外仍是一个 Component | **替换** 一种算法 |
| 结构 | 链式包装，每层都委托 inner | Context 持有一个 Strategy，通常 **互斥** |
| 例子 | 会员折 + 券 + 包装费层层套 | 「用平台券算法还是店铺券算法」二选一 |

若促销规则 **互斥**（只能用一种券），用策略；若 **可叠加**，用装饰器或显式 **价格管道**（pipeline）。

### 拆接口减轻转发负担

当只有部分装饰器关心 `ReserveInventory`，可拆小接口：

```go
type Pricer interface { Total() int64 }
type Validator interface { Validate() error }
type InventoryHolder interface { ReserveInventory() error }
```

装饰器只实现它改动的接口；`CheckoutService` 用类型断言或辅助函数组合约束。小项目 **3～5 个方法** 全转发即可。

### 透明性：装饰器是否暴露 inner

多数场景 **不暴露** `inner`，客户端只认最外层 `OrderLine`。若运营要「展示原价与实付」，可加 **可选** 方法或单独 `PriceBreakdown(line OrderLine)` 遍历装饰链——Go 可用 type switch 或 `interface{ Unwrap() OrderLine }` 约定。

## 小结

记住这四点即可：

1. **同一接口上可叠加增强 → 装饰器**：持有 `inner`，先委托再增强，避免 `2^N` 子类。
2. **组装层套娃、Client 不变**：`CheckoutService` 仍只调 `line.Total()`，顺序在组装函数里约定。
3. **与组合正交**：组合是树 **包含** 子行；装饰是 **包装** 单行；child 可以是装饰过的 `ProductLine`。
4. **别滥用**：跨行满减、复杂促销引擎不属于单行装饰；接口越来越胖时拆小接口或管道化。

[组合模式](/cs-fundamentals/design-patterns/composite) 统一了 **订单明细树** 的遍历；装饰器模式统一了 **单行上的可选增强** 如何动态组合。下一篇 [外观模式](/cs-fundamentals/design-patterns/facade) 关注 **下单用例** 如何穿过库存、支付、通知等子系统而不让每个入口重复编排。

## 参考阅读

- [x] [Refactoring.Guru - 装饰器模式](https://refactoringguru.cn/design-patterns/decorator) (2026-06-22)
- [x] [菜鸟教程 - 装饰器模式](https://www.runoob.com/design-pattern/decorator-pattern.html) (2026-06-22)
