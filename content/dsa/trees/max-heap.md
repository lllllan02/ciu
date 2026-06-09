---
title: 大顶堆
---

> 源码: https://github.com/lllllan02/ciu/tree/master/code/max-heap
>
> 参考: [OI Wiki - 二叉堆](https://oi-wiki.org/ds/binary-heap/)、[Hello Algo - 堆](https://www.hello-algo.com/chapter_heap/heap/)

大顶堆（Max Heap）是一种 **二叉堆**，用完全二叉树组织数据，并满足 **堆序性**：每个节点的值 **不小于** 其左右孩子的值。因此堆顶（根节点）始终是全局最大值。

本实现采用 **数组存储** 的定容二叉堆，通过 `sift_up` / `sift_down` 维护堆性质，支持 $O(\log n)$ 的插入与删除最大值。

## 核心概念

### 完全二叉树

堆在逻辑上是一棵完全二叉树：除最后一层外每层均满，最后一层节点从左到右连续排列。这一性质使得树可以用 **一维数组** 紧凑表示，无需指针。

### 堆序性（大顶堆）

对任意下标 $i$（$i > 0$）：

$$
\text{data}[\text{parent}(i)] \ge \text{data}[i]
$$

等价地，每个节点都不小于其孩子。中序遍历 **不保证** 有序，但根始终是最大值。

### 数组下标关系

对下标为 $i$ 的节点（根为 0）：

| 关系 | 公式 |
| :--- | :--- |
| 左孩子 | $2i + 1$ |
| 右孩子 | $2i + 2$ |
| 父节点 | $\lfloor (i - 1) / 2 \rfloor$ |

本实现中的辅助函数：

```c
static int left(int i)   { return i * 2 + 1; }
static int right(int i)  { return i * 2 + 2; }
static int parent(int i) { return (i - 1) / 2; }
```

## 数据结构

```c
typedef struct MaxHeap {
    int* data;
    int size;
    int capacity;
} MaxHeap;
```

- `data`：存储堆元素的数组，下标 0 为堆顶。
- `size`：当前元素个数。
- `capacity`：数组容量（定容，满时 `push` 触发断言）。

## API

| 函数 | 说明 |
| :--- | :--- |
| `create_max_heap(capacity)` | 创建定容大顶堆 |
| `destroy_max_heap(heap)` | 释放堆 |
| `push(heap, value)` | 插入元素，$O(\log n)$ |
| `pop(heap)` | 删除并返回最大值，$O(\log n)$ |
| `top(heap)` | 返回堆顶（最大值），不删除 |
| `size(heap)` | 当前元素个数 |
| `is_empty(heap)` | 是否为空 |
| `is_full(heap)` | 是否已满 |

### 插入（push）

1. 将新元素放到数组末尾（完全二叉树的下一个叶子位置），`size++`。
2. 从该位置 **向上调整（sift up）**：若当前值大于父节点则交换，直到满足堆序性或到达根。

```c
static void sift_up(MaxHeap* h, int i) {
    while (i > 0) {
        int p = parent(i);
        if (h->data[i] <= h->data[p]) break;
        swap(h, i, p);
        i = p;
    }
}
```

### 删除最大值（pop）

1. 保存堆顶 `data[0]` 作为返回值。
2. 将 **最后一个元素** 移到堆顶，`size--`。
3. 从堆顶 **向下调整（sift down）**：与左右孩子中较大者交换，直到满足堆序性或成为叶子。

```c
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

连续 `pop` 会按 **降序** 输出所有元素，这也是 **堆排序** 的核心思路。

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

## 测试

```bash
cd code/max-heap
gcc -Wall -Wextra -std=c11 -o main main.c
./main
```

测试覆盖：创建与销毁、`push` / `pop` / `top`、`is_empty` / `is_full` / `size`，以及每次操作后堆序性的校验（每个节点不小于其左右孩子）。
