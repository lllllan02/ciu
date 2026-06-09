---
title: 归并排序
order: 1
---

> 源码（迭代）: https://github.com/lllllan02/ciu/tree/master/code/merge-sort
>
> 源码（递归）: https://github.com/lllllan02/ciu/tree/master/code/merge-sort-recur
>
> 源码（链表）: https://github.com/lllllan02/ciu/tree/master/code/merge-sort-linked-list
>
> 参考: [菜鸟教程 - 归并排序](https://www.runoob.com/w3cnote/merge-sort.html)

归并排序（Merge Sort）是一种 **分治** 比较排序算法：将序列分成两半分别排序后 **合并** 两个有序子序列。时间复杂度稳定为 $O(n \log n)$，是 **稳定** 排序。数组实现通常需要 $O(n)$ 辅助缓冲区；链表实现通过改指针完成合并，无需额外数组，是对链表做通用排序的推荐方式。

合并时使用 `<=` 取左段元素，保证相等元素的相对顺序不变。数组的两种写法仅在分治方式上不同，合并逻辑一致。

## API

| 函数 | 说明 |
| :--- | :--- |
| `merge_sort(arr, len)` | 对 `arr[0..len-1]` 升序排序 |
| `merge_sort(head)` | 对链表升序排序，返回排序后的头节点 |

## 数组：两种实现

| | 自顶向下（递归） | 自底向上（迭代） |
| :--- | :--- | :--- |
| 源码 | [`code/merge-sort-recur`](https://github.com/lllllan02/ciu/tree/master/code/merge-sort-recur) | [`code/merge-sort`](https://github.com/lllllan02/ciu/tree/master/code/merge-sort) |
| 分治方式 | 先 `mid` 切分再递归 | 段长从 1 倍增，逐轮合并 |
| 调用栈 | $O(\log n)$ | $O(1)$，纯循环 |
| 辅助空间 | $O(n)$ 合并缓冲 | $O(n)$ 合并缓冲 |
| 时间复杂度 | $O(n \log n)$ | $O(n \log n)$ |

### 自顶向下（递归）

以 `mid = len / 2` 切分左右子数组，递归排序后再合并；思路更贴近分治定义。

1. **基准**：若 `len <= 1`，子数组已有序，直接返回。
2. **切分**：以 `mid = len / 2` 将 `[0, mid)` 与 `[mid, len)` 分为左右两半。
3. **递归**：分别对左右子数组调用 `merge_sort_recur`。
4. **合并**：双指针合并两个有序段写入辅助缓冲区，再拷贝回 `src`。

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

每一层递归在合并后都将结果写回 `src`，最终有序结果落在原数组 `arr` 上。

### 自底向上（迭代）

按段长 $1, 2, 4, \ldots$ 逐轮两两合并，用一块辅助缓冲区与 `src` / `dst` 乒乓切换，避免递归栈开销。

1. **分段**：段长 `seg` 从 1 开始，每轮翻倍。
2. **合并**：对每个起点 `start`，将 `[start, start+seg)` 与 `[start+seg, start+2·seg)` 两个有序段合并写入缓冲区。
3. **切换**：每轮结束后交换 `src` 与 `dst`，使排序结果始终落在其中一个缓冲区。
4. **写回**：若最终结果不在原数组，拷贝回 `arr`。

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

## 链表实现

链表版沿用自顶向下分治，但切分与合并都通过改指针完成，不需要 $O(n)$ 辅助数组。

1. **基准**：若链表为空或仅一个节点，已有序，直接返回 `head`。
2. **切分**：快慢指针找到中点，`prev->next = NULL` 断开前后两段。
3. **递归**：分别对前后两段调用 `merge_sort`。
4. **合并**：双指针按值大小拼接节点，返回新头节点。

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
    if (!head || !head->next) {
        return head;
    }

    Node *second = split(head);
    head = merge_sort(head);
    second = merge_sort(second);
    return merge(head, second);
}
```

快慢指针切分示意：

```c
static Node *split(Node *head) {
    Node *slow = head, *fast = head, *prev = NULL;

    while (fast && fast->next) {
        prev = slow;
        slow = slow->next;
        fast = fast->next->next;
    }
    prev->next = NULL;
    return slow;
}
```

## 复杂度分析

设序列长度为 $n$。

| 项目 | 数组（递归 / 迭代） | 链表 |
| :--- | :--- | :--- |
| **时间** | $O(n \log n)$ | $O(n \log n)$ |
| **额外空间** | $O(n)$ 辅助缓冲区 | $O(\log n)$ 递归栈；合并原地改指针 |
| **稳定性** | 稳定 | 稳定 |

## 测试

数组两种实现与链表实现共用同一套测试场景，覆盖基本乱序、已有序、逆序、重复元素，以及空序列、单元素、含负数等边界情况。

```bash
cd code/merge-sort-recur
gcc -Wall -Wextra -std=c11 -o main main.c
./main

cd ../merge-sort
gcc -Wall -Wextra -std=c11 -o main main.c
./main

cd ../merge-sort-linked-list
gcc -Wall -Wextra -std=c11 -o main main.c
./main
```
