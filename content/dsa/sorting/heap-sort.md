---
title: 堆排序
order: 7
---

## 定义

堆排序（Heap Sort）是一种 **原地** 比较排序算法，建立在 **大顶堆** 上：反复取出堆顶（最大值）放到数组末尾，再对剩余部分重新调整堆，最终得到升序数组。

本质上是 **选择排序 + 堆**：选择排序每轮找最大值需 $O(n)$ 扫描，堆将「找最大」优化为 $O(\log n)$，总时间稳定 $O(n \log n)$，且仅需 $O(1)$ 额外空间。相比快排，堆排序最坏情况不会退化到 $O(n^2)$；代价是 **不稳定**，且实践中常数因子通常大于快排。

堆的基础操作见 [树](/dsa/trees) 中 [堆](/dsa/trees#堆heap--优先级队列priority-queue--二叉堆binary-heap) 与 [大顶堆](/dsa/trees/max-heap) 实现。

**特性**：平均 $O(n \log n)$ · 最好 $O(n \log n)$ · 最坏 $O(n \log n)$ · 空间 $O(1)$ · 原地 · 不稳定

## 过程

工作原理：先在原数组上建 **大顶堆**，再反复「交换堆顶到末尾 + 下滤」。

1. **建堆**：自底向上，从最后一个有孩子的节点到根，依次 **下滤**，使每个父节点都不小于其孩子。
2. **取堆顶**：堆顶（当前最大值）与堆的最后一个元素交换，最大值移到末尾就位。
3. **缩小堆**：堆的有效范围减 1，对新堆顶下滤，恢复大顶堆。
4. **结束**：重复 2–3 直至全部有序。

![堆排序动画](/images/heap-sort.gif)

## 性质

### 稳定性

堆排序是 **不稳定** 排序算法。本质是 **堆上的选择排序**：每轮把当前最大值换到数组末尾，建堆与下滤过程中也会在树上 **远距离交换** 元素；相等键的相对顺序既不依赖输入先后，也无法通过「相等时不交换」这类局部修补保证。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n \log n)$ | 建堆 $O(n)$ + $n - 1$ 次下滤 |
| **最坏** | $O(n \log n)$ | 与输入分布无关 |
| **平均** | $O(n \log n)$ | 同上 |

建堆采用 Floyd 自底向上，复杂度 $O(n)$，优于逐个 `push` 的 $O(n \log n)$。

### 空间复杂度

$O(1)$。直接在原数组上建堆与下滤，仅常数辅助变量。

## [代码实现](https://github.com/lllllan02/ciu/tree/master/code/heap-sort)

本仓库实现。Floyd 自底向上建堆 + 反复取堆顶；下滤时若父节点已不小于较大子节点则 **提前返回**：

```c
static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static void sift_down(int *arr, int start, int end) {
    int parent = start;
    int child = parent * 2 + 1;
    while (child <= end) {
        if (child + 1 <= end && arr[child] < arr[child + 1]) {
            child++;
        }
        if (arr[parent] >= arr[child]) return;

        swap(arr + parent, arr + child);
        parent = child;
        child = parent * 2 + 1;
    }
}

void heap_sort(int *arr, int len) {
    if (len <= 1) return;

    int end = (len - 2) / 2;
    for (int i = end; i >= 0; i--) {
        sift_down(arr, i, len - 1);
    }

    for (int i = len - 1; i > 0; i--) {
        swap(arr, arr + i);
        sift_down(arr, 0, i - 1);
    }
}
```

## 参考阅读

- [x] [OI Wiki - 堆排序](https://oi-wiki.org/basic/heap-sort/) (2026-06-10)
- [x] [菜鸟教程 - 堆排序](https://www.runoob.com/w3cnote/heap-sort.html) (2026-06-10)
- [x] [大顶堆](/dsa/trees/max-heap) (2026-06-09)
- [x] [OI Wiki - 二叉堆](https://oi-wiki.org/ds/binary-heap/) (2026-06-10)
- [ ] [Heap sort in 4 minutes (video)](https://youtu.be/2DmK_H7IdTo)
