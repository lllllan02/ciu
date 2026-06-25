---
title: 访问者模式
order: 22
---

**访问者模式**（Visitor）表示一个作用于某对象结构中的各元素的操作；它使你可以在不改变各元素的类的前提下定义作用于这些元素的新操作。

通俗地说，对象结构相对稳定，但要在其上做的操作会不断增加——把每种操作单独做成访问者，元素只负责接待访问者；新增一类操作加新访问者即可，不必往元素类上不断堆新方法。

## 问题

对账要导出 JSON、WMS 要拣货单、财务要税务分类、合规要审计日志——这些操作都要 **区分单品和礼盒**，且各自递归规则不同。最直接的做法是在 `OrderLine` 接口上 **不断加方法**：`ExportJSON()`、`ToPickList()`、`TaxCategory()`……

操作种类少时还能应付；每加一种报表，问题就会一起暴露：

1. **接口膨胀**：`OrderLine` 上 7+ 方法；加「海关申报格式」要改接口和所有实现，违反 [接口隔离](/cs-fundamentals/design-patterns#设计原则)。
2. **职责混杂**：`ProductLine` 既算价，又懂 JSON、税务、审计，违反 **单一职责**。
3. **递归重复**：每种 Export 各自写 traverse，与迭代器的 walk **重复且不一致**。
4. **开闭困难**：新 `GiftLine` 节点要实现全部报表方法，哪怕拣货只关心叶子 SKU。

本质矛盾是：**对象结构（明细类型）相对稳定**，但 **作用于其上的操作种类** 持续增加；不应 **把每种操作都塞进元素接口**。典型写法如下：

```go
type OrderLine interface {
    Total() int64
    Validate() error
    ExportJSON(w io.Writer) error    // 每种报表加一个方法…
    AppendPickList(out *[]PickItem)
    TaxLines(out *[]TaxRow)
}
```

## 解决方案

定义 **OrderLineVisitor**；**OrderLine** 增加 `Accept(v OrderLineVisitor)`；**ProductLine** / **BundleLine** / **GiftLine** 各自 **Accept**；各 **ConcreteVisitor** 实现 **Visit\***。

### 访问者（Visitor）接口

```go
type OrderLineVisitor interface {
    VisitProduct(p ProductLine) error
    VisitBundle(b BundleLine) error
    VisitGift(g GiftLine) error
}
```

每 **新增元素类型** 要 **改 Visitor 接口**——与 **新增操作只加 Visitor 类** 形成 **权衡**（见 [优缺点](#优缺点)）。

### 元素（Element）与 Accept

```go
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

type GiftLine struct {
    SKU      string
    Quantity int
    // 赠品：不计价但要出现在审计
}

func (g GiftLine) Accept(v OrderLineVisitor) error {
    return v.VisitGift(g)
}
```

**VisitBundle 是否递归** 由 **约定** 决定：本文 **Accept 统一递归子节点**；`VisitBundle` 只处理 **容器自身元数据**（礼盒 ID、名称）。

### 具体访问者：拣货单导出

```go
type PickListExporter struct {
    items []PickItem
}

func (e *PickListExporter) VisitProduct(p ProductLine) error {
    e.items = append(e.items, PickItem{SKU: p.SKU, Qty: p.Quantity})
    return nil
}

func (e *PickListExporter) VisitBundle(b BundleLine) error {
    // 容器本身不出库；子节点由 Accept 递归
    return nil
}

func (e *PickListExporter) VisitGift(g GiftLine) error {
    e.items = append(e.items, PickItem{SKU: g.SKU, Qty: g.Quantity, Gift: true})
    return nil
}

func (e *PickListExporter) Result() []PickItem { return e.items }
```

### 具体访问者：JSON 导出（容器有结构）

```go
type JSONExporter struct {
    w     io.Writer
    depth int
}

func (e *JSONExporter) VisitProduct(p ProductLine) error {
    _, err := fmt.Fprintf(e.w, "%s{\"type\":\"product\",\"sku\":%q,\"qty\":%d}\n",
        indent(e.depth), p.SKU, p.Quantity)
    return err
}

func (e *JSONExporter) VisitBundle(b BundleLine) error {
    if _, err := fmt.Fprintf(e.w, "%s{\"type\":\"bundle\",\"id\":%q,\"children\":[\n",
        indent(e.depth), b.BundleID); err != nil {
        return err
    }
    e.depth++
    return nil // children 由 Accept 递归写入
}

func (e *JSONExporter) VisitGift(g GiftLine) error {
    _, err := fmt.Fprintf(e.w, "%s{\"type\":\"gift\",\"sku\":%q}\n", indent(e.depth), g.SKU)
    return err
}
```

### 具体访问者：税务分类

```go
type TaxVisitor struct {
    rows []TaxRow
    rate float64
}

func (v *TaxVisitor) VisitProduct(p ProductLine) error {
    v.rows = append(v.rows, TaxRow{
        SKU: p.SKU, Taxable: p.UnitPrice * int64(p.Quantity), Rate: v.rate,
    })
    return nil
}

func (v *TaxVisitor) VisitBundle(b BundleLine) error {
    return nil // 税在叶子 SKU 上汇总
}

func (v *TaxVisitor) VisitGift(GiftLine) error {
    return nil // 赠品不计税
}
```

### 对象结构（ObjectStructure）与客户端

```go
type OrderLines struct {
    lines []OrderLine
}

func (o OrderLines) AcceptAll(v OrderLineVisitor) error {
    for _, line := range o.lines {
        if err := line.Accept(v); err != nil {
            return err
        }
    }
    return nil
}

// WMS 服务
func ExportPickList(order OrderLines) []PickItem {
    exp := &PickListExporter{}
    _ = order.AcceptAll(exp)
    return exp.Result()
}
```

**Checkout** 仍调 `line.Total()`（组合内聚）；**报表/WMS** 调 **Visitor**——**核心写路径** 与 **读模型导出** 分层。


## 适用场景

1. **对象结构稳定、操作常增**：明细类型固定，报表/导出/审计 **不断新增**。
2. **避免胖接口**：`OrderLine` **只留核心领域方法** + `Accept`。
3. **多种 unrelated 操作**：JSON、拣货、税务 **逻辑差异大**，不宜全塞 `Total()` 旁支。
4. **与组合树配合**：Composite **Accept 递归**；Visitor **按类型处理**。
5. **只读遍历为主**：导出/统计；**写操作** 仍用 **组合内聚** 或 [命令](/cs-fundamentals/design-patterns/command)。

**不必强行使用**：

- **操作 3～5 个且稳定**——留在 [组合](/cs-fundamentals/design-patterns/composite) 接口即可（见组合文 [接口膨胀与访问者](/cs-fundamentals/design-patterns/composite#接口膨胀与访问者)）。
- **元素类型频繁增加**——每个新类型要 **改所有 Visitor**。
- **只需扁平叶子列表**——[迭代器](/cs-fundamentals/design-patterns/iterator) `LeafSKUIterator` 更简单。
- **单一可换算法**——[策略](/cs-fundamentals/design-patterns/strategy)。

常见例子：编译器 AST Visitor、lint 规则、文档对象模型、图形编辑器「导出 SVG/PDF」、SQL AST。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭（对操作）** | 新操作 **新 Visitor**，少改 Element |
| **单一职责** | 导出逻辑 **在 Visitor** |
| **消除胖接口** | `OrderLine` **不膨胀** |
| **相关操作内聚** | `TaxVisitor` **集中税务规则** |
| **双重分派清晰** | 少 scattered type switch |

| 缺点 | 说明 |
| :--- | :--- |
| **开闭（对元素）** | 新 `GiftLine` → **改 Visitor 接口 + 所有实现** |
| **破坏封装？** | Visitor 需 **读 Element 字段**——Go 常 **同包或导出字段** |
| **递归约定** | Accept 谁递归 **要团队统一** |
| **Go 无 Visitor 语法糖** | 接口方法 **随类型增长** |
| **与迭代重叠** | 简单扁平 **Iterator 够用** |

## 实践

> **阅读提示**：先掌握「**Accept → VisitXxx，Composite 递归 Accept**」即可。本节是工程变体；初学可先跳过。

### 与组合、迭代器、装饰器

```text
Checkout 写路径
  → line.Total() / Validate() / ReserveInventory()（组合内聚）

WMS 读路径
  → PickListExporter.AcceptAll（访问者）

仅需叶子 SKU、无 bundle 元数据
  → LeafSKUIterator（迭代器）

行级 CouponLine 装饰器
  → Total() 仍装饰；ExportJSON 可能 VisitProduct 读 **未装饰** 或 **专门 PricingVisitor**
```

[装饰器](/cs-fundamentals/design-patterns/decorator) 包装 **计价**；Visitor 读 **结构** 时要 **约定是否穿透装饰器**——常 **Accept 在装饰器上** 委托 `inner.Accept`。

### 装饰器实现 Accept

```go
type CouponLine struct {
    Inner OrderLine
    Off   int64
}

func (c CouponLine) Accept(v OrderLineVisitor) error {
    return c.Inner.Accept(v) // 导出结构忽略券，或 Visitor 读外层
}

func (c CouponLine) Total() int64 {
    return max(c.Inner.Total()-c.Off, 0)
}
```

### 泛型 Visitor（Go 1.18+，减少 Visit 重复）

```go
type Visitor[T any] interface {
    VisitProduct(ProductLine) (T, error)
    VisitBundle(BundleLine) (T, error)
}

// 或 map[reflect.Type]func——权衡类型安全
```

小项目 **显式 Visit 方法** 更清晰。

### 何时从组合切到访问者

| 信号 | 动作 |
| :--- | :--- |
| `OrderLine` **>7 个** 且半数 **只服务报表** | 迁到 Visitor |
| 加 **GiftLine** 要改 **5 个 Export\*** | 引入 Accept |
| 仅 **多一种扁平导出** | Iterator 够 |

### 测试策略

```go
func TestPickListExporter_FlattensProduct(t *testing.T) {
    lines := OrderLines{lines: []OrderLine{
        ProductLine{SKU: "a", Quantity: 2},
    }}
    exp := &PickListExporter{}
    _ = lines.AcceptAll(exp)
    if len(exp.Result()) != 1 || exp.Result()[0].SKU != "a" {
        t.Fatal(exp.Result())
    }
}

func TestPickListExporter_BundleRecurses(t *testing.T) {
    lines := OrderLines{lines: []OrderLine{
        BundleLine{Children: []OrderLine{
            ProductLine{SKU: "x", Quantity: 1},
        }},
    }}
    exp := &PickListExporter{}
    _ = lines.AcceptAll(exp)
    if len(exp.Result()) != 1 {
        t.Fatal("expected leaf from bundle")
    }
}
```

## 小结

记住这四点即可：

1. **操作进 Visitor**：JSON/拣货/税务 **各一个 ConcreteVisitor**。
2. **元素只 Accept**：`ProductLine.Accept` → `VisitProduct`；**双重分派**。
3. **Composite 递归 Accept**：`BundleLine` **子节点继续 Accept**，Visitor **不必手写 walk**。
4. **与组合分层**：`Total/Validate` **留组合**；**报表类只读操作** 用 Visitor。

[组合模式](/cs-fundamentals/design-patterns/composite) 解决了 **「树形明细上核心行为一致」**；访问者模式解决了 **「在不改节点类的前提下，持续增加作用于树上的各类操作」**——把 **易变的操作维度** 从 **稳定的结构接口** 上 **剥离**，在操作频繁扩展时保持 [开闭](/cs-fundamentals/design-patterns#设计原则) 与 **接口隔离**。

## 参考阅读

- [x] [Refactoring.Guru - 访问者模式](https://refactoringguru.cn/design-patterns/visitor) (2026-06-22)
- [x] [菜鸟教程 - 访问者模式](https://www.runoob.com/design-pattern/visitor-pattern.html) (2026-06-22)
