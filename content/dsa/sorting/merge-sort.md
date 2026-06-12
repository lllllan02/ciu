---
title: 归并排序
order: 5
---

## 定义

归并排序（Merge Sort）是一种 **分治** 比较排序算法：将序列分成两半分别排序后，**合并** 两个有序子序列。最好、最坏与平均时间复杂度均为 $O(n \log n)$，是 **稳定** 排序。

与[快排](/dsa/sorting/quick-sort)相比，归并排序的时间复杂度 **稳定** 为 $O(n \log n)$，不依赖输入分布；代价是数组实现通常需要 $O(n)$ 辅助缓冲区。在链表上通过改指针完成合并，无需额外数组，是对链表做通用排序的推荐方式。

## 过程

工作原理：分治三步——**切分**、**递归排序**、**合并**。

1. **切分**：从中间将序列分为左右两半。
2. **排序**：分别对左右子序列排序，直至每段只剩一个元素（天然有序）。
3. **合并**：左右两段均已有序，从各自开头依次比较，取较小者放入结果；相等时先取左侧，保证稳定。
4. **结束**：层层向上合并，最终得到有序序列。

![归并排序动画](/images/merge-sort.gif)

动图对全部 15 个元素执行相同流程。取前 5 个 `[3, 44, 38, 5, 47]` 示意切分与合并：

```
        [3, 44, 38, 5, 47]
       /                 \
   [3, 44, 38]           [5, 47]
   /      \               /    \
[3, 44]   [38]        [5]      [47]
 /  \
[3] [44]
```

自底向上合并：

| 步骤 | 左段 | 右段 | 合并结果 |
| :--- | :--- | :--- | :--- |
| 1 | `[3]` | `[44]` | `[3, 44]` |
| 2 | `[3, 44]` | `[38]` | `[3, 38, 44]` |
| 3 | `[5]` | `[47]` | `[5, 47]` |
| 4 | `[3, 38, 44]` | `[5, 47]` | `[3, 5, 38, 44, 47]` |

## 性质

### 稳定性

归并排序是 **稳定** 排序算法。合并时若左右两段当前元素相等，先取左侧，相等键的相对顺序保持不变。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n \log n)$ | 始终分治合并，与输入分布无关 |
| **最坏** | $O(n \log n)$ | 同上 |
| **平均** | $O(n \log n)$ | 同上 |

### 空间复杂度

| 实现 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **数组** | $O(n)$ | 合并阶段需要辅助缓冲区 |
| **链表** | $O(\log n)$ | 递归栈；合并通过改指针，无需额外数组 |

## 代码实现

### 基础写法（数组 · 递归 · 自顶向下）

> 源码: https://github.com/lllllan02/ciu/tree/master/code/merge-sort-recur

以 `mid` 切分后递归排序再合并，思路最贴近分治定义：

```c
static void merge_sort_recur(int *src, int *dst, int len) {
    if (len <= 1) return;

    int s1 = 0, e1 = len / 2;
    int s2 = e1, e2 = len;

    merge_sort_recur(src + s1, dst + s1, e1 - s1);
    merge_sort_recur(src + s2, dst + s2, e2 - s2);

    int index = 0;
    while (s1 < e1 && s2 < e2) {
        dst[index++] = src[s1] <= src[s2] ? src[s1++] : src[s2++];
    }
    while (s1 < e1) dst[index++] = src[s1++];
    while (s2 < e2) dst[index++] = src[s2++];

    for (int i = 0; i < len; i++) {
        src[i] = dst[i];
    }
}
```

### 优化写法（数组 · 自底向上）

> 源码: https://github.com/lllllan02/ciu/tree/master/code/merge-sort

段长从 1 倍增逐轮两两合并，纯循环实现，避免递归栈开销：

```c
for (int seg = 1; seg < len; seg <<= 1) {
    for (int start = 0; start < len; start += 2 * seg) {
        int s1 = start, e1 = min(start + seg, len);
        int s2 = e1,   e2 = min(start + 2 * seg, len);
        int index = start;

        while (s1 < e1 && s2 < e2) {
            dst[index++] = src[s1] <= src[s2] ? src[s1++] : src[s2++];
        }
        while (s1 < e1) dst[index++] = src[s1++];
        while (s2 < e2) dst[index++] = src[s2++];
    }
    // 交换 src / dst
}

if (src != arr) {
    for (int i = 0; i < len; i++) {
        arr[i] = src[i];
    }
}
```

### 链表实现

> 源码: https://github.com/lllllan02/ciu/tree/master/code/merge-sort-linked-list

快慢指针切分，递归排序后按值拼接，无需 $O(n)$ 辅助数组：

```c
static Node *merge(Node *left, Node *right) {
    Node dummy = {0, NULL};
    Node *tail = &dummy;

    while (left && right) {
        if (left->data <= right->data) {
            tail->next = left;
            left = left->next;
        } else {
            tail->next = right;
            right = right->next;
        }
        tail = tail->next;
    }
    tail->next = left ? left : right;
    return dummy.next;
}

Node *merge_sort(Node *head) {
    if (!head || !head->next) return head;

    Node *second = split(head);
    head = merge_sort(head);
    second = merge_sort(second);
    return merge(head, second);
}
```

## 参考阅读

- [x] [OI Wiki - 归并排序](https://oi-wiki.org/basic/merge-sort/) (2026-06-10)
- [x] [菜鸟教程 - 归并排序](https://www.runoob.com/w3cnote/merge-sort.html) (2026-06-09)
- [ ] [链表的归并排序](http://www.geeksforgeeks.org/merge-sort-for-linked-list/)
- [ ] [插入排序 & 归并排序（视频）](https://www.youtube.com/watch?v=Kg4bqzAqRBM&index=3&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [归并排序（视频）](https://www.youtube.com/watch?v=GCae1WNvnZM&index=3&list=PL89B61F78B552C1AB)
- [ ] [Merge sort in 3 minutes (video)](https://youtu.be/4VqmGXwpLqc)
- [ ] [使用外部数组（C 语言）](http://www.cs.yale.edu/homes/aspnes/classes/223/examples/sorting/mergesort.c)
- [ ] [使用外部数组（Python 语言）](https://github.com/jwasham/practice-python/blob/master/merge_sort/merge_sort.py)
- [ ] [对原数组直接排序（C++）](https://github.com/jwasham/practice-cpp/blob/master/merge_sort/merge_sort.cc)
