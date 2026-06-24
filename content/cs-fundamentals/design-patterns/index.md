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

## 设计模式

### 创建型模式

| 模式 | 别名 | 描述 |
| :--- | :--- | :--- |
| [工厂方法模式](/cs-fundamentals/design-patterns/factory) | 虚拟构造器、Factory Method | 提供一种创建对象的方式，使创建过程与使用过程分离。 |
| [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory) | Abstract Factory | 提供创建一系列相关或相互依赖对象的接口，而无需指定它们具体的类。 |
| [生成器模式](/cs-fundamentals/design-patterns/builder) | 建造者模式、Builder | 分步骤构建复杂对象，将构建过程与最终表示分离。 |
| [原型模式](/cs-fundamentals/design-patterns/prototype) | 克隆、Clone、Prototype | 通过复制已有实例创建新对象，将克隆过程与使用过程分离。 |
| [单例模式](/cs-fundamentals/design-patterns/singleton) | 单件模式、Singleton | 保证一个类仅有一个实例，并提供一个访问它的全局访问点。 |

### 结构型模式

| 模式 | 别名 | 描述 |
| :--- | :--- | :--- |
| [适配器模式](/cs-fundamentals/design-patterns/adapter) | 封装器模式、Wrapper、Adapter | 将一个类的接口转换成客户期望的另一个接口，使原本因接口不兼容而不能一起工作的类可以一起工作。 |
| [桥接模式](/cs-fundamentals/design-patterns/bridge) | Bridge | 将抽象部分与实现部分分离，使它们都可以独立地变化。 |
| [组合模式](/cs-fundamentals/design-patterns/composite) | 对象树、Object Tree、Composite | 将对象组合成树形结构以表示部分-整体层次，使客户端对单个对象和组合对象的使用具有一致性。 |
| [装饰器模式](/cs-fundamentals/design-patterns/decorator) | 装饰模式、装饰者模式、Wrapper、Decorator | 动态地给对象添加额外的职责，比生成子类更灵活地扩展功能。 |
| [外观模式](/cs-fundamentals/design-patterns/facade) | 门面模式、Facade | 为复杂子系统提供统一的高层接口，使客户端更容易使用整组能力。 |
| [享元模式](/cs-fundamentals/design-patterns/flyweight) | 缓存、Cache、Flyweight | 运用共享技术支持大量细粒度对象，将内部状态与外部状态分离并按键复用。 |
| [代理模式](/cs-fundamentals/design-patterns/proxy) | Proxy | 为其他对象提供替身以控制对其的访问，在委托真实主题前后插入懒加载、鉴权、远程或缓存等逻辑。 |

### 行为型模式

| 模式 | 别名 | 描述 |
| :--- | :--- | :--- |
| [责任链模式](/cs-fundamentals/design-patterns/chain-of-responsibility) | 职责链模式、命令链、CoR、Chain of Command、Chain of Responsibility | 使多个对象都有机会处理请求，将处理者连成链并沿链传递，直到有对象处理或全会签通过。 |
| [命令模式](/cs-fundamentals/design-patterns/command) | 动作、事务、Action、Transaction、Command | 将请求封装为对象，从而支持参数化、排队、日志记录以及撤销与重做。 |
| [迭代器模式](/cs-fundamentals/design-patterns/iterator) | Iterator | 提供顺序访问聚合元素的方法而不暴露内部表示，支持多种遍历策略与分页流式访问。 |
| [中介者模式](/cs-fundamentals/design-patterns/mediator) | 调解人、控制器、Intermediary、Controller、Mediator | 用中介对象封装一组对象之间的交互，使同事不必相互引用，降低网状耦合并集中联动规则。 |
| [备忘录模式](/cs-fundamentals/design-patterns/memento) | 快照、Snapshot、Memento | 在不破坏封装的前提下捕获并外部保存对象状态，以便之后由发起人将其恢复到先前状态。 |
| [观察者模式](/cs-fundamentals/design-patterns/observer) | 事件订阅者、监听者、Event-Subscriber、Listener、Observer | 定义一对多依赖，主题状态变更时自动通知所有观察者，使写状态与扇出副作用解耦。 |
| [状态模式](/cs-fundamentals/design-patterns/state) | State | 允许对象在内部状态改变时改变行为，将各态允许的操作与状态迁移封装在独立 State 中。 |
| [策略模式](/cs-fundamentals/design-patterns/strategy) | Strategy | 封装一族可互换算法，使它们可独立于客户端替换，消除 Context 内的条件分支。 |
| [模板方法模式](/cs-fundamentals/design-patterns/template-method) | Template Method | 在模板方法中定义算法骨架，将可变步骤延迟到子类或钩子，复用流程并固定顺序与补偿。 |
| [访问者模式](/cs-fundamentals/design-patterns/visitor) | Visitor | 在不改变元素类的前提下，将作用于结构上各元素的操作封装为访问者，通过 Accept/Visit 双重分派。 |

## 其他资源

- [ ] [UML 统一建模语言概览 (视频)](https://www.youtube.com/watch?v=3cmzqZzwNDM&list=PLGLfVvz_LVvQ5G-LdJ8RLqe-ndo7QITYc&index=3)
- [ ] [系列视频（27 个）](https://www.youtube.com/playlist?list=PLF206E906175C7E07)
- [ ] [书籍：《Head First 设计模式》](https://www.amazon.com/Head-First-Design-Patterns-Freeman/dp/0596007124)
- [Handy reference: 101 Design Patterns & Tips for Developers](https://sourcemaking.com/design-patterns-and-tips)
