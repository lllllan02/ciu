---
title: 备忘录模式
order: 17
---

**备忘录模式** 在不破坏封装的前提下，捕获一个对象的内部状态，并在该对象之外保存这个状态，以便以后可以将该对象恢复到原先的状态。

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

定义 **Memento**（对 Caretaker 窄接口）；**Originator** 负责 **从当前状态创建 Memento** 与 **从 Memento 恢复**；**Caretaker** 只存 **`[]Memento` 或栈**。

### 备忘录（Memento）——对外 opaque

Go 用 **非导出 struct + 同包 Originator** 或 **导出但零字段访问** 的句柄：

```go
// 包 orderdraft：Caretaker 只能持有 *Snapshot，不能读字段
type Snapshot struct {
    id        string
    createdAt time.Time
    label     string
    // 以下字段小写——仅 Originator 同包可构造/读取
    lines     []OrderLine
    address   Address
    coupon    string
    note      string
    version   int
}

// 对 Caretaker 暴露的窄接口：只有元数据
type Memento interface {
    ID() string
    CreatedAt() time.Time
    Label() string
}

func (s *Snapshot) ID() string           { return s.id }
func (s *Snapshot) CreatedAt() time.Time { return s.createdAt }
func (s *Snapshot) Label() string        { return s.label }
```

若 Memento 需 **跨包** 传递，可用 **导出 struct + 未导出字段**（同 module 内 Originator 仍可访问）或 **序列化 bytes** 由 Originator **独家 Deserialize**。

### 发起人（Originator）——OrderDraft

```go
type OrderDraft struct {
    id      string
    lines   []OrderLine
    address Address
    coupon  string
    note    string
    version int
}

func (d *OrderDraft) Save(label string) Memento {
    return &Snapshot{
        id:        uuid.NewString(),
        createdAt: time.Now(),
        label:     label,
        lines:     d.deepCopyLines(), // Originator 知道如何深拷贝组合树
        address:   d.address,
        coupon:    d.coupon,
        note:      d.note,
        version:   d.version,
    }
}

func (d *OrderDraft) Restore(m Memento) error {
    snap, ok := m.(*Snapshot)
    if !ok {
        return ErrInvalidMemento
    }
    if snap.version > d.version+1 {
        return ErrSnapshotTooNew // 可选：防并发乱序
    }
    d.lines = cloneLines(snap.lines)
    d.address = snap.address
    d.coupon = snap.coupon
    d.note = snap.note
    d.version = snap.version
    return d.validateInvariants()
}

func cloneLines(src []OrderLine) []OrderLine {
    // 与 Composite 树一致：递归 Clone OrderLine
    out := make([]OrderLine, len(src))
    copy(out, src)
    return out
}

func (d *OrderDraft) deepCopyLines() []OrderLine {
    return cloneLines(d.lines)
}

func (d *OrderDraft) validateInvariants() error {
    if len(d.lines) == 0 {
        return ErrEmptyDraft
    }
    return nil
}
```

**深拷贝与校验** 只在 Originator；Caretaker **never** `snap.lines = ...`。

### 保管者（Caretaker）——历史栈

```go
type History struct {
    stack []Memento
    limit int
}

func NewHistory(limit int) *History {
    return &History{limit: limit}
}

func (h *History) Push(m Memento) {
    h.stack = append(h.stack, m)
    if h.limit > 0 && len(h.stack) > h.limit {
        h.stack = h.stack[1:]
    }
}

func (h *History) Pop() (Memento, error) {
    if len(h.stack) == 0 {
        return nil, ErrNothingToRestore
    }
    n := len(h.stack) - 1
    m := h.stack[n]
    h.stack = h.stack[:n]
    return m, nil
}

func (h *History) List() []Memento {
    out := make([]Memento, len(h.stack))
    copy(out, h.stack)
    return out // UI 只显示 Label / CreatedAt
}
```

Caretaker **不负责** Restore——只 **把 Memento 还给** Originator：

```go
m, err := history.Pop()
if err != nil {
    return err
}
return draft.Restore(m)
```

### 客户端——运营编辑会话

```go
type EditSession struct {
    draft   *OrderDraft
    history *History
}

func (s *EditSession) BeginEdit() {
    s.history.Push(s.draft.Save("before_edit"))
}

func (s *EditSession) UndoCheckpoint() error {
    m, err := s.history.Pop()
    if err != nil {
        return err
    }
    return s.draft.Restore(m)
}

func (s *EditSession) AutoSave(label string) {
    s.history.Push(s.draft.Save(label))
}
```

用户 **改地址、改券、改明细** 可仍走 [命令](/cs-fundamentals/design-patterns/command) **逐步落库**；**会话级回滚** 调 `UndoCheckpoint()` **整包恢复**。


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

> **阅读提示**：先掌握「**Originator Save/Restore，Caretaker 只存 opaque**」即可。本节是工程变体；初学可先跳过。

### 与命令模式组合

```text
EditSession
  BeginEdit → history.Push(draft.Save())
  每步操作 → invoker.Run(AdjustQuantityCommand)  // 命令：单步 Undo
  「还原检查点」→ draft.Restore(history.Pop())    // 备忘录：整包状态
  Submit → facade.PlaceOrder(...)
```

**命令** 管 **可审计的单步**；**备忘录** 管 **会话检查点**——产品「撤销上一步」vs「回到自动保存」。

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

### 与中介者、组合一起用

```go
// 结算页 Mediator 持有 CheckoutContext
func (m *CheckoutMediator) SaveCheckpoint(label string) Memento {
    return m.origin.Save(label) // Originator = 可序列化的 Draft
}

// 恢复后 Refresh 各 Colleague
func (m *CheckoutMediator) RestoreCheckpoint(mem Memento) error {
    if err := m.origin.Restore(mem); err != nil {
        return err
    }
    return m.refreshAll(context.Background())
}
```

[组合](/cs-fundamentals/design-patterns/composite) 明细树 **深拷贝** 在 `OrderDraft.deepCopyLines`；[中介者](/cs-fundamentals/design-patterns/mediator) **Restore 后** 统一 `Refresh` 面板。

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

### 测试策略

```go
func TestOrderDraft_RestorePreservesLines(t *testing.T) {
    d := &OrderDraft{lines: []OrderLine{ProductLine{SKU: "a", Quantity: 2}}}
    mem := d.Save("t0")
    d.lines[0] = ProductLine{SKU: "a", Quantity: 99}
    if err := d.Restore(mem); err != nil {
        t.Fatal(err)
    }
    if pl := d.lines[0].(ProductLine); pl.Quantity != 2 {
        t.Fatal("expected qty 2")
    }
}

func TestHistory_CannotMutateSnapshot(t *testing.T) {
    d := &OrderDraft{/* … */}
    h := NewHistory(10)
    h.Push(d.Save("x"))
    mem, _ := h.Pop()
    // 包外测试：Memento 接口无 Lines 字段——编译期无法篡改
    _ = d.Restore(mem)
}
```

## 小结

记住这四点即可：

1. **快照 opaque**：Caretaker **只存** `Memento`；**读写字段** 仅 Originator。
2. **Save / Restore 成对**：深拷贝、校验 **集中在 Originator**。
3. **与 Command 分层**：命令 **单步 Undo**；备忘录 **检查点 / 整包恢复**。
4. **成本要管**：`History.limit`、持久化 **encode 在包内**、大订单 **增量 Memento**。

[命令模式](/cs-fundamentals/design-patterns/command) 解决了 **「把操作变成可撤销对象」**；备忘录解决了 **「在不泄露内部结构的前提下，保存并恢复任意复杂状态」**——让运营草稿、结算页自动保存与崩溃恢复 **在封装边界内** 安全演进，并符合 [单一职责](/cs-fundamentals/design-patterns#设计原则)（Caretaker 只管保管，Originator 管状态语义）。

## 参考阅读

- [x] [Refactoring.Guru - 备忘录模式](https://refactoringguru.cn/design-patterns/memento) (2026-06-22)
- [x] [菜鸟教程 - 备忘录模式](https://www.runoob.com/design-pattern/memento-pattern.html) (2026-06-22)
