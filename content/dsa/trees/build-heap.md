---
title: 线性时间构建堆
order: 5
---

**构建堆（Build Heap）** 指将一组无序元素整理为满足堆序性的完全二叉树。对大顶堆而言，即保证每个节点都不小于其左右孩子，使堆顶为全局最大值。

## 前提：数组表示与下标关系

与 [大顶堆](/dsa/trees/max-heap) 相同，完全二叉树用一维数组表示，根下标为 0：

| 关系 | 公式 |
| :--- | :--- |
| 左孩子 | $2i + 1$ |
| 右孩子 | $2i + 2$ |
| 父节点 | $\lfloor (i - 1) / 2 \rfloor$ |

**最后一个非叶子节点** 的下标为 `parent(n - 1)`，即 $\lfloor (n - 2) / 2 \rfloor$。从该节点到根，依次对每个下标做一次下滤，即可将整棵完全二叉树调整为大顶堆。

## [方法一：自顶向下](https://github.com/lllllan02/ciu/tree/master/code/max-heap)

将 $n$ 个元素依次插入空堆：每插入一个元素放到数组末尾，再沿父链 **上滤（sift up）** 恢复堆序性。

```c
// 大顶堆：逐个 push，每次 sift_up
for (int i = 0; i < n; i++) {
    push(heap, arr[i]);
}

void push(MaxHeap* heap, int value) {
    heap->data[heap->size++] = value;
    sift_up(heap, heap->size - 1);
}

static void sift_up(MaxHeap* h, int i) {
    while (i > 0) {
        int p = parent(i);
        if (h->data[i] <= h->data[p]) break;
        swap(h, i, p);
        i = p;
    }
}
```

单次 `push` 至多调整 $O(\log n)$ 层，共 $n$ 次，故总时间 **$O(n \log n)$**。

这种方式适合 **流式插入**（元素逐个到来、堆在增长），也是 [大顶堆](/dsa/trees/max-heap) 的日常用法。

## [方法二：自底向上 Floyd 建堆](https://github.com/lllllan02/ciu/tree/master/code/heap-sort)

若 **整段数组已给定**，可跳过逐个 `push`，直接利用完全二叉树的结构：

1. 将输入数组视为一棵完全二叉树（可能尚未满足堆序性）。
2. 从 **最后一个非叶子节点** `parent(n - 1)` 到根 `0`，对每个下标调用一次 **下滤（sift down / heapify）**。

靠近叶子的节点要么已是叶子、要么子树很浅，下滤代价小；只有靠近根的少数节点才可能下沉较深。总量级为 $O(n)$。

```c
// 堆排序中的建堆阶段
int end = (len - 2) / 2;
for (int i = end; i >= 0; i--) {
    sift_down(arr, i, len - 1);
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
```

`sift_down` 与 [大顶堆](/dsa/trees/max-heap) 中的下滤逻辑一致：若当前节点小于左右孩子中的较大者，则交换并继续下沉，直到满足堆序性或成为叶子。

[堆排序](/dsa/sorting/heap-sort) 先以此法建大顶堆，再反复交换堆顶与末尾并缩小堆范围，完成原地排序。

## 与自顶向下的对比

| | 自顶向下 `push` | 自底向上 Floyd |
| :--- | :--- | :--- |
| 适用场景 | 元素逐个到来 | 已有完整数组 |
| 调整方向 | 上滤（向根） | 下滤（向叶） |
| 时间复杂度 | $O(n \log n)$ | $O(n)$ |
| 本仓库实现 | `max_heap.c` → `push` | `heap_sort.c` → 建堆循环 |

## 为何 Floyd 建堆是 $O(n)$

直觉上，$n$ 个节点各做一次下滤，每次最多 $O(\log n)$，相乘像是 $O(n \log n)$。但 Floyd 建堆的实际代价没这么大，原因在于 **节点离叶子越近，能下沉的层数越少**。

完全二叉树里，**越靠近底部，节点越多**；而能沉很多层的，只有少数靠近根的节点。大致可以这样看：

- **大量叶子和近叶节点**：要么不用动，要么最多交换一两层。
- **少量靠近根的节点**：可能一路沉到较深处，但个数大约只有 $n/2$、$n/4$、$n/8$……这样递减。

把「每个节点要沉多少层」和「这一层有多少节点」乘在一起再相加，会发现：浅层节点多、但几乎不动；深层节点少、但可能动得多——两边 **互相抵消**，总量级仍是 **与 $n$ 成正比**，而不是 $n$ 次各 $O(\log n)$ 的简单相加。

所以自底向上建堆是 **线性时间**。

## 与堆排序的关系

[堆排序](/dsa/sorting/heap-sort) 的第一阶段就是 Floyd 建堆；第二阶段才是 $O(n \log n)$ 的「反复取堆顶」。因此：

- **仅建堆**：$O(n)$
- **建堆 + 排序**：$O(n \log n)$

若只需维护一个优先级队列并支持动态插入，用 [大顶堆](/dsa/trees/max-heap) 的 `push` 更合适；若要对 **已有数组** 一次性建堆或原地排序，Floyd 自底向上更高效。

## 参考阅读

- [x] [Hello Algo - 构建堆](https://www.hello-algo.com/chapter_heap/build_heap/) (2026-06-09)
- [x] [大顶堆](/dsa/trees/max-heap) (2026-06-08)
- [x] [堆排序](/dsa/sorting/heap-sort) (2026-06-09)
- [x] [OI Wiki - 二叉堆](https://oi-wiki.org/ds/binary-heap/) (2026-06-08)
