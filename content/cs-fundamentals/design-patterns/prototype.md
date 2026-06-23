---
title: 原型模式
order: 4
---

**原型模式** 通过 **复制已有实例** 来创建新对象，使 **克隆过程与使用过程分离**：调用方从原型拿到拷贝，再按需改少量字段，不必重复昂贵的初始化。

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

## 意图

用一句话说：**用原型实例指定创建对象的种类，并通过复制该原型创建新对象。**

把「如何得到一份安全、独立的拷贝」封装进原型自身的 `Clone()`（或注册表统一克隆）；调用方只依赖原型接口，按名或按引用取克隆体，再改买家、变量等差异字段即可。

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

## 组装实践

> **阅读提示**：先掌握「`Clone()` + 改差异字段」即可。本节是 Go 项目里的实现细节；初学可先跳过。

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

### 与生成器组合

[生成器](/cs-fundamentals/design-patterns/builder) 适合 **启动时** 拼出复杂母版；[原型](/cs-fundamentals/design-patterns/prototype) 适合 **运行时** 复制：

```go
// 启动：生成器 → 注册原型
mother, _ := NewOrderBuilder().
    Name("…").AddItem("sku-1001", 1).PaymentMethod("alipay").Build()
reg.Register("starter_bundle", mother)

// 运行时：克隆 → 填差异
p, _ := reg.Clone("starter_bundle")
_ = p.SubmitClone("user@example.com", nil)
```

不必二选一：母版用 Builder，副本用 Prototype。

### 与工厂方法的区别

[工厂方法](/cs-fundamentals/design-patterns/factory) 回答 **「造哪一种 PaymentProcessor」**；原型回答 **「从哪份已有 OrderTemplate 母版复制」**。支付渠道选型仍在组装层 `NewAlipayProcessor()`；订单模板复制在业务提交路径 `Clone("bundle_reorder")`。

### `json.Marshal` / `Unmarshal` 当克隆？

偶尔用于 **快速深拷贝 DTO**（结构简单、无 `time.Time` 精度等特殊需求）：

```go
func cloneViaJSON[T any](src T) (T, error) {
    var dst T
    b, err := json.Marshal(src)
    if err != nil {
        return dst, err
    }
    err = json.Unmarshal(b, &dst)
    return dst, err
}
```

生产路径更推荐 **显式 `Clone()`**：性能可预期、类型安全、不依赖 JSON tag。原型模式的价值正在于 **把拷贝规则写在类型旁边**，而不是藏在一行魔法序列化里。

## 小结

记住这四点即可：

1. **已有「对的」实例、且复制比重建划算 → 考虑原型**：`Clone()` 封装深拷贝，提交侧只改差异字段。
2. **母版只构建一次、注册后视为只读**：避免污染注册表里的原型。
3. **与工厂 / 生成器分工不同**：工厂选型、生成器拼母版、原型在运行时复制母版。
4. **Go 无内置克隆**：`Clone()` 的质量决定模式是否成立；切片、map、指针要逐项处理。

上一篇的 [生成器模式](/cs-fundamentals/design-patterns/builder) 管 **单个复杂对象分步构建**；本篇管 **从已有实例安全复制**。字段从零拼 → 生成器；同一订单模板提交一千次 → 原型。克隆出的订单最终还要交给统一基础设施发出去；下一篇 [单例模式](/cs-fundamentals/design-patterns/singleton) 讨论 **结算中心、限流器、连接池为何可能需要进程内唯一**。

## 参考阅读

- [x] [生成器模式](/cs-fundamentals/design-patterns/builder) — 创建型模式前置，常用于构建母版
- [x] [Refactoring.Guru - 原型模式](https://refactoringguru.cn/design-patterns/prototype) (2026-06-17)
- [x] [菜鸟教程 - 原型模式](https://www.runoob.com/design-pattern/prototype-pattern.html) (2026-06-17)
