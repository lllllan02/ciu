---
title: 设计模式
order: 1
---

设计模式讨论的是**可复用的对象协作方式**；理解它们之前，最好先熟悉下面这些面向对象设计原则。模式文档里提到的「违反某某原则」，通常指的就是这些。

## 设计原则

### SOLID

| 原则 | 英文 | 要点 |
| :--- | :--- | :--- |
| **单一职责** | Single Responsibility (SRP) | 一个类只应有一个引起它变化的原因；职责过多时拆分。 |
| **开闭** | Open/Closed (OCP) | 对扩展开放、对修改关闭；加新行为时尽量新增代码，而不是改旧代码。 |
| **里氏替换** | Liskov Substitution (LSP) | 子类对象应能替换父类对象使用，且不破坏程序正确性。 |
| **接口隔离** | Interface Segregation (ISP) | 客户端不应被迫依赖它用不到的接口；大接口拆成小接口。 |
| **依赖倒置** | Dependency Inversion (DIP) | 高层模块不依赖低层模块的具体实现，二者都依赖抽象（接口/抽象类）。 |

### 其他常见原则

| 原则 | 要点 |
| :--- | :--- |
| **合成复用** | 优先用组合（has-a）扩展行为，而不是靠继承（is-a）堆叠子类。 |
| **迪米特法则** | 又称最少知识原则：对象只与直接朋友通信，少跨层调用，降低耦合。 |
| **针对接口编程** | 声明类型、传参、返回值尽量用抽象类型，运行时再绑定具体实现。 |

> 更详细的 SOLID 学习资源见 [选修扩展 · SOLID](/optional#一些主题的额外内容)。

- [x] [菜鸟教程 - 设计模式介绍](https://www.runoob.com/design-pattern/design-pattern-intro.html) (2026-06-17)
- [x] [Refactoring.Guru - 设计模式](https://refactoringguru.cn/design-patterns) (2026-06-17)

## 贯穿示例：电商订单系统

本章多篇文档会沿用同一个「电商订单系统」示例，但每篇关注的是不同层面的设计问题：不是反复重写同一个订单系统，而是从一个小场景逐步展开它在真实项目里会遇到的变化。

- **创建型模式** 关注「对象怎么来」：先选择支付宝 / 微信支付 / 信用卡等支付渠道，再把支付渠道、订单模板、日志成套组装；当单条订单字段越来越多时，用生成器构建请求；当订单模板已经预先构建好时，用原型复制；当结算中心必须全局共享限流和连接池时，再讨论单例。
- **结构型模式** 关注「对象怎么拼」：已有银行网关、支付 SDK 与内部 `PaymentProcessor` 接口不一致时，用适配器接入；当支付请求形态（直接支付、分期、退款）和支付后端（支付宝、微信支付、Stripe）都会独立扩展时，用桥接拆开两个变化维度；当订单明细从单件 SKU 扩展为套餐、礼盒等嵌套结构时，用组合让结算与库存预占对整棵树使用同一套接口；当同一行明细要灵活叠加会员价、优惠券、礼品包装等可选增强时，用装饰器动态包装而不必穷举子类；当下单流程要依次穿过校验、库存、计价、支付、落库、通知等多个子系统时，用外观提供统一的 `PlaceOrder` 入口，避免每个 HTTP 控制器重复编排；当大促导出或批量对账产生海量明细、同一 SKU 的商品元数据在内存中重复出现时，用享元分离内部状态与外部状态并按 SKU 共享；当订单查询需要懒加载明细、读操作要统一鉴权、或库存服务在远端且需要缓存与超时控制时，用代理在与仓库/库存相同的接口上插入访问控制再委托给真实实现。

因此，读每篇时可以把它理解为同一个电商订单系统在不同阶段遇到的新约束：**选实现、配套装、构建订单、复用订单模板、共享结算、兼容外部接口、拆分变化维度**。

## 设计模式

### 创建型模式

| 模式 | 描述 |
| :--- | :--- |
| [工厂方法模式](/cs-fundamentals/design-patterns/factory) | 提供一种创建对象的方式，使创建过程与使用过程分离。 |
| [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory) | 提供创建一系列相关或相互依赖对象的接口，而无需指定它们具体的类。 |
| [生成器模式](/cs-fundamentals/design-patterns/builder) | 分步骤构建复杂对象，将构建过程与最终表示分离。 |
| [原型模式](/cs-fundamentals/design-patterns/prototype) | 通过复制已有实例创建新对象，将克隆过程与使用过程分离。 |
| [单例模式](/cs-fundamentals/design-patterns/singleton) | 保证一个类仅有一个实例，并提供一个访问它的全局访问点。 |

### 结构型模式

| 模式 | 描述 |
| :--- | :--- |
| [适配器模式](/cs-fundamentals/design-patterns/adapter) | 将一个类的接口转换成客户期望的另一个接口，使原本因接口不兼容而不能一起工作的类可以一起工作。 |
| [桥接模式](/cs-fundamentals/design-patterns/bridge) | 将抽象部分与实现部分分离，使它们都可以独立地变化。 |
| [组合模式](/cs-fundamentals/design-patterns/composite) | 将对象组合成树形结构以表示部分-整体层次，使客户端对单个对象和组合对象的使用具有一致性。 |
| [装饰器模式](/cs-fundamentals/design-patterns/decorator) | 动态地给对象添加额外的职责，比生成子类更灵活地扩展功能。 |
| [外观模式](/cs-fundamentals/design-patterns/facade) | 为复杂子系统提供统一的高层接口，使客户端更容易使用整组能力。 |
| [享元模式](/cs-fundamentals/design-patterns/flyweight) | 运用共享技术支持大量细粒度对象，将内部状态与外部状态分离并按键复用。 |
| [代理模式](/cs-fundamentals/design-patterns/proxy) | 为其他对象提供替身以控制对其的访问，在委托真实主题前后插入懒加载、鉴权、远程或缓存等逻辑。 |

## 其他资源

- [ ] [UML 统一建模语言概览 (视频)](https://www.youtube.com/watch?v=3cmzqZzwNDM&list=PLGLfVvz_LVvQ5G-LdJ8RLqe-ndo7QITYc&index=3)
- [ ] [系列视频（27 个）](https://www.youtube.com/playlist?list=PLF206E906175C7E07)
- [ ] [书籍：《Head First 设计模式》](https://www.amazon.com/Head-First-Design-Patterns-Freeman/dp/0596007124)
- [Handy reference: 101 Design Patterns & Tips for Developers](https://sourcemaking.com/design-patterns-and-tips)
