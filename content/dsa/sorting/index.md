---
title: 排序
description: 比较排序、稳定性与线性时间排序。
order: 4
---

排序是把一组元素按键值大小重排成有序序列的操作。常见基于比较的算法在平均与最坏时间、额外空间、是否**稳定**以及适用容器（数组还是链表）上各有取舍；面试里既要能讲清思路与复杂度，也要能手写归并、快排等核心实现。下面按算法分类展开。

### 稳定性与比较排序基础

若两个键相等的对象在排序输出中与输入数据集中的顺序相同，则称该排序算法是**稳定的**（常见面试题：「快排是稳定的吗？」——标准实现不是）。

- [x] [排序算法的稳定性](https://en.wikipedia.org/wiki/Sorting_algorithm#Stability) (2026-06-09)
- [x] [排序算法的稳定性](http://www.geeksforgeeks.org/stability-in-sorting-algorithms/) (2026-06-09)
- [ ] [排序算法的稳定性](http://stackoverflow.com/questions/1517793/stability-in-sorting-algorithms)
- [ ] [排序算法 - 稳定性](http://homepages.math.uic.edu/~leon/cs-mcs401-s08/handouts/stability.pdf)

### 归并排序

分治：将数组分成两半分别排序后合并。时间复杂度稳定为 $O(n \log n)$，额外空间通常为 $O(n)$，是**稳定**排序。在链表上同样高效，是对链表做通用排序的推荐方式。

- [x] [菜鸟教程 - 归并排序](https://www.runoob.com/w3cnote/merge-sort.html) (2026-06-09)
- [ ] [链表的归并排序](http://www.geeksforgeeks.org/merge-sort-for-linked-list/)
- [ ] [插入排序 & 归并排序（视频）](https://www.youtube.com/watch?v=Kg4bqzAqRBM&index=3&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [归并排序（视频）](https://www.youtube.com/watch?v=GCae1WNvnZM&index=3&list=PL89B61F78B552C1AB)
- [ ] [Merge sort in 3 minutes (video)](https://youtu.be/4VqmGXwpLqc)
- [ ] [使用外部数组（C 语言）](http://www.cs.yale.edu/homes/aspnes/classes/223/examples/sorting/mergesort.c)
- [ ] [使用外部数组（Python 语言）](https://github.com/jwasham/practice-python/blob/master/merge_sort/merge_sort.py)
- [ ] [对原数组直接排序（C++）](https://github.com/jwasham/practice-cpp/blob/master/merge_sort/merge_sort.cc)

> - [x] [实现归并排序](/dsa/sorting/merge-sort) (2026-06-09)

### 快速排序

选基准分区后递归排序。平均 $O(n \log n)$，最坏 $O(n^2)$；标准实现**不稳定**，更适合随机访问的数组。

- [x] [菜鸟教程 - 快速排序](https://www.runoob.com/w3cnote/quick-sort-2.html) (2026-06-10)
- [ ] [快排（视频）](https://www.youtube.com/watch?v=y_G9BkAm6B8&index=4&list=PL89B61F78B552C1AB)
- [ ] [Quick sort in 4 minutes (video)](https://youtu.be/Hoixgm4-P4M)
- [ ] [随机算法: 矩阵相乘, 快排, Freivalds' 算法（视频）](https://www.youtube.com/watch?v=cNB2lADK3_s&index=8&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)
- [ ] [实现（C 语言）](http://www.cs.yale.edu/homes/aspnes/classes/223/examples/randomization/quick.c)
- [ ] [实现（C 语言）](https://github.com/jwasham/practice-c/blob/master/quick_sort/quick_sort.c)
- [ ] [实现（Python 语言）](https://github.com/jwasham/practice-python/blob/master/quick_sort/quick_sort.py)

> - [x] [实现快速排序](/dsa/sorting/quick-sort) (2026-06-10)

### 堆排序

利用堆结构反复取出极值。时间 $O(n \log n)$，原地排序，**不稳定**；堆数据结构见 [树](/dsa/trees) 中 [堆](/dsa/trees#堆heap--优先级队列priority-queue--二叉堆binary-heap) 一节。

- [ ] [Heap sort in 4 minutes (video)](https://youtu.be/2DmK_H7IdTo)

> - [x] [实现堆排序](/dsa/sorting/heap-sort) (2026-06-09)

### 插入排序

逐个将元素插入已排序部分的正确位置。最坏与平均均为 $O(n^2)$，**稳定**；数据近乎有序时表现很好。

- [ ] [插入排序（视频）](https://www.youtube.com/watch?v=c4BRHC7kTaQ&index=2&list=PL89B61F78B552C1AB)
- [ ] [Insertion sort in 2 minutes (video)](https://youtu.be/JU767SDMDvA)

> - [ ] 动手实现：最坏、平均时间复杂度均为 O(n^2)。

### 选择排序

每轮从未排序部分选出最小（或最大）元素放到正确位置。最坏与平均均为 $O(n^2)$，**不稳定**。

- [ ] [选择排序（视频）](https://www.youtube.com/watch?v=6nDMgr0-Yyo&index=8&list=PL89B61F78B552C1AB)
- [ ] [Selection sort in 3 minutes (video)](https://youtu.be/g-PGLbMth_g)

> - [ ] 动手实现：最坏、平均时间复杂度均为 O(n^2)。

### 冒泡排序

相邻元素两两比较并交换，较大元素逐步「冒泡」到末尾。最坏与平均均为 $O(n^2)$，**稳定**；教学常用，实际工程较少采用。

- [ ] [冒泡排序（视频）](https://www.youtube.com/watch?v=P00xJgWzz2c&index=1&list=PL89B61F78B552C1AB)
- [ ] [冒泡排序分析（视频）](https://www.youtube.com/watch?v=ni_zk257Nqo&index=7&list=PL89B61F78B552C1AB)
- [ ] [Bubble sort in 2 minutes (video)](https://youtu.be/xli_FI7CuzA)

### 基数排序与线性时间排序（可选）

不基于元素间比较，在键值范围有限时可达到 $O(n)$ 量级；包括计数排序、基数排序等。

- [ ] [基数排序](http://www.cs.yale.edu/homes/aspnes/classes/223/notes.html#radixSort)
- [ ] [基数排序（视频）](https://www.youtube.com/watch?v=xhr26ia4k38)
- [ ] [基数排序, 计数排序 (线性时间内)（视频）](https://www.youtube.com/watch?v=Nz1KZXbghj8&index=7&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [CS 61B 2014-04-21：基数排序（视频）](https://archive.org/details/ucberkeley_webcast_pvbBMd-3NoI)
- [ ] [线性时间内的排序（视频）](https://www.youtube.com/watch?v=pOKy3RZbSws&list=PLUl4u3cNGP61hsJNdULdudlRL493b-XZf&index=14)

### 综合资源

- [ ] [CS 61B Lecture 29：排序 I（视频）](https://archive.org/details/ucberkeley_webcast_EiUvYS2DT6I)
- [ ] [CS 61B Lecture 30：排序 II（视频）](https://archive.org/details/ucberkeley_webcast_2hTY3t80Qsk)
- [ ] [CS 61B Lecture 32：排序 III（视频）](https://archive.org/details/ucberkeley_webcast_Y6LOLpxg6Dc)
- [ ] [CS 61B Lecture 33：排序 V（视频）](https://archive.org/details/ucberkeley_webcast_qNMQ4ly43p4)
- [ ] [[Review] Sorting (playlist) in 18 minutes](https://www.youtube.com/playlist?list=PL9xmBV_5YoZOZSbGAXAPIq1BeUf4j20pl)
- [ ] [15 种排序算法可视化（视频）](https://www.youtube.com/watch?v=kPRA0W1kECg)

如果你需要有关此主题的更多详细信息，请参阅「[一些主题的额外内容](/optional#一些主题的额外内容)」中的「排序」部分。
