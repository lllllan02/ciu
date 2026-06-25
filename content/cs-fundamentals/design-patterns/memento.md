---
title: 备忘录模式
order: 17
---

**备忘录模式**（Memento）亦称 **快照**，在不破坏封装的前提下，捕获一个对象的内部状态，并在该对象之外保存这个状态，以便以后可以将该对象恢复到原先的状态。

通俗地说，需要「先保存、后还原」时，对象自己负责生成和恢复快照，外面只负责存取，不去动内部细节；这样既能在多步操作后回到某个检查点，又不破坏封装。

## 问题

运营编辑未支付订单时，可能要 **多步试改再一键回到某个检查点**；代客下单草稿还要 **自动存档、崩溃恢复**。需要 **保存和恢复复杂对象状态**，但状态结构应该 **封装在对象内部**。

最直接的做法是让 Caretaker **直接序列化整个 struct**，或 Handler **手写还原逻辑**。字段少时还能应付；明细树嵌套深了，问题就会一起暴露：

1. **封装被破坏**：History 持有完整 `OrderDraft`，UI 层可直接改明细而不经领域方法。
2. **快照不一致**：浅拷贝导致明细树共享；深拷贝逻辑散落在 Caretaker 外。
3. **恢复漏字段**：手写 Revert 只还原部分字段，券与行级分摊的不变量被破坏。
4. **与命令 Undo 分工不清**：单步改数量用 Command 够用；连续十步混改后「回到编辑开始前」需要 **全量快照**。

本质矛盾是：**需要保存/恢复复杂状态**，但 Caretaker 只应 **像存文件一样存 opaque 快照**，不应 **知道明细怎么深拷贝、怎么校验恢复**。典型写法如下：

```go
type DraftHistory struct {
    snapshots []OrderDraft // Caretaker 能改任意字段；浅拷贝共享 Lines
}

func (h *DraftHistory) Save(d OrderDraft) {
    h.snapshots = append(h.snapshots, d)
}
```

## 解决方案

定义 **Memento**（对 Caretaker 只暴露元数据）；**Originator** 负责 `Save` / `Restore`；**Caretaker** 只存 `[]Memento`，**不** 读写字段。

### 三种角色

```go
type Memento interface {
    ID() string
    Label() string
}

type Snapshot struct {
    id, label string
    lines     []OrderLine // 小写：仅 Originator 同包可读写
    address   Address
    coupon    string
}

type OrderDraft struct {
    lines   []OrderLine
    address Address
    coupon  string
}

func (d *OrderDraft) Save(label string) Memento {
    return &Snapshot{
        id: uuid.NewString(), label: label,
        lines: d.deepCopyLines(), address: d.address, coupon: d.coupon,
    }
}

func (d *OrderDraft) Restore(m Memento) error {
    snap := m.(*Snapshot)
    d.lines = cloneLines(snap.lines)
    d.address, d.coupon = snap.address, snap.coupon
    return d.validateInvariants()
}
```

深拷贝与校验 **只在 Originator**；Caretaker 拿到的 `Memento` **看不到** `lines` / `coupon`。

### Caretaker 与客户端

```go
type History struct {
    stack []Memento
}

func (h *History) Push(m Memento) { h.stack = append(h.stack, m) }

func (h *History) Pop() (Memento, error) {
    if len(h.stack) == 0 {
        return nil, ErrNothingToRestore
    }
    m := h.stack[len(h.stack)-1]
    h.stack = h.stack[:len(h.stack)-1]
    return m, nil
}

// 开始编辑前存检查点；恢复时 Pop 再 Restore
history.Push(draft.Save("before_edit"))
m, _ := history.Pop()
_ = draft.Restore(m)
```

单步改数量仍可用 [命令](/cs-fundamentals/design-patterns/command) `Undo`；**整包回到检查点** 才用备忘录。持久化、增量快照、与 Command 组合见 **实践** 一节。


## 适用场景

1. **需要状态快照**：草稿自动保存、编辑器检查点、游戏存档。
2. **恢复需保持封装**：Caretaker **不能** 依赖 `OrderDraft` 字段布局。
3. **多字段一致恢复**：券、明细、分摊 **必须同一时刻** 写回。
4. **命令 Undo 不够**：逆操作难定义或 **栈太深**。
5. **时间旅行 / 对比**：存多个 Label 快照供运营 **预览恢复**（只 Restore 到临时副本更佳）。

**不必强行使用**：

- **单字段、明确逆操作**——[命令](/cs-fundamentals/design-patterns/command) `Undo` 更轻。
- **immutable 值对象**——直接 **替换引用** 即可。
- **只需复制分支**——[原型](/cs-fundamentals/design-patterns/prototype) `Clone`。
- **快照极大且频繁**——考虑 **增量 Memento** 或 **事件溯源**，不是经典全量 Memento。

常见例子：文本编辑器版本、IDE Local History、Git stash 思想、数据库 savepoint（概念相近）、表单 **还原到上次自动保存**。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **封装** | Caretaker **不依赖** Originator 内部结构 |
| **恢复一致** | Originator **一次性** 校验不变量 |
| **简化 Caretaker** | 只 **存 Memento 句柄** |
| **可叠加快照策略** | 全量 / 增量 Memento **Originator 内换** |

| 缺点 | 说明 |
| :--- | :--- |
| **内存/存储成本** | 全量快照 **贵**；需 **limit、增量** |
| **Originator 负担** | Save/Restore、深拷贝 **都在 Originator** |
| **版本兼容** | 字段演进时 **旧 Memento 迁移** 要设计 |
| **与 Command 边界** | 团队需约定 **何时快照 vs 何时命令** |
| **Go 封装技巧** | 跨包 opaque 需 **接口 + 私有实现** 或 **[]byte** |

## 实践

### 持久化 Caretaker（崩溃恢复）

```go
type DraftStore struct {
    repo SnapshotRepository // 只存 []byte + metadata
}

func (s *DraftStore) Persist(draft *OrderDraft, label string) error {
    m := draft.Save(label)
    snap := m.(*Snapshot)
    blob, err := snap.encode() // 包内序列化
    if err != nil {
        return err
    }
    return s.repo.Put(context.Background(), snap.ID(), blob, snap.Label())
}

func (s *DraftStore) Load(draft *OrderDraft, id string) error {
    blob, err := s.repo.Get(context.Background(), id)
    if err != nil {
        return err
    }
    m, err := decodeSnapshot(blob) // 仅 orderdraft 包
    if err != nil {
        return err
    }
    return draft.Restore(m)
}
```

DB 存 **bytes**；**decode** 只在 Originator 包——外部 **无法** 手改 `lines`。

### 增量备忘录（大订单）

| 策略 | 做法 | 适用 |
| :--- | :--- | :--- |
| **全量** | `deepCopyLines()` | 明细 < 几百行 |
| **增量** | Memento 只存 **changed line ids + 旧值** | 大促超大单 |
| **写时复制** | 持久化数据结构 **版本链** | 高级；接近 Event Sourcing |

增量仍由 **Originator.Save** 决定 **存什么**；Caretaker **不变**。

### Memento 版本迁移

```go
func decodeSnapshot(blob []byte) (Memento, error) {
    var wire snapshotWire
    if err := json.Unmarshal(blob, &wire); err != nil {
        return nil, err
    }
    switch wire.V {
    case 1:
        return fromV1(wire)
    case 2:
        return fromV2(wire)
    default:
        return nil, ErrUnsupportedSnapshotVersion
    }
}
```

**Originator 字段变了**——旧快照 **在 decode 层迁移**，不交给 Caretaker。

## 关联

- 你可以同时使用 [命令模式](/cs-fundamentals/design-patterns/command) 和备忘录模式来实现「撤销」。在这种情况下，命令用于对目标对象执行各种不同的操作，备忘录用来保存一条命令执行前该对象的状态。
- 你可以同时使用备忘录模式和 [迭代器模式](/cs-fundamentals/design-patterns/iterator) 来获取当前迭代器的状态，并且在需要的时候进行回滚。
- 有时候 [原型模式](/cs-fundamentals/design-patterns/prototype) 可以作为备忘录模式的一个简化版本，其条件是你需要在历史记录中存储的对象的状态比较简单，不需要链接其他外部资源，或者链接可以方便地重建。

## 参考阅读

- [x] [Refactoring.Guru - 备忘录模式](https://refactoringguru.cn/design-patterns/memento) (2026-06-22)
- [x] [菜鸟教程 - 备忘录模式](https://www.runoob.com/design-pattern/memento-pattern.html) (2026-06-22)
