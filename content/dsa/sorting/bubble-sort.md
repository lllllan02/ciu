---
title: 冒泡排序
order: 1
---

## 定义

冒泡排序（Bubble Sort）是一种简单的 **原地** 比较排序算法：反复对相邻元素 **两两比较**，逆序则交换，使元素像气泡一样逐步「冒」向区间一端，故称为冒泡排序。

**特性**：平均 $O(n^2)$ · 最好 $O(n)$ · 最坏 $O(n^2)$ · 空间 $O(1)$ · 原地 · **稳定**

## 过程

共进行至多 $n - 1$ 趟扫描。每趟沿固定方向遍历 **未排序** 区间，相邻元素两两比较并交换，使目标极值逐步换至区间一端；一趟结束后，该端元素就位，即为当前未排序部分的极值。

![冒泡排序动画](/images/bubble-sort.gif)

## 性质

### 稳定性

冒泡排序是 **稳定** 排序算法。仅在相邻两元素前者严格大于后者时才交换，相等元素不会互换位置，相对顺序保持不变。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n)$ | 已有序时，一趟扫描无交换即可结束 |
| **最坏** | $O(n^2)$ | 逆序时约 $\frac{(n-1)n}{2}$ 次比较与交换 |
| **平均** | $O(n^2)$ | 随机数据下比较次数同阶 |

### 空间复杂度

$O(1)$。原地排序，仅使用常数个辅助变量。

## [代码实现](https://github.com/lllllan02/ciu/tree/master/code/bubble-sort)

用 `flag` 控制是否继续扫描：每趟开始前置为 `false`，扫描中一旦发生交换便置为 `true`；若整趟结束仍为 `false`，说明序列已有序，提前退出循环：

```c {11,15}
static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void bubble_sort(int* arr, int len) {
    if (len <= 1) return;

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
}
```

## 参考阅读

- [x] [OI Wiki - 冒泡排序](https://oi-wiki.org/basic/bubble-sort/) (2026-06-10)
- [x] [菜鸟教程 - 冒泡排序](https://www.runoob.com/w3cnote/bubble-sort.html) (2026-06-10)
- [ ] [冒泡排序（视频）](https://www.youtube.com/watch?v=P00xJgWzz2c&index=1&list=PL89B61F78B552C1AB)
- [ ] [冒泡排序分析（视频）](https://www.youtube.com/watch?v=ni_zk257Nqo&index=7&list=PL89B61F78B552C1AB)
- [ ] [Bubble sort in 2 minutes (video)](https://youtu.be/xli_FI7CuzA)
