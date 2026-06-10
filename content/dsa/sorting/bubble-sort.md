---
title: 冒泡排序
order: 1
---

## 定义

冒泡排序（Bubble Sort）是一种简单的 **原地** 比较排序算法。执行过程中，较小的元素像气泡一样逐步「浮」到数列一端，较大的元素沉到另一端，故称为冒泡排序。

## 过程

工作原理：每次检查 **相邻两个** 元素，若前者大于后者则交换；重复扫描，直到某一趟扫描中不再发生交换，或已确定末尾若干元素就位。

经过 $i$ 趟扫描后，数列末尾 $i$ 项必然是当前最大的 $i$ 项，因此最多扫描 $n - 1$ 遍即可完成排序。

本实现外层用 `i` 控制未排序区间右边界，内层从左向右相邻比较；每轮将当前未排序区间中的最大值交换到 `arr[i]`：

1. **外层循环**：`i` 从 `len - 1` 递减到 `0`。
2. **内层扫描**：`j` 从 `0` 到 `i - 1`，比较 `arr[j]` 与 `arr[j + 1]`。
3. **交换**：若 `arr[j] > arr[j + 1]`，交换两者。
4. **一轮结束**：`arr[i]` 就位，下一轮缩小右边界。

以 `[5, 3, 8, 4, 2]` 为例，第一轮（`i = 4`）内层过程：

| 步骤 | 比较 | 交换后 |
| :--- | :--- | :--- |
| 1 | `5 > 3` | `[3, 5, 8, 4, 2]` |
| 2 | `5 < 8` | 不变 |
| 3 | `8 > 4` | `[3, 5, 4, 8, 2]` |
| 4 | `8 > 2` | `[3, 5, 4, 2, 8]` |

## 性质

### 稳定性

冒泡排序是 **稳定** 排序算法。本实现仅在 `arr[j] > arr[j + 1]` 时交换，相等元素不会互换位置，相对顺序保持不变。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n^2)$（本实现） | 本实现每轮固定扫完未排序区间，不做提前终止 |
| **最好** | $O(n)$（常见优化） | 若加入「本轮无交换则结束」优化，已有序时一趟即可 |
| **最坏** | $O(n^2)$ | 约执行 $\frac{(n-1)n}{2}$ 次比较与交换 |
| **平均** | $O(n^2)$ | 随机数据下比较次数同阶 |

### 空间复杂度

$O(1)$。原地排序，仅使用常数个辅助变量。

## 代码实现

> 源码: https://github.com/lllllan02/ciu/tree/master/code/bubble-sort

### 基础写法

本仓库实现。外层用 `i` 控制未排序区间右边界，每轮固定扫完当前区间：

```c
static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void bubble_sort(int* arr, int len) {
    for (int i = len - 1; i >= 0; i--) {
        for (int j = 0; j < i; j++) {
            if (arr[j] > arr[j + 1]) {
                swap(arr + j, arr + j + 1);
            }
        }
    }
}
```

### 优化写法

用 `flag` 标记本轮是否发生交换，某轮无交换则提前结束；已有序时时间复杂度降至 $O(n)$：

```c
bool flag = true;
while (flag) {
    flag = false;
    for (int i = 0; i < len - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            flag = true;
            swap(arr + i, arr + i + 1);
        }
    }
}
```

## 参考阅读

- [x] [OI Wiki - 冒泡排序](https://oi-wiki.org/basic/bubble-sort/) (2026-06-10)
- [x] [菜鸟教程 - 冒泡排序](https://www.runoob.com/w3cnote/bubble-sort.html) (2026-06-10)
- [ ] [冒泡排序（视频）](https://www.youtube.com/watch?v=P00xJgWzz2c&index=1&list=PL89B61F78B552C1AB)
- [ ] [冒泡排序分析（视频）](https://www.youtube.com/watch?v=ni_zk257Nqo&index=7&list=PL89B61F78B552C1AB)
- [ ] [Bubble sort in 2 minutes (video)](https://youtu.be/xli_FI7CuzA)
