---
title: 桶排序
order: 9
---

## 定义

桶排序（Bucket Sort）是一种 **分布** 排序算法：将元素按规则散列到若干 **桶** 中，对每个桶分别排序，再按桶序拼接，得到有序序列。

与 [计数排序](/dsa/sorting/counting-sort) 为每个键值开单独计数不同，桶排序把 **一段连续值域** 映射到同一个桶，桶内再用比较排序（如插入排序）细排。适用于 **数据均匀分布在有限区间** 的场景；若大量元素落入同一桶，桶内排序会退化，最坏可达 $O(n^2)$。

桶排序是 **稳定** 的（取决于桶内排序是否稳定）。

## 过程

工作原理：先把值域切成若干 **桶**（每个桶对应一段连续的值），再把元素按值散入各桶，桶内排序后按桶序拼接。

**分桶规则**决定「某个值该进哪个桶」。以下示例设 10 个桶，每桶宽度为 10：值落在 $[0, 9]$ 进第 0 桶，$[10, 19]$ 进第 1 桶，……，$[90, 99]$ 及更大值进第 9 桶。例如 `25` 落在 $[20, 29]$，归入第 2 桶；`64` 落在 $[60, 69]$，归入第 6 桶。

| 桶编号 | 值域 |
| :--- | :--- |
| 0 | $[0, 9]$ |
| 1 | $[10, 19]$ |
| … | … |
| 9 | $[90, 99]$ 及更大值 |

规则确定后，分四步执行：

1. **统计**：第一遍扫描输入，按规则判断每个元素该进哪个桶，记录各桶元素个数。
2. **分桶**：按统计结果为各桶分配空间，第二遍扫描，将元素放入对应桶。
3. **桶内排序**：对每个非空桶单独排序（常用插入排序等稳定算法）。
4. **拼接**：按桶编号从小到大，依次取出各桶元素，写回有序序列。

以 `[64, 34, 25, 12, 22, 11, 90]` 为例：

| 阶段 | 桶 1 | 桶 2 | 桶 3 | 桶 6 | 桶 9 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| 分桶后 | `[12, 11]` | `[25, 22]` | `[34]` | `[64]` | `[90]` |
| 桶内排序后 | `[11, 12]` | `[22, 25]` | `[34]` | `[64]` | `[90]` |

按桶序拼接得 `[11, 12, 22, 25, 34, 64, 90]`。

## 性质

### 稳定性

桶排序是 **稳定** 排序算法。拼接时按桶序依次取出，不同桶之间不会交叉；桶内若使用稳定排序（如插入排序），相等键的相对顺序在桶内得以保持，整体因此稳定。

稳定性依赖桶内排序：若桶内改用不稳定算法（如快排），整体也不再稳定。

### 时间复杂度

| 情况 | 复杂度 | 说明 |
| :--- | :--- | :--- |
| **最好** | $O(n + k)$ | 元素均匀分散到 $k$ 个桶，每桶 $O(1)$ 个，桶内排序开销极小 |
| **最坏** | $O(n^2)$ | 全部元素落入同一桶，桶内插入排序退化为 $O(n^2)$ |
| **平均** | $O(n + k)$ | 数据均匀分布时，每桶约 $n/k$ 个，总开销近线性 |

$k$ 为桶的数量，即 `BUCKET_NUM`；分桶 $O(n)$，桶内排序合计取决于分布。

### 空间复杂度

$O(n)$。先统计各桶元素个数，再按精确大小分配，辅助空间总量与元素数同阶；另需 $O(k)$ 存放桶指针与计数。按惯例归入 **非原地**。

## 代码实现

> 源码: https://github.com/lllllan02/ciu/tree/master/code/bucket-sort

### 基础写法

本仓库实现。`BUCKET_NUM` 为文件级常量，先统计、精确分配，桶内插入排序后按序拼接：

```c
enum { BUCKET_NUM = 10 };

static void insertion_sort(int* arr, int len) {
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

static int bucket_index(int value) {
    int index = value / BUCKET_NUM;
    return index >= BUCKET_NUM ? BUCKET_NUM - 1 : index;
}

void bucket_sort(int* arr, int len) {
    if (len <= 0) return;

    int counts[BUCKET_NUM] = {0};

    for (int i = 0; i < len; i++) {
        counts[bucket_index(arr[i])]++;
    }

    int** buckets = malloc(BUCKET_NUM * sizeof(int*));
    int* pos = malloc(BUCKET_NUM * sizeof(int));
    for (int i = 0; i < BUCKET_NUM; i++) {
        buckets[i] = counts[i] ? malloc(counts[i] * sizeof(int)) : NULL;
        pos[i] = 0;
    }

    for (int i = 0; i < len; i++) {
        int index = bucket_index(arr[i]);
        buckets[index][pos[index]++] = arr[i];
    }

    int idx = 0;
    for (int i = 0; i < BUCKET_NUM; i++) {
        if (counts[i] == 0) continue;
        insertion_sort(buckets[i], counts[i]);
        for (int j = 0; j < counts[i]; j++) {
            arr[idx++] = buckets[i][j];
        }
        free(buckets[i]);
    }

    free(buckets);
    free(pos);
}
```

### 优化写法

**浮点数分桶**：将 $[min, max]$ 线性映射到桶下标，适用于均匀分布的浮点数据：

```c
static int float_bucket_index(float value, float min_val, float max_val, int bucket_num) {
    int index = (int)((value - min_val) / (max_val - min_val) * bucket_num);
    if (index >= bucket_num) index = bucket_num - 1;
    if (index < 0) index = 0;
    return index;
}
```

桶内元素较少时，也可将插入排序换为 [计数排序](/dsa/sorting/counting-sort)（键值范围已知且较小时更快）。

## 参考阅读

- [x] [OI Wiki - 桶排序](https://oi-wiki.org/basic/bucket-sort/) (2026-06-11)
- [x] [菜鸟教程 - 桶排序](https://www.runoob.com/w3cnote/bucket-sort.html) (2026-06-11)
- [ ] [Bucket sort in 4 minutes (video)](https://youtu.be/NAzOkAD0z3Q)
