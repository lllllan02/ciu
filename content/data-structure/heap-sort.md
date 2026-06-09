---
title: 堆排序
---

> 源码: https://github.com/lllllan02/ciu/tree/master/code/heap-sort
>
> 参考: [菜鸟教程 - 堆排序](https://www.runoob.com/w3cnote/heap-sort.html)、[大顶堆](/data-structure/max-heap)、[OI Wiki - 二叉堆](https://oi-wiki.org/ds/binary-heap/)

堆排序（Heap Sort）是一种 **原地** 比较排序算法，利用 **大顶堆** 的堆顶始终是最大值这一性质：反复取出堆顶放到数组末尾，再对剩余部分重新调整为大顶堆，最终得到升序数组。

本实现直接在待排序数组上建堆与下滤，无需额外堆结构；`heapify` 与 [大顶堆](/data-structure/max-heap) 中的 `sift_down` 逻辑一致。

## 核心思路

堆排序可以看作「在数组上模拟大顶堆的 `pop`」：

1. **建堆**：将无序数组调整为大顶堆。
2. **排序**：交换堆顶与当前堆的最后一个元素（最大值就位），缩小堆范围并对新的堆顶 **下滤（heapify / sift down）**。

连续取出堆顶等价于按降序输出；每次放到数组末尾，则整体为升序。

## 数组下标关系

与 [大顶堆](/data-structure/max-heap) 相同，完全二叉树用一维数组表示，根下标为 0：

| 关系 | 公式 |
| :--- | :--- |
| 左孩子 | $2i + 1$ |
| 右孩子 | $2i + 2$ |
| 父节点 | $\lfloor (i - 1) / 2 \rfloor$ |

## API

| 函数 | 说明 |
| :--- | :--- |
| `heap_sort(arr, len)` | 对 `arr[0..len-1]` 原地升序排序 |

## 算法步骤

### 1. 建堆（Floyd 自底向上）

从 **最后一个非叶子节点** `parent(len - 1)` 到根，依次对每个下标调用 `heapify`，时间复杂度 $O(n)$（优于逐个 `push` 建堆的 $O(n \log n)$）。

```c
int end = parent(len - 1);
for (int i = end; i >= 0; i--) {
    heapify(arr, i, len - 1);
}
```

### 2. 排序（反复取堆顶）

将堆顶（当前最大值）与堆的最后一个元素交换，堆规模减 1，再对新的堆顶下标 0 调用 `heapify`。循环到 `i > 0` 即可（`i == 0` 时无需再交换）。

```c
for (int i = len - 1; i > 0; i--) {
    swap(arr, arr + i);
    heapify(arr, 0, i - 1);
}
```

### 下滤（heapify）

参数 `start` 为待调整节点下标，`end` 为当前堆的 **最后一个有效下标**（含）。与 `sift_down` 相同：若当前节点小于左右孩子中的较大者，则交换并继续下沉，直到满足堆序性或成为叶子。

```c
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
```

## 复杂度分析

设数组长度为 $n$。

| 项目 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **建堆** | $O(n)$ | Floyd 自底向上 |
| **排序阶段** | $O(n \log n)$ | $n - 1$ 次取堆顶，每次下滤 $O(\log n)$ |
| **总时间** | $O(n \log n)$ | 最坏、平均、最好均为 $O(n \log n)$ |
| **额外空间** | $O(1)$ | 原地，仅常数辅助变量 |
| **稳定性** | 不稳定 | 远距离交换可能打乱相等元素的相对顺序 |

## 与大顶堆实现的对比

| | 大顶堆（`max_heap`） | 堆排序（本实现） |
| :--- | :--- | :--- |
| 数据结构 | 独立 `MaxHeap` 结构体 | 直接在原数组上操作 |
| 建堆 | 逐个 `push` + `sift_up` | Floyd 自底向上 `heapify` |
| 取最大值 | `pop` 返回并缩小 `size` | `swap` 到数组末尾并缩小堆范围 |
| 额外空间 | $O(n)$ 数组（或复用输入） | $O(1)$ 原地 |
| 输出 | 连续 `pop` 为降序 | 原地完成后为升序 |

## 测试

```bash
cd code/heap-sort
gcc -Wall -Wextra -std=c11 -o main main.c
./main
```

测试覆盖：基本乱序、已有序、逆序、重复元素，以及空数组、单元素、含负数等边界情况。
