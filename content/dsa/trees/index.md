---
title: 树
description: 树遍历、BST、堆与优先级队列。
order: 3
---

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

> - [x] [实现二叉搜索树](/dsa/trees/binary-search-trees) (2026-06-05)
> - [x] [实现平衡二叉搜索树](/dsa/trees/balanced-binary-search-trees) (2026-06-08)

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

> - [x] [实现大顶堆](/dsa/trees/max-heap) (2026-06-08)
> - [x] [实现堆排序](/dsa/sorting/heap-sort) (2026-06-09)
> - [x] [线性时间构建堆](/dsa/trees/build-heap) (2026-06-09)
