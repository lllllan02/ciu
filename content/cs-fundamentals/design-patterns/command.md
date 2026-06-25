---
title: 命令模式
order: 14
---

**命令模式**（Command）亦称 **动作**、**事务**，可将请求转换为一个包含与请求相关的所有信息的独立对象。该转换让你能根据不同的请求将方法参数化、延迟请求执行或将其放入队列中，且能实现可撤销操作。

通俗地说，每个操作包成一个独立对象，谁执行、何时执行、执行过了没有，都可以单独管理；于是自然支持排队、撤销、重做和审计回放，多种入口也能共用同一套操作抽象。

## 问题

你在做 **运营后台**：工具栏上有「改数量」「改地址」「取消订单」等按钮；订单详情页右键菜单、批量任务脚本、MQ 补偿 Worker 也要触发 **同一类写操作**，且产品要求 **撤销 / 重做** 与 **操作审计**。

最直接的做法是给每种操作建 **按钮子类** 或让各入口 **直接调** `OrderService`：

```go
type AdjustQtyButton struct { orders OrderService; /* … */ }
func (b *AdjustQtyButton) OnClick() { b.orders.SetLineQuantity(/* … */) }

type CancelOrderButton struct { orders OrderService }
func (b *CancelOrderButton) OnClick() { b.orders.Cancel(/* … */) }
// … 每个操作一个子类；HTTP Handler、脚本再各写一遍
```

操作少时还能应付；入口一多，问题就会一起暴露：

1. **UI 与领域紧耦合**：改 `OrderService` 签名，工具栏、菜单、Handler、脚本 **全要改**——表现层依赖不稳定的业务 API。
2. **同一操作多处重复**：「取消订单」在工具栏、详情菜单、大促回滚脚本里 **各写一遍**，补偿逻辑不一致。
3. **无法统一撤销**：改数量没快照旧值；Undo 只能手写反向 SQL，中途失败的批量关单 **无法回滚已执行部分**。
4. **难以排队与回放**：离线同步、削峰批处理需要 **把操作存成对象再执行**——现在只有即时方法调用，日志也无法 **重放**。

本质矛盾是：**Admin 层只应触发操作**，**OrderService 层才管业务规则**；同一操作又会从 **多种 UI / 多种时机** 触发，且常要 **记录、延迟、撤销**——却用 **分散的直接调用** 或 **按钮子类爆炸** 来表达。

## 解决方案

优秀设计会把 **表现层** 与 **业务逻辑层** 分开：Admin UI 负责渲染与捕获输入；`OrderService` 负责改数量、取消单等领域规则。两层之间不应让 Handler **直接** `orders.SetLineQuantity(...)`，而应将 **请求的全部细节**（调谁、什么方法、哪些参数）抽成 **命令对象**。

### 命令连接两层

```text
Admin 按钮 / Handler  ──触发──▶  Command.Execute()
                                      │
                                      ▼
                               OrderService（Receiver）
```

Admin 对象 **只持有 Command 引用** 并触发它，不必知道 Receiver 怎么处理；Command 在 `Execute` 里把调用 **委派** 给 `OrderService`。同一 `CancelOrderCommand` 可绑到工具栏按钮、右键菜单和大促回滚脚本—— **一处定义，多处触发**。

### 角色与代码

| 角色 | 电商中的对应 |
| :--- | :--- |
| **Client** | 组装层：创建 `AdjustQuantityCommand` 并交给 Invoker / 按钮 |
| **Command** | `Execute` / `Undo` 接口 |
| **Concrete Command** | `AdjustQuantityCommand`：保存 `orderID`、`sku`、`newQty` 与 Receiver |
| **Receiver** | `OrderService`：真正改库 |
| **Invoker（Sender）** | `Invoker` 或 `AdminSession`：执行命令、维护历史栈 |

```go
type Command interface {
    Execute(ctx context.Context) error
    Undo(ctx context.Context) error
}

type OrderService interface {
    GetLineQuantity(ctx context.Context, orderID, sku string) (int, error)
    SetLineQuantity(ctx context.Context, orderID, sku string, qty int) error
    Cancel(ctx context.Context, orderID string) error
}

type AdjustQuantityCommand struct {
    orders         OrderService
    orderID, sku   string
    newQty, oldQty int
}

func (c *AdjustQuantityCommand) Execute(ctx context.Context) error {
    qty, err := c.orders.GetLineQuantity(ctx, c.orderID, c.sku)
    if err != nil {
        return err
    }
    c.oldQty = qty // Execute 时快照，供 Undo；勿在构造时读
    return c.orders.SetLineQuantity(ctx, c.orderID, c.sku, c.newQty)
}

func (c *AdjustQuantityCommand) Undo(ctx context.Context) error {
    return c.orders.SetLineQuantity(ctx, c.orderID, c.sku, c.oldQty)
}
```

改地址、取消单 **各一个 ConcreteCommand**；不可撤销操作（如已发短信）让 `Undo` 返回 `ErrUndoNotSupported`。

### Invoker 与 Admin 入口

```go
type Invoker struct {
    history []Command
}

func (i *Invoker) Run(ctx context.Context, cmd Command) error {
    if err := cmd.Execute(ctx); err != nil {
        return err
    }
    i.history = append(i.history, cmd)
    return nil
}

func (i *Invoker) Undo(ctx context.Context) error {
    if len(i.history) == 0 {
        return ErrNothingToUndo
    }
    cmd := i.history[len(i.history)-1]
    i.history = i.history[:len(i.history)-1]
    return cmd.Undo(ctx)
}

type AdminAction struct {
    cmd     Command
    invoker *Invoker
}

func (a *AdminAction) Trigger(ctx context.Context) error {
    return a.invoker.Run(ctx, a.cmd)
}
```

Client 在启动时 **预配置** 命令（注入 `OrderService` 与参数），再挂到按钮 / Handler；Invoker **不创建** 命令，只 **执行** 客户端传入的实例——与 [Refactoring.Guru](https://refactoringguru.cn/design-patterns/command) 中的 Sender 职责一致。

### 类比：厨房订单小票

服务员把「两份加辣、少冰」写在 **订单小票** 上贴进厨房队列；厨师按小票做菜，不必跑回桌边确认。小票就是 **命令**：在烹饪前一直排队，包含 **全部参数**；Admin 触发命令、Worker 异步消费命令，是同一抽象。

## 适用场景

1. **用操作参数化对象**：同一 `Invoker.Run` 或 `AdminAction` 可绑定改数量、改地址、取消单——运行时 **换命令即换行为**。
2. **排队、延迟、远程执行**：离线 App 同步、MQ 削峰关单——命令 **可序列化** 后入队、重试、发到远程 Worker。
3. **撤销 / 重做**：运营后台编辑会话；命令历史栈或持久化 `operation_log`（复杂状态可配合 [备忘录](/cs-fundamentals/design-patterns/memento)）。
4. **宏操作**：大促回滚 = 多个 `CancelOrderCommand` 组合（见 **实践**）。
5. **解耦 UI 与领域**：HTTP、CLI、定时任务 **只依赖 Command 接口**，不 import 十几个 Service 方法。

**不必强行使用**：

- **一次性、单入口、永不撤销** 的 `orders.Cancel`——直接调方法更简单。
- **整单 PlaceOrder** 级编排——用 [外观](/cs-fundamentals/design-patterns/facade)；命令管 **可撤销的单步写**，不替代 Facade。
- **多个处理者择一处理**——用 [责任链](/cs-fundamentals/design-patterns/chain-of-responsibility)。
- **运行时切换算法**（会员价怎么算）——用 [策略](/cs-fundamentals/design-patterns/strategy)；命令是 **操作记录**，不是 **算法族**。

常见例子：GUI `Action` / `Runnable`、CQRS 写侧、数据库事务日志、线程池任务队列。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **单一职责** | 触发（Admin / Invoker）与执行（Receiver）解耦 |
| **开闭** | 新操作 = 新 `ConcreteCommand`，少改 Invoker |
| **撤销 / 重做 / 延迟** | 命令对象即操作记录，可入栈、序列化、回放 |
| **可组合** | 宏命令、与责任链 / 外观 **正交** 分层 |
| **多入口复用** | 同一命令绑工具栏、菜单、脚本 |

| 缺点 | 说明 |
| :--- | :--- |
| **类数量增加** | 每个操作一个命令类型 |
| **Undo 语义难** | 分布式 / 已发货场景常需 Saga 或禁止 Undo |
| **状态快照成本** | 备份旧值占内存；也可改用 **反向命令** 但未必可实现 |
| **宏命令部分失败** | 需定义补偿策略，不能假设 `Undo` 总能救场 |

## 关联

- [责任链模式](/cs-fundamentals/design-patterns/chain-of-responsibility)、命令模式、[中介者模式](/cs-fundamentals/design-patterns/mediator) 和 [观察者模式](/cs-fundamentals/design-patterns/observer) 均用于在不同对象之间传递请求，但各自采用不同的方法。责任链模式按顺序传递请求，直到有一个接收者处理它；命令模式在发送者和请求者之间建立单向连接；中介者模式让发送者和请求者完全消除相互引用，只能通过中介对象间接通信；观察者模式允许接收者动态订阅或取消订阅接收请求。
- 处理者（Handler）通常以命令模式的形式实现。在这种情况下，你可以对由请求所代表的同一个上下文对象执行许多不同的操作。还有另一种实现方式，即请求本身是一个命令对象。在这种情况下，你可以对由一系列不同上下文所组成的链执行同一个操作。
- 你可以使用命令模式和 [备忘录模式](/cs-fundamentals/design-patterns/memento) 来实现「撤销」。在这种情况下，命令用于对目标对象执行各种不同的操作，备忘录用来在命令执行之前保存该对象的状态。
- 命令模式和 [策略模式](/cs-fundamentals/design-patterns/strategy) 看上去很像，因为两者均能用某些行为来参数化对象。但是，它们的意图完全不同。
  - 使用命令模式，你可以将任何操作转换为对象，该对象中的操作参数则成为对象的成员变量。你可以延迟执行该操作、将其放入队列、记录操作历史或者向远程服务发送对象等。
  - 使用策略模式，你通常可以描述实现同一目标的不同方式，使你在同一个上下文类中切换不同的算法。
- 可以使用 [原型模式](/cs-fundamentals/design-patterns/prototype) 来保存命令模式的历史记录。
- [访问者模式](/cs-fundamentals/design-patterns/visitor) 可以被看作是命令模式的增强版本，其对象能对不同类的多种对象执行操作。

## 参考阅读

- [x] [Refactoring.Guru - 命令模式](https://refactoringguru.cn/design-patterns/command) (2026-06-22)
- [x] [菜鸟教程 - 命令模式](https://www.runoob.com/design-pattern/command-pattern.html) (2026-06-22)
