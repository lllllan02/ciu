---
title: 原型模式
order: 4
---

**原型模式** 亦称 **克隆**（Clone）、**Prototype**，用原型实例指定创建对象的种类，并通过复制这些原型创建新的对象。

通俗地说，既然已经有一份「对的」对象，直接复制它再改少量差异即可，不必从零构造；复制怎么做才安全、怎么保证独立，由原型自己封装，调用方只取拷贝、再改需要变的部分。

## 问题

业务里经常要造 **结构相同、只有少量字段不同** 的对象——比如按同一份订单模板给不同买家下单，只改买家 ID 和个别变量。最直接的做法是 **照着模板手工填一个新 struct**。

字段少时还能应付；模板一多、嵌套一深，问题就会一起暴露：

1. **从头造太慢**：模板要从磁盘加载、预热配置，每条订单都重新拼一遍，浪费 I/O 和 CPU。
2. **手工拷贝易错**：切片、map 赋值只复制引用，改新订单会 **污染原模板**；深拷贝逻辑散落各处。
3. **职责搅在一起**：调用方既要选模板，又要自己实现安全复制，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。
4. **扩展成本高**：新增一种模板，每个提交点都要认识新类型并写一套拷贝代码，而不是统一 `Clone()`。

本质矛盾是：**你手里已经有一份「对的」对象**，却还要走 **从零构造或逐字段手工复制** 的弯路。典型写法如下：

```go
func sendFromTemplate(tpl *OrderTemplate, buyer string) error {
    n := &OrderTemplate{
        BuyerID:  buyer,
        Items:    tpl.Items,    // 切片共享——改 n 会污染 tpl
        Metadata: tpl.Metadata, // map 同理
        // …还有十几个字段要逐个抄
    }
    return submit(n)
}
```

## 解决方案

把「安全复制」收进原型；提交侧只 **克隆 + 改差异字段**。

### 原型接口

所有可克隆订单模板实现同一接口：

```go
type OrderPrototype interface {
    Clone() OrderPrototype
    // 便于提交层使用；也可拆成独立 Product 接口
    SubmitClone(buyer string, vars map[string]string) error
}
```

### 具体原型

订单模板在 **加载 / 构建阶段** 初始化一次（可用 [生成器](/cs-fundamentals/design-patterns/builder)）；`Clone()` 负责 **深拷贝** 可变字段：

```go
type OrderTemplate struct {
    buyerID       string
    invoiceTitle  string
    name          string
    items         []OrderItem
    paymentMethod string
    priority      int
    gifts         []string
    metadata      map[string]string
}

func (n *OrderTemplate) Clone() OrderPrototype {
    items := append([]OrderItem(nil), n.items...)
    gifts := append([]string(nil), n.gifts...)
    meta := make(map[string]string, len(n.metadata))
    for k, v := range n.metadata {
        meta[k] = v
    }
    return &OrderTemplate{
        invoiceTitle:  n.invoiceTitle,
        name:          n.name,
        items:         items,
        paymentMethod: n.paymentMethod,
        priority:      n.priority,
        gifts:         gifts,
        metadata:      meta,
        // buyerID 留空，由每次下单填入
    }
}

func (n *OrderTemplate) SubmitClone(buyer string, vars map[string]string) error {
    cloned := n.Clone().(*OrderTemplate)
    cloned.buyerID = buyer
    cloned.items = materializeItems(n.items, vars)
    return submit(cloned)
}
```

### 原型注册表（常见变体）

多套订单模板在 `init` 或启动时注册，提交时按名克隆：

```go
type OrderTemplateRegistry struct {
    protos map[string]OrderPrototype
}

func NewOrderTemplateRegistry() *OrderTemplateRegistry {
    return &OrderTemplateRegistry{protos: make(map[string]OrderPrototype)}
}

func (r *OrderTemplateRegistry) Register(name string, p OrderPrototype) {
    r.protos[name] = p
}

func (r *OrderTemplateRegistry) Clone(name string) (OrderPrototype, error) {
    p, ok := r.protos[name]
    if !ok {
        return nil, fmt.Errorf("prototype: unknown template %q", name)
    }
    return p.Clone(), nil
}

// 启动时：用生成器造好「母版」，注册为原型
func bootstrapTemplates(reg *OrderTemplateRegistry) error {
    orderShipped, err := NewOrderBuilder().
        Name("复购套餐").
        AddItem("sku-1001", 1).
        PaymentMethod("alipay").
        Priority(1).
        Gift("coupon-10").
        Metadata("template_id", "bundle_reorder_v2").
        Build()
    if err != nil {
        return err
    }
    reg.Register("bundle_reorder", orderShipped)
    // …注册 flash_sale_bundle、membership_renewal 等
    return nil
}
```

### 使用者

业务 **不** 再手工复制字段，也不重复跑构建流程：

```go
func submitBundleReorder(reg *OrderTemplateRegistry, buyer, sku string) error {
    proto, err := reg.Clone("bundle_reorder")
    if err != nil {
        return err
    }
    return proto.SubmitClone(buyer, map[string]string{"SKU": sku})
}
```

与「每次 New + 手抄字段」对比：

| | 手工 field 复制 | 原型 |
| :--- | :--- | :--- |
| 正确性 | 易浅拷贝、漏字段 | `Clone()` 一处维护深拷贝 |
| 性能 | 重复解析 / 构建 | 母版只初始化一次 |
| 扩展 | 新订单模板改多处提交逻辑 | 注册新原型，提交仍 `Clone(name)` |
| 可读性 | 提交函数里堆赋值 | 「从哪份订单模板来」语义清晰 |


## 适用场景

1. **创建成本高于复制**：订单模板解析、网络拉取默认配置、复杂校验只在母版做一次。
2. **大量相似对象、仅少量字段不同**：批量订单、多买家同一订单明细、游戏实体刷怪（同 archetype 不同坐标）。
3. **调用方不应依赖具体类名**：依赖 `OrderPrototype`，具体订单模板在注册表或配置里切换。
4. **需要保存 / 恢复对象快照**：撤销栈、配置快照、试验性分支（克隆后尝试，失败丢弃）。
5. **与 [生成器](/cs-fundamentals/design-patterns/builder) 分工**：生成器负责 **造母版**；原型负责 **运行时复制母版**。

常见例子：文档订单模板引擎、GUI 控件复制粘贴、数据库记录「另存为」、配置 Profile 复制。

**不必强行使用**：对象很轻、每次字段组合差异大、或克隆比 `New` 更贵（母版极大而只改一个 int）——直接 `NewXxx()` 或生成器更简单。没有共享可变状态时，浅拷贝即可，也不必上完整原型层次。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **绕过重复构造** | 昂贵初始化摊到母版，运行时只复制 |
| **少写创建类层次** | 不必为每种变体子类化工厂 |
| **克隆语义集中** | 深拷贝规则只在 `Clone()` 维护 |
| **运行时换订单模板** | 注册表改 key 或热更新母版（注意并发） |
| **与配置结合自然** | 「订单模板名 → 原型」映射贴近运营心智 |

| 缺点 | 说明 |
| :--- | :--- |
| **深拷贝要自己写** | Go 无内置克隆；嵌套指针、循环引用要仔细处理 |
| **母版被误改则全盘受影响** | 注册表中的实例必须视为只读，或每次 `Clone` 前从不可变存储取 |
| **循环引用难克隆** | 图结构需 visited 表或重新设计所有权 |
| **调试链略长** | 「这份从哪个原型来」要跳注册表或接口 |
| **与序列化拷贝混淆** | `json` 往返能深拷贝，但有性能与类型损失，不宜当默认方案 |

## 实践

### 深拷贝与浅拷贝

| 字段类型 | 赋值 `b = a` | 安全 `Clone` 常见做法 |
| :--- | :--- | :--- |
| 值类型（`int`、`string`） | 已拷贝 | 直接复制 |
| 切片 | 共享底层数组 | `append([]T(nil), s...)` |
| map | 共享 | `make` 后逐 key 复制 |
| 指针 / 接口 | 共享指向对象 | 视情况递归 `Clone` 或不可变共享 |

```go
// 反例：Clone 里仍共享 map
func (n *OrderTemplate) BadClone() *OrderTemplate {
    cp := *n
    cp.metadata = n.metadata // 浅拷贝——危险
    return &cp
}
```

单元测试应覆盖：**改克隆体不影响母版**（尤其切片、map）。

### 母版只读

注册进 `OrderTemplateRegistry` 的实例，提交路径 **只读**；所有变更在 `Clone()` 返回的副本上进行：

```go
func (r *OrderTemplateRegistry) Clone(name string) (OrderPrototype, error) {
    p, ok := r.protos[name]
    if !ok {
        return nil, fmt.Errorf("prototype: unknown template %q", name)
    }
    // 永远 Clone，不把母版指针交给调用方去改
    return p.Clone(), nil
}
```

若运营要「热更新订单模板」，用 **替换注册表条目**（新 `Build()` 后 `Register`），而不是原地改已注册 struct 的字段。

## 关联

- 许多设计在初期会先用 [工厂方法模式](/cs-fundamentals/design-patterns/factory)（较简单，也便于通过子类定制），随后再演化为 [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory)原型模式或 [生成器模式](/cs-fundamentals/design-patterns/builder)（更灵活，也更复杂）。
- [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory) 通常基于一组工厂方法，但你也可以使用原型模式来生成这些产品。
- 原型模式可用于保存 [命令模式](/cs-fundamentals/design-patterns/command) 的历史记录。
- 大量使用 [组合模式](/cs-fundamentals/design-patterns/composite) 和 [装饰模式](/cs-fundamentals/design-patterns/decorator) 的设计通常可从原型的使用中获益——你可以通过该模式来复制复杂结构，而非从零开始重新构造。
- 原型并不基于继承，因此没有继承的缺点；另一方面，原型需要对被复制对象进行复杂的初始化。[工厂方法模式](/cs-fundamentals/design-patterns/factory) 基于继承，但不需要这些初始化步骤。
- 有时候原型模式可以作为 [备忘录模式](/cs-fundamentals/design-patterns/memento) 的简化版本——条件是你要在历史记录中存储的对象状态比较简单，不需要链接其他外部资源，或者链接可以方便地重建。
- [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory)、[生成器模式](/cs-fundamentals/design-patterns/builder) 和原型模式都可以用 [单例模式](/cs-fundamentals/design-patterns/singleton) 来实现。

## 参考阅读

- [x] [Refactoring.Guru - 原型模式](https://refactoringguru.cn/design-patterns/prototype) (2026-06-17)
- [x] [菜鸟教程 - 原型模式](https://www.runoob.com/design-pattern/prototype-pattern.html) (2026-06-17)
