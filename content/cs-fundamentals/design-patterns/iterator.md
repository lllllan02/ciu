---
title: 迭代器模式
order: 15
---

**迭代器模式**（Iterator）提供一种方法顺序访问一个聚合对象中的各个元素，而又不暴露该对象的内部表示。

通俗地说，遍历集合的方式单独封装，调用方逐个取元素即可，不必知道数据在底层是数组、链表还是分页查询；换存储或换遍历策略只改迭代器，使用它的代码不用动。

## 问题

仓储导出要 **扁平化所有叶子 SKU**，报表要 **按状态分页扫订单**，对账要 **游标式读明细而不一次加载百万行**——不同消费者需要 **不同顺序、不同过滤** 地访问同一批数据。

最直接的做法是 **暴露内部结构**（`[]OrderLine`、`[]Order`），让调用方自己写 `for` 和递归。存储不变时还能应付；从内存切片迁到 DB 分页时，问题就会一起暴露：

1. **遍历逻辑重复**：导出、对账、库存释放各自写递归，加一种明细类型要改 **每一处** walk。
2. **与存储紧耦合**：报表直接 `orders[i]` 分页；换成 DB cursor 或 ES scroll 时，**所有** 调用方都要重写。
3. **多种策略难并存**：同一棵树要「仅叶子」「含 bundle 节点」「跳过赠品」——全塞进 `Order` 的方法会 **接口膨胀**。
4. **封装被破坏**：调用方可以直接改 `order.Lines`，破坏不变量。

本质矛盾是：**多种消费者** 要以 **不同方式访问同一集合**，但 **底层存储** 会随性能需求变化；客户端不应 **绑定某一种 `for` 写法**。典型写法如下：

```go
func exportPickList(order Order) []PickItem {
    var out []PickItem
    var walk func(OrderLine) // 手写递归——Validate、Reserve 又要写一遍
    walk = func(line OrderLine) { /* switch ProductLine / BundleLine */ }
    for _, line := range order.Lines { walk(line) }
    return out
}
```

## 解决方案

定义 **迭代器** 接口；**聚合** 提供工厂方法创建迭代器；各 **具体迭代器** 持有遍历状态（栈、cursor、offset）。

### 迭代器（Iterator）接口

```go
type Iterator[T any] interface {
    HasNext() bool
    Next() (T, error)
}

// 可选：支持 early stop、资源释放
type Closer interface {
    Close() error
}
```

若用 Go 1.23+ `iter.Seq[T]`，Aggregate 可返回 **`func(yield func(T) bool)`**，调用方 `for v := range seq`——语义等价，见 [实践](#实践)。

### 聚合（Aggregate）——订单明细

```go
type OrderLines interface {
    // 默认：深度优先，含 bundle 节点
    Iterator() Iterator[OrderLine]
    // 仅叶子 SKU，供仓储拣货
    LeafSKUIterator() Iterator[PickItem]
}

type InMemoryOrder struct {
    lines []OrderLine
}

func (o *InMemoryOrder) Iterator() Iterator[OrderLine] {
    return &DepthFirstLineIterator{
        stack: append([]OrderLine(nil), o.lines...),
    }
}

func (o *InMemoryOrder) LeafSKUIterator() Iterator[PickItem] {
    return &LeafSKUIterator{
        stack: append([]OrderLine(nil), o.lines...),
    }
}
```

**不暴露** `lines` 字段给包外；测试包内可用 **构造器** 注入。

### 具体迭代器：深度优先（含组合节点）

```go
type DepthFirstLineIterator struct {
    stack []OrderLine
}

func (it *DepthFirstLineIterator) HasNext() bool {
    return len(it.stack) > 0
}

func (it *DepthFirstLineIterator) Next() (OrderLine, error) {
    if !it.HasNext() {
        return nil, ErrIteratorDone
    }
    n := len(it.stack) - 1
    line := it.stack[n]
    it.stack = it.stack[:n]
    if b, ok := line.(BundleLine); ok {
        // 子节点逆序压栈 → 深度优先正序弹出
        for i := len(b.Children) - 1; i >= 0; i-- {
            it.stack = append(it.stack, b.Children[i])
        }
    }
    return line, nil
}
```

用 **显式栈** 代替递归，避免 **深树栈溢出**；顺序与 **先根深度优先** 一致。

### 具体迭代器：仅叶子 SKU（扁平拣货）

```go
type LeafSKUIterator struct {
    stack []OrderLine
}

func (it *LeafSKUIterator) HasNext() bool {
    for len(it.stack) > 0 {
        top := it.stack[len(it.stack)-1]
        if _, ok := top.(ProductLine); ok {
            return true
        }
        it.stack = it.stack[:len(it.stack)-1]
        if b, ok := top.(BundleLine); ok {
            for i := len(b.Children) - 1; i >= 0; i-- {
                it.stack = append(it.stack, b.Children[i])
            }
        }
    }
    return false
}

func (it *LeafSKUIterator) Next() (PickItem, error) {
    if !it.HasNext() {
        return PickItem{}, ErrIteratorDone
    }
    n := len(it.stack) - 1
    line := it.stack[n].(ProductLine)
    it.stack = it.stack[:n]
    return PickItem{SKU: line.SKU, Qty: line.Quantity}, nil
}
```

仓储 **`exportPickList`** 改为：

```go
func exportPickList(order OrderLines) []PickItem {
    var out []PickItem
    it := order.LeafSKUIterator()
    for it.HasNext() {
        item, _ := it.Next()
        out = append(out, item)
    }
    return out
}
```

**不再** 复制 `walk` 递归；新增明细类型时 **只改 Iterator**（或 Composite + Iterator 各管一层）。

### 聚合（Aggregate）——订单仓储与过滤迭代器

```go
type OrderFilter struct {
    Status string
    Page   Page
}

type Page struct {
    Offset int
    Limit  int
}

type OrderRepository interface {
    Iterator(ctx context.Context, f OrderFilter) Iterator[Order]
}

// 内存实现：演示过滤 + 分页
type InMemoryOrderRepo struct {
    orders []Order
}

func (r *InMemoryOrderRepo) Iterator(ctx context.Context, f OrderFilter) Iterator[Order] {
    return &FilteredOrderIterator{
        ctx: ctx, src: r.orders, filter: f, cursor: f.Page.Offset,
    }
}

type FilteredOrderIterator struct {
    ctx    context.Context
    src    []Order
    filter OrderFilter
    cursor int
    count  int
    ready  Order
    hasReady bool
}

func (it *FilteredOrderIterator) advance() bool {
    for it.cursor < len(it.src) && it.count < it.filter.Page.Limit {
        o := it.src[it.cursor]
        it.cursor++
        if it.filter.Status != "" && o.Status != it.filter.Status {
            continue
        }
        // 跳过 Page.Offset 之前的匹配项
        if it.filter.Page.Offset > 0 {
            it.filter.Page.Offset--
            continue
        }
        it.ready, it.hasReady = o, true
        it.count++
        return true
    }
    it.hasReady = false
    return false
}

func (it *FilteredOrderIterator) HasNext() bool {
    if it.hasReady {
        return true
    }
    return it.advance()
}

func (it *FilteredOrderIterator) Next() (Order, error) {
    if !it.HasNext() {
        return Order{}, ErrIteratorDone
    }
    o := it.ready
    it.hasReady = false
    return o, nil
}
```

DB 实现可把 **`FilteredOrderIterator`** 换成 **持有 `*sql.Rows` 或 ES scroll id**——**报表 Client 代码不变**。

### 客户端（Client）——报表与批处理

```go
func reportPaidOrders(ctx context.Context, repo OrderRepository, page, size int) ([]OrderSummary, error) {
    it := repo.Iterator(ctx, OrderFilter{
        Status: "paid",
        Page:   Page{Offset: page * size, Limit: size},
    })
    var sums []OrderSummary
    for it.HasNext() {
        order, err := it.Next()
        if err != nil {
            return nil, err
        }
        sums = append(sums, summarize(order))
    }
    return sums, nil
}

// 与命令模式：批量关单
func enqueueCancelPaidExpired(ctx context.Context, repo OrderRepository, q *CommandQueue) error {
    it := repo.Iterator(ctx, OrderFilter{Status: "paid_expired"})
    for it.HasNext() {
        order, err := it.Next()
        if err != nil {
            return err
        }
        q.Enqueue(NewCancelOrderCommand(ordersSvc, order.ID))
    }
    return nil
}
```

Client **只依赖** `Iterator[Order]`，不依赖 `[]Order`。


## 适用场景

1. **隐藏集合内部表示**：切片、链表、树、DB 游标、RPC 流 **统一访问**。
2. **同一聚合多种遍历**：全树 / 仅叶子 / 跳过赠品 / 逆序。
3. **分页与流式处理**：大促对账、导出 **不把百万行一次载入内存**。
4. **过滤与懒计算**：`FilterPaidIterator` **包装** 内层 Iterator，符合 [开闭](/cs-fundamentals/design-patterns#设计原则)。
5. **与组合树配合**：Composite 表达结构；Iterator 表达 **扁平或定制顺序的访问**。
6. **批处理管道**：`for` Iterator + [命令](/cs-fundamentals/design-patterns/command) / 领域服务 **解耦「取下一个」与「处理」**。

**不必强行使用**：

- **固定小 slice、单一 `range`**——直接 `for _, line := range order.Lines`。
- **只需聚合结果**（总价）——[组合](/cs-fundamentals/design-patterns/composite) 的 `Total()` 足够，不必先 Iterator 再求和。
- **操作类型很多、节点类稳定**——考虑 **访问者** 而非多个 Iterator 各拷一份遍历。
- **并发修改集合**——Iterator 需 **快照 / 版本号 / 只读副本**；否则 `range` 同样不安全。

常见例子：`java.util.Iterator`、C++ STL 迭代器、Go `database/sql.Rows`、ES scroll、Kubernetes `ListWatch` 分页、文件目录 walk 的抽象。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **解耦客户端与聚合** | 报表不依赖 `[]Order` 或 SQL |
| **开闭** | 新遍历 = 新 ConcreteIterator |
| **多种策略并存** | 同一 `OrderLines` 上 `LeafSKUIterator` vs `DepthFirstLineIterator` |
| **支持懒加载与分页** | Iterator 按需 `Next`，控制内存 |
| **单一职责** | 遍历算法 **不在** 业务 Service 里复制 |

| 缺点 | 说明 |
| :--- | :--- |
| **抽象层增加** | 简单列表可能 **过度** |
| **并发修改** | 遍历中改集合需 **约定**（快照或 fail-fast） |
| **与语言特性重叠** | Go `range`、`iter.Seq` 已很强；显式 Iterator 为 **换存储 / 多策略** 服务 |
| **错误处理** | `Next() (T, error)` 需区分 **Done vs 失败** |
| **组合过滤 Iterator 链** | 多层装饰式 Iterator 调试时要 **命名清晰** |

## 实践

> **阅读提示**：先掌握「**Aggregate 提供 Iterator，Client 只 Next**」即可。本节是工程变体；初学可先跳过。

### Go 1.23+ `iter.Seq` 薄封装

```go
func (o *InMemoryOrder) AllLeafSKUs() iter.Seq[PickItem] {
    return func(yield func(PickItem) bool) {
        it := o.LeafSKUIterator()
        for it.HasNext() {
            item, err := it.Next()
            if err != nil {
                return
            }
            if !yield(item) {
                return
            }
        }
    }
}

// Client
for item := range order.AllLeafSKUs() {
    pickList = append(pickList, item)
}
```

**early stop**：`yield` 返回 `false` 即停——比经典 Iterator 的 **Break** 更 Go 惯用。

### 装饰式过滤 Iterator

```go
type FilterIterator[T any] struct {
    inner Iterator[T]
    pred  func(T) bool
    next  T
    ok    bool
}

func (f *FilterIterator[T]) HasNext() bool {
    for f.inner.HasNext() {
        v, err := f.inner.Next()
        if err != nil {
            return false
        }
        if f.pred(v) {
            f.next, f.ok = v, true
            return true
        }
    }
    return false
}

func (f *FilterIterator[T]) Next() (T, error) {
    if !f.ok && !f.HasNext() {
        var zero T
        return zero, ErrIteratorDone
    }
    f.ok = false
    return f.next, nil
}

// 在「已支付」Iterator 外再包「金额 > 1000」
it := &FilterIterator[Order]{
    inner: repo.Iterator(ctx, OrderFilter{Status: "paid"}),
    pred:  func(o Order) bool { return o.Total > 1000 },
}
```

**开闭**：新条件 **新 pred 或新 FilterIterator**，不改 `FilteredOrderIterator` 源码。

### 与组合、命令、外观一起用

```text
OrderLines（Composite 树）
  → LeafSKUIterator → WMS 导出
  → DepthFirstLineIterator → 审计日志

OrderRepository.Iterator
  → ReportService 分页
  → CommandQueue Worker 批量 CancelOrderCommand

CheckoutFacade.PlaceOrder
  → 内部可能对 Lines 做 Validate（Composite）
  → 落库后 SearchIndexer 用 OrderLineIterator 扇出 SKU
```

[组合](/cs-fundamentals/design-patterns/composite) 管 **节点行为**；迭代器管 **访问顺序与暴露边界**；[命令](/cs-fundamentals/design-patterns/command) 管 **对 Iterator 取出的每个 Order 做什么**。

### 并发与快照

| 场景 | 做法 |
| :--- | :--- |
| 遍历中可能改购物车 | Iterator 构造时 **拷贝 slice 头** 或 **版本号** 检测 |
| DB 分页 | **稳定排序键** + `WHERE id > ? LIMIT`；勿无 ORDER BY 深分页 |
| 长事务导出 | **只读副本** 或 **快照隔离**；Iterator `Close` 释放连接 |

```go
type SnapshotLineIterator struct {
    lines []OrderLine // 构造时 deep copy 或 immutable 共享
    idx   int
}
```

### 测试策略

```go
func TestLeafSKUIterator_FlattensBundle(t *testing.T) {
    order := &InMemoryOrder{lines: []OrderLine{
        BundleLine{Children: []OrderLine{
            ProductLine{SKU: "a", Quantity: 1},
            ProductLine{SKU: "b", Quantity: 2},
        }},
    }}
    var skus []string
    it := order.LeafSKUIterator()
    for it.HasNext() {
        item, _ := it.Next()
        skus = append(skus, item.SKU)
    }
    if len(skus) != 2 || skus[0] != "a" {
        t.Fatal(skus)
    }
}

func TestFilteredOrderIterator_Pagination(t *testing.T) {
    repo := &InMemoryOrderRepo{orders: []Order{
        {ID: "1", Status: "paid"},
        {ID: "2", Status: "pending"},
        {ID: "3", Status: "paid"},
    }}
    it := repo.Iterator(context.Background(), OrderFilter{
        Status: "paid", Page: Page{Offset: 0, Limit: 10},
    })
    var ids []string
    for it.HasNext() {
        o, _ := it.Next()
        ids = append(ids, o.ID)
    }
    if len(ids) != 2 {
        t.Fatal(ids)
    }
}
```

## 小结

记住这四点即可：

1. **遍历即对象**：`LeafSKUIterator`、`FilteredOrderIterator` 封装 **怎么取下一个**，Client 不碰内部 `[]OrderLine` / SQL。
2. **Aggregate 只造 Iterator**：`OrderLines.LeafSKUIterator()`、`OrderRepository.Iterator(filter)`——换存储 **改 ConcreteAggregate + ConcreteIterator**。
3. **与 Composite 分层**：Composite 的 `Total()` **递归聚合**；Iterator **按需逐个元素** 给导出、报表、批处理。
4. **Go 工程**：简单场景 `range`；多策略 / 分页 / 隐藏存储 用 **显式 Iterator 或 `iter.Seq`**；与 [命令](/cs-fundamentals/design-patterns/command) 组合做 **「遍历 + 入队」** 管道。

[组合模式](/cs-fundamentals/design-patterns/composite) 解决了 **「树形明细上操作一致」**；迭代器解决 **「访问这棵树或订单列表时，调用方不必知道怎么存、怎么扫」**——把 **遍历算法** 从业务 Service 和聚合内部 **抽到可替换的 Iterator**，让仓储迁移与多种导出视图在 [开闭](/cs-fundamentals/design-patterns#设计原则) 下演进。

## 参考阅读

- [x] [Refactoring.Guru - 迭代器模式](https://refactoringguru.cn/design-patterns/iterator) (2026-06-22)
- [x] [菜鸟教程 - 迭代器模式](https://www.runoob.com/design-pattern/iterator-pattern.html) (2026-06-22)
