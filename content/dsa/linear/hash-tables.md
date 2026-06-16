---
title: 哈希表
order: 5
---

哈希表以 **键值对（Key-Value）** 存储数据，通过哈希函数将键映射到桶下标，理想情况下插入、查找、删除的平均时间均为 $O(1)$。本实现采用 **链式法（Separate Chaining）** 处理冲突：每个桶挂一条单链表，哈希到同一位置的键值对依次链接。

**特性**：平均 $O(1)$ 增删查 · 最坏 $O(n)$ · 固定桶数 · 字符串键

> 源码: https://github.com/lllllan02/ciu/tree/master/code/hash-table

## 实现要求

- **桶数组**：`data` 是 `KeyValue*` 指针数组，长度固定为创建时指定的 `size`，初始均为 `NULL`。
- **链式冲突**：同一桶内的多个键值对通过 `next` 串成单链表。
- **字符串键值**：键和值均为 C 字符串，插入时用 `strdup` 复制，表负责释放。
- **更新语义**：`add` 时若键已存在，释放旧值并写入新值，不新增节点。
- **头插法**：新键插入对应桶的链表头部，$O(1)$ 完成。
- **固定桶数**：不实现扩容与 rehash，便于专注理解链式结构本身。

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

## 关键实现

### 哈希函数

对字符串逐字符累乘 31 后取模，思路与 Java `String.hashCode` 类似：

$$
h(key) = |(\sum_i key[i] \times 31^i) \bmod m|
$$

```c
int hash(const char* key, const int m) {
    int h = 0;
    for (int i = 0; key[i] != '\0'; ++i) {
        h = h * 31 + key[i];
    }
    return abs(h % m);
}
```

`abs` 保证下标非负（C 语言 `%` 对负数结果实现相关）。

### 查找

先算桶下标，再沿该桶链表用 `strcmp` 逐节点比较键名，找到返回节点指针，否则返回 `NULL`。`get` 和 `exists` 都基于 `find` 实现。

### 插入与更新

```c
void add(HashTable* table, const char* key, const char* value) {
    KeyValue* kv = find(table, key);
    if (kv != NULL) {
        free(kv->value);
        kv->value = strdup(value);
        return;
    }

    int index = hash(key, table->size);
    kv = malloc(sizeof(KeyValue));
    kv->key = strdup(key);
    kv->value = strdup(value);
    kv->next = table->data[index];   // 头插
    table->data[index] = kv;
}
```

### 删除

在目标桶链表中遍历，维护前驱指针 `pre`：

- 删头节点：更新 `data[index] = kv->next`
- 删中间/尾节点：改 `pre->next = kv->next`

释放节点的 `key`、`value` 和节点本身。函数命名为 `remove_key`，避免与 C 标准库 `remove` 冲突。

### 销毁

逐桶遍历链表，释放每个节点的键、值和节点，最后释放桶数组和表结构。

## 冲突示意

```
桶[0] -> NULL
桶[1] -> [foo:1] -> [bar:2] -> NULL   // 多个键落在同一桶
桶[2] -> NULL
...
```

## 复杂度分析

设表中共 $n$ 个键值对，桶数为 $m$，单条链表平均长度 $n/m$。

| 操作 | 平均 | 最坏 | 说明 |
| :--- | :--- | :--- | :--- |
| **插入 / 更新** | $O(1)$ | $O(n)$ | 头插；最坏时所有键落在同一桶 |
| **查找** | $O(1)$ | $O(n)$ | 沿链表 `strcmp` 比较 |
| **删除** | $O(1)$ | $O(n)$ | 单次遍历目标桶链表 |
| **空间** | $O(n + m)$ | — | $n$ 个节点 + $m$ 个桶指针 |

负载因子 $\alpha = n/m$ 过大时链表变长，操作退化；生产环境通常在 $\alpha$ 超过阈值（如 0.75）时扩容并 rehash 所有键值对。
