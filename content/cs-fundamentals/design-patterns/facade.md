---
title: 外观模式
order: 10
---

**外观模式** 为子系统中的一组接口提供一个统一的接口，使得这一子系统更加容易使用。

通俗地说，多个子系统之间的调用顺序、出错怎么回滚，集中到一个统一入口里编排；各客户端只调这一个门面完成一件事，不必逐个认识子系统，也不必自己拼流程。

## 问题

下单远不止「算总价、扣库存」——还要调支付、落库、发通知、开发票、写审计，且 **顺序固定、失败要补偿**。最直接的做法是让 HTTP 控制器 **自己串联所有子系统**。

入口少时还能应付；每多一个调用方（移动端、后台代客、定时任务），同一套编排就要 **复制一遍**：

1. **与子系统紧耦合**：控制器直接依赖六七个类型，改一个子系统要改所有入口。
2. **编排重复且易错**：预占 → 扣款 → 落库 → 失败回滚的顺序各写一遍，漏一步就超卖或重复扣款。
3. **职责混杂**：HTTP 层本应管参数和响应，却承担 **分布式事务式编排**。
4. **测试困难**：单测下单 happy path 必须 mock 全部依赖并断言调用顺序。

本质矛盾是：**完成一次业务用例** 需要 **固定顺序的多步协作**，这段编排 **不应** 在每个客户端里各写一遍。典型写法如下：

```go
func (c *CheckoutController) PlaceOrder(ctx context.Context, req PlaceOrderRequest) error {
    // 校验 → 预占 → 计价 → 扣款 → 落库 → 通知 → 发票 → 审计
    // 支付失败要 Release，落库失败要 Refund——补偿逻辑散落在此
    if err := c.payment.Pay(...); err != nil {
        c.inventory.ReleaseAll(req.Lines)
        return err
    }
    // …
}
```

## 解决方案

定义 **外观** `CheckoutFacade`，组合持有各 **子系统**（Subsystem）；对外一个 **粗粒度** 方法，对内 **按序委托** 并处理失败。

### 子系统（Subsystem）

各子系统保持 **独立 package / 接口**，可被其他用例单独使用（如仅查库存、仅退款）：

```go
type InventoryService interface {
    Reserve(lines []OrderLine) error
    Release(lines []OrderLine) error
}

type PricingEngine interface {
    Total(lines []OrderLine) int64
}

type PaymentProcessor interface {
    Pay(order Order) error
    Refund(orderID string) error
}

type OrderRepository interface {
    NextID() string
    Save(ctx context.Context, order Order) error
}

type NotificationService interface {
    SendOrderConfirmation(orderID string) error
}

type InvoiceService interface {
    Issue(orderID string) error
}

type AuditService interface {
    Log(event, orderID string) error
}
```

`PricingEngine` 内部可遍历 [组合](/cs-fundamentals/design-patterns/composite) 树与 [装饰器](/cs-fundamentals/design-patterns/decorator) 链——外观 **不展开** 明细结构。

### 外观（Facade）

```go
type CheckoutFacade struct {
    inventory InventoryService
    pricing   PricingEngine
    payment   PaymentProcessor
    orders    OrderRepository
    notify    NotificationService
    invoice   InvoiceService
    audit     AuditService
}

func NewCheckoutFacade(
    inv InventoryService,
    pricing PricingEngine,
    pay PaymentProcessor,
    orders OrderRepository,
    notify NotificationService,
    invoice InvoiceService,
    audit AuditService,
) CheckoutFacade {
    return CheckoutFacade{
        inventory: inv, pricing: pricing, payment: pay,
        orders: orders, notify: notify, invoice: invoice, audit: audit,
    }
}

func (f CheckoutFacade) PlaceOrder(ctx context.Context, req PlaceOrderRequest) (OrderResult, error) {
    lines := req.Lines

    for _, line := range lines {
        if err := line.Validate(); err != nil {
            return OrderResult{}, fmt.Errorf("validate: %w", err)
        }
    }
    if err := f.inventory.Reserve(lines); err != nil {
        return OrderResult{}, fmt.Errorf("reserve: %w", err)
    }

    amount := f.pricing.Total(lines)
    orderID := f.orders.NextID()
    order := Order{ID: orderID, Lines: lines, Amount: amount, UserID: req.UserID}

    if err := f.payment.Pay(order); err != nil {
        _ = f.inventory.Release(lines)
        return OrderResult{}, fmt.Errorf("pay: %w", err)
    }

    if err := f.orders.Save(ctx, order); err != nil {
        _ = f.payment.Refund(orderID)
        _ = f.inventory.Release(lines)
        return OrderResult{}, fmt.Errorf("save: %w", err)
    }

    // 落库成功后：异步步骤失败通常记日志 + 重试，不整单回滚（按业务约定）
    _ = f.notify.SendOrderConfirmation(orderID)
    _ = f.invoice.Issue(orderID)
    _ = f.audit.Log("order.placed", orderID)

    return OrderResult{OrderID: orderID, Amount: amount}, nil
}
```

> **对照「问题」一节**：`CheckoutFacade.PlaceOrder` 与 `CheckoutController.PlaceOrder` **业务步骤相同**——这正是预期。差别在于 **这段逻辑归属哪一层、会被几个入口调用**。下面 Client 一节展示 **多入口共用**；若你只在 Controller 里写 Facade 调用、编排仍散落别处，就没有用到外观的核心价值。

### 客户端（Client）

```go
type CheckoutController struct {
    checkout CheckoutFacade
}

func (c *CheckoutController) HandlePlaceOrder(w http.ResponseWriter, r *http.Request) {
    var req PlaceOrderRequest
    if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
        http.Error(w, err.Error(), http.StatusBadRequest)
        return
    }
    result, err := c.checkout.PlaceOrder(r.Context(), req)
    if err != nil {
        http.Error(w, err.Error(), http.StatusUnprocessableEntity)
        return
    }
    _ = json.NewEncoder(w).Encode(result)
}
```

组装层注入真实子系统或 fake：

```go
facade := NewCheckoutFacade(
    inventorySvc, pricingEngine, paymentProcessor,
    orderRepo, notifySvc, invoiceSvc, auditSvc,
)

// 多个入口注入 **同一个** Facade——编排只维护在 Facade 内
httpCtrl := &CheckoutController{checkout: facade}
adminCtrl := &AdminController{checkout: facade}
worker := &OrderRetryWorker{checkout: facade}
```

新增 `RiskService.Check` → **只改** `CheckoutFacade.PlaceOrder` 一处；三个入口 **都不改**。这才是与「问题」里单 Controller 写法对比时，Facade 代码 **看起来一样** 却 **仍然值得** 的原因。


## 适用场景

1. **多个子系统协作完成一个用例**：下单、开户、编译链接、部署发布——步骤固定、入口多样。
2. **要降低上层学习成本**：新同学只需学 `PlaceOrder`，不必先读六个 package 的 README。
3. **编排与补偿应集中维护**：支付失败释放库存、落库失败触发退款——改一处即可。
4. **符合迪米特法则**：控制器、脚本、测试 **只依赖 Facade**，子系统 refactor 不影响所有调用方。
5. **分层架构中的「应用服务」**：Domain 管规则，Infrastructure 管 I/O，Facade / Application Service 管 **用例级编排**。

**不必强行使用**：

- **只有一个入口、且编排不会复用**——把逻辑放在 Controller 或一个普通 Service 里即可；Facade 主要是 **换层**，不是魔法。
- 子系统 **只有一个**、调用 **一步完成**——多一层 Facade 只是 pass-through。
- 需要的是 **改接口兼容** 而非简化多系统——用 [适配器](/cs-fundamentals/design-patterns/adapter)。
- 多个同级对象 **互相** 引用、关系复杂——考虑 **中介者**（Mediator），不是外观。
- 要在 **同一接口上叠加重试、日志**——用 [装饰器](/cs-fundamentals/design-patterns/decorator) 包装 `PaymentProcessor`，不是 Facade 包一切。
- Facade 膨胀成 **上帝类**（所有业务规则都写进去）——把规则下沉子系统或领域层，外观保持 **薄编排**。

常见例子：SLF4J 对多种日志 backend、`javax.faces.context.FacesContext`、编译器前端对词法/语法/语义各阶段、Kubernetes client 对 REST 资源组的简化操作、IDE「Build Project」对编译+链接+打包的封装。

## 优缺点

| 优点 | 说明 |
| :--- | :--- |
| **简化客户端** | 一个 `PlaceOrder` 替代六七个 import 与顺序记忆 |
| **迪米特 / 低耦合** | 上层不依赖子系统具体类型，子系统可替换实现 |
| **编排集中** | 补偿、重试、异步步骤的约定在一处文档化与实现 |
| **复用** | HTTP、MQ、CLI 共用同一 Facade |
| **不破坏子系统** | 库存模块仍可直接 `Reserve` 给「仅预占不支付」场景 |

| 缺点 | 说明 |
| :--- | :--- |
| **可能变成上帝类** | 所有新需求都往 Facade 塞 if，违反单一职责 |
| **额外间接层** | 调试要进 Facade 再看调了哪个 Subsystem |
| **隐藏子系统能力** | 客户端若只需「查库存」，仍应 **直接** 调 `InventoryService`，不必强行经 Facade |
| **分布式事务错觉** | Facade 内 `Pay` + `Save` 不是 ACID——仍需 Saga、Outbox、幂等等 **真实** 一致性方案 |
| **测试 Facade 仍要 mock 多依赖** | 但 mock 边界清晰：断言 **顺序与补偿**，而非测每个子系统内部 |

## 实践

> **阅读提示**：先掌握「Facade 组合 Subsystem + 单一 PlaceOrder + Client 只调 Facade」即可。本节是工程变体；初学可先跳过。

### 薄外观 vs 应用服务

在 DDD / 整洁架构里，**Application Service** 与 Facade **角色重合**：编排用例、无核心业务规则。约定：

- **Facade / Application Service**：校验输入形状、调领域服务、调基础设施、发领域事件。
- **Domain**：`OrderLine.Validate`、`PricingEngine` 的满减规则。
- **Infrastructure**：`OrderRepository` 的 SQL、`PaymentProcessor` 的 HTTP。

```go
// 领域：单行校验仍在 OrderLine（组合/装饰/叶子）
// 应用：Facade 只负责「何时调 Validate」
if err := f.validator.Validate(lines); err != nil { ... }
```

把 `Validate` 循环抽成 `OrderValidator` 子系统，Facade 更薄。

### 与适配器、桥接、装饰叠加

组装层构造 Facade 时，子系统可以是 **已包装** 的实现：

```go
pay := RetryProcessor{
    Inner: NewStripeAdapter(StripeClient{}, cfg.CustomerID),
    MaxRetries: 3,
}
facade := NewCheckoutFacade(inv, pricing, pay, orders, notify, invoice, audit)
```

- [适配器](/cs-fundamentals/design-patterns/adapter)：`pay` 字段类型仍是 `PaymentProcessor`。
- [桥接](/cs-fundamentals/design-patterns/bridge)：`CheckoutAPI` 管支付 **请求形态**，Facade 管 **整单用例**——可并存：Facade 内部 `payment.Pay` 委托给 `InstallmentCheckoutAPI`。
- [装饰器](/cs-fundamentals/design-patterns/decorator)：`RetryProcessor` 装饰支付，不影响 Facade 接口。

### 异步与「尽力而为」步骤

通知、发票、审计 **失败后是否回滚整单** 是业务决策。常见做法：

```go
if err := f.orders.Save(ctx, order); err != nil {
    _ = f.payment.Refund(orderID)
    _ = f.inventory.Release(lines)
    return OrderResult{}, err
}

// 关键路径结束；以下失败记 metrics + 投递重试队列
if err := f.notify.SendOrderConfirmation(orderID); err != nil {
    log.Error("notify failed", "order", orderID, "err", err)
    f.retryQueue.Enqueue(NotifyJob{OrderID: orderID})
}
```

外观 **文档化** 哪些是 **强一致** 步骤、哪些是 **最终一致**，避免误以为 `PlaceOrder` 等于分布式事务。

### 多个 Facade 划分边界

按 **用例** 拆，而不是一个 Facade 包全站：

| Facade | 职责 |
| :--- | :--- |
| `CheckoutFacade` | 下单、预占、支付、落库 |
| `RefundFacade` | 退款、释库存、冲发票 |
| `CatalogFacade` | 查商品、库存快照（只读） |

客户端只依赖需要的 Facade；子系统可在多个 Facade 间 **共享**。

### 与中介者的区别

| | 外观 | 中介者 |
| :--- | :--- | :--- |
| 方向 | **单向**：Client → Facade → Subsystems | **多向**：同事类 ↔ Mediator ↔ 同事类 |
| 目的 | 简化 **外部** 使用 | 减少 **内部** 同事类之间的直接引用 |
| 例子 | `PlaceOrder` 调六个服务 | 对话框里列表与详情面板通过 Mediator 同步选中项 |

电商里 **下单编排** 是典型外观；**购物车 UI 组件互斥** 更像中介者。

### 测试策略

```go
func TestCheckoutFacade_PayFailsReleasesInventory(t *testing.T) {
    inv := &fakeInventory{}
    pay := &fakePayment{err: errors.New("declined")}
    f := NewCheckoutFacade(inv, fakePricing{}, pay, fakeOrders{}, noopNotify{}, noopInvoice{}, noopAudit{})

    _, err := f.PlaceOrder(context.Background(), sampleRequest())
    if err == nil {
        t.Fatal("expected error")
    }
    if !inv.released {
        t.Fatal("expected inventory release on pay failure")
    }
}
```

子系统 **各自** 单测；Facade 单测 **编排与补偿**；端到端再测一条 happy path。

### 可选 Facade 接口

便于 mock 与多实现：

```go
type OrderCheckout interface {
    PlaceOrder(ctx context.Context, req PlaceOrderRequest) (OrderResult, error)
}

var _ OrderCheckout = CheckoutFacade{}
```

控制器依赖 `OrderCheckout`，测试注入 `fakeCheckoutFacade`。

## 小结

记住这四点即可：

1. **教材三条是一回事**：降耦合、简操作、藏细节——都是让客户端 **少直接碰多个子系统**；多入口时额外体现为 **编排 DRY**。
2. **「复杂」= 用起来复杂**：子系统多、步骤多、客户端要学得多；**不是** 单步算法难，Facade 也 **不** 让编排代码消失，只是 **挪到 Facade、对客户端隐藏**。
3. **外观要薄**：领域规则在子系统；Facade 管 **协作顺序**，别变上帝类。
4. **别与适配器混淆**：适配器 **翻译一个** 接口；外观 **聚合多个** 子系统成 **一个** 用例入口。

[装饰器模式](/cs-fundamentals/design-patterns/decorator) 统一了 **单行上的可选增强**；外观模式统一了 **下单用例的编排写在哪**。放回电商订单系统：明细结构与计价增强就绪后，**同一段**「预占 → 支付 → 落库 → 补偿」不必在 HTTP、后台、重试 worker 里各抄一遍——放进 `CheckoutFacade`，上层只调 `PlaceOrder`。当批量导出或对账时 **同一 SKU 元数据在内存中重复出现**，下一篇 [享元模式](/cs-fundamentals/design-patterns/flyweight) 说明如何分离内部状态与外部状态并按 SKU 共享。

## 参考阅读

- [x] [Refactoring.Guru - 外观模式](https://refactoringguru.cn/design-patterns/facade) (2026-06-22)
- [x] [菜鸟教程 - 外观模式](https://www.runoob.com/design-pattern/facade-pattern.html) (2026-06-22)
