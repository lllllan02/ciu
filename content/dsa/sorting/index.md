---
title: 排序
description: 比较排序、稳定性与线性时间排序。
order: 4
---

排序是把一组元素按键值大小重排成有序序列的操作。常见基于比较的算法在平均与最坏时间、额外空间、是否**稳定**以及适用容器（数组还是链表）上各有取舍；面试里既要能讲清思路与复杂度，也要能手写归并、快排等核心实现。下面先给出常见算法对比，再按分类展开。

## 稳定性与比较排序基础

若两个键相等的对象在排序输出中与输入数据集中的顺序相同，则称该排序算法是**稳定的**（常见面试题：「快排是稳定的吗？」——标准实现不是）。

- [x] [排序算法的稳定性](https://en.wikipedia.org/wiki/Sorting_algorithm#Stability) (2026-06-09)
- [x] [排序算法的稳定性](http://www.geeksforgeeks.org/stability-in-sorting-algorithms/) (2026-06-09)
- [ ] [排序算法的稳定性](http://stackoverflow.com/questions/1517793/stability-in-sorting-algorithms)
- [ ] [排序算法 - 稳定性](http://homepages.math.uic.edu/~leon/cs-mcs401-s08/handouts/stability.pdf)

## 排序算法对比

| 排序算法 | 平均时间复杂度 | 最好情况 | 最坏情况 | 空间复杂度 | 排序方式 | 稳定性 |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| [冒泡排序](/dsa/sorting/bubble-sort) | $O(n^2)$ | $O(n)$ | $O(n^2)$ | $O(1)$ | 原地 | **稳定** |
| [选择排序](/dsa/sorting/selection-sort) | $O(n^2)$ | $O(n^2)$ | $O(n^2)$ | $O(1)$ | 原地 | 不稳定 |
| [插入排序](/dsa/sorting/insertion-sort) | $O(n^2)$ | $O(n)$ | $O(n^2)$ | $O(1)$ | 原地 | **稳定** |
| 希尔排序 | $O(n \log n)$ | $O(n \log^2 n)$ | $O(n \log^2 n)$ | $O(1)$ | 原地 | 不稳定 |
| 归并排序 | $O(n \log n)$ | $O(n \log n)$ | $O(n \log n)$ | $O(n)$ | 非原地 | **稳定** |
| 快速排序 | $O(n \log n)$ | $O(n \log n)$ | $O(n^2)$ | $O(\log n)$ | 原地 | 不稳定 |
| 堆排序 | $O(n \log n)$ | $O(n \log n)$ | $O(n \log n)$ | $O(1)$ | 原地 | 不稳定 |
| 计数排序 | $O(n + k)$ | $O(n + k)$ | $O(n + k)$ | $O(k)$ | 非原地 | **稳定** |
| 桶排序 | $O(n + k)$ | $O(n + k)$ | $O(n^2)$ | $O(n + k)$ | 非原地 | **稳定** |
| 基数排序 | $O(n \times k)$ | $O(n \times k)$ | $O(n \times k)$ | $O(n + k)$ | 非原地 | **稳定** |

> 表中 $k$ 表示键值范围或位数等辅助参数；「原地」指额外空间为常数级（不含输入本身），「非原地」通常需要与 $n$ 或 $k$ 相关的辅助空间。

### 希尔排序

- [x] [菜鸟教程 - 希尔排序](https://www.runoob.com/w3cnote/shell-sort.html) (2026-06-10)
- [x] [OI Wiki - 希尔排序](https://oi-wiki.org/basic/shell-sort/) (2026-06-10)

插入排序的改进：按递减间隔分组做插入排序，逐步缩小间隔至 1。平均约 $O(n \log n)$，最好与最坏约 $O(n \log^2 n)$（与增量序列有关）；原地、**不稳定**。

### 归并排序

分治：将数组分成两半分别排序后合并。平均、最好与最坏均为 $O(n \log n)$，额外空间 $O(n)$，非原地、**稳定**。在链表上同样高效，是对链表做通用排序的推荐方式。

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

选基准分区后递归排序。平均与最好 $O(n \log n)$，最坏 $O(n^2)$；额外空间 $O(\log n)$（递归栈），原地、**不稳定**；更适合随机访问的数组。

- [x] [菜鸟教程 - 快速排序](https://www.runoob.com/w3cnote/quick-sort-2.html) (2026-06-10)
- [ ] [快排（视频）](https://www.youtube.com/watch?v=y_G9BkAm6B8&index=4&list=PL89B61F78B552C1AB)
- [ ] [Quick sort in 4 minutes (video)](https://youtu.be/Hoixgm4-P4M)
- [ ] [随机算法: 矩阵相乘, 快排, Freivalds' 算法（视频）](https://www.youtube.com/watch?v=cNB2lADK3_s&index=8&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)
- [ ] [实现（C 语言）](http://www.cs.yale.edu/homes/aspnes/classes/223/examples/randomization/quick.c)
- [ ] [实现（C 语言）](https://github.com/jwasham/practice-c/blob/master/quick_sort/quick_sort.c)
- [ ] [实现（Python 语言）](https://github.com/jwasham/practice-python/blob/master/quick_sort/quick_sort.py)

> - [x] [实现快速排序](/dsa/sorting/quick-sort) (2026-06-10)

### 堆排序

利用堆结构反复取出极值。平均、最好与最坏均为 $O(n \log n)$；原地、**不稳定**。堆数据结构见 [树](/dsa/trees) 中 [堆](/dsa/trees#堆heap--优先级队列priority-queue--二叉堆binary-heap) 一节。

- [ ] [Heap sort in 4 minutes (video)](https://youtu.be/2DmK_H7IdTo)

> - [x] [实现堆排序](/dsa/sorting/heap-sort) (2026-06-09)

### 计数排序

不基于元素间比较，统计每个键值出现次数后按序回填。平均、最好与最坏均为 $O(n + k)$，额外空间 $O(k)$（$k$ 为键值范围），非原地、**稳定**；适用于整数且范围不大。

### 桶排序

将元素分到有限个有序桶中，再对各桶分别排序后合并。平均 $O(n + k)$，最好 $O(n + k)$，最坏 $O(n^2)$；额外空间 $O(n + k)$，非原地、**稳定**；适用于数据均匀分布在有限区间。

### 基数排序

按位（或按 digit）从低位到高位依次做稳定排序（常用计数排序作子过程）。平均、最好与最坏均为 $O(n \times k)$（$k$ 为位数或基数相关参数），额外空间 $O(n + k)$，非原地、**稳定**。

- [ ] [基数排序](http://www.cs.yale.edu/homes/aspnes/classes/223/notes.html#radixSort)
- [ ] [基数排序（视频）](https://www.youtube.com/watch?v=xhr26ia4k38)
- [ ] [基数排序, 计数排序 (线性时间内)（视频）](https://www.youtube.com/watch?v=Nz1KZXbghj8&index=7&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [CS 61B 2014-04-21：基数排序（视频）](https://archive.org/details/ucberkeley_webcast_pvbBMd-3NoI)
- [ ] [线性时间内的排序（视频）](https://www.youtube.com/watch?v=pOKy3RZbSws&list=PLUl4u3cNGP61hsJNdULdudlRL493b-XZf&index=14)

## 综合资源

- [ ] [CS 61B Lecture 29：排序 I（视频）](https://archive.org/details/ucberkeley_webcast_EiUvYS2DT6I)
- [ ] [CS 61B Lecture 30：排序 II（视频）](https://archive.org/details/ucberkeley_webcast_2hTY3t80Qsk)
- [ ] [CS 61B Lecture 32：排序 III（视频）](https://archive.org/details/ucberkeley_webcast_Y6LOLpxg6Dc)
- [ ] [CS 61B Lecture 33：排序 V（视频）](https://archive.org/details/ucberkeley_webcast_qNMQ4ly43p4)
- [ ] [[Review] Sorting (playlist) in 18 minutes](https://www.youtube.com/playlist?list=PL9xmBV_5YoZOZSbGAXAPIq1BeUf4j20pl)
- [ ] [15 种排序算法可视化（视频）](https://www.youtube.com/watch?v=kPRA0W1kECg)

如果你需要有关此主题的更多详细信息，请参阅「[一些主题的额外内容](/optional#一些主题的额外内容)」中的「排序」部分。
