---
title: 选择排序
order: 2
---

## 定义

选择排序（Selection Sort）是一种简单直观的 **原地** 比较排序算法：每轮在未排序区间中选出 **极值**（最小或最大），将其换到区间一端，使已排序部分逐步扩展。

**特性**：平均 $O(n^2)$ · 最好 $O(n^2)$ · 最坏 $O(n^2)$ · 空间 $O(1)$ · 原地 · **不稳定**

## 过程

每趟扫描未排序区间，找出其中的 **极值** 并与区间边界元素交换——该极值就此就位，已排序部分扩展一位。每趟至少确定一个元素的位置，因此 $n$ 个元素最多进行 $n - 1$ 趟即可排完。

![选择排序动画](/images/selection-sort-animate.svg)

## 性质

### 稳定性

选择排序是 **不稳定** 排序算法。每趟将未排序区间的 **极值** 与区间边界元素 **互换**，两者通常 **并不相邻**；边界元素落位时会 **跨过** 区间内的其他元素。若途中存在与边界元素 **键相等** 的对象，一次交换就可能打乱它们的相对顺序。相等时即便在选取极值时有固定取舍规则，也 **不能** 避免这种跨换。

以升序为例，`[3a, 3b, 1]`（`3a`、`3b` 值均为 3）：

| 趟次 | 操作 | 结果 |
| :--- | :--- | :--- |
| 第 1 趟 | 极值 `1` 与边界 `3a` 交换 | `[1, 3b, 3a]` |
| 第 2 趟 | 极值 `3b` 已在边界，无需交换 | `[1, 3b, 3a]` |

输入时 `3a` 在 `3b` 前面，排序后 `3b` 在 `3a` 前面，相对顺序被破坏。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n^2)$ | 比较次数固定，已有序时仍须扫完全部区间 |
| **最坏** | $O(n^2)$ | 约 $(n-1) + (n-2) + \cdots + 1$ 次比较 |
| **平均** | $O(n^2)$ | 随机数据下比较次数同阶 |

比较次数无法省略，但交换次数至多 $n - 1$ 次。可在交换前判断极值是否已在区间边界——已在则跳过；对已有序输入，交换次数可降至 $0$。

### 空间复杂度

$O(1)$。原地排序，仅使用常数个辅助变量。

## [代码实现](https://github.com/lllllan02/ciu/tree/master/code/selection-sort)

### 基础写法

用 `i != min` 判断极值是否已在边界，避免无意义的自交换：

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
