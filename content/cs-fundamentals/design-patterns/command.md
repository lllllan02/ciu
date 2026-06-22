---
title: 命令模式
order: 14
---

**命令模式**（Command）把 **一次操作请求** 封装成 **独立对象**：对象里带上 **执行所需参数**，对外暴露统一的 `Execute`（以及可选的 `Undo`）；**调用方**（Invoker）只持有命令接口，不必知道 **具体改的是哪张表、调了哪个 Service 方法**——从而把 **「谁发起」** 与 **「谁干活」** 拆开，并天然支持 **排队、日志、撤销、重做与宏命令**。

与 [责任链模式](/cs-fundamentals/design-patterns/chain-of-responsibility) 的 **分工** 很常被问到：责任链解决 **「请求沿链传递，由某一环处理」**；命令解决 **「把单次操作变成可存储、可调度、可回滚的一等公民」**——链上的每一环仍是 **当场执行** 的 Handler，命令则是 **延迟执行、可入队、可记入历史栈** 的请求对象。与 [外观模式](/cs-fundamentals/design-patterns/facade) 也不同：[外观](/cs-fundamentals/design-patterns/facade) 编排 **一整条用例**（`PlaceOrder`）；命令封装 **原子或组合后的单步操作**（`AdjustLineQuantity`、`CancelOrder`），便于 **撤销一步** 而不是回滚整条 Facade 流程。

下文继续用「电商订单系统」：[外观](/cs-fundamentals/design-patterns/facade) 已提供下单编排；[责任链](/cs-fundamentals/design-patterns/chain-of-responsibility) 已处理下单前校验与退款审批。当 **运营后台** 要改地址、改数量并 **撤销**；**大促结束** 要 **批量关单** 且可 **重试**；**离线 App** 要把操作 **先入队、联网后再执行**；审计要求 **每条人工改动可回放** 时，若 Controller 直接调 `OrderService.UpdateAddress`，会出现 **无法 Undo、无法排队、调用方与领域方法紧耦合**——命令把 **操作本身** 对象化。

## 问题

运营工具与客服工单开始直接调用 `OrderService`：

```go
func (h *AdminHandler) AdjustQuantity(w http.ResponseWriter, r *http.Request) {
    orderID := r.URL.Query().Get("order_id")
    sku := r.URL.Query().Get("sku")
    qty, _ := strconv.Atoi(r.URL.Query().Get("qty"))
    if err := h.orders.SetLineQuantity(r.Context(), orderID, sku, qty); err != nil {
        http.Error(w, err.Error(), 500)
        return
    }
    // 产品又要「撤销上一次改数量」——这里没有记录旧值，也没统一 Undo
}

func (s *FlashSaleRollback) RollbackOversold(ctx context.Context, ids []string) error {
    for _, id := range ids {
        if err := s.orders.Cancel(ctx, id); err != nil {
            return err // 中途失败：前面已取消的怎么回滚？也没法重试单条
        }
    }
    return nil
}
```

1. **调用方与接收者紧耦合**：HTTP Handler、MQ Consumer、CLI 都 **直接依赖** `OrderService` 的十几个方法；改签名要 **改所有入口**。
2. **无法撤销 / 重做**：改数量、改地址、加备注 **没有统一历史**；产品要「撤销」只能 **手写反向 SQL**，易漏字段。
3. **难以排队与重试**：离线同步、削峰关单、失败重试 需要 **把操作存下来再执行**——现在只有 **即时方法调用**。
4. **审计与回放弱**：日志里记了「谁调了 API」，但 **没有可重放的命令对象**；合规要求 **逐条回放人工操作** 时困难。
5. **宏操作难组合**：「选中 200 单批量改仓」「大促回滚取消+释放库存」若在每个脚本里 **for 循环调 Service**，失败补偿 **散落各处**。
6. **与 Facade / 责任链的分工错位**：[外观](/cs-fundamentals/design-patterns/facade) 管 **完整用例编排**；[责任链](/cs-fundamentals/design-patterns/chain-of-responsibility) 管 **可插拔校验链**——这里要解决的是 **把「改订单一行」这类操作封装成可调度、可撤销的请求对象**。

本质矛盾是：**同一类业务操作** 会从 **多种入口、多种时机** 触发（同步 HTTP、异步队列、批处理脚本），且常要 **记录、重试、撤销**；却用 **分散的直接方法调用** 表达，调用方 **认识每一个 Receiver 方法**。

### 命令 vs 简单函数 / 事件

| 方式 | 特点 | 何时够用 |
| :--- | :--- | :--- |
| **直接调 Service** | 简单、零抽象 | 一次性脚本、永不需要 Undo/队列 |
| **领域事件** | 通知「已发生什么」，常不可撤销 | 只读扇出、审计流水、跨服务广播 |
| **命令对象** | 封装「要做什么」，可 Execute/Undo/Enqueue | 需要撤销、排队、宏命令、操作日志 |

事件与命令 **可并存**：`AdjustQuantityCommand.Execute` 成功后 **再发** `OrderLineChanged` 事件通知搜索索引——命令管 **写与回滚**，事件管 **扇出**。

## 意图

用一句话说：**将一个请求封装为一个对象，从而让你可以用不同的请求对客户进行参数化，对请求排队或记录请求日志，以及支持可撤销的操作。**

引入 **命令**（Command）统一接口；**具体命令** 持有参数并委托 **接收者**（Receiver）执行；**调用者**（Invoker）只调 `command.Execute()`，可选维护 **历史栈** 做 Undo/Redo：

```go
cmd := NewAdjustQuantityCommand(orders, orderID, sku, newQty)
if err := invoker.Run(ctx, cmd); err != nil {
    return err
}
// 运营点「撤销」
if err := invoker.Undo(ctx); err != nil { ... }
```

GoF 从 **结构** 角度的定义：

> 将一个请求封装为一个对象，从而使你可用不同的请求对客户进行参数化，对请求排队或记录请求日志，以及支持可撤销的操作。

### 和责任链、外观、策略有啥不同

| | 命令 | 责任链 | 外观 | 策略 |
| :--- | :--- | :--- | :--- | :--- |
| **动机** | **封装操作为对象**；解耦 Invoker 与 Receiver | **解耦发送方与多个处理者** | **简化多子系统编排** | **互换算法** |
| **核心抽象** | `Command`（Execute/Undo） | `Handler`（Handle/转发） | `Facade`（PlaceOrder 等） | `Strategy`（Calculate 等） |
| **是否存储请求** | **是**——可入队、日志、宏命令 | 通常 **当场沿链传递** | 编排 **立即执行** | 策略对象可复用，但语义是 **算法** 不是 **操作** |
| **典型能力** | Undo、Redo、队列、批处理 | 可插拔校验、分级审批 | 固定用例流程 | 运行时换计价/支付路由规则 |
| **电商例子** | 改数量并撤销、批量关单入队 | 下单前风控链 | 预占+支付+落库 | 会员价 vs 普通价算法 |

#### 命令像「把 Facade 的一步拆成对象」吗？

**部分像，但层次不同。** Facade 的 `PlaceOrder` 是 **用例级、多子系统、难整体 Undo** 的编排；命令适合 **领域内有明确逆操作的单步**，例如 `SetLineQuantity` ↔ 恢复旧数量、`ReserveInventory` ↔ `ReleaseInventory`。实践中常见分层：

```text
PlaceOrder（Facade 用例）
  → 内部可记录 Saga 步骤；单步补偿常建模为 Command

AdjustQuantity（Command）
  → Execute 调 OrderService；Undo 写回 oldQty
```

大促 **整单取消** 若涉及支付退款、库存、通知，Undo 往往是 **另一条补偿用例**，不单靠一个 Command——但 **「取消请求」本身** 仍可封装为 `CancelOrderCommand` **入队重试**。

## 解决方案

定义 **命令接口**；各 **具体命令** 保存 Receiver 引用与参数，在 `Execute` / `Undo` 里调用 Receiver；**Invoker** 执行命令并可选维护历史。

### 命令（Command）接口

```go
type Command interface {
    Execute(ctx context.Context) error
    Undo(ctx context.Context) error
}

// 不可撤销的操作（如发不可撤回短信）可实现 NoUndo
type NoUndo struct{}

func (NoUndo) Undo(context.Context) error { return ErrUndoNotSupported }
```

若只需排队、不需 Undo，可拆成 `Executable` 接口；文档为与 GoF 对齐保留 `Undo`。

### 接收者（Receiver）——领域服务

```go
type OrderService interface {
    GetLineQuantity(ctx context.Context, orderID, sku string) (int, error)
    SetLineQuantity(ctx context.Context, orderID, sku string, qty int) error
    UpdateShippingAddress(ctx context.Context, orderID string, addr Address) error
    Cancel(ctx context.Context, orderID string) error
}
```

命令 **不替代** Receiver 的业务规则；只是把 **一次调用 + 逆操作所需状态** 包起来。

### 具体命令：改数量（可撤销）

```go
type AdjustQuantityCommand struct {
    orders  OrderService
    orderID string
    sku     string
    newQty  int
    oldQty  int // Execute 前快照，供 Undo
}

func NewAdjustQuantityCommand(orders OrderService, orderID, sku string, newQty int) *AdjustQuantityCommand {
    return &AdjustQuantityCommand{
        orders: orders, orderID: orderID, sku: sku, newQty: newQty,
    }
}

func (c *AdjustQuantityCommand) Execute(ctx context.Context) error {
    qty, err := c.orders.GetLineQuantity(ctx, c.orderID, c.sku)
    if err != nil {
        return err
    }
    c.oldQty = qty
    return c.orders.SetLineQuantity(ctx, c.orderID, c.sku, c.newQty)
}

func (c *AdjustQuantityCommand) Undo(ctx context.Context) error {
    return c.orders.SetLineQuantity(ctx, c.orderID, c.sku, c.oldQty)
}
```

`oldQty` 在 **首次 Execute** 时捕获——**不要**在构造命令时读，否则并发下可能不准。

### 具体命令：改地址

```go
type UpdateAddressCommand struct {
    orders  OrderService
    orderID string
    newAddr Address
    oldAddr Address
}

func (c *UpdateAddressCommand) Execute(ctx context.Context) error {
    // 从 orders 读 oldAddr（略）
    return c.orders.UpdateShippingAddress(ctx, c.orderID, c.newAddr)
}

func (c *UpdateAddressCommand) Undo(ctx context.Context) error {
    return c.orders.UpdateShippingAddress(ctx, c.orderID, c.oldAddr)
}
```

### 调用者（Invoker）——执行与历史栈

```go
type Invoker struct {
    history []Command
    cursor  int // 指向「已执行」栈顶；Undo 用
}

func (i *Invoker) Run(ctx context.Context, cmd Command) error {
    if err := cmd.Execute(ctx); err != nil {
        return err
    }
    // 新执行截断「重做」分支
    i.history = append(i.history[:i.cursor], cmd)
    i.cursor = len(i.history)
    return nil
}

func (i *Invoker) Undo(ctx context.Context) error {
    if i.cursor == 0 {
        return ErrNothingToUndo
    }
    i.cursor--
    return i.history[i.cursor].Undo(ctx)
}

func (i *Invoker) Redo(ctx context.Context) error {
    if i.cursor >= len(i.history) {
        return ErrNothingToRedo
    }
    cmd := i.history[i.cursor]
    if err := cmd.Execute(ctx); err != nil {
        return err
    }
    i.cursor++
    return nil
}
```

运营后台 **一个 Invoker 实例 per 编辑会话**；HTTP 无状态场景可把命令 **序列化进 DB** 再做 Undo（见 [组装实践](#组装实践)）。

### 宏命令（Composite Command）

```go
type MacroCommand struct {
    cmds []Command
}

func (m *MacroCommand) Execute(ctx context.Context) error {
    for _, c := range m.cmds {
        if err := c.Execute(ctx); err != nil {
            return err
        }
    }
    return nil
}

func (m *MacroCommand) Undo(ctx context.Context) error {
    for j := len(m.cmds) - 1; j >= 0; j-- {
        if err := m.cmds[j].Undo(ctx); err != nil {
            return err
        }
    }
    return nil
}

// 大促回滚：批量取消（每条是一个 CancelOrderCommand）
func NewFlashSaleRollback(orders OrderService, ids []string) *MacroCommand {
    cmds := make([]Command, len(ids))
    for i, id := range ids {
        cmds[i] = NewCancelOrderCommand(orders, id)
    }
    return &MacroCommand{cmds: cmds}
}
```

宏命令 **Execute 正序、Undo 逆序**；任一步失败时，若需 **部分回滚**，要在 `Execute` 内 **try/compensate** 或改用 **Saga**（命令模式作 **单步补偿单元**）。

### 客户端（Client）——Admin API

```go
type AdminHandler struct {
    invoker *Invoker
    orders  OrderService
}

func (h *AdminHandler) AdjustQuantity(w http.ResponseWriter, r *http.Request) {
    cmd := NewAdjustQuantityCommand(h.orders, orderID, sku, newQty)
    if err := h.invoker.Run(r.Context(), cmd); err != nil {
        http.Error(w, err.Error(), 500)
        return
    }
}

func (h *AdminHandler) Undo(w http.ResponseWriter, r *http.Request) {
    if err := h.invoker.Undo(r.Context()); err != nil {
        http.Error(w, err.Error(), 400)
    }
}
```

Client **只构造具体命令类型**（或用工厂）；执行路径 **统一走 Invoker**。


## 适用场景

1. **要把操作参数化**：同一 `Invoker.Run` 可执行改数量、改地址、取消单——**多态请求**。
2. **撤销 / 重做**：运营后台、可视化配置、文档编辑器式交互。
3. **排队、延迟、重试**：离线同步、削峰批处理、MQ 消费 **一条消息 = 一个 Command**。
4. **审计与回放**：命令对象 **序列化存库**，合规回放「谁在何时执行了什么」。
5. **宏操作与脚本化**：批量关单、批量改仓 = `MacroCommand` 或命令列表。
6. **解耦 UI 与领域**：按钮 / 快捷键 **只绑定 Command**，不直接调 Service。

**不必强行使用**：

- **一次性、永不撤销、单入口** 的 `orders.Cancel`——直接调方法更简单。
- **整单 PlaceOrder** 级编排——用 [外观](/cs-fundamentals/design-patterns/facade)；命令管 **单步**，不是替代 Facade。
- **多个处理者择一处理请求**——用 [责任链](/cs-fundamentals/design-patterns/chain-of-responsibility)，不是命令。
- **运行时切换算法**（计价策略）——用 Strategy；命令是 **操作记录**，不是 **算法族**。
- **Undo 成本极高或不可能**（已发货、已扣款）——命令仍可 **入队执行**，但 `Undo` 返回 `ErrUndoNotSupported` 或走 **人工补偿流程**。

常见例子：文本编辑器 Undo/Redo、GUI `Action`/`Runnable`、线程池 `Runnable`、CQRS 写模型、数据库事务日志、游戏输入回放、Redis `MULTI` 内的命令队列思想。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **解耦调用方与接收者** | Handler 只依赖 `Command` + `Invoker` |
| **开闭** | 新操作 **新 ConcreteCommand**，少改 Invoker |
| **撤销 / 重做** | 统一历史栈或持久化命令日志 |
| **可组合** | 宏命令、管道式「命令链」 |
| **天然适配队列** | 序列化后异步执行、失败重试 |
| **审计友好** | 命令对象即操作记录 |

| 缺点 | 说明 |
| :--- | :--- |
| **类数量增加** | 每个操作一个命令类型；可用工厂或代码生成缓解 |
| **Undo 语义难** | 逆操作要 **业务上可定义**；分布式场景常需 Saga |
| **状态快照** | 命令要保存 **足够的旧值**；构造时机要防并发脏读 |
| **与事件职责重叠** | 团队需约定：**命令 = 意图**，**事件 = 已发生事实** |
| **宏命令部分失败** | 要定义 **补偿策略**，不能假设 `Undo` 总能救场 |

## 组装实践

> **阅读提示**：先掌握「**Command 封装操作 + Receiver 干活 + Invoker 执行**」即可。本节是工程变体；初学可先跳过。

### 持久化命令日志（跨请求 Undo）

HTTP 无状态时，把命令 **序列化** 进 `operation_log`：

```go
type StoredCommand struct {
    ID      string
    Type    string          // "adjust_qty"
    Payload json.RawMessage
    UserID  string
    Undone  bool
}

// Execute 成功后写入 log；Undo 时读 Payload 重建 ConcreteCommand
func (s *OperationLogService) Replay(ctx context.Context, id string) error {
    rec, err := s.repo.Get(ctx, id)
    // ...
    cmd, err := s.factory.FromStored(rec)
    return cmd.Execute(ctx)
}
```

**工厂**（Factory）根据 `Type` 反序列化——与 [工厂方法](/cs-fundamentals/design-patterns/factory) 配合，避免 `switch` 散落在各处。

### 命令队列与重试

```go
type CommandQueue struct {
    ch chan Command
}

func (q *CommandQueue) Enqueue(cmd Command) {
    q.ch <- cmd
}

func (q *CommandQueue) Worker(ctx context.Context) {
    for {
        select {
        case cmd := <-q.ch:
            if err := cmd.Execute(ctx); err != nil {
                metrics.Inc("command_failed")
                // 死信 / 指数退避重试
            }
        case <-ctx.Done():
            return
        }
    }
}
```

离线 App **同步购物车**、大促 **异步关单** 适合 **Enqueue**；注意命令 **可序列化**（参数用 ID，不要塞 `*sql.Tx`）。

### 与外观、责任链一起用

```text
HTTP PlaceOrder
  → preCheckChain.Handle（责任链）
  → facade.PlaceOrderCore（外观）

Admin AdjustQuantity
  → invoker.Run(AdjustQuantityCommand)（命令）

Batch rollback
  → queue.Enqueue(MacroCommand{...})
```

[责任链](/cs-fundamentals/design-patterns/chain-of-responsibility) 在 **下单前**；[外观](/cs-fundamentals/design-patterns/facade) 在 **用例编排**；命令在 **可撤销 / 可排队的单步写操作**——三层 **正交**。

### Undo 与领域约束

| 操作 | Undo 是否可行 | 做法 |
| :--- | :--- | :--- |
| 改未支付订单数量 | 可行 | 快照 `oldQty` |
| 改已发货地址 | 常不可行 | `Undo` 返回错误或转 **工单** |
| 取消已支付订单 | 需退款 | `CancelOrderCommand.Undo` 触发 **RefundCommand** 或禁止 Undo |

在 `Execute` 前用 **领域状态机** 校验：`if order.Status == Shipped { return ErrNotAdjustable }`。

### 测试策略

```go
func TestAdjustQuantityCommand_UndoRestoresOldQty(t *testing.T) {
    orders := &fakeOrders{lines: map[string]int{"sku-a": 2}}
    cmd := NewAdjustQuantityCommand(orders, "o1", "sku-a", 5)
    _ = cmd.Execute(context.Background())
    _ = cmd.Undo(context.Background())
    if orders.lines["sku-a"] != 2 {
        t.Fatal("expected qty 2 after undo")
    }
}

func TestMacroCommand_UndoReverseOrder(t *testing.T) {
    var log []string
    c1 := &recordingCommand{name: "a", log: &log}
    c2 := &recordingCommand{name: "b", log: &log}
    m := &MacroCommand{cmds: []Command{c1, c2}}
    _ = m.Execute(context.Background())
    _ = m.Undo(context.Background())
    // Undo 应先 b 后 a
    if log[len(log)-2] != "undo-b" || log[len(log)-1] != "undo-a" {
        t.Fatal(log)
    }
}
```

## 小结

记住这四点即可：

1. **操作即对象**：改数量、取消单、改地址各是一个 `Command`，带参数与可选 `Undo`。
2. **Invoker 统一执行**：调用方 `Run(cmd)`，不 scattered 调十个 Service 方法。
3. **与 Facade / 责任链分层**：Facade 编排 **整单用例**；责任链做 **前置检查**；命令管 **可撤销、可排队的单步写**。
4. **宏命令与队列**：批量关单 = 多个 `ConcreteCommand` 组合或入队；失败要有 **补偿或重试** 策略。

[责任链](/cs-fundamentals/design-patterns/chain-of-responsibility) 解决 **「谁来处理这个请求」**；命令解决 **「把这个操作存下来、排队、撤销、回放」**——把 **请求** 从 **方法调用** 提升为 **一等对象**，让运营工具与异步 Worker 在同一套抽象上协作，并符合 [开闭](/cs-fundamentals/design-patterns#设计原则) 与 **单一职责**。

## 参考阅读

- [x] [外观模式](/cs-fundamentals/design-patterns/facade) — 用例级编排；与单步命令分层配合
- [x] [责任链模式](/cs-fundamentals/design-patterns/chain-of-responsibility) — 前置校验链；与命令触发时机不同
- [x] [工厂方法模式](/cs-fundamentals/design-patterns/factory) — 从持久化记录重建 ConcreteCommand
- [x] [Refactoring.Guru - 命令模式](https://refactoringguru.cn/design-patterns/command) (2026-06-22)
- [x] [菜鸟教程 - 命令模式](https://www.runoob.com/design-pattern/command-pattern.html) (2026-06-22)
