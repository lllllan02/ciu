---
title: 访问者模式
order: 22
---

**访问者模式**（Visitor）表示一个作用于某对象结构中的各元素的操作；它使你可以在不改变各元素的类的前提下定义作用于这些元素的新操作。

通俗地说，**树形结构（订单明细）相对稳定，但要在上面做的操作（导出、拣货、计税）会越来越多**——把每种操作单独做成「访问者」，节点只负责「接待」访问者；新增操作加新访问者即可，不必往 `ProductLine`、`BundleLine` 上不断堆 `ExportJSON`、`ToPickList`……

## 问题

运营后台已有 [组合模式](/cs-fundamentals/design-patterns/composite) 搭好的 **订单明细树**：`ProductLine`（单品）、`BundleLine`（礼盒，含子明细）、`GiftLine`（赠品）。结算、校验、预占库存都走 `OrderLine` 上的 `Total()`、`Validate()`、`ReserveInventory()`——**「树里有哪些节点类型、怎么递归」这一维已经稳定**。

**另一维在涨**：对账要导出 JSON、WMS 要拣货单、财务要税务行、合规要海关申报……都要 **区分单品 / 礼盒 / 赠品**，**遍历整棵树**，且各自规则不同。

本质矛盾是：**对象结构（节点类型）相对稳定，但作用于其上的操作种类持续增加**。[组合模式](/cs-fundamentals/design-patterns/composite) 解决的是 **结构维**（Client 调 `line.Total()` 不必写 `if bundle`）；访问者要解决的是 **操作维**（JSON、拣货、税务如何不断扩展）。而两种常见做法都会在 **结构 × 操作** 的交叉点上反复付税：

| 做法 | 操作写在哪 | 加一种新导出时 |
| :--- | :--- | :--- |
| **A. 胖接口** | 塞进 `OrderLine` 及各节点 | 改接口 + **所有** 节点实现 |
| **B. 客户端 switch** | 留在 `appendPickList` 等函数里 | **新写一遍** type switch + 树遍历 |

**路径 A——操作堆进节点**（报表少时多态尚能应付）：

```go
type OrderLine interface {
    Total() int64
    Validate() error
    ReserveInventory() error
    ExportJSON(w io.Writer) error      // 新需求 1
    AppendPickList(out *[]PickItem)     // 新需求 2
    TaxLines(out *[]TaxRow)             // 新需求 3
    // 营销又要海关申报格式……
}
```

`ProductLine` 的主业是 **计价与库存**；JSON、拣货、税务是 **辅助导出**，塞进来 **违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)**。且 `ProductLine` 已是线上核心——每加一种格式就要动它，**回归风险大**；加 `GiftLine` 还要实现 **全部** 导出方法，哪怕拣货只关心叶子 SKU。

**路径 B——操作堆进客户端**（不把方法加进接口，但噩梦换了个地方）：

```go
func appendPickList(line OrderLine, out *[]PickItem) {
    switch n := line.(type) {
    case ProductLine:
        *out = append(*out, PickItem{SKU: n.SKU, Qty: n.Quantity})
    case BundleLine:
        for _, child := range n.Children {
            appendPickList(child, out) // 每种操作各自写递归
        }
    case GiftLine:
        *out = append(*out, PickItem{SKU: n.SKU, Qty: n.Quantity, Gift: true})
    }
    // 加 TaxLines？再复制一遍 switch…
}
```

`ExportJSON`、`TaxLines` 各 **复制一套** switch 与 walk——与组合里统一的递归 **重复且易不一致**（有的忘了递归子节点，有的多走一层）。

**需要的是**：在 **少改节点类** 的前提下，让 **新操作独立扩展**——这正是访问者模式要解决的。

## 解决方案

定义 **OrderLineVisitor**；**OrderLine** 增加 `Accept(v)`，由节点 **双重分派** 到 `Visit*`（替代客户端 type switch）；各 **ConcreteVisitor** 封装一种操作；**BundleLine** 在 Accept 内 **递归** 子节点。

### 访问者与元素

```go
type OrderLineVisitor interface {
    VisitProduct(p ProductLine) error
    VisitBundle(b BundleLine) error
    VisitGift(g GiftLine) error
}

type OrderLine interface {
    Total() int64
    Validate() error
    ReserveInventory() error
    Accept(v OrderLineVisitor) error
}

func (p ProductLine) Accept(v OrderLineVisitor) error {
    return v.VisitProduct(p)
}

func (b BundleLine) Accept(v OrderLineVisitor) error {
    if err := v.VisitBundle(b); err != nil {
        return err
    }
    for _, child := range b.Children {
        if err := child.Accept(v); err != nil {
            return err
        }
    }
    return nil
}

func (g GiftLine) Accept(v OrderLineVisitor) error {
    return v.VisitGift(g)
}
```

客户端 **只调** `line.Accept(v)`，不再 `switch` 类型；**哪种 Visit 被调用** 由节点决定，**Visit 里做什么** 由访问者决定。双分派直觉、流程图见 **实践**。

### 具体访问者

```go
type PickListExporter struct {
    items []PickItem
}

func (e *PickListExporter) VisitProduct(p ProductLine) error {
    e.items = append(e.items, PickItem{SKU: p.SKU, Qty: p.Quantity})
    return nil
}

func (e *PickListExporter) VisitBundle(BundleLine) error { return nil } // 容器不出库

func (e *PickListExporter) VisitGift(g GiftLine) error {
    e.items = append(e.items, PickItem{SKU: g.SKU, Qty: g.Quantity, Gift: true})
    return nil
}

func (e *PickListExporter) Result() []PickItem { return e.items }
```

JSON 导出、税务分类、合规审计 **各一个 ConcreteVisitor**，套路相同。

### 客户端

```go
func ExportPickList(lines []OrderLine) []PickItem {
    exp := &PickListExporter{}
    for _, line := range lines {
        _ = line.Accept(exp)
    }
    return exp.Result()
}
```

**写路径**（结算）仍调 `line.Total()`；**读路径**（报表/WMS）调 Visitor。JSON、Tax、礼盒走读示例见 **实践**。

## 适用场景

1. **对象结构稳定、操作常增**：明细类型固定，导出/统计/审计 **不断新增**（RG：对复杂结构 **所有元素** 执行某操作）。
2. **清理辅助行为**：把 JSON、拣货、税务从 `OrderLine` **抽到访问者**，主类专注 `Total/Validate`（RG：清理非主要业务逻辑）。
3. **行为仅对部分类型有意义**：如税务只关心 `ProductLine`，`VisitBundle` / `VisitGift` 留空即可（RG：行为仅在部分类有意义时抽取访问者）。
4. **与 [组合](/cs-fundamentals/design-patterns/composite) 树配合**：在组合树上跑操作；Accept 内 **统一递归**。
5. **与 [迭代器](/cs-fundamentals/design-patterns/iterator) 配合**：迭代器负责 **怎么遍历**，访问者负责 **遍历到每个元素做什么**。

**不必强行使用**：

- **操作少且稳定**（3～5 个）——留在组合接口即可（见组合文 [接口膨胀与访问者](/cs-fundamentals/design-patterns/composite#接口膨胀与访问者)）。
- **元素类型频繁增加**——每加一种明细要 **改所有 Visitor**。
- **只需扁平叶子列表、无容器元数据**——`LeafSKUIterator` 更简单。
- **单一可替换算法**——用 [策略](/cs-fundamentals/design-patterns/strategy)。

常见例子：编译器 AST Visitor、lint 规则、DOM 操作、图形编辑器导出 SVG/PDF、SQL AST。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭（对操作）** | 新行为 **新 Visitor**，少改 Element（RG：开闭原则） |
| **单一职责** | 导出逻辑 **在 Visitor**；节点管结构域 |
| **相关操作内聚** | `TaxVisitor` **集中** 税务规则 |
| **可携带中间状态** | Visitor 在遍历树时可 **累积** `items`、`rows` 等 |

| 缺点 | 说明 |
| :--- | :--- |
| **开闭（对元素）** | 新 `GiftLine` → **改 Visitor 接口 + 所有实现** |
| **封装** | Visitor 需读节点字段——Go 常 **同包** 或导出字段 |
| **Accept 递归约定** | 谁负责 walk 子节点 **要团队统一** |
| **Go 无语法糖** | `Visit*` 随类型增长，接口 **会变宽** |

## 实践

### 双分派与 Accept

Go **不能** 靠重载让 `Visit(line OrderLine)` 自动分到 `VisitProduct`——编译期只知道接口类型。访问者把 **类型选择** 交给节点：

```go
for _, line := range order.Lines {
    _ = line.Accept(pickListExporter) // 客户端不再 switch
}
// ProductLine.Accept → v.VisitProduct(p)
```

```text
ExportPickList(order)
       │
       ▼
  PickListExporter
       │
       ▼
  每行 line.Accept(exp)
       ├── ProductLine → VisitProduct → 记入 SKU
       ├── BundleLine  → VisitBundle → 递归子节点 Accept
       └── GiftLine    → VisitGift   → 记入 SKU + 赠品标记
```

换 `TaxVisitor` 走同一棵树，规则换成计税——节点 **只接待**，不必自己学会审计（仓库巡检员类比：专员按类型执行动作，节点表明身份即可）。

## 关联

- 你可以将 [访问者模式](/cs-fundamentals/design-patterns/visitor) 视为 [命令模式](/cs-fundamentals/design-patterns/command) 的加强版本，其对象可对 **不同类的多种对象** 执行操作。
- 你可以使用访问者对整个 [组合模式](/cs-fundamentals/design-patterns/composite) 树执行操作。
- 可以同时使用访问者和 [迭代器模式](/cs-fundamentals/design-patterns/iterator) 来遍历复杂数据结构，并对其中的元素执行所需操作，即使这些元素所属的类完全不同。

## 参考阅读

- [x] [Refactoring.Guru - 访问者模式](https://refactoringguru.cn/design-patterns/visitor) (2026-06-22)
- [x] [菜鸟教程 - 访问者模式](https://www.runoob.com/design-pattern/visitor-pattern.html) (2026-06-22)
