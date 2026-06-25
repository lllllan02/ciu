---
title: 外观模式
order: 10
---

**外观模式**（Facade）亦称 **门面模式**，为子系统中的一组接口提供一个统一的接口，使得这一子系统更加容易使用。

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

各子系统保持 **独立接口**，可被其他用例单独使用（如仅查库存、仅退款）。下单用例里 **强一致** 路径通常只需四个：

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
```

通知、发票、审计等 **尽力而为** 步骤可另接子系统，或在落库成功后异步投递——外观 **只编排关键路径**，不必把全站服务都塞进 struct。`PricingEngine` 内部可遍历 [组合](/cs-fundamentals/design-patterns/composite) 树与 [装饰器](/cs-fundamentals/design-patterns/decorator) 链，外观 **不展开** 明细结构。

### 外观（Facade）

```go
type CheckoutFacade struct {
    inventory InventoryService
    pricing   PricingEngine
    payment   PaymentProcessor
    orders    OrderRepository
}

func (f CheckoutFacade) PlaceOrder(ctx context.Context, req PlaceOrderRequest) (string, error) {
    lines := req.Lines
    if err := f.inventory.Reserve(lines); err != nil {
        return "", err
    }

    order := Order{
        ID:     f.orders.NextID(),
        Lines:  lines,
        Amount: f.pricing.Total(lines),
        UserID: req.UserID,
    }

    if err := f.payment.Pay(order); err != nil {
        _ = f.inventory.Release(lines)
        return "", err
    }
    if err := f.orders.Save(ctx, order); err != nil {
        _ = f.payment.Refund(order.ID)
        _ = f.inventory.Release(lines)
        return "", err
    }
    return order.ID, nil
}
```

> **对照「问题」一节**：步骤与 Controller 里手写的一样——差别在于编排 **归属 Facade**，可被多个入口共用；若只在 Controller 里调 Facade、别处仍复制编排，就没有用到外观的核心价值。

### 客户端（Client）

HTTP、后台、Worker 等 **只依赖 Facade**，不再 import 各子系统：

```go
type CheckoutController struct {
    checkout CheckoutFacade
}

func (c *CheckoutController) PlaceOrder(ctx context.Context, req PlaceOrderRequest) (string, error) {
    return c.checkout.PlaceOrder(ctx, req)
}

// 组装层：多个入口注入 **同一个** Facade
facade := CheckoutFacade{inventory: inv, pricing: pricing, payment: pay, orders: orders}
httpCtrl := &CheckoutController{checkout: facade}
worker := &OrderRetryWorker{checkout: facade}
```

新增风控校验 → **只改** `CheckoutFacade.PlaceOrder` 一处；各入口 **都不改**。


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

### 多个 Facade 划分边界

按 **用例** 拆，而不是一个 Facade 包全站：

| Facade | 职责 |
| :--- | :--- |
| `CheckoutFacade` | 下单、预占、支付、落库 |
| `RefundFacade` | 退款、释库存、冲发票 |
| `CatalogFacade` | 查商品、库存快照（只读） |

客户端只依赖需要的 Facade；子系统可在多个 Facade 间 **共享**。

### 测试策略

Facade 单测 **不测** SQL、HTTP、满减规则——那些在各子系统自己的测试里覆盖。Facade 只测 **用例编排**：**调用顺序**、**失败时是否补偿**、**成功时是否短路**。

| 测什么 | 在哪测 | 手段 |
| :--- | :--- | :--- |
| 库存算法、支付签名、SQL | 各 Subsystem 包 | 真实逻辑 + 集成测 |
| 预占 → 支付 → 落库顺序；支付失败 Release；落库失败 Refund+Release | `checkout_facade_test.go` | **Fake** 子系统，记录调用了谁 |
| HTTP 绑参、鉴权 | Controller 包 | mock `OrderCheckout` 接口 |
| 全链路 | e2e | 真实或 testcontainer |

**Fake 要点**：实现与 Facade 相同的四个接口，用 **布尔 / 计数 / 切片** 记录「是否被调用、调用顺序、传入的 orderID」——不必模拟真实业务。

```go
type fakeInventory struct {
    reserveErr error
    released   bool
}

func (f *fakeInventory) Reserve([]OrderLine) error { return f.reserveErr }
func (f *fakeInventory) Release([]OrderLine) error {
    f.released = true
    return nil
}

type fakePayment struct {
    payErr    error
    refunded  bool
    refundID  string
}

func (f *fakePayment) Pay(order Order) error { return f.payErr }
func (f *fakePayment) Refund(orderID string) error {
    f.refunded = true
    f.refundID = orderID
    return nil
}

type fakeOrders struct {
    saveErr error
    saved   bool
}

func (fakeOrders) NextID() string { return "ord-1" }
func (f *fakeOrders) Save(context.Context, Order) error {
    f.saved = true
    return f.saveErr
}

type fakePricing struct{}

func (fakePricing) Total([]OrderLine) int64 { return 9900 }
```

用 **表驱动** 覆盖编排分支——每个 case 注入不同的 fake，断言 **副作用** 而非 error 字符串：

```go
func TestCheckoutFacade_PlaceOrder(t *testing.T) {
    lines := []OrderLine{{SKU: "tea-001", Quantity: 1}}
    req := PlaceOrderRequest{Lines: lines, UserID: "u1"}

    tests := []struct {
        name       string
        inv        *fakeInventory
        pay        *fakePayment
        orders     *fakeOrders
        wantErr    bool
        wantID     string
        wantSaved  bool
        wantRefund bool
        wantRelease bool
    }{
        {
            name:      "happy path",
            inv:       &fakeInventory{},
            pay:       &fakePayment{},
            orders:    &fakeOrders{},
            wantID:    "ord-1",
            wantSaved: true,
        },
        {
            name:        "pay fails releases inventory",
            inv:         &fakeInventory{},
            pay:         &fakePayment{payErr: errors.New("declined")},
            orders:      &fakeOrders{},
            wantErr:     true,
            wantRelease: true,
        },
        {
            name:        "save fails refunds and releases",
            inv:         &fakeInventory{},
            pay:         &fakePayment{},
            orders:      &fakeOrders{saveErr: errors.New("db down")},
            wantErr:     true,
            wantRefund:  true,
            wantRelease: true,
        },
        {
            name:    "reserve fails short-circuits",
            inv:     &fakeInventory{reserveErr: errors.New("oos")},
            pay:     &fakePayment{},
            orders:  &fakeOrders{},
            wantErr: true,
            // Pay / Save 不应被调用——可在 fakePayment 里加 called 字段断言
        },
    }

    for _, tt := range tests {
        t.Run(tt.name, func(t *testing.T) {
            f := CheckoutFacade{
                inventory: tt.inv,
                pricing:   fakePricing{},
                payment:   tt.pay,
                orders:    tt.orders,
            }
            id, err := f.PlaceOrder(context.Background(), req)

            if tt.wantErr && err == nil {
                t.Fatal("expected error")
            }
            if !tt.wantErr && err != nil {
                t.Fatalf("unexpected error: %v", err)
            }
            if id != tt.wantID {
                t.Fatalf("order id: got %q want %q", id, tt.wantID)
            }
            if tt.orders.saved != tt.wantSaved {
                t.Fatalf("saved: got %v want %v", tt.orders.saved, tt.wantSaved)
            }
            if tt.pay.refunded != tt.wantRefund {
                t.Fatalf("refunded: got %v want %v", tt.pay.refunded, tt.wantRefund)
            }
            if tt.inv.released != tt.wantRelease {
                t.Fatalf("released: got %v want %v", tt.inv.released, tt.wantRelease)
            }
        })
    }
}
```

**Controller 层** 不必再 fake 四个子系统——依赖 `OrderCheckout` 接口，注入只返回固定值的 `fakeCheckout` 即可；HTTP 编解码另写小测试。

**常见误区**：在 Facade 测试里断言 `Total()` 算出来是 9900——那是 `PricingEngine` 的职责；Facade 测试只需 `pricing` 被调用过（若需严格顺序，可在 fake 里 append 调用日志再 `cmp.Diff`）。

## 关联

- 外观模式为现有对象定义了一个新接口，[适配器模式](/cs-fundamentals/design-patterns/adapter) 则会试图运用已有的接口。适配器通常只封装一个对象，外观通常会作用于整个对象子系统上。
- 当只需对客户端代码隐藏子系统创建对象的方式时，你可以使用 [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory) 来代替外观。
- [享元模式](/cs-fundamentals/design-patterns/flyweight) 展示了如何生成大量的小型对象，外观则展示了如何用一个对象来代表整个子系统。
- 外观和 [中介者模式](/cs-fundamentals/design-patterns/mediator) 的职责类似：它们都尝试在大量紧密耦合的类中组织起合作。
  - 外观为子系统中的所有对象定义了一个简单接口，但是它不提供任何新功能。子系统本身不会意识到外观的存在。子系统中的对象可以直接进行交流。
  - 中介者将系统中组件的沟通行为中心化。各组件只知道中介者对象，无法直接相互交流。
- 外观类通常可以转换为 [单例模式](/cs-fundamentals/design-patterns/singleton) 类，因为在大部分情况下一个外观对象就足够了。
- 外观与 [代理模式](/cs-fundamentals/design-patterns/proxy) 的相似之处在于它们都缓存了一个复杂实体并自行对其进行初始化。代理与其服务对象遵循同一接口，使得自己和服务对象可以互换，在这一点上它与外观不同。

## 参考阅读

- [x] [Refactoring.Guru - 外观模式](https://refactoringguru.cn/design-patterns/facade) (2026-06-22)
- [x] [菜鸟教程 - 外观模式](https://www.runoob.com/design-pattern/facade-pattern.html) (2026-06-22)
