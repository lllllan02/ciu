---
title: 快速排序
order: 6
---

## 定义

快速排序（Quick Sort）是一种 **分治** 比较排序算法：选取 **基准（pivot）**，将数组 **分区** 为小于基准与大于等于基准两部分，再对子区间递归（或迭代）排序。

相比[归并排序](/dsa/sorting/merge-sort)，快排的优势在于 **原地分区**，平均 $O(n \log n)$ 且常数因子小，对随机访问数组（如内存数组）非常高效，是工程中最常用的比较排序之一。代价是标准实现 **不稳定**，最坏情况可达 $O(n^2)$（与基准选取有关）。

**特性**：平均 $O(n \log n)$ · 最好 $O(n \log n)$ · 最坏 $O(n^2)$ · 空间 $O(\log n)$ · 原地 · 不稳定

## 过程

每次对待排区间 `[start, end]` 做一次 **分区**，再对 pivot 左右两侧的子区间重复，直至区间长度 ≤ 1：

1. **选基准**：从区间内取一个元素作为 pivot（常见取中点或随机位置）。
2. **分区**：原地调整元素，使 pivot **左侧** 都不大于 pivot、**右侧** 都不小于 pivot；pivot 就此到达最终位置。
3. **分治**：对左、右子区间继续上述过程。

![快速排序动画](/images/quick-sort.gif)

## 性质

### 稳定性

快速排序是 **不稳定** 排序算法。分区靠 **交换** 把元素挪到 pivot 两侧，只判断与 pivot 的大小，**不维护** 相等键的输入先后。交换时两元素往往 **不相邻**，可能 **跨过** 中间的相等键，相对顺序因而被打乱。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n \log n)$ | 每次分区较均衡，如基准始终为中位数 |
| **最坏** | $O(n^2)$ | 每次分区极不平衡，如已有序且基准固定取端点 |
| **平均** | $O(n \log n)$ | 随机或一般数据下表现优秀 |

中点基准在多数情况下较均衡；若需降低最坏情况风险，可改用 **随机基准** 或 **三数取中**。

### 空间复杂度

$O(\log n)$（平均）。递归栈或显式区间栈深度平均 $O(\log n)$，最坏 $O(n)$。

## [代码实现](https://github.com/lllllan02/ciu/tree/master/code/quick-sort)

### 双指针分区

要在原数组上完成分区，可用 **双指针** 相向扫描。对当前待排区间：

```
pivot ← 区间中点元素的值
左指针 ← 区间左端，右指针 ← 区间右端

当 左指针 ≤ 右指针：
    左指针右移，直到指向的元素 ≥ pivot
    右指针左移，直到指向的元素 ≤ pivot
    若 左指针 ≤ 右指针：
        交换两指针指向的元素
        左指针、右指针各向中间挪一步

分区完成 → 左子区间 [区间左端 .. 右指针]，右子区间 [左指针 .. 区间右端]
```

### 基础写法

本仓库实现。**迭代 + 显式栈**，用上述双指针完成分区，基准取区间中点元素值：

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
