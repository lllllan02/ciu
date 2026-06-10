---
title: 快速排序
order: 6
---

## 定义

快速排序（Quick Sort）是一种 **分治** 比较排序算法：选取 **基准（pivot）**，将数组 **分区** 为小于基准与大于等于基准两部分，再对子区间递归（或迭代）排序。

相比归并排序，快排的优势在于 **原地分区**，平均 $O(n \log n)$ 且常数因子小，对随机访问数组（如内存数组）非常高效，是工程中最常用的比较排序之一。代价是标准实现 **不稳定**，最坏情况可达 $O(n^2)$（与基准选取有关）。

## 过程

工作原理：每次选一个基准，通过双指针将区间划分为「≤ pivot」与「≥ pivot」两段，再分别处理两侧子区间。

1. **选基准**：取当前区间中点元素值 `mid` 作为 pivot。
2. **分区**：`left` 从区间左端、`right` 从右端向中间扫描：
   - `arr[left] < mid` 时 `left++`；
   - `arr[right] > mid` 时 `right--`；
   - 若 `left <= right`，交换 `arr[left]` 与 `arr[right]`，然后 `left++`、`right--`。
3. **划分子区间**：`[start, right]` 侧与 `[left, end]` 侧分别继续排序。
4. **结束**：所有子区间长度 ≤ 1 时，整体有序。

以 `[5, 3, 8, 4, 2]`、区间 `[0, 4]`、`mid = 4` 为例，分区过程：

| 步骤 | `left` | `right` | 操作 | 结果 |
| :--- | :--- | :--- | :--- | :--- |
| 初始 | `0` | `4` | — | `[5, 3, 8, 4, 2]` |
| 1 | `2` | `4` | 交换 `8` 与 `2` | `[5, 3, 2, 4, 8]` |
| 2 | `2` | `3` | 交换 `2` 与 `4` | `[5, 3, 4, 2, 8]` |

分区后左侧 `[5, 3, 4, 2]`、右侧 `[8]` 继续处理，直至全部有序。

## 性质

### 稳定性

快速排序是 **不稳定** 排序算法。根本原因在于 **分区会批量重组元素**：每轮把区间拆成左右两段分别递归排序，相等元素可能被分到不同子区间，最终相对顺序取决于分区边界，而非输入时的先后。

本实现中，相等键参与交换只是上述现象的一种表现，**不能**据此把不稳定完全归因于「相等时交换」——即便某步避免交换两个相等的数，递归分区后相等元素仍可能分居两侧而乱序；在比较条件里简单加等号也 **不能** 让标准原地快排变稳定。

以 `[2, 3a, 3b, 1]`、区间 `[0, 3]`、基准 `mid = 3` 为例，分区时 `3a` 与 `3b` 会被交换：

| 步骤 | `left` | `right` | 操作 | 结果 |
| :--- | :--- | :--- | :--- | :--- |
| 1 | `1` | `3` | `2 < 3`，`left++` | — |
| 2 | `1` | `1` | `3b > 3` 不成立，`right--` 至 `1` | — |
| 3 | `1` | `1` | 交换 `arr[1]` 与 `arr[2]` | `[2, 3b, 3a, 1]` |

这说明了「相等元素也可能被交换」，但即便改掉这一步，分区 + 递归的结构仍会破坏稳定性。需要稳定排序时应改用归并排序等算法，或采用带额外空间的稳定快排变体。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n \log n)$ | 每次分区较均衡，如基准始终为中位数 |
| **最坏** | $O(n^2)$ | 每次分区极不平衡，如已有序且基准固定取端点 |
| **平均** | $O(n \log n)$ | 随机或一般数据下表现优秀 |

中点基准在多数情况下较均衡；若需降低最坏情况风险，可改用 **随机基准** 或 **三数取中**。

### 空间复杂度

$O(\log n)$（平均）。递归栈或显式区间栈深度平均 $O(\log n)$，最坏 $O(n)$。本实现预分配 $O(n)$ 的区间栈缓冲区，但同一时刻栈中区间数为 $O(\log n)$ 级。

## 代码实现

> 源码: https://github.com/lllllan02/ciu/tree/master/code/quick-sort

### 基础写法

本仓库实现。**迭代 + 显式栈**，基准取区间中点元素值，双指针分区：

```c
typedef struct Range {
    int start, end;
} Range;

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void quick_sort(int *arr, int len) {
    if (len <= 0) return;

    Range *ranges = malloc(len * sizeof(Range));
    int index = 0;
    ranges[index++] = (Range){0, len - 1};

    while (index) {
        Range range = ranges[--index];
        if (range.start >= range.end) continue;

        int left = range.start, right = range.end;
        int mid = arr[(range.start + range.end) / 2];
        while (left <= right) {
            while (arr[left] < mid) left++;
            while (arr[right] > mid) right--;

            if (left <= right) {
                swap(arr + left, arr + right);
                left++, right--;
            }
        }

        if (left < range.end) ranges[index++] = (Range){left, range.end};
        if (right > range.start) ranges[index++] = (Range){range.start, right};
    }
}
```

### 优化写法

**递归 + 随机基准**，避免在已有序输入上反复选到极端 pivot 导致 $O(n^2)$：

```c
static int partition(int *arr, int start, int end) {
    int pivot_idx = start + rand() % (end - start + 1);
    swap(arr + pivot_idx, arr + end);

    int pivot = arr[end];
    int i = start;
    for (int j = start; j < end; j++) {
        if (arr[j] < pivot) {
            swap(arr + i, arr + j);
            i++;
        }
    }
    swap(arr + i, arr + end);
    return i;
}

static void quick_sort_recur(int *arr, int start, int end) {
    if (start >= end) return;
    int p = partition(arr, start, end);
    quick_sort_recur(arr, start, p - 1);
    quick_sort_recur(arr, p + 1, end);
}

void quick_sort(int *arr, int len) {
    if (len <= 0) return;
    quick_sort_recur(arr, 0, len - 1);
}
```

## 参考阅读

- [x] [OI Wiki - 快速排序](https://oi-wiki.org/basic/quick-sort/) (2026-06-10)
- [x] [菜鸟教程 - 快速排序](https://www.runoob.com/w3cnote/quick-sort-2.html) (2026-06-10)
- [ ] [快排（视频）](https://www.youtube.com/watch?v=y_G9BkAm6B8&index=4&list=PL89B61F78B552C1AB)
- [ ] [Quick sort in 4 minutes (video)](https://youtu.be/Hoixgm4-P4M)
- [ ] [随机算法: 矩阵相乘, 快排, Freivalds' 算法（视频）](https://www.youtube.com/watch?v=cNB2lADK3_s&index=8&list=PLUl4u3cNGP6317WaSNfmCvGym2ucw3oGp)
- [ ] [实现（C 语言）](http://www.cs.yale.edu/homes/aspnes/classes/223/examples/randomization/quick.c)
- [ ] [实现（C 语言）](https://github.com/jwasham/practice-c/blob/master/quick_sort/quick_sort.c)
- [ ] [实现（Python 语言）](https://github.com/jwasham/practice-python/blob/master/quick_sort/quick_sort.py)
