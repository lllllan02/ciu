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

| 模式 | 描述 |
| :--- | :--- |
| [工厂方法模式](/cs-fundamentals/design-patterns/factory) | 提供一种创建对象的方式，使创建过程与使用过程分离。 |
| [抽象工厂模式](/cs-fundamentals/design-patterns/abstract-factory) | 提供创建一系列相关或相互依赖对象的接口，而无需指定它们具体的类。 |
| [生成器模式](/cs-fundamentals/design-patterns/builder) | 分步骤构建复杂对象，将构建过程与最终表示分离。 |
| [原型模式](/cs-fundamentals/design-patterns/prototype) | 通过复制已有实例创建新对象，将克隆过程与使用过程分离。 |

## 其他资源

- [ ] [UML 统一建模语言概览 (视频)](https://www.youtube.com/watch?v=3cmzqZzwNDM&list=PLGLfVvz_LVvQ5G-LdJ8RLqe-ndo7QITYc&index=3)
- [ ] [系列视频（27 个）](https://www.youtube.com/playlist?list=PLF206E906175C7E07)
- [ ] [书籍：《Head First 设计模式》](https://www.amazon.com/Head-First-Design-Patterns-Freeman/dp/0596007124)
- [Handy reference: 101 Design Patterns & Tips for Developers](https://sourcemaking.com/design-patterns-and-tips)
