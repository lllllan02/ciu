---
title: 二叉搜索树
order: 1
---

> 源码: https://github.com/lllllan02/ciu/tree/master/code/binary-search-tree

二叉搜索树（Binary Search Tree，BST）是一种二叉树，满足 **BST 性质**：对任意节点，左子树所有值 **小于** 该节点，右子树所有值 **大于** 该节点。在树较平衡时，查找、插入、删除的平均时间复杂度均为 $O(\log n)$；若插入顺序不当退化为链表，则最坏为 $O(n)$。C++ 的 `std::set` / `std::map`、Java 的 `TreeMap` 等有序容器都建立在这一思想之上（通常配合自平衡）。

本实现为 **普通 BST**（非平衡），节点动态分配，以递归为主，便于理解树的基本操作与遍历。

## 核心概念

### BST 性质

对树中每个节点 $v$：

$$
\forall x \in \text{left}(v),\ x < v.\text{value} \quad;\quad \forall y \in \text{right}(v),\ y > v.\text{value}
$$

中序遍历（左 → 根 → 右）输出 **升序** 序列，这是 BST 最常用的性质之一。

### 树的高度

本实现中 **空树高度为 0**，单节点高度为 1，递归定义为：

$$
h(\text{null}) = 0,\quad h(v) = \max(h(v.\text{left}),\ h(v.\text{right})) + 1
$$

### 删除的三种情况

删除值为 `value` 的节点时：

1. **叶子节点**：直接释放，父指针置 `NULL`。
2. **只有一个子节点**：用该子节点替换被删节点。
3. **有两个子节点**：用 **左子树的最大值**（前驱）替换当前节点值，再递归删除左子树中该前驱。

```
        5                    4
      /   \      删 5      /   \
     3     7    ----->    3     7
    / \   / \            / \   / \
   1  4  6  8           1     6  8
```

## 数据结构

```c
typedef struct BSTNode {
    int value;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;
```

- 无单独「树」结构体，以 `BSTNode*` 作为根指针；空树为 `NULL`。
- `insert`、`delete_value` 返回新的根指针（根被删或插入到空树时可能变化）。

## API

| 函数 | 说明 |
| :--- | :--- |
| `insert(node, value)` | 插入值，返回根；已存在则不变 |
| `delete_tree(node)` | 后序释放整棵树 |
| `delete_value(node, value)` | 删除指定值，返回新根 |
| `is_in_tree(node, value)` | 值是否存在 |
| `get_min(node)` / `get_max(node)` | 最小 / 最大值；空树返回 `-1` |
| `get_count(node)` | 节点个数 |
| `get_height(node)` | 树高；空树为 0 |
| `get_successor(node, value)` | 中序后继；不存在则 `-1` |
| `is_between(node, min, max)` | 所有节点是否满足 `min < value < max` |
| `pre_order_traversal` | 先序遍历（根 → 左 → 右），打印 |
| `in_order_traversal` | 中序遍历（左 → 根 → 右），打印 |
| `post_order_traversal` | 后序遍历（左 → 右 → 根），打印 |
| `level_order_traversal` | 层序遍历（BFS），打印 |

### 插入

沿 BST 性质递归：小于当前节点走左子树，大于走右子树，相等则忽略（不重复插入）。

### 查找

`is_in_tree` 递归比较当前节点与目标值；也可利用 BST 性质在 $O(h)$ 内单向下降（本实现采用递归全树比较，逻辑更直观）。

### 后继（Successor）

中序遍历中紧挨在 `value` **之后** 的值：

- 若 `value` 有右子树：后继为右子树最小值。
- 否则：沿根向下，记录最后一次「当前值大于 `value`」的节点。

### 层序遍历

用大小为 `get_count(node)` 的数组模拟队列，按层从左到右访问并打印。

## 复杂度分析

设树高为 $h$，节点数为 $n$。

| 操作 | 平均时间复杂度 | 最坏时间复杂度 | 说明 |
| :--- | :--- | :--- | :--- |
| **插入** | $O(\log n)$ | $O(n)$ | 沿高度路径递归 |
| **查找** | $O(\log n)$ | $O(n)$ | 与树高相关 |
| **删除** | $O(\log n)$ | $O(n)$ | 含找前驱 |
| **最小 / 最大** | $O(\log n)$ | $O(n)$ | 沿最左 / 最右链 |
| **后继** | $O(\log n)$ | $O(n)$ | 单次自顶向下 |
| **计数 / 高度** | $O(n)$ | $O(n)$ | 访问每个节点 |
| **遍历** | $O(n)$ | $O(n)$ | 每个节点一次 |
| **空间复杂度** | $O(n)$ | $O(n)$ | 节点存储；递归栈 $O(h)$ |

## 测试

```bash
cd code/binary-search-tree
gcc -Wall -Wextra -std=c11 -o main main.c
./main
```

测试覆盖：插入与计数、最值、高度、查找、`is_between`、删除（叶子 / 单子 / 双子 / 根）、后继、整树释放。
