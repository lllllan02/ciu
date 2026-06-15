---
title: 大顶堆
order: 3
---

大顶堆（Max Heap）是一种 **二叉堆**，用完全二叉树组织数据，并满足 **堆序性**：每个节点的值 **不小于** 其左右孩子的值。因此堆顶（根节点）始终是全局最大值。

本实现采用 **数组存储** 的定容二叉堆，通过 `sift_up` / `sift_down` 维护堆性质，支持 $O(\log n)$ 的插入与删除最大值。

**特性**：`push` / `pop` $O(\log n)$ · `top` $O(1)$ · 空间 $O(n)$ · 定容数组

## 完全二叉树与堆序性

堆在逻辑上是一棵 **完全二叉树**：除最后一层外每层均满，最后一层节点从左到右连续排列。大顶堆要求每个节点 **不小于** 其左右孩子；中序遍历 **不保证** 有序，但根始终是最大值。

## 数组表示

完全二叉树可压平为一维数组，根下标为 0：

| 关系 | 公式 |
| :--- | :--- |
| 左孩子 | $2i + 1$ |
| 右孩子 | $2i + 2$ |
| 父节点 | $\lfloor (i - 1) / 2 \rfloor$ |

`MaxHeap` 用 `data` 数组存元素，`size` 为当前个数，`capacity` 为容量（满时 `push` 触发断言）。

## [代码实现](https://github.com/lllllan02/ciu/tree/master/code/max-heap)

`push` 将新元素放到数组末尾后 **上滤**；`pop` 将末尾元素移到堆顶后 **下滤**：

```c
void push(MaxHeap* heap, int value) {
    heap->data[heap->size++] = value;
    sift_up(heap, heap->size - 1);
}

int pop(MaxHeap* heap) {
    int t = top(heap);
    heap->data[0] = heap->data[--heap->size];
    sift_down(heap, 0);
    return t;
}

static void sift_up(MaxHeap* h, int i) {
    while (i > 0) {
        int p = parent(i);
        if (h->data[i] <= h->data[p]) break;
        swap(h, i, p);
        i = p;
    }
}

static void sift_down(MaxHeap* h, int i) {
    while (true) {
        int l = left(i), r = right(i), down = i;
        if (l < h->size && h->data[l] > h->data[down]) down = l;
        if (r < h->size && h->data[r] > h->data[down]) down = r;
        if (down == i) break;
        swap(h, i, down);
        i = down;
    }
}
```

连续 `pop` 会按 **降序** 输出所有元素，这也是 [堆排序](/dsa/sorting/heap-sort) 的核心思路；一次性建堆见 [线性时间构建堆](/dsa/trees/build-heap)。

## 复杂度分析

设堆中有 $n$ 个元素，树高 $h = \lfloor \log_2 n \rfloor$。

| 操作 | 时间复杂度 | 说明 |
| :--- | :--- | :--- |
| **push** | $O(\log n)$ | 至多沿树高向上调整一次 |
| **pop** | $O(\log n)$ | 至多沿树高向下调整一次 |
| **top** | $O(1)$ | 直接读 `data[0]` |
| **空间复杂度** | $O(n)$ | 数组存储 |

## 与优先级队列

堆是 **优先级队列（Priority Queue）** 的经典实现：每次取出的都是当前优先级最高（此处为数值最大）的元素。C++ 的 `std::priority_queue`、Java 的 `PriorityQueue` 底层通常即为二叉堆。

| | 本实现（定容数组堆） | 标准库优先级队列 |
| :--- | :--- | :--- |
| 底层结构 | 数组 + 完全二叉树 | 通常为二叉堆 |
| 扩容 | 不支持（满则断言） | 动态扩容 |
| 取最值 | `top` / `pop` | `top` / `pop` |
| 插入 | `push` | `push` |

## 参考阅读

- [x] [OI Wiki - 二叉堆](https://oi-wiki.org/ds/binary-heap/) (2026-06-08)
- [x] [Hello Algo - 堆](https://www.hello-algo.com/chapter_heap/heap/) (2026-06-08)
- [x] [线性时间构建堆](/dsa/trees/build-heap) (2026-06-09)
- [x] [堆排序](/dsa/sorting/heap-sort) (2026-06-09)
