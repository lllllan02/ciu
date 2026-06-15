---
title: 计数排序
order: 8
---

## 定义

计数排序（Counting Sort）是一种 **非比较** 排序算法：不通过元素间大小比较，而是统计每个键值出现次数，再按键值从小到大依次写回，得到有序序列。

适用于 **键值为整数且范围不大** 的场景。设 $n$ 为元素个数、$k$ 为键值范围（或上界与下界之差），时间复杂度为 $O(n + k)$。当 $k$ 与 $n$ 同阶甚至更大时，辅助空间与扫描开销会抵消优势，不如比较排序实用。

计数排序是 **稳定** 的，也是 [基数排序](/dsa/sorting#基数排序) 常用的子过程。

**特性**：平均 $O(n + k)$ · 最好 $O(n + k)$ · 最坏 $O(n + k)$ · 空间 $O(n + k)$ · 非原地 · **稳定**

## 过程

工作原理分三步：**计数**、**前缀和**、**放置**。

1. **计数**：遍历输入，统计每个键值出现的次数。
2. **前缀和**：将计数数组从左到右累加，得到每个键值在输出中的末尾位置。
3. **放置**：从右向左遍历输入，按前缀和指示的位置写入输出，每放置一个便将对应计数减 1。
4. **结束**：输出填满，序列有序。

![计数排序动画](/images/counting-sort-animate.svg)

**为何需要前缀和？** 计数后只知道每个键值出现了几次，还不知道该放进输出的哪个位置。前缀和把频次累加成 **末尾位置**——键值 $v$ 的前缀和即输出中 $v$ 的最后一个槽位；放置时查表即可定位，每放一个便将对应计数减 1。配合从右向左遍历输入，同一键值中先出现的元素仍排在前面，从而保持稳定。

## 性质

### 稳定性

计数排序是 **稳定** 排序算法。放置阶段 **从右向左** 遍历输入，同一键值中后出现的元素先写入输出中更靠后的位置，相等键的相对顺序得以保持。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n + k)$ | 计数 $O(n)$ + 前缀和 $O(k)$ + 放置 $O(n)$ |
| **最坏** | $O(n + k)$ | 与输入分布无关 |
| **平均** | $O(n + k)$ | 同上 |

$k$ 为键值范围，即 $\text{max\_val} - \text{min\_val} + 1$。

### 空间复杂度

$O(n + k)$。需要长度为 $k$ 的计数数组，以及与输入等长的输出缓冲区。

## [代码实现](https://github.com/lllllan02/ciu/tree/master/code/counting-sort)

本仓库实现。前缀和 + 辅助数组 + 从右向左放置；`min_val` / `max_val` 将键值平移到 $[0, k)$，可处理含负数的数据：

```c
void counting_sort(int* arr, int len, int min_val, int max_val) {
    if (len <= 1) return;

    int range = max_val - min_val + 1;
    int count[range];
    int output[len];
    for (int i = 0; i < range; i++) count[i] = 0;

    for (int i = 0; i < len; i++) {
        count[arr[i] - min_val]++;
    }

    for (int i = 1; i < range; i++) {
        count[i] += count[i - 1];
    }

    for (int i = len - 1; i >= 0; i--) {
        int idx = --count[arr[i] - min_val];
        output[idx] = arr[i];
    }

    for (int i = 0; i < len; i++) {
        arr[i] = output[i];
    }
}
```

## 参考阅读

- [x] [OI Wiki - 计数排序](https://oi-wiki.org/basic/counting-sort/) (2026-06-10)
- [x] [菜鸟教程 - 计数排序](https://www.runoob.com/w3cnote/counting-sort.html) (2026-06-10)
- [ ] [基数排序, 计数排序 (线性时间内)（视频）](https://www.youtube.com/watch?v=Nz1KZXbghj8&index=7&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [Counting sort in 3 minutes (video)](https://youtu.be/OKd534EWcdk)
