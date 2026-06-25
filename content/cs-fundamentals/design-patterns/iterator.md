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

定义 **迭代器** 接口；**聚合** 提供 `Iterator()`；Client **只调** `HasNext` / `Next`，不碰底层是切片还是 SQL。

### 迭代器与聚合

```go
type Iterator[T any] interface {
    HasNext() bool
    Next() (T, error)
}

type OrderFilter struct {
    Status string
    Limit  int
}

type OrderRepository interface {
    Iterator(ctx context.Context, f OrderFilter) Iterator[Order]
}
```

### 具体迭代器

```go
type orderIterator struct {
    orders []Order
    filter OrderFilter
    idx    int
    count  int
    next   Order
}

func (it *orderIterator) HasNext() bool {
    for it.idx < len(it.orders) && it.count < it.filter.Limit {
        o := it.orders[it.idx]
        it.idx++
        if it.filter.Status != "" && o.Status != it.filter.Status {
            continue
        }
        it.next = o
        it.count++
        return true
    }
    return false
}

func (it *orderIterator) Next() (Order, error) {
    return it.next, nil
}
```

内存版在 `Iterator()` 里返回 `&orderIterator{...}`；DB 版内层换成 `*sql.Rows` 或游标，**接口不变**。

### 客户端

```go
func reportPaidOrders(ctx context.Context, repo OrderRepository) ([]OrderSummary, error) {
    it := repo.Iterator(ctx, OrderFilter{Status: "paid", Limit: 50})
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
```

订单明细树（扁平化 bundle 内 SKU）可复用 [组合模式](/cs-fundamentals/design-patterns/composite) 的 `Walk`；多种遍历策略见 **实践** 一节。


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
| **多种策略并存** | 同一 Repository 可换过滤条件；明细树用 `Walk` 或独立 Iterator |
| **支持懒加载与分页** | Iterator 按需 `Next`，控制内存 |
| **单一职责** | 遍历算法 **不在** 业务 Service 里复制 |

| 缺点 | 说明 |
| :--- | :--- |
| **抽象层增加** | 简单列表可能 **过度** |
| **并发修改** | 遍历中改集合需 **约定**（快照或 fail-fast） |
| **与语言特性重叠** | Go `range`、`iter.Seq` 已很强；显式 Iterator 为 **换存储 / 多策略** 服务 |
| **错误处理** | `Next() (T, error)` 需区分 **Done vs 失败** |
| **组合过滤 Iterator 链** | 多层装饰式 Iterator 调试时要 **命名清晰** |

## 关联

- 你可以使用迭代器来遍历 [组合模式](/cs-fundamentals/design-patterns/composite) 的树。
- 你可以同时使用 [工厂方法模式](/cs-fundamentals/design-patterns/factory) 和迭代器来让集合子类返回不同类型的迭代器，并使迭代器与集合相匹配。
- 你可以同时使用 [备忘录模式](/cs-fundamentals/design-patterns/memento) 和迭代器来获取当前迭代器的状态，并在需要时进行回滚。
- 你可以同时使用 [访问者模式](/cs-fundamentals/design-patterns/visitor) 和迭代器来遍历复杂的数据结构，并对其元素进行特定操作，即使这些元素属于完全不同的类。

## 参考阅读

- [x] [Refactoring.Guru - 迭代器模式](https://refactoringguru.cn/design-patterns/iterator) (2026-06-22)
- [x] [菜鸟教程 - 迭代器模式](https://www.runoob.com/design-pattern/iterator-pattern.html) (2026-06-22)
