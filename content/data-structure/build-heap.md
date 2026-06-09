---
title: 线性时间构建堆
---

> 源码: [堆排序 `heapify` 建堆](https://github.com/lllllan02/ciu/tree/master/code/heap-sort)、[大顶堆 `push` 建堆](https://github.com/lllllan02/ciu/tree/master/code/max-heap)
>
> 参考: [Hello Algo - 构建堆](https://www.hello-algo.com/chapter_heap/build_heap/)、[大顶堆](/data-structure/max-heap)、[堆排序](/data-structure/heap-sort)、[OI Wiki - 二叉堆](https://oi-wiki.org/ds/binary-heap/)

**构建堆（Build Heap）** 指将一组无序元素整理为满足堆序性的完全二叉树。对大顶堆而言，即保证每个节点都不小于其左右孩子，使堆顶为全局最大值。

本仓库有两种典型建堆方式：

| 方式 | 思路 | 时间复杂度 | 对应实现 |
| :--- | :--- | :--- | :--- |
| **自顶向下** | 逐个插入，每次 `sift_up` | $O(n \log n)$ | [大顶堆](/data-structure/max-heap) 的 `push` |
| **自底向上（Floyd）** | 从最后一个非叶子节点到根，依次 `sift_down` | $O(n)$ | [堆排序](/data-structure/heap-sort) 的建堆阶段 |

直觉上「对每个节点都下滤一次」像是 $O(n \log n)$，但 Floyd 自底向上建堆的总量级是 **线性** 的——关键在于 **越靠近叶子，节点越少、可下沉层数也越少**。

## 前提：数组表示与下标关系

与 [大顶堆](/data-structure/max-heap) 相同，完全二叉树用一维数组表示，根下标为 0：

| 关系 | 公式 |
| :--- | :--- |
| 左孩子 | $2i + 1$ |
| 右孩子 | $2i + 2$ |
| 父节点 | $\lfloor (i - 1) / 2 \rfloor$ |

**最后一个非叶子节点** 的下标为 `parent(n - 1)`，即 $\lfloor (n - 2) / 2 \rfloor$。从该节点到根，依次对每个下标做一次下滤，即可将整棵完全二叉树调整为大顶堆。

## 方法一：自顶向下（$O(n \log n)$）

将 $n$ 个元素依次插入空堆：每插入一个元素放到数组末尾，再沿父链 **上滤（sift up）** 恢复堆序性。

```c
// 大顶堆：逐个 push，每次 sift_up
for (int i = 0; i < n; i++) {
    push(heap, arr[i]);
}
```

单次 `push` 至多调整 $O(\log n)$ 层，共 $n$ 次，故总时间 **$O(n \log n)$**。

这种方式适合 **流式插入**（元素逐个到来、堆在增长），也是 [大顶堆](/data-structure/max-heap) 的日常用法。

## 方法二：自底向上 Floyd 建堆（$O(n)$）

若 **整段数组已给定**，可跳过逐个 `push`，直接利用完全二叉树的结构：

1. 将输入数组视为一棵完全二叉树（可能尚未满足堆序性）。
2. 从 **最后一个非叶子节点** `parent(n - 1)` 到根 `0`，对每个下标调用一次 **下滤（sift down / heapify）**。

靠近叶子的节点要么已是叶子、要么子树很浅，下滤代价小；只有靠近根的少数节点才可能下沉较深。总量级为 $O(n)$。

```c
// 堆排序中的建堆阶段
int end = parent(len - 1);
for (int i = end; i >= 0; i--) {
    heapify(arr, i, len - 1);
}
```

`heapify` 与 [大顶堆](/data-structure/max-heap) 中的 `sift_down` 逻辑一致：若当前节点小于左右孩子中的较大者，则交换并继续下沉，直到满足堆序性或成为叶子。

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

[堆排序](/data-structure/heap-sort) 先以此法建大顶堆，再反复交换堆顶与末尾并缩小堆范围，完成原地排序。

## 为何 Floyd 建堆是 $O(n)$

设树高为 $h = \lfloor \log_2 n \rfloor$，**高度为 $k$ 的节点**（叶层高度为 0）最多下滤 $k$ 层。

高度为 $k$ 的节点个数至多为 $\lceil n / 2^{k+1} \rceil$（靠近叶子的层节点更多）。建堆总代价上界为：

$$
\sum_{k=0}^{h} \left\lceil \frac{n}{2^{k+1}} \right\rceil \cdot O(k)
= O\!\left(n \sum_{k=0}^{\infty} \frac{k}{2^{k+1}}\right)
= O(n)
$$

因为级数 $\sum_{k=0}^{\infty} k / 2^k$ 收敛于常数 2。因此自底向上建堆是 **线性时间**，而非 $n$ 次各 $O(\log n)$ 的简单相加。

### 与自顶向下的对比

| | 自顶向下 `push` | 自底向上 Floyd |
| :--- | :--- | :--- |
| 适用场景 | 元素逐个到来 | 已有完整数组 |
| 调整方向 | 上滤（向根） | 下滤（向叶） |
| 时间复杂度 | $O(n \log n)$ | $O(n)$ |
| 本仓库实现 | `max_heap.c` → `push` | `heap_sort.c` → 建堆循环 |

## 小顶堆

将比较方向反转即可：下滤时与 **较小** 的孩子交换，上滤时若当前值 **小于** 父节点则交换。Floyd 自底向上的 $O(n)$ 结论同样成立。

## 与堆排序的关系

[堆排序](/data-structure/heap-sort) 的第一阶段就是 Floyd 建堆；第二阶段才是 $O(n \log n)$ 的「反复取堆顶」。因此：

- **仅建堆**：$O(n)$
- **建堆 + 排序**：$O(n \log n)$

若只需维护一个优先级队列并支持动态插入，用 [大顶堆](/data-structure/max-heap) 的 `push` 更合适；若要对 **已有数组** 一次性建堆或原地排序，Floyd 自底向上更高效。

## 测试

建堆逻辑已包含在堆排序与大顶堆的测试中：

```bash
# Floyd 自底向上建堆（堆排序建堆阶段）
cd code/heap-sort
gcc -Wall -Wextra -std=c11 -o main main.c
./main

# 自顶向下建堆（逐个 push）
cd code/max-heap
gcc -Wall -Wextra -std=c11 -o main main.c
./main
```
