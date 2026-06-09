---
title: 哈希表
order: 5
---

> 源码: https://github.com/lllllan02/ciu/tree/master/code/hash-table

哈希表（Hash Table）是一种以 **键值对（Key-Value）** 存储数据的数据结构。通过 **哈希函数** 将键映射到数组下标（桶），在理想情况下，插入、查找、删除的平均时间复杂度均为 $O(1)$。Python 的 `dict`、Java 的 `HashMap`、缓存与计数表等都建立在这一思想之上。

本实现采用 **链式法（Separate Chaining）** 处理冲突：每个桶维护一条链表，哈希到同一位置的多个键值对依次挂在链表上。

## 核心概念

### 哈希函数

将任意长度的键 `key` 映射到 `[0, m)` 范围内的整数，其中 `m` 为桶的数量（表大小）：

$$
h(key) = |(\sum_i key[i] \times 31^i) \bmod m|
$$

实现上对字符串逐字符累乘 31 后取模，与 Java `String.hashCode` 的思路类似。

### 冲突与链式法

不同键可能算出相同下标（**哈希冲突**）。链式法不在表内探测空位，而是在该桶的链表头部插入新节点；查找、删除时沿链表比较键名。

```
桶[0] -> NULL
桶[1] -> [foo:1] -> [bar:2] -> NULL   // 多个键落在同一桶
桶[2] -> NULL
...
```

### 负载因子

**负载因子** $\alpha = n / m$（$n$ 为元素个数，$m$ 为桶数）。$\alpha$ 过大时链表变长，操作退化接近 $O(n)$。生产环境通常在 $\alpha$ 超过阈值时 **扩容并 rehash**；当前实现为固定桶数，便于理解链式结构本身。

## 数据结构

```c
typedef struct KeyValue {
    char* key;
    char* value;
    struct KeyValue* next;
} KeyValue;

typedef struct HashTable {
    KeyValue** data;  // 桶数组，每个元素指向链表头
    int size;         // 桶的数量 m
} HashTable;
```

- `data`：`KeyValue*` 指针数组，长度为 `size`，初始均为 `NULL`。
- 每个 `KeyValue` 通过 `next` 构成单链表；键、值均使用 `strdup` 复制，表负责释放。

## API

| 函数 | 说明 |
| :--- | :--- |
| `create_table(size)` | 创建含 `size` 个桶的哈希表 |
| `destroy_table(table)` | 释放所有键值对与桶数组 |
| `hash(key, m)` | 计算键在模 `m` 下的哈希下标 |
| `find(table, key)` | 返回键对应的 `KeyValue*`，不存在则 `NULL` |
| `get(table, key)` | 返回值字符串，不存在则 `NULL` |
| `exists(table, key)` | 键是否存在 |
| `add(table, key, value)` | 插入；键已存在则 **更新** 值 |
| `remove_key(table, key)` | 删除指定键（命名为 `remove_key` 以避免与 C 标准库 `remove` 冲突） |

### 插入与更新

`add` 先 `find`：若键已存在，释放旧值并 `strdup` 新值；否则在对应桶的 **链表头** 插入新节点（头插 $O(1)$）。

### 删除

在目标桶的链表中遍历，维护前驱指针 `pre`：删除头节点时更新 `data[index]`，否则修改 `pre->next`，释放节点后返回。

## 复杂度分析

设表中共 $n$ 个键值对，桶数为 $m$，单条链表平均长度为 $n/m$。

| 操作 | 平均时间复杂度 | 最坏时间复杂度 | 说明 |
| :--- | :--- | :--- | :--- |
| **插入 / 更新** | $O(1)$ | $O(n)$ | 头插；最坏时所有键落在同一桶 |
| **查找** | $O(1)$ | $O(n)$ | 沿链表 `strcmp` 比较 |
| **删除** | $O(1)$ | $O(n)$ | 单次遍历目标桶链表 |
| **空间复杂度** | $O(n + m)$ | — | $n$ 个节点 + $m$ 个桶指针 |

## 测试

```bash
cd code/hash-table
gcc -Wall -Wextra -std=c11 -o main main.c
./main
```

测试覆盖：建表、哈希函数、增删查、键更新、以及 `size=1` 时的冲突与链上删除。
