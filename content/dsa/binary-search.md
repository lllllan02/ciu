---
title: 二分查找
---

> 源码: https://github.com/lllllan02/ciu/tree/master/code/binary-search

在 **已排序** 的整型数组中查找目标值：每次与中间元素比较，根据大小关系缩小左边界或右边界，直到找到目标或区间为空。时间复杂度 $O(\log n)$。

## 实现

| 函数 | 说明 | 额外空间 |
| :--- | :--- | :--- |
| `binary_search` | 迭代：循环维护 `[l, r]` | $O(1)$ |
| `binary_search_recur` | 递归：在子区间 `[l, r]` 上查找 | $O(\log n)$ 调用栈 |

找到时返回下标，未找到返回 `-1`。
