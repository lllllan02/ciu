---
title: 平衡二叉搜索树
order: 2
---

平衡二叉搜索树（Balanced Binary Search Tree）在 [二叉搜索树](/dsa/trees/binary-search-trees) 的 BST 性质之上，通过旋转操作约束树高，使查找、插入、删除的最坏时间复杂度稳定在 $O(\log n)$。

本实现采用 **AVL 树** 策略：每个节点维护 `height`，当左右子树高度差超过 1 时触发旋转修复。

**特性**：插入 / 删除 / 查找最坏 $O(\log n)$ · 空间 $O(n)$ · 需维护高度与旋转

## 为什么需要平衡树

普通 BST 在有序输入下会退化成链表，树高变为 $n$，所有操作最坏 $O(n)$。操作的代价与树高 $h$ 直接相关；**平衡树** 通过局部旋转把高度压在 $O(\log n)$，从而保证性能稳定。

## 平衡性的定义

AVL 要求：以 $T$ 为根的树，左右子树也都是 AVL 树，且左右子树高度差不超过 1。本实现用 **平衡因子** 表示这一差值：

$$
\text{bf}(v) = h(v.\text{left}) - h(v.\text{right})
$$

当 $|\text{bf}(v)| > 1$ 时触发旋转。空子树高度视为 0，单节点高度为 1。

## 旋转

二叉平衡树只有 **左旋** 和 **右旋** 两种调整操作，均 **不改变中序遍历序列**，因此保持 BST 性质。旋转可能改变子树根节点，调用方需接收返回的新根指针。

### 右旋（LL）

将 $A$ 的左孩子 $B$ 向右上提为子树根，$A$ 成为 $B$ 的右孩子，$B$ 原来的右子树 $T_2$ 挂到 $A$ 的左侧。

![右旋（LL 平衡旋转）](https://oi-wiki.org/ds/images/bst-rotate.svg)

### 左旋（RR）

与右旋镜像：将 $A$ 的右孩子 $B$ 向左上提为子树根，$A$ 成为 $B$ 的左孩子，$B$ 原来的左子树挂到 $A$ 的右侧。

### 四种失衡形态

| 类型 | 成因 | 调整 |
| :--- | :--- | :--- |
| **LL** | 左孩子的左子树过高 | 对 $T$ 右旋 |
| **RR** | 右孩子的右子树过高 | 对 $T$ 左旋 |
| **LR** | 左孩子的右子树过高 | 先对左孩子左旋，再对 $T$ 右旋 |
| **RL** | 右孩子的左子树过高 | 先对右孩子右旋，再对 $T$ 左旋 |

![LL 型](https://oi-wiki.org/ds/images/bst-LL.svg)

![RR 型](https://oi-wiki.org/ds/images/bst-RR.svg)

![LR 型](https://oi-wiki.org/ds/images/bst-LR.svg)

![RL 型](https://oi-wiki.org/ds/images/bst-RL.svg)

## [代码实现](https://github.com/lllllan02/ciu/tree/master/code/balanced_bst)

旋转与统一 rebalance 逻辑如下；插入、删除在递归返回路径上调用 `rebalance`：

```c
static TreeNode* rotate_left(TreeNode* root) {
    if (root->right == NULL) return root;
    TreeNode* new_root = root->right;
    root->right = new_root->left;
    new_root->left = root;
    update_height(root);
    update_height(new_root);
    return new_root;
}

static TreeNode* rotate_right(TreeNode* root) {
    if (root->left == NULL) return root;
    TreeNode* new_root = root->left;
    root->left = new_root->right;
    new_root->right = root;
    update_height(root);
    update_height(new_root);
    return new_root;
}

static TreeNode* rebalance(TreeNode* root) {
    update_height(root);
    int bf = balance_factor(root);
    if (bf > 1) {
        if (balance_factor(root->left) < 0) {  // LR
            root->left = rotate_left(root->left);
        }
        return rotate_right(root);             // LL
    } else if (bf < -1) {
        if (balance_factor(root->right) > 0) { // RL
            root->right = rotate_right(root->right);
        }
        return rotate_left(root);              // RR
    }
    return root;
}
```

## 复杂度分析

AVL 树高度 $h \le 1.44 \log_2(n + 2) - 0.328$，因此所有沿树高路径的操作均为 $O(\log n)$。

| 操作 | 时间复杂度 | 说明 |
| :--- | :--- | :--- |
| **插入** | $O(\log n)$ | 递归下降 + 至多一次双旋 |
| **删除** | $O(\log n)$ | 递归下降 + 回溯 rebalance |
| **查找** | $O(\log n)$ | 沿高度路径 |
| **空间复杂度** | $O(n)$ | 节点存储；递归栈 $O(\log n)$ |

## 参考阅读

- [x] [OI Wiki - 二叉搜索树 & 平衡树](https://oi-wiki.org/ds/bst/) (2026-06-05)
- [x] [二叉搜索树](/dsa/trees/binary-search-trees) (2026-06-05)
- 旋转图示来源：[OI Wiki - 二叉搜索树 & 平衡树](https://oi-wiki.org/ds/bst/)（CC BY-SA 4.0）
