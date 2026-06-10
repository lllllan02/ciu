---
title: 归并排序
order: 5
---

## 定义

归并排序（Merge Sort）是一种 **分治** 比较排序算法：将序列分成两半分别排序后，**合并** 两个有序子序列。最好、最坏与平均时间复杂度均为 $O(n \log n)$，是 **稳定** 排序。

与快排相比，归并排序的时间复杂度 **稳定** 为 $O(n \log n)$，不依赖输入分布；代价是数组实现通常需要 $O(n)$ 辅助缓冲区。在链表上通过改指针完成合并，无需额外数组，是对链表做通用排序的推荐方式。

## 过程

工作原理：分治三步——**切分**、**递归排序**、**合并**。

1. **切分**：以 `mid = len / 2` 将序列分为左右两半（或按段长倍增迭代合并）。
2. **排序**：分别对左右子序列排序，直至子序列长度 ≤ 1。
3. **合并**：双指针比较左右有序段，依次取较小者写入缓冲区；相等时取左侧，保证稳定性。
4. **结束**：合并完成后写回原数组（或链表拼接完毕）。

以 `[5, 3, 8, 4, 2]` 为例，自顶向下切分与合并：

```
        [5, 3, 8, 4, 2]
       /              \
   [5, 3, 8]          [4, 2]
   /      \            /    \
[5, 3]    [8]      [4]      [2]
 /  \
[5] [3]

合并：[5]+[3]→[3,5] | [3,5]+[8]→[3,5,8] | [4]+[2]→[2,4] | [3,5,8]+[2,4]→[2,3,4,5,8]
```

## 性质

### 稳定性

归并排序是 **稳定** 排序算法。合并时取 `src[s1] <= src[s2]` 的左侧元素，相等键保持原有相对顺序。

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

> 源码（递归）: https://github.com/lllllan02/ciu/tree/master/code/merge-sort-recur
>
> 源码（迭代）: https://github.com/lllllan02/ciu/tree/master/code/merge-sort
>
> 源码（链表）: https://github.com/lllllan02/ciu/tree/master/code/merge-sort-linked-list

### 基础写法（数组 · 自顶向下）

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
