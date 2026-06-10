---
title: 选择排序
order: 2
---

## 定义

选择排序（Selection Sort）是一种简单直观的 **原地** 比较排序算法。每次从未排序区间中找出最小元素，与区间左端交换，使已排序区间从左向右扩展。

## 过程

工作原理：外层 `i` 划定已排序前缀 `[0, i)` 与未排序后缀 `[i, len)`，每轮在后缀中找最小下标 `min`，再与 `arr[i]` 交换。

1. **外层循环**：`i` 从 `0` 到 `len - 1`。
2. **找最小**：令 `min = i`，内层 `j` 从 `i + 1` 扫描到 `len - 1`，若 `arr[j] < arr[min]` 则更新 `min`。
3. **交换**：若 `i != min`，交换 `arr[i]` 与 `arr[min]`。
4. **一轮结束**：`arr[i]` 为全局第 `i` 小的元素，下一轮 `i++`。

以 `[5, 3, 8, 4, 2]` 为例，第一轮（`i = 0`）找最小值过程：

| 步骤 | `j` | 比较 | `min` |
| :--- | :--- | :--- | :--- |
| 初始 | — | — | `0`（值 `5`） |
| 1 | `1` | `3 < 5` | `1` |
| 2 | `2` | `8 > 3` | `1` |
| 3 | `3` | `4 > 3` | `1` |
| 4 | `4` | `2 > 3` | `1` |

交换 `arr[0]` 与 `arr[1]` 后得 `[3, 5, 8, 4, 2]`；后续轮次继续，直至全部有序。

## 性质

### 稳定性

选择排序（数组实现）是 **不稳定** 排序算法。最小元素可能从远处交换到 `arr[i]`，相等元素的相对顺序可能被打破。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n^2)$ | 比较次数固定，已有序时仍须扫完全部区间 |
| **最坏** | $O(n^2)$ | 约 $(n-1) + (n-2) + \cdots + 1$ 次比较 |
| **平均** | $O(n^2)$ | 随机数据下比较次数同阶 |

交换次数至多 $n - 1$ 次；本实现在 `i == min` 时跳过交换，已有序输入可降至 $O(1)$ 次交换。

### 空间复杂度

$O(1)$。原地排序，仅使用常数个辅助变量。

## 代码实现

> 源码: https://github.com/lllllan02/ciu/tree/master/code/selection-sort

### 基础写法

本仓库实现。每轮找最小下标，仅在 `i != min` 时交换，避免无意义的自交换：

```c
static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void selection_sort(int* arr, int len) {
    for (int i = 0; i < len; i++) {
        int min = i;
        for (int j = i + 1; j < len; j++) {
            if (arr[j] < arr[min]) min = j;
        }

        if (i != min) swap(arr + i, arr + min);
    }
}
```

## 参考阅读

- [x] [OI Wiki - 选择排序](https://oi-wiki.org/basic/selection-sort/) (2026-06-10)
- [x] [菜鸟教程 - 选择排序](https://www.runoob.com/w3cnote/selection-sort.html) (2026-06-10)
- [ ] [选择排序（视频）](https://www.youtube.com/watch?v=6nDMgr0-Yyo&index=8&list=PL89B61F78B552C1AB)
- [ ] [Selection sort in 3 minutes (video)](https://youtu.be/g-PGLbMth_g)
