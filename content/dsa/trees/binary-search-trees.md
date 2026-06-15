---
title: 二叉搜索树
order: 1
---

二叉搜索树（Binary Search Tree，BST）是一种二叉树，满足 **BST 性质**：对任意节点，左子树所有值 **小于** 该节点，右子树所有值 **大于** 该节点。在树较平衡时，查找、插入、删除的平均时间复杂度均为 $O(\log n)$；若插入顺序不当退化为链表，则最坏为 $O(n)$。C++ 的 `std::set` / `std::map`、Java 的 `TreeMap` 等有序容器都建立在这一思想之上（通常配合自平衡）。

本实现为 **普通 BST**（非平衡），节点动态分配，以递归为主，便于理解树的基本操作与遍历。

**特性**：插入 / 查找 / 删除平均 $O(\log n)$ · 最坏 $O(n)$ · 空间 $O(n)$ · 非平衡

## BST 性质

对树中每个节点，左子树键值均小于当前节点，右子树键值均大于当前节点。**中序遍历**（左 → 根 → 右）输出 **升序** 序列，这是 BST 最常用的性质之一。

## 树的高度

本实现中 **空树高度为 0**，单节点高度为 1，递归定义为子树较高一侧的高度加 1。

## 删除的三种情况

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

## [代码实现](https://github.com/lllllan02/ciu/tree/master/code/binary-search-tree)

插入沿 BST 性质递归下降；删除在定位目标后按上述三种情况处理：

```c
BSTNode* insert(BSTNode* node, const int value) {
    if (node == NULL) {
        node = malloc(sizeof(BSTNode));
        node->value = value;
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    if (node->value > value) {
        node->left = insert(node->left, value);
    } else if (node->value < value) {
        node->right = insert(node->right, value);
    }
    return node;
}

BSTNode* delete_value(BSTNode* node, int value) {
    if (node == NULL) return NULL;
    if (node->value > value) {
        node->left = delete_value(node->left, value);
    } else if (node->value < value) {
        node->right = delete_value(node->right, value);
    } else {
        if (node->left == NULL || node->right == NULL) {
            BSTNode* child = node->left ? node->left : node->right;
            free(node);
            return child;
        }
        node->value = get_max(node->left);
        node->left = delete_value(node->left, node->value);
    }
    return node;
}
```

无单独「树」结构体，以 `BSTNode*` 作为根指针；`insert`、`delete_value` 返回新的根指针（根被删或插入到空树时可能变化）。

## 复杂度分析

设树高为 $h$，节点数为 $n$。普通 BST 的性能与树形密切相关：随机数据下 $h \approx O(\log n)$，有序插入则 $h = n$。

| 操作 | 平均时间复杂度 | 最坏时间复杂度 | 说明 |
| :--- | :--- | :--- | :--- |
| **插入** | $O(\log n)$ | $O(n)$ | 沿高度路径递归 |
| **查找** | $O(\log n)$ | $O(n)$ | 与树高相关 |
| **删除** | $O(\log n)$ | $O(n)$ | 含找前驱 |
| **空间复杂度** | $O(n)$ | $O(n)$ | 节点存储；递归栈 $O(h)$ |

有序输入下最坏 $O(n)$ 正是 [平衡二叉搜索树](/dsa/trees/balanced-binary-search-trees) 要解决的问题。

## 参考阅读

- [x] [OI Wiki - 二叉搜索树 & 平衡树](https://oi-wiki.org/ds/bst/) (2026-06-05)
- [x] [平衡二叉搜索树](/dsa/trees/balanced-binary-search-trees) (2026-06-08)
