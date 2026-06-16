---
title: 动态数组
order: 1
---

动态数组（Dynamic Array）在内存中占据连续空间，支持 $O(1)$ 随机访问；当元素数量超出容量时自动扩容，删除过多时按需缩容。本实现用原生指针管理 `int` 数组，刻意使用指针算术而非下标访问，以熟悉 C 语言内存模型。

**特性**：随机访问 $O(1)$ · 末尾增删平摊 $O(1)$ · 中间增删 $O(n)$ · 空间 $O(n)$

> 源码: https://github.com/lllllan02/ciu/tree/master/code/array

## 实现要求

- **连续内存**：`data` 指向一块 `int` 数组，`size` 记录当前元素个数，`capacity` 记录已分配容量。
- **自动扩容**：`size == capacity` 时追加元素，容量翻倍（`× 2`）。
- **自动缩容**：删除后 `size < capacity / 4` 且容量大于最小值（16）时，容量减半（`÷ 2`），避免频繁抖动。
- **初始容量**：向上取整到不小于 `2 × 期望容量` 的最小值，形式为 `16 × 2^k`（16、32、64…）。
- **指针访问**：读写元素用 `*(data + i)`，不用 `data[i]`。
- **边界处理**：非法索引或空数组 `pop` 时打印错误并退出。

## 数据结构

```c
const int MIN_CAPACITY = 16;    // 最小初始容量
const int GROWTH_FACTOR = 2;    // 扩容倍数
const int SHRINK_FACTOR = 4;    // 缩容阈值：size < capacity / 4 时减半

typedef struct Array {
    int *data;
    int size;
    int capacity;
} Array;
```

## 关键实现

### 容量计算

`determine_capacity` 将用户期望容量向上取整：从最小容量 16 开始，若仍小于 `2 × 用户容量` 则不断翻倍，直到 `实际容量 ≥ 2 × 用户容量`。结果始终是 16 的 2 的幂次倍（16、32、64…）：

```c
static int determine_capacity(int capacity) {
    int true_capacity = MIN_CAPACITY;
    while (capacity * GROWTH_FACTOR > true_capacity) {
        true_capacity *= GROWTH_FACTOR;
    }
    return true_capacity;
}
```

### 扩缩容

`resize` 在每次增删前被调用，传入操作完成后的目标 `size`：

- 若 `size > arr->size` 且 `arr->size == arr->capacity` → 容量 `× 2`
- 若 `size < arr->size` 且 `size < capacity / 4` 且 `capacity > MIN_CAPACITY` → 容量 `÷ 2`
- 实际调整用 `realloc` 原地扩展或收缩

```c
static void resize(Array *arr, int size) {
    int new_capacity = arr->capacity;

    if (size > arr->size && arr->size == arr->capacity) {
        new_capacity = arr->capacity * GROWTH_FACTOR;
    } else if (size < arr->size
            && arr->capacity > MIN_CAPACITY
            && size < arr->capacity / SHRINK_FACTOR) {
        new_capacity = arr->capacity / GROWTH_FACTOR;
    }

    if (new_capacity != arr->capacity) {
        arr->data = realloc(arr->data, sizeof(int) * new_capacity);
        arr->capacity = new_capacity;
    }
}
```

### 中间插入与删除

中间位置操作需要移动后续元素，用 `memmove` 完成块拷贝：

```c
// 在 index 处插入：先扩容，右移 [index, size)，再写入
memmove(arr->data + index + 1, arr->data + index,
        sizeof(int) * (arr->size - index));
*(arr->data + index) = value;

// 删除 index 处：左移 [index+1, size)，再缩容
memmove(arr->data + index, arr->data + index + 1,
        sizeof(int) * (arr->size - index - 1));
```

`prepend` 等价于 `insert(arr, 0, value)`；`remove_value` 遍历数组，每找到一次目标值就调用 `remove_at` 并回退索引，以删除所有匹配项。

## 复杂度分析

| 操作 | 时间复杂度 | 说明 |
| :--- | :--- | :--- |
| **随机访问** | $O(1)$ | 指针偏移直接定位 |
| **末尾插入/删除** | $O(1)$ 平摊 | 扩容/缩容时为 $O(n)$，平摊后均摊 $O(1)$ |
| **中间插入/删除** | $O(n)$ | 需移动后续所有元素 |
| **查找** | $O(n)$ | 线性扫描 |
| **空间复杂度** | $O(n)$ | 预留空间通常不超过 $2n$ |
