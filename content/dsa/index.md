---
title: 数据结构与算法
description: 面试核心：线性结构、树、图、排序与高级算法。
---

按「线性结构 → 算法技巧 → 树 → 排序 → 图 → 高级算法」的顺序学习；每一类都包含概念说明、学习资源与动手实现。

## 学习路线

| 部分 | 说明 |
| :--- | :--- |
| [线性结构](#线性结构) | 数组、链表、栈、队列、哈希表 |
| [算法技巧](#算法技巧) | 二分查找、按位运算 |
| [树](#树) | 树遍历、BST、堆与优先级队列 |
| [排序](#排序) | 比较排序、稳定性与线性时间排序 |
| [图](#图) | 表示法、BFS/DFS、最短路与最小生成树 |
| [高级算法](#高级算法) | 递归、回溯与动态规划 |

---

## 线性结构

线性结构（数组、链表、栈、队列）与基于哈希的映射是后续树、图和算法题的基础。

### 数组（Arrays）

数组把元素放在**连续内存**里，用下标可在 $O(1)$ 时间内随机访问任意位置；在末尾增删通常也很快，但在中间插入或删除需要挪动后面的元素，一般是 $O(n)$。固定长度的数组容量写死；**动态数组**在元素变多时自动扩容（常见做法是容量翻倍），是多数语言里 `vector`、列表等容器的底层思路。

- [x] [Harvard CS50 - 数组（视频）](https://www.youtube.com/watch?v=tI_tIZFyKBw&t=3009s) (2026-01-29)
- [ ] [UC San Diego - 数组（视频）](https://www.coursera.org/lecture/data-structures/arrays-OsBSF)
- [ ] [UC San Diego - 动态数组（视频）](https://www.coursera.org/lecture/data-structures/dynamic-arrays-EwbnV)
- [ ] [UC Berkeley CS61B - 线性和多维数组（视频）](https://archive.org/details/ucberkeley_webcast_Wp8oiO_CZZE)（从 15 分 32 秒开始）
- [ ] [Python - 嵌套列表（视频）](https://www.youtube.com/watch?v=1jtrQqYpt7g)

> - [x] [实现动态数组](/dsa/arrays) (2026-02-01)

### 链表（Linked Lists）

链表由一个个**节点**串成，每个节点存数据和指向下一个节点的指针；元素在内存里不必连续。已知位置时，在头尾或中间插入、删除往往只需改指针，是 $O(1)$；但按序号访问要从头往后走，是 $O(n)$。**单向链表**只指向前驱；**双向链表**多一个指向前节点的指针，便于从尾部往前删改，代价是每个节点多占一点空间。

- [x] [Harvard CS50 - 链表（视频）](https://www.youtube.com/watch?v=2T-A_GFuoTo&t=650s) (2026-02-07)
- [x] [Michael Sambol - 4 分钟了解链表（视频）](https://youtu.be/F8AbOfQwl1c) (2026-02-28)
- [x] [MyCodeSchool - 链表 C 语言实现（视频）](https://www.youtube.com/watch?v=QN6FPiD0Gzo) 不是整个视频，只是关于 Node 结构和内存分配的部分。(2026-02-28)
- [x] [Steve Summit - 指向指针的指针（文章）](https://www.eskimo.com/~scs/cclass/int/sx8.html) 的确：你需要关于"指向指针的指针"的相关知识：（因为当你传递一个指针到一个函数时，该函数可能会改变指针所指向的地址）该页只是为了让你了解"指向指针的指针"这一概念。但我并不推荐这种链式遍历的风格。因为，这种风格的代码，其可读性和可维护性太低。(2026-02-28)
- [ ] [UC Berkeley CS61B - 链表 1（视频）](https://archive.org/details/ucberkeley_webcast_htzJdKoEmO0)
- [ ] [UC Berkeley CS61B - 链表 2（视频）](https://archive.org/details/ucberkeley_webcast_-c4I3gFYe3w)
- [ ] [UC San Diego - 单链表（视频）](https://www.coursera.org/lecture/data-structures/singly-linked-lists-kHhgK)
- [ ] [UC San Diego - 链表 vs 数组：核心差异（视频）](https://www.coursera.org/lecture/data-structures-optimizing-performance/core-linked-lists-vs-arrays-rjBs9)
- [ ] [UC San Diego - 链表 vs 数组：现实世界应用（视频）](https://www.coursera.org/lecture/data-structures-optimizing-performance/in-the-real-world-lists-vs-arrays-QUaUd)
- [ ] [UC San Diego - 双向链表介绍（视频）](https://www.coursera.org/learn/data-structures/lecture/jpGKD/doubly-linked-lists) 并不需要实现。

> - [x] [实现单向链表](/dsa/linked-lists) (2026-02-28)

### 堆栈（Stack）

堆栈是一种 **后进先出（LIFO）** 的线性结构：只在同一端压入（push）和弹出（pop），查看栈顶也是 $O(1)$。用数组或链表都能实现，数组版更简单。常见于函数调用栈、括号匹配、DFS、撤销操作等「先处理最近压入的」场景。

- [x] [Michael Sambol - Stacks in 3 minutes](https://youtu.be/KcT3aVgrrpU) (2026-03-01)
- [ ] [UC San Diego - 堆栈](https://www.coursera.org/learn/data-structures/lecture/UdKzQ/stacks)

可以不实现，因为使用数组来实现是微不足道的事。

### 队列（Queue）

队列是一种 **先进先出（FIFO）** 的线性结构：从一端入队（enqueue）、另一端出队（dequeue），两端操作都是 $O(1)$。与堆栈相对，适合 BFS、任务调度、消息缓冲等「先来的先处理」场景。用数组实现时常配合**循环队列**，避免频繁搬移元素；用链表实现则头尾各维护指针即可。

- [x] [Michael Sambol - Queues in 3 minutes](https://youtu.be/D6gu-_tmEpQ) (2026-03-01)
- [ ] [圆形队列](https://en.wikipedia.org/wiki/Circular_buffer)
- [ ] [UC San Diego - 队列](https://www.coursera.org/learn/data-structures/lecture/EShpq/queue)

> - [x] [实现队列](/dsa/queue) (2026-03-04)

### 哈希表（Hash table）

哈希表用**哈希函数**把键映射到桶下标，在理想情况下插入、查找、删除的平均时间都是 $O(1)$。不同键可能落到同一位置（冲突），常见处理方式是**链式法**（每个桶挂链表）或**开放寻址**（在表里探测下一个空位）。语言里的 `dict` / `map`、缓存、计数表等都依赖这一结构；负载因子过高时要扩容并 rehash。

- [x] [OI Wiki - 哈希表](https://oi-wiki.org/ds/hash/) (2026-05-29)
- [x] [菜鸟教程 - 哈希表](https://www.runoob.com/data-structures/dsa-hash-table.html) (2026-05-29)
- [ ] [链式哈希表（视频）](https://www.youtube.com/watch?v=0M_kIqhwbFo&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=8)
- [ ] [Table Doubling 和 Karp-Rabin（视频）](https://www.youtube.com/watch?v=BRO7mVIFt08&index=9&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [Open Addressing 和密码型哈希（Cryptographic Hashing）（视频）](https://www.youtube.com/watch?v=rvdJDijO2Ro&index=10&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [PyCon 2010：强大的字典（视频）](https://www.youtube.com/watch?v=C4Kc8xzcA68)
- [ ] [PyCon 2017：字典更强大（视频）](https://www.youtube.com/watch?v=66P5FMkWoVU)
- [ ] [(高级) 随机化：通用和完美哈希（视频）](https://www.youtube.com/watch?v=z0lJ2k0sl1g&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp&index=11)
- [ ] [(进阶)完美哈希（Perfect hashing）（视频）](https://www.youtube.com/watch?v=N0COwN14gt0&list=PL2B4EEwhKD-NbwZ4ezj7gyc_3yNrojKM9&index=4)
- [ ] [[复习]4 分钟了解哈希表（视频）](https://youtu.be/knV86FlSXJ8)
- [ ] [核心哈希表（视频）](https://www.coursera.org/lecture/data-structures-optimizing-performance/core-hash-tables-m7UuP)
- [ ] [数据结构（视频）](https://www.coursera.org/learn/data-structures/home/week/4)
- [ ] [电话簿问题（视频）](https://www.coursera.org/lecture/data-structures/phone-book-problem-NYZZP)

#### 分布式哈希表

- [ ] [Dropbox 中的即时上传和存储优化（视频）](https://www.coursera.org/lecture/data-structures/instant-uploads-and-storage-optimization-in-dropbox-DvaIb)
- [ ] [分布式哈希表（视频）](https://www.coursera.org/lecture/data-structures/distributed-hash-tables-tvH8H)

> - [x] [实现链式哈希表](/dsa/hash-tables) (2026-06-01)

---

## 算法技巧

### 二分查找（Binary search）

二分查找在 **已排序** 的序列上工作：每次取中间元素与目标比较，根据大小关系丢弃左半或右半，直到找到或区间为空。每轮比较都能排除约一半元素，时间复杂度 $O(\log n)$；用循环实现时额外空间只需 $O(1)$。这是分治思想的典型应用，也是找边界、最小满足条件等许多算法题的基础模板。

- [x] [OI Wiki - 二分查找](https://oi-wiki.org/basic/binary/) (2026-06-01)
- [ ] [二分查找（视频）](https://www.youtube.com/watch?v=D5SrAga1pno)
- [ ] [二分查找（视频）](https://www.khanacademy.org/computing/computer-science/algorithms/binary-search/a/binary-search)
- [ ] [详情](https://www.topcoder.com/community/data-science/data-science-tutorials/binary-search/)
- [ ] [蓝图](https://leetcode.com/discuss/general-discussion/786126/python-powerful-ultimate-binary-search-template-solved-many-problems)
- [ ] [【复习】四分钟二分查找(视频)](https://youtu.be/fDKIpRe8GW4)

> - [x] [实现二分查找](/dsa/binary-search) (2026-06-02)

### 按位运算（Bitwise operations）

按位运算在底层表示、掩码、权限标志和性能优化里很常见；面试里常考 2 的幂、符号位与经典 bit trick。下面按「进制与速查 → 运算符 → 补码 → 技巧 → 置位与交换」顺序展开。

#### 2 的幂与速查

- [x] [OI Wiki - 进位制](https://oi-wiki.org/math/numeral-sys/base/) (2026-06-04)
- [ ] [Bits 速查表](https://github.com/jwasham/coding-interview-university/blob/main/extras/cheat%20sheets/bits-cheat-sheet.pdf) ── 你需要知道大量 2 的幂数值（从 2^1 到 2^16 及 2^32）

#### 位运算符

好好理解位操作符的含义：&、|、^、~、>>、<<

- [x] [菜鸟教程 - 位运算](https://www.runoob.com/w3cnote/bit-operation.html) (2026-06-03)
- [x] [OI Wiki - 位操作](https://oi-wiki.org/misc/bit/) (2026-06-03)
- [ ] [Wikipedia - 位操作](https://en.wikipedia.org/wiki/Bit_manipulation)
- [ ] [Wikipedia - 按位运算](https://en.wikipedia.org/wiki/Bitwise_operation)
- [ ] [字码（words）](https://en.wikipedia.org/wiki/Word_(computer_architecture))
- [ ] [位操作（视频）](https://www.youtube.com/watch?v=7jkIUgLC29I)
- [ ] [C 语言编程教程 2-10：按位运算（视频）](https://www.youtube.com/watch?v=d0AwjSpNXR0)
- [ ] [位元抚弄者（The Bit Twiddler）](http://bits.stephan-brumme.com/)
- [ ] [交互式位元抚弄者（The Bit Twiddler Interactive）](http://bits.stephan-brumme.com/interactive.html)
- [ ] [练习位操作](https://pconrad.github.io/old_pconrad_cs16/topics/bitOps/)
- [x] [绝对整型（Absolute Integer）](/dsa/bitwise-absolute-integer) (2026-06-03)

#### 补数与补码

- [x] [菜鸟教程 - 原码、反码、补码](https://www.runoob.com/w3cnote/sign-magnitude.html) (2026-06-04)
- [ ] [补数（1s Complement）](https://en.wikipedia.org/wiki/Ones%27_complement)
- [ ] [补码（2s Complement）](https://en.wikipedia.org/wiki/Two%27s_complement)
- [ ] [二进制：利 & 弊（为什么我们要使用补码）（视频）](https://www.youtube.com/watch?v=lKTsv6iVxV4)

#### 位运算技巧

- [ ] [Bithacks](https://graphics.stanford.edu/~seander/bithacks.html)
- [ ] [位操作技巧（Bit Hacks）（视频）](https://www.youtube.com/watch?v=ZusiKXcz_ac)

#### 计算置位（Set Bits）

- [ ] [计算一个字节中置位（Set Bits）的四种方式（视频）](https://youtu.be/Hzuzo9NJrlc)
- [ ] [计算比特位](https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan)
- [ ] [如何在一个 32 位的整型中计算置位（Set Bits）的数量](http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer)

#### 交换

- [ ] [交换（Swap）](http://bits.stephan-brumme.com/swap.html)

---

## 树

树是一种**层次化**的非线性结构：节点通过边连接，有且仅有一个根，除根外每个节点只有一个父节点，且不存在环。树是文件系统、DOM、表达式解析以及堆、BST 等许多数据结构的基础。

### 树基础（Trees）

树由**节点**和**边**组成：从根到任意节点有唯一路径。在**二叉树**中，每个节点最多有左、右两个孩子。访问所有节点常用 **BFS（广度优先 / 层序）** 或 **DFS（深度优先：先序、中序、后序）**；二者也是图遍历与大量面试题的基础模板，遍历一棵含 $n$ 个节点的树时间复杂度为 $O(n)$。

- [x] [菜鸟教程 - 树形结构](https://www.runoob.com/data-structures/dsa-tree.html) (2026-06-04)
- [x] [OI Wiki - 树基础](https://oi-wiki.org/graph/tree-basic/) (2026-06-05)
- [ ] [树的介绍（视频）](https://www.coursera.org/learn/data-structures/lecture/95qda/trees)
- [ ] [树遍历（视频）](https://www.coursera.org/lecture/data-structures/tree-traversal-fr51b)
- [ ] [BFS（广度优先搜索）和 DFS（深度优先搜索）（视频）](https://www.youtube.com/watch?v=uWL6FJhq5fM)
- [ ] [[复习]4 分钟内的广度优先搜索（视频）](https://youtu.be/HZ5YTanv5QE)
- [ ] [[复习] 4 分钟内的深度优先搜索（视频）](https://youtu.be/Urx87-NMm6c)
- [ ] [[复习]11 分钟内的树遍历（播放列表）（视频）](https://www.youtube.com/playlist?list=PL9xmBV_5YoZO1JC2RgEi04nLy6D-rKk6b)

### 二叉查找树（Binary search trees）

**二叉搜索树（BST）** 在二叉树基础上满足 BST 性质：左子树所有键值**小于**当前节点，右子树**大于**当前节点，**中序遍历**得到升序序列。树较平衡时，查找、插入、删除的平均时间均为 $O(\log n)$；若按有序顺序插入会退化为链表，最坏为 $O(n)$。语言里的 `std::map` / `TreeMap` 等有序容器都建立在这一思想上，工程实现通常配合 AVL、红黑树等**平衡树**维持高度。

- [x] [OI Wiki - 二叉搜索树 & 平衡树](https://oi-wiki.org/ds/bst/) (2026-06-05)
- [ ] [二叉搜索树复习（视频）](https://www.youtube.com/watch?v=x6At0nzX92o&index=1&list=PLA5Lqm4uh9Bbq-E0ZnqTIa8LRaL77ica6)
- [ ] [介绍（视频）](https://www.coursera.org/learn/data-structures/lecture/E7cXP/introduction)
- [ ] [MIT（视频）](https://www.youtube.com/watch?v=9Jry5-82I68)
- C/C++:
    - [ ] [二叉查找树 —— 在 C/C++ 中实现（视频）](https://www.youtube.com/watch?v=COZK7NATh4k&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P&index=28)
    - [ ] [BST 的实现 —— 在堆栈和堆中的内存分配（视频）](https://www.youtube.com/watch?v=hWokyBoo0aI&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P&index=29)
    - [ ] [在二叉查找树中找到最小和最大的元素（视频）](https://www.youtube.com/watch?v=Ut90klNN264&index=30&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P)
    - [ ] [寻找二叉树的高度（视频）](https://www.youtube.com/watch?v=_pnqMz5nrRs&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P&index=31)
    - [ ] [二叉树的遍历 —— 广度优先和深度优先策略（视频）](https://www.youtube.com/watch?v=9RHO6jU--GU&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P&index=32)
    - [ ] [二叉树：层序遍历（视频）](https://www.youtube.com/watch?v=86g8jAQug04&index=33&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P)
    - [ ] [二叉树的遍历：先序、中序、后序（视频）](https://www.youtube.com/watch?v=gm8DUJJhmY4&index=34&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P)
    - [ ] [判断一棵二叉树是否为二叉查找树（视频）](https://www.youtube.com/watch?v=yEwSGhSsT0U&index=35&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P)
    - [ ] [从二叉查找树中删除一个节点（视频）](https://www.youtube.com/watch?v=gcULXE7ViZw&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P&index=36)
    - [ ] [二叉查找树中序遍历的后继者（视频）](https://www.youtube.com/watch?v=5cPbNCrdotA&index=37&list=PL2_aWCzGMAwI3W_JlcBbtYTwiQSsOTa6P)

> - [x] [实现二叉搜索树](/dsa/binary-search-trees) (2026-06-05)
> - [x] [实现平衡二叉搜索树](/dsa/balanced-binary-search-trees) (2026-06-08)

### 堆（Heap） / 优先级队列（Priority Queue） / 二叉堆（Binary Heap）

**堆**是一棵满足堆性质的完全二叉树：大顶堆中父节点不小于子节点，小顶堆反之。用数组存储时，父子下标有固定换算关系，插入与删除极值可在 $O(\log n)$ 内完成；**优先级队列**的底层通常就是堆。堆排序、Top-K、Dijkstra 等场景都依赖这一结构。

- [x] [百度百科 - 完全二叉树](https://baike.baidu.com/item/%E5%AE%8C%E5%85%A8%E4%BA%8C%E5%8F%89%E6%A0%91/7773232) (2026-06-08)
- [x] [OI Wiki - 堆简介](https://oi-wiki.org/ds/heap/) (2026-06-08)
- [x] [OI Wiki - 二叉堆](https://oi-wiki.org/ds/binary-heap/) (2026-06-08)
- [x] [Hello Algo - 堆](https://www.hello-algo.com/chapter_heap/heap/) (2026-06-08)
- [x] [Hello Algo - 构建堆](https://www.hello-algo.com/chapter_heap/build_heap/) (2026-06-09)
- [x] [菜鸟教程 - 堆排序](https://www.runoob.com/w3cnote/heap-sort.html) (2026-06-09)
- [ ] [堆（Heap）](https://en.wikipedia.org/wiki/Heap_(data_structure))
- [ ] [堆简介（视频）](https://www.coursera.org/lecture/data-structures/introduction-2OpTs)
- [ ] [二叉树（视频）](https://www.coursera.org/learn/data-structures/lecture/GRV2q/binary-trees)
- [ ] [树高度备注（视频）](https://www.coursera.org/learn/data-structures/supplement/S5xxz/tree-height-remark)
- [ ] [基本操作（视频）](https://www.coursera.org/learn/data-structures/lecture/0g1dl/basic-operations)
- [ ] [完全二叉树（视频）](https://www.coursera.org/learn/data-structures/lecture/gl5Ni/complete-binary-trees)
- [ ] [伪代码（视频）](https://www.coursera.org/learn/data-structures/lecture/HxQo9/pseudocode)
- [ ] [堆排序 - 跳转到开始部分（视频）](https://youtu.be/odNJmw5TOEE?list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&t=3291)
- [ ] [堆排序（视频）](https://www.coursera.org/lecture/data-structures/heap-sort-hSzMO)
- [ ] [构建堆（视频）](https://www.coursera.org/lecture/data-structures/building-a-heap-dwrOS)
- [ ] [MIT：堆和堆排序（视频）](https://www.youtube.com/watch?v=B7hVxCmfPtM&index=4&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [CS 61B Lecture 24：优先队列（视频）](https://archive.org/details/ucberkeley_webcast_yIUFT6AKBGE)
- [ ] [线性时间构建堆（大顶堆）](https://www.youtube.com/watch?v=MiyLo8adrWw)
- [ ] [[复习] 13 分钟了解堆（视频）](https://www.youtube.com/playlist?list=PL9xmBV_5YoZNsyqgPW-DNwUeT8F8uhWc6)

> - [x] [实现大顶堆](/dsa/max-heap) (2026-06-08)
> - [x] [实现堆排序](/dsa/heap-sort) (2026-06-09)
> - [x] [线性时间构建堆](/dsa/build-heap) (2026-06-09)

---

## 排序（Sorting）

排序是把一组元素按键值大小重排成有序序列的操作。常见基于比较的算法在平均与最坏时间、额外空间、是否**稳定**以及适用容器（数组还是链表）上各有取舍；面试里既要能讲清思路与复杂度，也要能手写归并、快排等核心实现。下面按「概念 → 课程 → 视频 → 代码 → 动手实现 → 补充」顺序展开；堆排序见前文 [堆](#堆heap--优先级队列priority-queue--二叉堆binary-heap) 一节。

### 稳定性

若两个键相等的对象在排序输出中与输入数据集中的顺序相同，则称该排序算法是**稳定的**（常见面试题：「快排是稳定的吗？」——标准实现不是）。

- [x] [排序算法的稳定性](https://en.wikipedia.org/wiki/Sorting_algorithm#Stability) (2026-06-09)
- [x] [排序算法的稳定性](http://www.geeksforgeeks.org/stability-in-sorting-algorithms/) (2026-06-09)
- [ ] [排序算法的稳定性](http://stackoverflow.com/questions/1517793/stability-in-sorting-algorithms)
- [ ] [排序算法 - 稳定性](http://homepages.math.uic.edu/~leon/cs-mcs401-s08/handouts/stability.pdf)

### 数组与链表

并不推荐直接对链表做通用排序，但**归并排序**在链表上可行且效率高；多数其它经典算法（快排、堆排序等）更适合随机访问的数组。

- [ ] [链表的归并排序](http://www.geeksforgeeks.org/merge-sort-for-linked-list/)

### 堆排序

堆排序很强大，不过是非稳定排序；实现与复杂度分析见前文 [堆的数据结构](#堆heap--优先级队列priority-queue--二叉堆binary-heap) 部分。

> - [x] [实现堆排序](/dsa/heap-sort) (2026-06-09)

### Coursera：Algorithms Part 1

#### 归并排序

- [ ] [1. 归并排序（Mergesort）](https://www.coursera.org/lecture/algorithms-part1/mergesort-ARWDq)
- [ ] [2. 自底向上的归并排序（Bottom up Mergesort）](https://www.coursera.org/learn/algorithms-part1/lecture/PWNEl/bottom-up-mergesort)
- [ ] [3. 排序复杂性（Sorting Complexity）](https://www.coursera.org/lecture/algorithms-part1/sorting-complexity-xAltF)
- [ ] [4. 比较器（Comparators）](https://www.coursera.org/lecture/algorithms-part1/comparators-9FYhS)
- [ ] [5. 稳定性（Stability）](https://www.coursera.org/learn/algorithms-part1/lecture/pvvLZ/stability)

#### 快速排序

- [ ] [1. 快速排序（Quicksort）](https://www.coursera.org/lecture/algorithms-part1/quicksort-vjvnC)
- [ ] [2. 选择排序（Selection）](https://www.coursera.org/lecture/algorithms-part1/selection-UQxFT)
- [ ] [3. 重复键（Duplicate Keys）](https://www.coursera.org/lecture/algorithms-part1/duplicate-keys-XvjPd)
- [ ] [4. 系统排序（System Sorts）](https://www.coursera.org/lecture/algorithms-part1/system-sorts-QBNZ7)

### 视频资源

#### UC Berkeley CS 61B

- [ ] [CS 61B Lecture 29：排序 I（视频）](https://archive.org/details/ucberkeley_webcast_EiUvYS2DT6I)
- [ ] [CS 61B Lecture 30：排序 II（视频）](https://archive.org/details/ucberkeley_webcast_2hTY3t80Qsk)
- [ ] [CS 61B Lecture 32：排序 III（视频）](https://archive.org/details/ucberkeley_webcast_Y6LOLpxg6Dc)
- [ ] [CS 61B Lecture 33：排序 V（视频）](https://archive.org/details/ucberkeley_webcast_qNMQ4ly43p4)
- [ ] [CS 61B 2014-04-21：基数排序（视频）](https://archive.org/details/ucberkeley_webcast_pvbBMd-3NoI)

#### 各经典算法

- [ ] [冒泡排序（视频）](https://www.youtube.com/watch?v=P00xJgWzz2c&index=1&list=PL89B61F78B552C1AB)
- [ ] [冒泡排序分析（视频）](https://www.youtube.com/watch?v=ni_zk257Nqo&index=7&list=PL89B61F78B552C1AB)
- [ ] [插入排序 & 归并排序（视频）](https://www.youtube.com/watch?v=Kg4bqzAqRBM&index=3&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [插入排序（视频）](https://www.youtube.com/watch?v=c4BRHC7kTaQ&index=2&list=PL89B61F78B552C1AB)
- [ ] [归并排序（视频）](https://www.youtube.com/watch?v=GCae1WNvnZM&index=3&list=PL89B61F78B552C1AB)
- [ ] [快排（视频）](https://www.youtube.com/watch?v=y_G9BkAm6B8&index=4&list=PL89B61F78B552C1AB)
- [ ] [选择排序（视频）](https://www.youtube.com/watch?v=6nDMgr0-Yyo&index=8&list=PL89B61F78B552C1AB)

#### [Review] 18 分钟排序综述

- [ ] [[Review] Sorting (playlist) in 18 minutes](https://www.youtube.com/playlist?list=PL9xmBV_5YoZOZSbGAXAPIq1BeUf4j20pl)
    - [ ] [Quick sort in 4 minutes (video)](https://youtu.be/Hoixgm4-P4M)
    - [ ] [Heap sort in 4 minutes (video)](https://youtu.be/2DmK_H7IdTo)
    - [ ] [Merge sort in 3 minutes (video)](https://youtu.be/4VqmGXwpLqc)
    - [ ] [Bubble sort in 2 minutes (video)](https://youtu.be/xli_FI7CuzA)
    - [ ] [Selection sort in 3 minutes (video)](https://youtu.be/g-PGLbMth_g)
    - [ ] [Insertion sort in 2 minutes (video)](https://youtu.be/JU767SDMDvA)

### 代码参考

#### 归并排序

- [ ] [使用外部数组（C 语言）](http://www.cs.yale.edu/homes/aspnes/classes/223/examples/sorting/mergesort.c)
- [ ] [使用外部数组（Python 语言）](https://github.com/jwasham/practice-python/blob/master/merge_sort/merge_sort.py)
- [ ] [对原数组直接排序（C++）](https://github.com/jwasham/practice-cpp/blob/master/merge_sort/merge_sort.cc)

#### 快速排序

- [ ] [实现（C 语言）](http://www.cs.yale.edu/homes/aspnes/classes/223/examples/randomization/quick.c)
- [ ] [实现（C 语言）](https://github.com/jwasham/practice-c/blob/master/quick_sort/quick_sort.c)
- [ ] [实现（Python 语言）](https://github.com/jwasham/practice-python/blob/master/quick_sort/quick_sort.py)

### 动手实现

- [ ] 归并：平均和最差情况的时间复杂度为 O(n log n)。
- [ ] 快排：平均时间复杂度为 O(n log n)。
- 选择排序和插入排序的最坏、平均时间复杂度都是 O(n^2)。
- 关于堆排序，请查看前文 [堆的数据结构](#堆heap--优先级队列priority-queue--二叉堆binary-heap) 部分。

### 补充：线性时间排序（可选）

- [Sedgewick──基数排序 (6 个视频)](https://www.coursera.org/learn/algorithms-part2/home/week/3)
    - [ ] [1. Java 中的字符串](https://www.coursera.org/learn/algorithms-part2/lecture/vGHvb/strings-in-java)
    - [ ] [2. 键值索引计数（Key Indexed Counting）](https://www.coursera.org/learn/algorithms-part2/lecture/2pi1Z/key-indexed-counting)
    - [ ] [3. Least Significant Digit First String Radix Sort](https://www.coursera.org/learn/algorithms-part2/lecture/c1U7L/lsd-radix-sort)
    - [ ] [4. Most Significant Digit First String Radix Sort](https://www.coursera.org/learn/algorithms-part2/lecture/gFxwG/msd-radix-sort)
    - [ ] [5. 3 中基数快速排序](https://www.coursera.org/learn/algorithms-part2/lecture/crkd5/3-way-radix-quicksort)
    - [ ] [6. 后继数组（Suffix Arrays）](https://www.coursera.org/learn/algorithms-part2/lecture/TH18W/suffix-arrays)
- [ ] [基数排序](http://www.cs.yale.edu/homes/aspnes/classes/223/notes.html#radixSort)
- [ ] [基数排序（视频）](https://www.youtube.com/watch?v=xhr26ia4k38)
- [ ] [基数排序, 计数排序 (线性时间内)（视频）](https://www.youtube.com/watch?v=Nz1KZXbghj8&index=7&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [随机算法: 矩阵相乘, 快排, Freivalds' 算法（视频）](https://www.youtube.com/watch?v=cNB2lADK3_s&index=8&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)
- [ ] [线性时间内的排序（视频）](https://www.youtube.com/watch?v=pOKy3RZbSws&list=PLUl4u3cNGP61hsJNdULdudlRL493b-XZf&index=14)

总结一下，这是 [15 种排序算法](https://www.youtube.com/watch?v=kPRA0W1kECg) 的可视化表示。如果你需要有关此主题的更多详细信息，请参阅「[一些主题的额外内容](/optional#一些主题的额外内容)」中的「排序」部分。

---

## 图（Graphs）

图用来表示节点（顶点）与边之间的关系，是计算机科学中许多问题的自然建模方式；本节内容较多，建议先掌握表示法与遍历，再深入最短路与最小生成树。

### 核心概念

- 有 4 种基本方式在内存里表示一个图：
    - 对象和指针
    - 邻接矩阵
    - 邻接表
    - 邻接图
- 熟悉以上每一种图的表示法，并了解各自的优缺点
- 广度优先搜索和深度优先搜索：知道它们的计算复杂度和设计上的权衡以及如何用代码实现它们
- 遇到一个问题时，首先尝试基于图的解决方案，如果没有再去尝试其他的

### MIT 课程

- [广度优先搜索](https://www.youtube.com/watch?v=s-CYnVz-uh4&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=13)
- [深度优先搜索](https://www.youtube.com/watch?v=AfSk24UTFS8&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=14)

### Skiena 课程

- [ ] [CSE373 2012 - 课程 11 - 图的数据结构（视频）](https://www.youtube.com/watch?v=OiXxhDrFruw&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b&index=11)
- [ ] [CSE373 2012 - 课程 12 - 广度优先搜索（视频）](https://www.youtube.com/watch?v=g5vF8jscteo&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b&index=12)
- [ ] [CSE373 2012 - 课程 13 - 图的算法（视频）](https://www.youtube.com/watch?v=S23W6eTcqdY&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b&index=13)
- [ ] [CSE373 2012 - 课程 14 - 图的算法 (1)（视频）](https://www.youtube.com/watch?v=WitPBKGV0HY&index=14&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b)
- [ ] [CSE373 2012 - 课程 15 - 图的算法 (2)（视频）](https://www.youtube.com/watch?v=ia1L30l7OIg&index=15&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b)
- [ ] [CSE373 2012 - 课程 16 - 图的算法 (3)（视频）](https://www.youtube.com/watch?v=jgDOQq6iWy8&index=16&list=PLOtl7M3yp-DV69F32zdK7YJcNXpTunF2b)

### 复习与经典算法

- [ ] [6.006 单源最短路径问题（视频）](https://www.youtube.com/watch?v=Aa2sqUhIn-E&index=15&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [6.006 Dijkstra 算法（视频）](https://www.youtube.com/watch?v=NSHizBK9JD8&t=1731s&ab_channel=MITOpenCourseWare)
- [ ] [6.006 Bellman-Ford 算法（视频）](https://www.youtube.com/watch?v=f9cVS_URPc0&ab_channel=MITOpenCourseWare)
- [ ] [6.006 加速 Dijkstra 算法（视频）](https://www.youtube.com/watch?v=CHvQ3q_gJ7E&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb&index=18)
- [ ] [Aduni：图算法 I - 拓扑排序，最小生成树，Prim 算法 - 讲座 6（视频）](https://www.youtube.com/watch?v=i_AQT_XfvD8&index=6&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm)
- [ ] [Aduni：图算法 II - DFS，BFS，Kruskal 算法，Union Find 数据结构 - 讲座 7（视频）](https://www.youtube.com/watch?v=ufj5_bppBsA&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=7)
- [ ] [Aduni：图算法 III：最短路径 - 讲座 8（视频）](https://www.youtube.com/watch?v=DiedsPsMKXc&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=8)
- [ ] [Aduni：图算法 IV：几何算法入门 - 讲座 9（视频）](https://www.youtube.com/watch?v=XIAQRlNkJAw&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=9)
- [ ] [CS 61B 2014：加权图（视频）](https://archive.org/details/ucberkeley_webcast_zFbq8vOZ_0k)
- [ ] [贪婪算法：最小生成树（视频）](https://www.youtube.com/watch?v=tKwnms5iRBU&index=16&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)
- [ ] [强连通分量 Kosaraju 算法图算法（视频）](https://www.youtube.com/watch?v=RpgcYiky7uw)
- [ ] [[复习] 最短路径算法（播放列表）16 分钟（视频）](https://www.youtube.com/playlist?list=PL9xmBV_5YoZO-Y-H3xIC9DGSfVYJng9Yw)
- [ ] [[复习] 最小生成树（播放列表）4 分钟（视频）](https://www.youtube.com/playlist?list=PL9xmBV_5YoZObEi3Hf6lmyW-CBfs7nkOV)

### Coursera 课程

- [ ] [图的算法（视频）](https://www.coursera.org/learn/algorithms-on-graphs/home/welcome)

### 动手实现

- [ ] DFS 邻接表 (递归)
- [ ] DFS 邻接表 (栈迭代)
- [ ] DFS 邻接矩阵 (递归)
- [ ] DFS 邻接矩阵 (栈迭代)
- [ ] BFS 邻接表
- [ ] BFS 邻接矩阵
- [ ] 单源最短路径问题 (Dijkstra)
- [ ] 最小生成树
- 基于 DFS 的算法 (根据上文 Aduni 的视频):
    - [ ] 检查环 (我们会先检查是否有环存在以便做拓扑排序)
    - [ ] 拓扑排序
    - [ ] 计算图中的连通分支
    - [ ] 列出强连通分量
    - [ ] 检查双向图

---

## 高级算法

### 递归（Recursion）

递归是函数直接或间接调用自身来分解子问题的技巧，常与分治、回溯结合；理解基线条件与调用栈是正确使用的前提。

#### Stanford 课程

- [ ] [课程 8 | 抽象编程（视频）](https://www.youtube.com/watch?v=gl3emqCuueQ&list=PLFE6E58F856038C69&index=8)
- [ ] [课程 9 | 抽象编程（视频）](https://www.youtube.com/watch?v=uFJhEPrbycQ&list=PLFE6E58F856038C69&index=9)
- [ ] [课程 10 | 抽象编程（视频）](https://www.youtube.com/watch?v=NdF1QDTRkck&index=10&list=PLFE6E58F856038C69)
- [ ] [课程 11 | 抽象编程（视频）](https://www.youtube.com/watch?v=p-gpaIGRCQI&list=PLFE6E58F856038C69&index=11)

#### 何时使用与尾递归

- 什么时候适合使用
- 尾递归会更好么?
    - [ ] [什么是尾递归以及为什么它如此糟糕?](https://www.quora.com/What-is-tail-recursion-Why-is-it-so-bad)
    - [ ] [尾递归（视频）](https://www.coursera.org/lecture/programming-languages/tail-recursion-YZic1)

#### 通用方法与回溯

- [ ] [解决任何递归问题的 5 个简单步骤（视频）](https://youtu.be/ngCos392W4w)

回溯蓝图: [Java](https://leetcode.com/problems/combination-sum/discuss/16502/A-general-approach-to-backtracking-questions-in-Java-(Subsets-Permutations-Combination-Sum-Palindrome-Partitioning)) · [Python](https://leetcode.com/problems/combination-sum/discuss/429538/General-Backtracking-questions-solutions-in-Python-for-reference-%3A)

### 动态规划（Dynamic Programming）

在你的面试中或许没有任何动态规划的问题，但能够知道一个题目可以使用动态规划来解决是很重要的。这一部分会有点困难，每个可以用动态规划解决的问题都必须先定义出递推关系，要推导出来可能会有点棘手。我建议先阅读和学习足够多的动态规划的例子，以便对解决 DP 问题的一般模式有个扎实的理解。

#### 视频

- [ ] [Skiena：CSE373 2020 - 讲座 19 - 动态规划简介（视频）](https://www.youtube.com/watch?v=wAA0AMfcJHQ&list=PLOtl7M3yp-DX6ic0HGT0PUX_wiNmkWkXx&index=18)
- [ ] [Skiena：CSE373 2020 - 讲座 20 - 编辑距离（视频）](https://www.youtube.com/watch?v=T3A4jlHlhtA&list=PLOtl7M3yp-DX6ic0HGT0PUX_wiNmkWkXx&index=19)
- [ ] [Skiena：CSE373 2020 - 讲座 20 - 编辑距离（续）（视频）](https://www.youtube.com/watch?v=iPnPVcZmRbE&list=PLOtl7M3yp-DX6ic0HGT0PUX_wiNmkWkXx&index=20)
- [ ] [Skiena：CSE373 2020 - 讲座 21 - 动态规划（视频）](https://www.youtube.com/watch?v=2xPE4Wz8coQ&list=PLOtl7M3yp-DX6ic0HGT0PUX_wiNmkWkXx&index=21)
- [ ] [Skiena：CSE373 2020 - 讲座 22 - 动态规划和复习（视频）](https://www.youtube.com/watch?v=Yh3RzqQGsyI&list=PLOtl7M3yp-DX6ic0HGT0PUX_wiNmkWkXx&index=22)
- [ ] [Simonson：动态规划 0（从 59:18 开始）（视频）](https://youtu.be/J5aJEcOr6Eo?list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&t=3558)
- [ ] [Simonson：动态规划 I - 第 11 讲（视频）](https://www.youtube.com/watch?v=0EzHjQ_SOeU&index=11&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm)
- [ ] [Simonson：动态规划 II - 第 12 讲（视频）](https://www.youtube.com/watch?v=v1qiRwuJU7g&list=PLFDnELG9dpVxQCxuD-9BSy2E7BWY3t5Sm&index=12)
- [ ] 单独的动态规划问题列表（每个都很短）: [动态规划（视频）](https://www.youtube.com/playlist?list=PLrmLmBdmIlpsHaNTPP_jHHDx_os9ItYXr)

#### 耶鲁课程笔记

- [ ] [动态规划](http://www.cs.yale.edu/homes/aspnes/classes/223/notes.html#dynamicProgramming)

#### Coursera 课程

- [ ] [RNA 二级结构问题（视频）](https://www.coursera.org/learn/algorithmic-thinking-2/lecture/80RrW/the-rna-secondary-structure-problem)
- [ ] [动态规划算法（视频）](https://www.coursera.org/learn/algorithmic-thinking-2/lecture/PSonq/a-dynamic-programming-algorithm)
- [ ] [DP 算法描述（视频）](https://www.coursera.org/learn/algorithmic-thinking-2/lecture/oUEK2/illustrating-the-dp-algorithm)
- [ ] [DP 算法的运行时间（视频）](https://www.coursera.org/learn/algorithmic-thinking-2/lecture/nfK2r/running-time-of-the-dp-algorithm)
- [ ] [DP vs 递归实现（视频）](https://www.coursera.org/learn/algorithmic-thinking-2/lecture/M999a/dp-vs-recursive-implementation)
- [ ] [全局成对序列排列（视频）](https://www.coursera.org/learn/algorithmic-thinking-2/lecture/UZ7o6/global-pairwise-sequence-alignment)
- [ ] [本地成对序列排列（视频）](https://www.coursera.org/learn/algorithmic-thinking-2/lecture/WnNau/local-pairwise-sequence-alignment)
