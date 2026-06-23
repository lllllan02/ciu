---
title: 组合模式
order: 8
---

**组合模式** 将对象组合成树形结构以表示「部分-整体」的层次结构，使得用户对单个对象和组合对象的使用具有一致性。

通俗地说，把单个对象和对象组合成树，对外提供同一套用法；调用方不必区分「叶子」还是「容器」，对整棵树调同一个方法就行——容器负责往下转发，叶子负责干具体的事。

## 问题

订单明细天然是 **树形结构**：一行可能是单个 SKU，也可能是「春节礼盒」——里面还有子商品、子套餐。结算、校验、预占库存都要 **遍历这棵树**。

没有统一抽象时，每种操作都要 **自己写一遍** `if 单品 else 套餐` 加递归。明细类型少时还能应付；操作一多，问题就会一起暴露：

1. **分支散落各处**：算总价、校验、预占各自维护一套判断，加一种明细类型（如赠品行）要改 **所有** 方法。
2. **调用方被迫认识树结构**：结算服务不能简单调 `line.Total()`，必须自己知道怎么递归。
3. **递归易错**：有的方法忘了递归子节点，有的多递归一层，嵌套深了 bug 难查。
4. **职责混杂**：业务流程和 **树怎么遍历** 搅在一起，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。

本质矛盾是：**部分-整体层次** 客观存在，客户端却 **不能** 用同一套接口对待「单个明细」和「明细组合」。典型写法如下：

```go
func cartTotal(items []CartItem) int64 {
    var sum int64
    for _, item := range items {
        switch item.Kind {
        case "product":
            sum += item.UnitPrice * int64(item.Quantity)
        case "bundle":
            sum += cartTotal(item.Children) // Validate、Reserve 又要写一遍
        }
    }
    return sum
}
```

## 解决方案

定义 **组件** 接口 `OrderLine`，让 **叶子**（单品）与 **组合**（套餐）都实现它；组合体在 `Total`、`Validate` 等方法里 **遍历 children 并聚合结果**。

### 组件（Component）

```go
type OrderLine interface {
    Total() int64
    Validate() error
    ReserveInventory() error
}
```

客户端只依赖这一接口，不依赖具体是 `ProductLine` 还是 `BundleLine`。

### 叶子（Leaf）

```go
type ProductLine struct {
    SKU      string
    Name     string
    Quantity int
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

### 组合（Composite）

```go
type BundleLine struct {
    BundleID string
    Name     string
    Children []OrderLine
}

func (b BundleLine) Total() int64 {
    var sum int64
    for _, child := range b.Children {
        sum += child.Total()
    }
    return sum
}

func (b BundleLine) Validate() error {
    if b.BundleID == "" {
        return fmt.Errorf("bundle line: missing bundle id")
    }
    if len(b.Children) == 0 {
        return fmt.Errorf("bundle %s: empty children", b.BundleID)
    }
    for _, child := range b.Children {
        if err := child.Validate(); err != nil {
            return err
        }
    }
    return nil
}

func (b BundleLine) ReserveInventory() error {
    for _, child := range b.Children {
        if err := child.ReserveInventory(); err != nil {
            return err
        }
    }
    return nil
}
```

### 客户端

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

组装一棵 **嵌套礼盒** 时，客户端仍只调 `Total()`——递归发生在 `BundleLine` 内部：

```go
giftBox := BundleLine{
    BundleID: "gift-2026",
    Name:     "春节礼盒",
    Children: []OrderLine{
        ProductLine{SKU: "tea-001", Quantity: 1, UnitPrice: 8800},
        BundleLine{
            BundleID: "snack-pack",
            Name:     "零食小包",
            Children: []OrderLine{
                ProductLine{SKU: "nut-002", Quantity: 2, UnitPrice: 1500},
            },
        },
    },
}

svc := &CheckoutService{lines: []OrderLine{giftBox, ProductLine{SKU: "book-99", Quantity: 1, UnitPrice: 4500}}}
_ = svc.Checkout() // 11800，无需 if bundle
```

新增 **赠品行**、**虚拟 bundle** → 新实现 `OrderLine`，`CheckoutService` **不必改**。


## 适用场景

1. **部分-整体层次稳定且需统一操作**：订单明细、菜单（含子菜单）、组织架构、文件系统（文件/目录）、UI 组件树（面板含按钮含子面板）。
2. **客户端应忽略组合与个体的差异**：结算只调 `Total()`，不关心底下几层 bundle。
3. **对整棵树做同一类遍历**：校验、计价、库存、权限检查、序列化——接口一致，Composite 负责递归。
4. **结构可能动态变化**：购物车增删子行、重组套餐——在 Composite 上 `Add`/`Remove`（安全组合）或重建 `Children` 切片。
5. **与生成器 / 工厂组合建树**：[生成器](/cs-fundamentals/design-patterns/builder) 产出 `ProductLine`，工厂方法创建预设 `BundleLine` 模板。

**不必强行使用**：

- 只有 **一层** 列表、永远不会有嵌套——`[]ProductLine` 直接循环更简单。
- 叶子与组合 **行为差异很大**（组合能打折、叶子不能）且客户端 **必须** 区分——硬套同一接口会让 Leaf 实现空方法或 panic，考虑拆接口或用访问者模式。
- 需要的是 **两个变化维度独立扩展**（如支付形态 × 支付后端）——那是 [桥接](/cs-fundamentals/design-patterns/bridge)，不是组合。
- 树很深且 **操作类型很多、组件类型相对固定**——可考虑 **访问者模式** 把新操作从 Component 接口上挪走，避免 `OrderLine` 接口越来越胖（见 [实践 · 接口膨胀与访问者](#接口膨胀与访问者)）。

常见例子：购物车明细树、文档大纲、图形编辑器（Group + Shape）、权限树、配置 JSON 的嵌套对象模型。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新明细类型 = 新 `OrderLine` 实现，Client 与已有 Leaf/Composite 少改 |
| **单一职责** | 遍历与聚合在 `BundleLine`；单品逻辑在 `ProductLine`；结算在 `CheckoutService` |
| **客户端简化** | 无 `switch kind`，统一调 `Total()` / `Validate()` |
| **递归自然** | 嵌套深度任意，结构自相似 |
| **符合合成复用** | 套餐 **组合** 子行，而非用继承堆 `GiftBox extends Product` |

| 缺点 | 说明 |
| :--- | :--- |
| **接口可能过宽** | 透明组合下 Leaf 也要实现 Client 用不到的方法（或留空） |
| **类型约束弱** | 运行时才能把「空 bundle」拦在 `Validate`；编译期难禁止 Leaf 被 `Add` 子节点 |
| **深树性能** | 每次 `Total` 全树遍历；极大购物车需缓存或增量维护 |
| **过度设计风险** | 扁平 SKU 列表套 Composite 多一层 indirection |

## 实践

> **阅读提示**：先掌握「`OrderLine` 接口 + Leaf/Composite 实现 + Client 只调接口」即可。本节是工程变体；初学可先跳过。

### 用生成器 / 工厂组装树

明细树常在 **下单前** 由 Builder 或工厂拼好，再交给 `CheckoutService`：

```go
func NewSpringGiftBox() OrderLine {
    return BundleLine{
        BundleID: "gift-2026",
        Name:     "春节礼盒",
        Children: []OrderLine{
            ProductLine{SKU: "tea-001", Quantity: 1, UnitPrice: 8800},
            NewSnackPack(), // 子工厂返回嵌套 BundleLine
        },
    }
}

func buildOrderFromBuilder(b *OrderBuilder) []OrderLine {
    order := b.Build()
    lines := make([]OrderLine, len(order.Items))
    for i, item := range order.Items {
        lines[i] = ProductLine{SKU: item.SKU, Quantity: item.Qty, UnitPrice: item.Price}
    }
    return lines
}
```

**构建树**（Builder/工厂）与 **遍历树**（Composite Client）分层：Builder 知道 SKU 与 bundle 模板；Checkout 只认 `OrderLine`。

### 安全组合：结构编辑 API

若购物车 UI 需要 **增删子行**，可把编辑方法只放在 Composite 上（或单独 `LineMutator` 接口）：

```go
type MutableBundle interface {
    OrderLine
    Add(child OrderLine)
    Remove(sku string) bool
}

func (b *BundleLine) Add(child OrderLine) {
    b.Children = append(b.Children, child)
}
```

Client 若 **只读结算**，仍依赖 `OrderLine`；编辑模块依赖 `MutableBundle`，避免 Leaf 实现无意义的 `Add`。

### 与装饰器的区别

两者都「持有同接口对象、往下委托」，容易看成同一种递归。差别在 **问的问题**：

| | 组合 | 装饰器 |
| :--- | :--- | :--- |
| **问的是** | 这一行 **由哪些独立明细组成** | 这一行 **还是原来那行**，叠了哪些规则 |
| **拓扑** | 树，1 → **N** 个兄弟 | 链，1 → **1** 个 inner |
| **递归** | **汇总** peer：`sum += child.Total()` | **变换** spine：`transform(inner.Total())` |
| **实体数** | 礼盒含茶 + 杯 = **2 件货** | 茶 + 券 + 包装 = **还是 1 行茶** |
| **child 能否独立存在** | 能，每个 SKU 可单独下单 | 不能，`CouponLine` 离开 `inner` 无意义 |

若把优惠券硬写成 bundle 的一个 child，计价也许能凑对，但列表、库存、满减分摊的语义都会错——优惠券不是一件商品。详见 [装饰器模式 · 装饰器像「只有一个子节点的 Bundle」吗？](/cs-fundamentals/design-patterns/decorator#装饰器像只有一个子节点的-bundle-吗)。

可叠加：`BundleLine` 的 child 可以是 `DiscountLine{inner: ProductLine{...}}`，只要 `DiscountLine` 也实现 `OrderLine`。

### 与迭代器 / 扁平化

报表、物流拆单有时需要 **扁平 SKU 列表**。在 Composite 上提供 **访问者** 或 `Walk(func(OrderLine))`，而不是让 Client 手写递归：

```go
func Walk(line OrderLine, fn func(OrderLine)) {
    fn(line)
    if b, ok := line.(BundleLine); ok {
        for _, child := range b.Children {
            Walk(child, fn)
        }
    }
}
```

Go 1.18+ 也可用泛型辅助函数收集所有 `ProductLine`，避免业务里复制遍历逻辑。

### 接口膨胀与访问者

当 `OrderLine` 上要加 `ExportJSON`、`ApplyCoupon`、`CalcTax` 等 **很多种** 操作时，Component 接口会 **越来越胖**，每个 Leaf/Composite 都要改。

两种缓解：

1. **拆小接口**：`Pricer`、`Validator`、`InventoryHolder`——Client 按需组合约束。
2. **[访问者模式](/cs-fundamentals/design-patterns/visitor)**：新操作 = 新 Visitor，Component 只保留 `Accept(Visitor)`。

小项目 **3～5 个方法** 留在 `OrderLine` 即可；方法超过 ~7 个且还在涨，再考虑访问者。

### 错误聚合与部分失败

`ReserveInventory` 递归时，若希望 **尽量预占、并汇总失败 SKU**：

```go
func (b BundleLine) ReserveInventory() error {
    var errs []error
    for _, child := range b.Children {
        if err := child.ReserveInventory(); err != nil {
            errs = append(errs, err)
        }
    }
    return errors.Join(errs...)
}
```

策略（全失败才回滚 vs 部分成功）属于业务规则，Composite 只负责 **把递归与聚合方式集中在一处**。

## 小结

记住这四点即可：

1. **部分-整体是树 → 组合**：单品与套餐都实现同一 `OrderLine`，Client 不写 `if bundle`。
2. **Composite 递归、Leaf 干活**：`BundleLine.Total()` 汇总子节点；`ProductLine.Total()` 算本行。
3. **与适配器、桥接正交**：组合管 **订单里有什么**；适配器管 **接口翻译**；桥接管 **两维独立变化**。
4. **注意接口别无限变胖**：只读遍历用透明组合；操作暴增时拆接口或访问者。

[桥接模式](/cs-fundamentals/design-patterns/bridge) 把 **支付请求形态与支付后端** 拆开；组合模式把 **订单明细的树形结构** 与 **结算/校验/库存** 的调用方式统一。放回电商订单系统这条主线：明细从扁平 SKU 长成嵌套套餐时，用组合让 `CheckoutService` 始终只面对 `OrderLine`。当同一行还要叠加会员价、优惠券、礼品包装等可选增强时，下一篇 [装饰器模式](/cs-fundamentals/design-patterns/decorator) 说明如何动态包装而不必穷举子类。

## 参考阅读

- [x] [Refactoring.Guru - 组合模式](https://refactoringguru.cn/design-patterns/composite) (2026-06-22)
- [x] [菜鸟教程 - 组合模式](https://www.runoob.com/design-pattern/composite-pattern.html) (2026-06-22)
