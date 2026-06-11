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
| [希尔排序](/dsa/sorting/shell-sort) | $O(n \log n)$ | $O(n \log^2 n)$ | $O(n \log^2 n)$ | $O(1)$ | 原地 | 不稳定 |
| [归并排序](/dsa/sorting/merge-sort) | $O(n \log n)$ | $O(n \log n)$ | $O(n \log n)$ | $O(n)$ | 非原地 | **稳定** |
| [快速排序](/dsa/sorting/quick-sort) | $O(n \log n)$ | $O(n \log n)$ | $O(n^2)$ | $O(\log n)$ | 原地 | 不稳定 |
| [堆排序](/dsa/sorting/heap-sort) | $O(n \log n)$ | $O(n \log n)$ | $O(n \log n)$ | $O(1)$ | 原地 | 不稳定 |
| [计数排序](/dsa/sorting/counting-sort) | $O(n + k)$ | $O(n + k)$ | $O(n + k)$ | $O(k)$ | 非原地 | **稳定** |
| [桶排序](/dsa/sorting/bucket-sort) | $O(n + k)$ | $O(n + k)$ | $O(n^2)$ | $O(n + k)$ | 非原地 | **稳定** |
| [基数排序](/dsa/sorting/radix-sort) | $O(n \times k)$ | $O(n \times k)$ | $O(n \times k)$ | $O(n + k)$ | 非原地 | **稳定** |

> 表中 $k$ 表示键值范围或位数等辅助参数；「原地」指额外空间为常数级（不含输入本身），「非原地」通常需要与 $n$ 或 $k$ 相关的辅助空间。

## 综合资源

- [ ] [CS 61B Lecture 29：排序 I（视频）](https://archive.org/details/ucberkeley_webcast_EiUvYS2DT6I)
- [ ] [CS 61B Lecture 30：排序 II（视频）](https://archive.org/details/ucberkeley_webcast_2hTY3t80Qsk)
- [ ] [CS 61B Lecture 32：排序 III（视频）](https://archive.org/details/ucberkeley_webcast_Y6LOLpxg6Dc)
- [ ] [CS 61B Lecture 33：排序 V（视频）](https://archive.org/details/ucberkeley_webcast_qNMQ4ly43p4)
- [ ] [[Review] Sorting (playlist) in 18 minutes](https://www.youtube.com/playlist?list=PL9xmBV_5YoZOZSbGAXAPIq1BeUf4j20pl)
- [ ] [15 种排序算法可视化（视频）](https://www.youtube.com/watch?v=kPRA0W1kECg)

如果你需要有关此主题的更多详细信息，请参阅「[一些主题的额外内容](/optional#一些主题的额外内容)」中的「排序」部分。
