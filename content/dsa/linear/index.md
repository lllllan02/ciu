---
title: 线性结构
description: 数组、链表、栈、队列、哈希表。
order: 1
---

线性结构（数组、链表、栈、队列）与基于哈希的映射是后续树、图和算法题的基础。

### 数组（Arrays）

数组把元素放在**连续内存**里，用下标可在 $O(1)$ 时间内随机访问任意位置；在末尾增删通常也很快，但在中间插入或删除需要挪动后面的元素，一般是 $O(n)$。固定长度的数组容量写死；**动态数组**在元素变多时自动扩容（常见做法是容量翻倍），是多数语言里 `vector`、列表等容器的底层思路。

- [x] [Harvard CS50 - 数组（视频）](https://www.youtube.com/watch?v=tI_tIZFyKBw&t=3009s) (2026-01-29)
- [ ] [UC San Diego - 数组（视频）](https://www.coursera.org/lecture/data-structures/arrays-OsBSF)
- [ ] [UC San Diego - 动态数组（视频）](https://www.coursera.org/lecture/data-structures/dynamic-arrays-EwbnV)
- [ ] [UC Berkeley CS61B - 线性和多维数组（视频）](https://archive.org/details/ucberkeley_webcast_Wp8oiO_CZZE)（从 15 分 32 秒开始）
- [ ] [Python - 嵌套列表（视频）](https://www.youtube.com/watch?v=1jtrQqYpt7g)

> - [x] [实现动态数组](/dsa/linear/arrays) (2026-02-01)

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

> - [x] [实现单向链表](/dsa/linear/linked-lists) (2026-02-28)

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

> - [x] [实现队列](/dsa/linear/queue) (2026-03-04)

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

> - [x] [实现链式哈希表](/dsa/linear/hash-tables) (2026-06-01)
