---
title: 插入排序
order: 3
---

## 定义

插入排序（Insertion Sort）是一种简单直观的 **原地** 比较排序算法。将待排元素划分为「已排序」与「未排序」两部分，每次从后者取一个元素插入前者中的正确位置。

打扑克牌时，每抓一张牌按大小插入手牌，即为插入排序的直观类比。

## 过程

工作原理：外层 `i` 扩展已排序前缀；内层将 `arr[i]` 向左移动，插入有序段中的正确位置。

1. **外层循环**：`i` 从 `0` 到 `len - 1`，`arr[0..i-1]` 为已排序区间。
2. **内层扫描**：`j` 从 `i - 1` 递减到 `0`，比较 `arr[j + 1]` 与 `arr[j]`。
3. **终止**：若 `arr[j + 1] >= arr[j]`，说明已就位，`break`。
4. **交换**：否则交换 `arr[j]` 与 `arr[j + 1]`，继续向左。
5. **一轮结束**：`arr[0..i]` 有序，下一轮 `i++`。

以 `[5, 3, 8, 4, 2]` 为例，第二轮（`i = 1`，待插入 `3`）内层过程：

| 步骤 | `j` | 比较 | 交换后 |
| :--- | :--- | :--- | :--- |
| 初始 | — | — | `[5, 3, 8, 4, 2]` |
| 1 | `0` | `3 < 5` | `[3, 5, 8, 4, 2]` |

`3` 就位；后续轮次依次插入 `8`（无需交换）、`4`、`2`，直至全部有序。

## 性质

### 稳定性

插入排序是 **稳定** 排序算法。本实现用 `arr[j + 1] >= arr[j]` 终止，仅在严格小于时交换，相等元素保持原序。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n)$ | 已有序时内层首轮即 `break` |
| **最坏** | $O(n^2)$ | 逆序时约 $\Theta(n^2)$ 次比较与交换 |
| **平均** | $O(n^2)$ | 随机数据下比较次数同阶 |

数据近乎有序时表现很好，是简单排序中适应性较强的一种。

### 空间复杂度

$O(1)$。原地排序，仅使用常数个辅助变量。

## 代码实现

> 源码: https://github.com/lllllan02/ciu/tree/master/code/insertion-sort

### 基础写法

本仓库实现。通过相邻交换将 `arr[i]` 向左插入，就位即停：

```c
static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void insertion_sort(int* arr, int len) {
    for (int i = 0; i < len; i++) {
        for (int j = i - 1; j >= 0; j--) {
            if (arr[j + 1] >= arr[j]) break;

            swap(arr + j, arr + j + 1);
        }
    }
}
```

### 优化写法

保存 `key` 后移较大元素，减少交换次数；用 `>` 判断后移，同样稳定：

```c
void insertion_sort(int* arr, int len) {
    for (int i = 1; i < len; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}
```

## 参考阅读

- [x] [OI Wiki - 插入排序](https://oi-wiki.org/basic/insertion-sort/) (2026-06-10)
- [x] [菜鸟教程 - 插入排序](https://www.runoob.com/w3cnote/insertion-sort.html) (2026-06-10)
- [ ] [插入排序（视频）](https://www.youtube.com/watch?v=c4BRHC7kTaQ&index=2&list=PL89B61F78B552C1AB)
- [ ] [Insertion sort in 2 minutes (video)](https://youtu.be/JU767SDMDvA)
