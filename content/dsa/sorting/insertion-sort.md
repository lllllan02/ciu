---
title: 插入排序
order: 3
---

## 定义

插入排序（Insertion Sort）是一种简单直观的 **原地** 比较排序算法：每次从未排序与已排序的 **交界处** 取出一个元素，在已排序部分 **由外向内** 比较找位，将需后让的元素逐一后移腾出空位，再插入该元素。

**特性**：平均 $O(n^2)$ · 最好 $O(n)$ · 最坏 $O(n^2)$ · 空间 $O(1)$ · 原地 · **稳定**

## 过程

一种常见写法（与本节代码一致）：初始时将左端首元素视为 **已排序**，其余为 **未排序**。**从左向右** 依次处理，每次取未排序部分最左端元素，在已排序部分 **从右向左** 定位插入点；每完成一次插入，已排序区间向右扩展一位，$n$ 个元素共需 $n - 1$ 次。

![插入排序动画](/images/insertion-sort-animate.svg)

## 性质

### 稳定性

插入排序是 **稳定** 排序算法。仅在待插入元素严格小于已排序部分中的元素时才后移，相等时不挪动，相对顺序保持不变。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n)$ | 已有序时内层首轮即 `break` |
| **最坏** | $O(n^2)$ | 逆序时约 $\Theta(n^2)$ 次比较与交换 |
| **平均** | $O(n^2)$ | 随机数据下比较次数同阶 |

数据近乎有序时表现很好，是简单排序中适应性较强的一种。

### 空间复杂度

$O(1)$。原地排序，仅使用常数个辅助变量。

## [代码实现](https://github.com/lllllan02/ciu/tree/master/code/insertion-sort)

保存待插入元素，较大元素逐一后移，再落位；相等时不后移，保持稳定。

```c
void insertion_sort(int* arr, int len) {
    for (int i = 1; i < len; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}
```

## 参考阅读

- [x] [OI Wiki - 插入排序](https://oi-wiki.org/basic/insertion-sort/) (2026-06-10)
- [x] [菜鸟教程 - 插入排序](https://www.runoob.com/w3cnote/insertion-sort.html) (2026-06-10)
- [ ] [插入排序（视频）](https://www.youtube.com/watch?v=c4BRHC7kTaQ&index=2&list=PL89B61F78B552C1AB)
- [ ] [Insertion sort in 2 minutes (video)](https://youtu.be/JU767SDMDvA)
