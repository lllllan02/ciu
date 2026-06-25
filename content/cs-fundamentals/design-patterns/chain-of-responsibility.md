---
title: 责任链模式
order: 13
---

**责任链模式**（Chain of Responsibility）亦称 **职责链模式**、**命令链**，使多个对象都有机会处理请求，从而避免请求的发送者与接收者之间的耦合；将这些对象连成一条链，并沿着这条链传递该请求，直到有一个对象处理它为止。

通俗地说，一连串规则各自负责一环，**链** 按顺序自动调用各 Handler，直到有人处理或全部检查完毕；发起方只调 `Chain.Handle`，加规则、调顺序改 `NewChain(...)` 即可，不必改调用方代码。

## 问题

下单前要依次做购物车校验、库存检查、优惠券验证、风控；退款要按金额分流到自动批、主管批、财务批。最直接的做法是在 `CheckoutFacade` 和 `RefundService` 里 **硬编码一长串 if**。

规则少时还能应付；每加一条风控或合规检查，问题就会一起暴露：

1. **难扩展**：加一条规则就要改 Facade 源码；不同渠道想关掉某一环，只能再加 `if cfg.X`。
2. **职责混杂**：Facade 既管 **编排**（预占→支付→落库），又管 **十余条校验**，违反 [单一职责](/cs-fundamentals/design-patterns#设计原则)。
3. **调用方仍知道太多**：B 端想跳过 App 专属风控、跨境站要多一环关税估算——各入口复制不同 if 顺序。
4. **审批规则难维护**：「100 元以下自动退」改成「VIP 自动退、普通 50 元以下」时，`switch amount` 又长又脆。

本质矛盾是：**完成一次业务** 往往需要 **多步检查或分级审批**，但 **链有多长、哪一步处理** 应能 **独立配置**；发送方只应 **把请求交给 Chain**。典型写法如下：

```go
func (f *CheckoutFacade) PlaceOrder(ctx context.Context, req PlaceOrderRequest) error {
    if err := validateCart(req); err != nil { return err }
    if err := f.checkInventory(ctx, req); err != nil { return err }
    if err := validateCoupon(req); err != nil { return err }
    if f.cfg.EnableFraud {
        if err := f.fraud.Check(ctx, req); err != nil { return err }
    }
    // … 真正的下单编排
    return f.placeOrderCore(ctx, req)
}
```

## 解决方案

定义 **处理者** 接口，各 **具体处理者** 只负责一条规则；**链** 持有 handler 列表并 **自动依次调用**。发送方只依赖 **链**。

### 处理者与链

```go
type OrderHandler interface {
    Handle(ctx context.Context, order *PlaceOrderContext) error
}

type Chain struct {
    handlers []OrderHandler
}

func NewChain(handlers ...OrderHandler) *Chain {
    return &Chain{handlers: handlers}
}

func (c *Chain) Handle(ctx context.Context, order *PlaceOrderContext) error {
    for _, h := range c.handlers {
        if err := h.Handle(ctx, order); err != nil {
            return err // 本环失败，链条短路
        }
    }
    return nil
}
```

Handler **不必** 知道下一环是谁，也 **不必** 调用 `callNext`——转发由 `Chain.Handle` 统一完成。

### 具体处理者

```go
type CartValidator struct{}

func (h *CartValidator) Handle(ctx context.Context, order *PlaceOrderContext) error {
    if len(order.Req.Lines) == 0 {
        return ErrEmptyCart
    }
    return nil // 通过即可，链会自动进入下一环
}
```

库存、优惠券、风控等 **各一个 Handler**，只写本环校验；任一环返回 `error` 即停止，不会进入 [外观](/cs-fundamentals/design-patterns/facade) 的 `placeOrderCore`。

### 两种链形态

| 形态 | 场景 | 链如何停 |
| :--- | :--- | :--- |
| **管道式** | 下单前校验 | 每环都必须 `return nil`；全部跑完才算成功 |
| **单处理者** | 退款审批 | Handler 返回 `(handled bool, err)`；`handled == true` 时链 **停止** |

退款链接口略不同，但思路一样——**链负责遍历，Handler 只管本环**：

```go
type ApprovalHandler interface {
    Handle(ctx context.Context, r *RefundContext) (handled bool, err error)
}

func (c *ApprovalChain) Handle(ctx context.Context, r *RefundContext) error {
    for _, h := range c.handlers {
        handled, err := h.Handle(ctx, r)
        if err != nil {
            return err
        }
        if handled {
            return nil
        }
    }
    return ErrNoHandler
}
```

`AutoRefundHandler` 小额批完返回 `(true, nil)`；大额返回 `(false, nil)`，链自动进入主管 / 财务。

### 组装与客户端

```go
preCheck := NewChain(
    &CartValidator{},
    &InventoryValidator{inventory: inv},
    &FraudValidator{fraud: fraud},
)

func (f *CheckoutFacade) PlaceOrder(ctx context.Context, req PlaceOrderRequest) error {
    if err := f.preCheck.Handle(ctx, &PlaceOrderContext{Req: req}); err != nil {
        return err
    }
    return f.placeOrderCore(ctx, req)
}
```

B 端可 `NewChain(cart, stock)` 拼更短链；`RefundService` 注入 `ApprovalChain`。测试时换 **短链** 或 mock 某一环即可，细节见 **实践** 一节。


## 适用场景

1. **多个对象可能处理同一请求，且处理者在运行时未确定**：退款分级、客服工单升级、日志级别过滤（只由某一环写盘）。
2. **想动态指定处理者集合或顺序**：App / B 端 / 跨境 **不同链**；大促临时 **加一环** 限流校验。
3. **发送方不应依赖具体处理者类型**：Facade、Controller 只认 `*Chain`。
4. **规则可独立测试、独立发布**：新风控环 **新 struct + 改组装**，不动 `placeOrderCore`。
5. **管道式前置检查**：下单、提现、入驻审核——**全会签** 用责任链比 Facade 内 `if` 堆叠更清晰。

**不必强行使用**：

- 只有 **固定两步、永不变** 的校验——直接两个函数调用更简单。
- 需要的是 **有副作用的固定编排**（预占→支付→落库）——用 [外观](/cs-fundamentals/design-patterns/facade)，不是责任链替代 Facade。
- 每一层都要 **包装同一接口并叠加计价**——用 [装饰器](/cs-fundamentals/design-patterns/decorator)。
- 只有 **一个** 接收者——不需要链。
- 处理者之间 **强顺序依赖且共享大量中间状态**——可能更适合 **显式状态机** 或工作流引擎（责任链仍可作 **轻量** 替代）。

常见例子：Servlet `Filter` 链、Netty `ChannelPipeline`、Kubernetes **准入控制器**（Admission Webhook 链）、前端事件冒泡、日志 appender 链。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **开闭** | 新校验 / 新审批档 **加 ConcreteHandler + 改组装** |
| **单一职责** | 每环一个文件一种规则；Facade 专注编排 |
| **发送方解耦** | Client 不 import 十个 Validator |
| **可组合** | 按渠道拼不同链；测试用短链 |
| **与 Facade 正交** | 前置链 + 核心编排 **分层清晰** |

| 缺点 | 说明 |
| :--- | :--- |
| **链未命中** | 单处理者链若 **无人处理**，需链尾兜底或明确 `ErrNoHandler` |
| **调试路径** | 请求在哪一环失败，要靠 **日志 / 链环命名**；过长链难 trace |
| **顺序敏感** | 管道式：先风控还是先库存，业务上要 **文档化** |
| **性能** | 环过多时 **每次请求遍历**；极热路径可合并或编译期链 |
| **与循环依赖** | Handler 互相引用子系统时，注意 **依赖方向** 仍指向领域服务 |

## 实践

### 可观测性

```go
func (h *CartValidator) Handle(ctx context.Context, order *PlaceOrderContext) error {
    ctx = context.WithValue(ctx, handlerKey{}, "cart")
    start := time.Now()
    err := h.handle(ctx, order)
    metrics.Record("precheck", "cart", time.Since(start), err)
    return err
}
```

每一环打 **name + latency**，排障时可见 **卡在哪一环**。


## 关联

- 责任链模式、[命令模式](/cs-fundamentals/design-patterns/command)、[中介者模式](/cs-fundamentals/design-patterns/mediator) 和 [观察者模式](/cs-fundamentals/design-patterns/observer) 均用于在不同对象之间传递请求，但各自采用不同的方法。责任链模式按顺序传递请求，直到有一个接收者处理它；命令模式在发送者和请求者之间建立单向连接；中介者模式让发送者和请求者完全消除相互引用，只能通过中介对象间接通信；观察者模式允许接收者动态订阅或取消订阅接收请求。
- 责任链通常和 [组合模式](/cs-fundamentals/design-patterns/composite) 一起使用。在这种情况下，叶组件接收到请求后，可以将请求传递给包含它的所有组件，直到根组件。
- 处理者（Handler）通常以 [命令模式](/cs-fundamentals/design-patterns/command) 的形式实现。在这种情况下，你可以对由请求所代表的同一个上下文对象执行许多不同的操作。还有另一种实现方式，即请求本身是一个命令对象。在这种情况下，你可以对由一系列不同上下文所组成的链执行同一个操作。
- 责任链和 [装饰模式](/cs-fundamentals/design-patterns/decorator) 的类结构非常相似。两者都依赖于递归组合，将执行过程传递给一系列对象。但是，两者有几点重要的不同。
  - 责任链的处理者能相互独立地执行工作，且可随时停止传递请求。
  - 装饰器可在遵循基本接口的情况下为对象新增行为。另外，装饰器无法中断请求的传递。

## 参考阅读

- [x] [Refactoring.Guru - 责任链模式](https://refactoringguru.cn/design-patterns/chain-of-responsibility) (2026-06-22)
- [x] [菜鸟教程 - 责任链模式](https://www.runoob.com/design-pattern/chain-of-responsibility-pattern.html) (2026-06-22)
