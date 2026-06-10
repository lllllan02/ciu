---
title: 堆排序
order: 7
---

## 定义

堆排序（Heap Sort）是一种 **原地** 比较排序算法，建立在 **大顶堆** 上：反复取出堆顶（最大值）放到数组末尾，再对剩余部分重新调整堆，最终得到升序数组。

本质上是 **选择排序 + 堆**：选择排序每轮找最大值需 $O(n)$ 扫描，堆将「找最大」优化为 $O(\log n)$，总时间稳定 $O(n \log n)$，且仅需 $O(1)$ 额外空间。相比快排，堆排序最坏情况不会退化到 $O(n^2)$；代价是 **不稳定**，且实践中常数因子通常大于快排。

堆的基础操作见 [树](/dsa/trees) 中 [堆](/dsa/trees#堆heap--优先级队列priority-queue--二叉堆binary-heap) 与 [大顶堆](/dsa/trees/max-heap) 实现。

## 过程

工作原理：先在原数组上建 **大顶堆**，再反复「交换堆顶到末尾 + 下滤」。

1. **建堆**：从最后一个非叶子节点到根，依次对每个下标 **下滤（heapify）**，将数组调整为大顶堆。
2. **取堆顶**：交换 `arr[0]` 与当前堆的最后一个元素，最大值就位。
3. **缩小堆**：堆范围减 1，对新的堆顶 `arr[0]` 下滤。
4. **结束**：重复步骤 2–3 直至堆只剩一个元素。

完全二叉树用一维数组表示（根下标为 0）：

| 关系 | 公式 |
| :--- | :--- |
| 左孩子 | $2i + 1$ |
| 右孩子 | $2i + 2$ |
| 父节点 | $\lfloor (i - 1) / 2 \rfloor$ |

以 `[5, 3, 8, 4, 2]` 为例，建堆后大顶堆为 `[8, 4, 5, 2, 3]`（数组表示），排序阶段：

| 轮次 | 交换 | 堆（未就位部分） |
| :--- | :--- | :--- |
| 1 | `8` ↔ `3` | `[5, 4, 3, 2 \| 8]` |
| 2 | `5` ↔ `2` | `[4, 2, 3 \| 5, 8]` |
| … | … | 继续直至有序 |

## 性质

### 稳定性

堆排序是 **不稳定** 排序算法。根本原因在于它本质是 **堆上的选择排序**：每轮把当前最大值换到数组末尾，建堆与下滤过程中也会在树上 **远距离交换** 元素；相等键的相对顺序既不依赖输入先后，也无法通过「相等时不交换」这类局部修补保证。

堆顶与末尾的交换只是表现之一；即便某次下滤避免了相等子节点的交换，排序阶段仍可能把较早出现的相等元素换到较后元素后面。与快排类似，**不能**把不稳定完全归因于某一种交换场景。

以 `[3a, 2, 3b]` 为例（`3a`、`3b` 值均为 3）：

| 阶段 | 操作 | 结果 |
| :--- | :--- | :--- |
| 建堆后 | — | `[3a, 2, 3b]` |
| 排序 `i = 2` | 交换堆顶 `3a` 与 `arr[2]` 的 `3b` | `[3b, 2, 3a]` |
| 排序 `i = 1` | 交换堆顶 `3b` 与 `arr[1]` 的 `2` | `[2, 3b, 3a]` |

输入时 `3a` 在 `3b` 前面，排序后变为 `3b` 在 `3a` 前面，相对顺序被破坏。需要稳定排序时应改用归并排序等算法。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n \log n)$ | 建堆 $O(n)$ + $n - 1$ 次下滤 |
| **最坏** | $O(n \log n)$ | 与输入分布无关 |
| **平均** | $O(n \log n)$ | 同上 |

建堆采用 Floyd 自底向上，复杂度 $O(n)$，优于逐个 `push` 的 $O(n \log n)$。

### 空间复杂度

$O(1)$。直接在原数组上建堆与下滤，仅常数辅助变量。

## 代码实现

> 源码: https://github.com/lllllan02/ciu/tree/master/code/heap-sort

### 基础写法

本仓库实现。Floyd 自底向上建堆 + 反复取堆顶，`heapify` 与 [大顶堆](/dsa/trees/max-heap) 中的 `sift_down` 逻辑一致：

```c
static int left(int i)  { return i * 2 + 1; }
static int right(int i) { return i * 2 + 2; }
static int parent(int i){ return (i - 1) / 2; }

static void heapify(int *arr, int start, int end) {
    while (start < end) {
        int down = start;
        int l = left(down), r = right(down);
        if (l <= end && arr[down] < arr[l]) down = l;
        if (r <= end && arr[down] < arr[r]) down = r;
        if (down == start) break;
        swap(arr + start, arr + down);
        start = down;
    }
}

void heap_sort(int *arr, int len) {
    int end = parent(len - 1);
    for (int i = end; i >= 0; i--) {
        heapify(arr, i, len - 1);
    }

    for (int i = len - 1; i > 0; i--) {
        swap(arr, arr + i);
        heapify(arr, 0, i - 1);
    }
}
```

### 优化写法

下滤时若父节点已不小于较大子节点则 **提前返回**，减少无效循环（思路同 OI Wiki 的 `sift_down`）：

```c
static void sift_down(int *arr, int start, int end) {
    int parent = start;
    int child = parent * 2 + 1;
    while (child <= end) {
        if (child + 1 <= end && arr[child] < arr[child + 1]) {
            child++;
        }
        if (arr[parent] >= arr[child]) return;

        swap(arr + parent, arr + child]);
        parent = child;
        child = parent * 2 + 1;
    }
}
```

建堆与排序阶段将 `heapify` 替换为 `sift_down` 即可，整体复杂度不变。

## 参考阅读

- [x] [OI Wiki - 堆排序](https://oi-wiki.org/basic/heap-sort/) (2026-06-10)
- [x] [菜鸟教程 - 堆排序](https://www.runoob.com/w3cnote/heap-sort.html) (2026-06-10)
- [x] [大顶堆](/dsa/trees/max-heap) (2026-06-09)
- [x] [OI Wiki - 二叉堆](https://oi-wiki.org/ds/binary-heap/) (2026-06-10)
- [ ] [Heap sort in 4 minutes (video)](https://youtu.be/2DmK_H7IdTo)
