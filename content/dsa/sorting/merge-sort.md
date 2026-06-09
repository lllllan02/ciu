---
title: 归并排序
order: 1
---

> 源码: https://github.com/lllllan02/ciu/tree/master/code/merge-sort
>
> 参考: [菜鸟教程 - 归并排序](https://www.runoob.com/w3cnote/merge-sort.html)

归并排序（Merge Sort）是一种 **分治** 比较排序算法：将数组分成两半分别排序后 **合并** 两个有序子数组。时间复杂度稳定为 $O(n \log n)$，是 **稳定** 排序。

本实现采用 **自底向上（Bottom-up）** 的迭代写法：按段长 $1, 2, 4, \ldots$ 逐轮两两合并，用一块辅助缓冲区与 `src` / `dst` 乒乓切换，避免递归栈开销。

## 核心思路

自底向上归并排序可以概括为：

1. **分段**：段长 `seg` 从 1 开始，每轮翻倍。
2. **合并**：对每个起点 `start`，将 `[start, start+seg)` 与 `[start+seg, start+2·seg)` 两个有序段合并写入缓冲区。
3. **切换**：每轮结束后交换 `src` 与 `dst`，使排序结果始终落在其中一个缓冲区。
4. **写回**：若最终结果不在原数组，拷贝回 `arr`。

合并时使用 `<=` 取左段元素，保证相等元素的相对顺序不变（**稳定**）。

## API

| 函数 | 说明 |
| :--- | :--- |
| `merge_sort(arr, len)` | 对 `arr[0..len-1]` 升序排序 |

## 算法步骤

### 1. 分配辅助缓冲区

```c
int *buf = malloc(len * sizeof(int));
int *src = arr;
int *dst = buf;
```

### 2. 按段长逐轮合并

外层 `seg` 每次左移一位（即翻倍）；内层 `start` 步长为 `2 * seg`，每次合并相邻两个长度为 `seg` 的有序段：

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
```

### 3. 写回原数组

段长轮数取决于 $\lceil \log_2 n \rceil$；若最终结果落在 `buf`，需拷贝回 `arr`：

```c
if (src != arr) {
    for (int i = 0; i < len; i++) {
        arr[i] = src[i];
    }
}
```

## 复杂度分析

设数组长度为 $n$。

| 项目 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **时间** | $O(n \log n)$ | 共 $\lceil \log_2 n \rceil$ 轮，每轮 $O(n)$ 合并 |
| **额外空间** | $O(n)$ | 一块辅助缓冲区 |
| **稳定性** | 稳定 | 合并时相等元素优先取左段 |

## 与自顶向下递归版的对比

| | 自顶向下（递归） | 自底向上（本实现） |
| :--- | :--- | :--- |
| 分治方式 | 先 `mid` 切分再递归 | 段长从 1 倍增，逐轮合并 |
| 调用栈 | $O(\log n)$ | $O(1)$，纯循环 |
| 辅助空间 | $O(n)$ 合并缓冲 | $O(n)$ 合并缓冲 |
| 时间复杂度 | $O(n \log n)$ | $O(n \log n)$ |

## 测试

```bash
cd code/merge-sort
gcc -Wall -Wextra -std=c11 -o main main.c
./main
```

测试覆盖：基本乱序、已有序、逆序、重复元素，以及空数组、单元素、含负数等边界情况。
