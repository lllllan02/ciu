---
title: 计数排序
order: 8
---

## 定义

计数排序（Counting Sort）是一种 **非比较** 排序算法：不通过元素间大小比较，而是统计每个键值出现次数，再按键值从小到大依次写回，得到有序序列。

适用于 **键值为整数且范围不大** 的场景。设 $n$ 为元素个数、$k$ 为键值范围（或上界与下界之差），时间复杂度为 $O(n + k)$。当 $k$ 与 $n$ 同阶甚至更大时，辅助空间与扫描开销会抵消优势，不如比较排序实用。

计数排序是 **稳定** 的，也是 [基数排序](/dsa/sorting#基数排序) 常用的子过程。

## 过程

工作原理分两步：**计数** 与 **回填**。

1. **计数**：遍历输入，令 `count[v]` 表示键值 `v` 的出现次数。
2. **回填**：按 `v` 从最小键到最大键递增，将 `v` 连续写入输出，每个 `v` 写 `count[v]` 次。
3. **结束**：输出填满，序列有序。

本实现假定键值落在 $[0, n)$，由调用方传入 `n` 作为 `count` 数组长度；键值即数组下标。

以 `[4, 2, 2, 8, 3, 3, 1]` 为例：

| 阶段 | 操作 | `count` 或结果 |
| :--- | :--- | :--- |
| 计数后 | 统计各键出现次数 | `count[1]=1, count[2]=2, count[3]=2, count[4]=1, count[8]=1` |
| 回填 `v=1` | 写 1 次 `1` | `[1, …]` |
| 回填 `v=2` | 写 2 次 `2` | `[1, 2, 2, …]` |
| 回填 `v=3` | 写 2 次 `3` | `[1, 2, 2, 3, 3, …]` |
| 回填 `v=4` | 写 1 次 `4` | `[1, 2, 2, 3, 3, 4, …]` |
| 回填 `v=8` | 写 1 次 `8` | `[1, 2, 2, 3, 3, 4, 8]` |

## 性质

### 稳定性

计数排序是 **稳定** 排序算法。回填阶段按键值从小到大扫描，同一键值的元素会 **连续、按计数顺序** 写出，相等键的相对顺序得以保持。

本实现直接覆盖原数组，对纯整数数组已稳定。若元素带有附属信息（如结构体中的 `key`），应使用 **前缀和 + 从右向左放置** 的写法，避免覆盖输入时打乱同键元素的先后。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n + k)$ | 计数 $O(n)$ + 回填至多扫 $k$ 个桶 |
| **最坏** | $O(n + k)$ | 与输入分布无关 |
| **平均** | $O(n + k)$ | 同上 |

$k$ 为键值范围，即参数 `n`。

### 空间复杂度

$O(k)$。需要长度为键值范围的 `count` 数组；本实现另在原数组上回填，不另开与 $n$ 等长的输出缓冲区。按惯例归入 **非原地**（辅助空间与 $k$ 相关）。

## 代码实现

> 源码: https://github.com/lllllan02/ciu/tree/master/code/counting-sort

### 基础写法

本仓库实现。键值范围 $[0, n)$ 由调用方指定，先计数再按桶序回填：

```c
void counting_sort(int* arr, int len, int n) {
    int count[n];
    for (int i = 0; i < n; i++) {
        count[i] = 0;
    }

    for (int i = 0; i < len; i++) {
        count[arr[i]] += 1;
    }

    int index = 0;
    for (int i = 0; i < n; i++) {
        while (count[i]--) {
            arr[index++] = i;
        }
    }
}
```

### 优化写法

**前缀和 + 辅助数组 + 逆序放置**，适用于任意有界整数范围，且对带附属信息的记录保证稳定：

```c
void counting_sort(int* arr, int* output, int len, int min_val, int max_val) {
    int range = max_val - min_val + 1;
    int count[range];
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
}
```

`min_val` / `max_val` 将键值平移到 $[0, \text{range})$，可处理含负数的数据；最后将 `output` 拷回 `arr` 即可。

## 参考阅读

- [x] [OI Wiki - 计数排序](https://oi-wiki.org/basic/counting-sort/) (2026-06-10)
- [x] [菜鸟教程 - 计数排序](https://www.runoob.com/w3cnote/counting-sort.html) (2026-06-10)
- [ ] [基数排序, 计数排序 (线性时间内)（视频）](https://www.youtube.com/watch?v=Nz1KZXbghj8&index=7&list=PLUl4u3cNGP61Oq3tWYp6V_F-5jb5L2iHb)
- [ ] [Counting sort in 3 minutes (video)](https://youtu.be/OKd534EWcdk)
