---
title: 享元模式
order: 11
---

**享元模式**（Flyweight）亦称 **缓存**，运用共享技术有效地支持大量细粒度的对象。

通俗地说，大量对象里重复出现的不变部分只存一份、大家共用；每个对象只保留会随情境变化的那点信息——对象数量再多，重复的元数据也不会跟着线性膨胀。

## 问题

大促或批量导出时，同一 SKU 会出现在 **成千上万条** 订单明细里。若每条明细都 **完整拷贝** 商品名、类目、税码、重量等字段，内存会随明细条数线性膨胀——而这些字段 **只跟 SKU 有关，跟具体哪一行无关**。

数据量小时看不出问题；百万行导出时，麻烦就会一起暴露：

1. **内存浪费**：同一爆款 SKU 的元数据在内存里重复 N 份。
2. **不变与可变混在一起**：商品名（随 SKU 不变）和数量、成交价（随订单变）塞进同一个 struct，改类目税率要扫全部明细。
3. **缓存难做**：商品目录已有一份，订单明细又各存一份快照——两套真相，同步困难。
4. **测试笨重**：构造 1 万行测试数据要复制 1 万份相同的商品名字符串。

本质矛盾是：**大量对象在不变数据上高度重复**，却用 **每个对象一份完整拷贝** 来建模。典型写法如下：

```go
type OrderLine struct {
    SKU         string
    ProductName string // 与 SKU 绑定，却在每条明细里各存一份
    Category    string
    TaxCode     string
    Quantity    int   // 随本行变化
    UnitPrice   int64 // 随促销/订单变化
}
```

## 解决方案

回到上面的 `OrderLine`：字段其实分两种——

- **各行相同、只跟 SKU 走**：商品名、类目、税码、重量…… 100 万行里 `"tea-001"` 的这些值 **一模一样**，没必要每行各拷一份。
- **各行不同、跟本行订单走**：数量、成交价、行号…… **每一行各自一份**。

做法就是 **拆开存**：第一类进共享的 `ProductSpec`，第二类留在每行的 `LineContext`。GoF 称前者 **内在状态**、后者 **外在状态**——**内**在共享对象 **里**，**外**在共享对象 **外**（由每行带着，调用时传进 `LineAmount(ctx)`）。

同一 SKU 的 `ProductSpec` 全进程只应有一份，由 **`ProductSpecFactory`** 按 SKU 查表、缓存、按需加载。

### 共享对象与外在状态

**享元** 就是按 SKU 共享的 `ProductSpec`；每行私有的数量、成交价放在 `LineContext` 里：

```go
type ProductSpec struct {
    SKU, Name, TaxCode string
}

func (p ProductSpec) LineAmount(ctx LineContext) int64 {
    return ctx.UnitPrice * int64(ctx.Quantity)
}

type LineContext struct {
    Quantity  int
    UnitPrice int64
}
```

`ProductSpec` **构造后只读**——类目、税码变更应 **换工厂里的实例**，不要原地改字段（否则所有引用该 SKU 的行会一起变）。

### 按 SKU 缓存的工厂

```go
type ProductSpecFactory struct {
    specs map[string]*ProductSpec
    load  func(sku string) *ProductSpec
}

func (f *ProductSpecFactory) Get(sku string) *ProductSpec {
    if spec, ok := f.specs[sku]; ok {
        return spec
    }
    spec := f.load(sku)
    f.specs[sku] = spec
    return spec
}
```

生产环境再加 `sync.RWMutex` 或 `sync.Map`；示例省略并发与错误处理，只保留 **「每个 SKU 至多一份」** 的意图。

### 客户端

加载明细时 **解析一次 SKU**，行对象持有 `*ProductSpec` 指针 + 本行 `LineContext`：

```go
type OrderLineView struct {
    spec *ProductSpec
    ctx  LineContext
}

func (l OrderLineView) Amount() int64 {
    return l.spec.LineAmount(l.ctx)
}

// 组装：spec, _ := factory.Get(line.SKU); view := OrderLineView{spec, line.ctx}
```

DB 可只存 `(sku, quantity, unit_price)`；报表进内存时再 `factory.Get` 绑定共享 spec，不必把 name/category 再读进每一行。

## 适用场景

1. **对象数量极大，且内在状态可枚举、可共享**：订单明细、地图瓦片、文档编辑器字符样式、游戏中的子弹/粒子 **类型**（非每颗子弹的坐标）。
2. **内在状态变体远少于外在状态**：SKU 种类有限，订单行与 `(sku, qty, price, orderID)` 组合却可成千上万。
3. **享元可被安全共享**：内在状态 **只读**，或变更通过 **版本 / 换键** 而非原地 mutate。
4. **外在状态可在使用时注入**：`LineAmount(ctx)`、`Render(ctx)`，而不是享元里藏 `currentOrderID`。
5. **与持久化策略配合**：DB 存 **快照或 SKU 引用**；内存密集计算用享元 **去重**。

**不必强行使用**：

- 订单行 **总共几百条**、SKU **几乎不重复**——工厂与 indirection 的开销 **大于** 省下的内存。
- **无法划分** 内在 / 外在——成交价有时要 **写回** 商品对象，或字段 **既像目录又像订单** 混在一起。
- 需要的是 **唯一全局实例**——用 [单例模式](/cs-fundamentals/design-patterns/singleton)，不是按 SKU 的享元池。
- 需要的是 **深拷贝已有订单**——用 [原型模式](/cs-fundamentals/design-patterns/prototype)；享元 **不** 复制 extrinsic state。
- 可变重对象 **借还复用**（连接、buffer）——用 **对象池** / `sync.Pool`，不是 Flyweight。
- 内在状态 **频繁 per-instance 不同**——该建模为普通对象，别硬享元。

常见例子：Java `Integer.valueOf` 小整数缓存、文本编辑器 **字形**（字体+字号+字符码共享，坐标外置）、游戏 **树木模型**（mesh 共享，位置外置）、CSS 类名 → 样式规则共享、IM 表情 **资源 id** 共享。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **降内存** | 重复 **内在状态** 只存一份，细粒度对象数量可很大 |
| **集中更新入口** | 类目/税码变更可 **换享元或 version**，而非扫 N 条 line |
| **与组合 / 装饰正交** | 树与装饰链仍可统一 `OrderLine`，底层用享元存 SKU 元数据 |
| **可测试** | 工厂注入 `load` fake，`ProductSpec` 纯函数式方法易单测 |

| 缺点 | 说明 |
| :--- | :--- |
| **复杂度上升** | 内在 / 外在划分、工厂生命周期、并发缓存都要设计 |
| **外在状态传递成本** | 每次 `LineAmount(ctx)` 要带 ctx；漏传则 bug subtle |
| **共享只读约束** | mutate 享元 **影响所有引用**——团队纪律或 immutable 构造 |
| **与订单快照语义** | 法务/审计要 **下单时刻商品名**——DB 仍要 snapshot，享元 **不能替代** 持久化真相 |
| **工厂内存上限** | SKU 种类 **百万级** 时，工厂 map 本身也要 **淘汰策略**（LRU） |

## 实践

### 持久化快照 vs 运行时享元

| 层 | 建议 |
| :--- | :--- |
| **订单 DB** | 存 `sku, quantity, unit_price, product_name_snapshot, snapshot_version`（合规） |
| **运行时报表** | 用 `ProductSpecFactory` + `LineContext`，**不** 把 name 再加载进每行 struct |
| **展示** | 审计单用 **快照**；运营大盘用 **当前目录享元** |

二者 **不矛盾**：享元优化 **内存中的重复**；快照保证 **历史不可抵赖**。

### 与 `sync.Pool` 的分工

| | 享元 | `sync.Pool` |
| :--- | :--- | :--- |
| **生命周期** | 进程级 **长期** 缓存 SKU 元数据 | **短生命周期** 临时对象（`[]byte`、解析器） |
| **状态** | **语义不变** 的内在状态 | 借出前 **Reset**，内容每次不同 |
| **键** | 业务键 SKU | 通常 **无键**，任意借还 |

导出 CSV 的 **行 buffer** 用 Pool；**商品名** 用享元——别混为一谈。

### 测试策略

```go
func TestProductSpecFactory_SharesInstance(t *testing.T) {
    loads := 0
    factory := NewProductSpecFactory(func(sku string) (*ProductSpec, error) {
        loads++
        return &ProductSpec{sku: sku, name: "tea"}, nil
    })
    a, _ := factory.Get("tea-001")
    b, _ := factory.Get("tea-001")
    if a != b {
        t.Fatal("expected same flyweight instance")
    }
    if loads != 1 {
        t.Fatal("expected load once")
    }
}

func TestProductSpec_LineAmountUsesExtrinsic(t *testing.T) {
    spec := &ProductSpec{sku: "tea-001"}
    ctx := LineContext{Quantity: 3, UnitPrice: 100}
    if got := spec.LineAmount(ctx); got != 300 {
        t.Fatalf("got %d", got)
    }
}
```

### 泛型工厂（Go 1.18+，可选）

多种享元类型时可抽：

```go
type FlyweightFactory[K comparable, F any] struct {
    mu   sync.RWMutex
    pool map[K]F
    new  func(K) (F, error)
}
```

业务仍按 **SKU / 字体键 / 图标 id** 各自实例化，避免过度抽象。

## 关联

- 你可以使用享元模式实现 [组合模式](/cs-fundamentals/design-patterns/composite) 树的共享叶节点以节省内存。
- 享元模式展示了如何生成大量的小型对象，[外观模式](/cs-fundamentals/design-patterns/facade) 则展示了如何用一个对象来代表整个子系统。
- 如果你能将对象的所有共享状态简化为一个享元对象，那么享元就和 [单例模式](/cs-fundamentals/design-patterns/singleton) 类似了。但这两个模式有两个根本性的不同。
  - 只会有一个单例实体，但是享元类可以有多个实体，各实体的内在状态也可以不同。
  - 单例对象可以是可变的。享元对象是不可变的。

## 参考阅读

- [x] [Refactoring.Guru - 享元模式](https://refactoringguru.cn/design-patterns/flyweight) (2026-06-22)
- [x] [菜鸟教程 - 享元模式](https://www.runoob.com/design-pattern/flyweight-pattern.html) (2026-06-22)
