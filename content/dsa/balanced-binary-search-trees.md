---
title: 平衡二叉搜索树
---

> 源码: https://github.com/lllllan02/ciu/tree/master/code/balanced_bst
>
> 参考: [OI Wiki - 二叉搜索树 & 平衡树](https://oi-wiki.org/ds/bst/)

平衡二叉搜索树（Balanced Binary Search Tree）在 [二叉搜索树](/dsa/binary-search-trees) 的 BST 性质之上，通过旋转操作约束树高，使查找、插入、删除的最坏时间复杂度稳定在 $O(\log n)$。

本实现采用 **AVL 树** 策略：每个节点维护 `height`，当左右子树高度差超过 1 时触发旋转修复。

## 为什么需要平衡树

使用搜索树的目的之一是缩短插入、删除和查找节点的时间。若树高为 $h$，最坏情况下查找需要比较 $h$ 次，时间复杂度不超过 $O(h)$；理想情况下所有操作均可达到 $O(\log n)$。

然而 $O(\log n)$ 只是理想情况。普通 BST 在最坏情况下会退化为链表——例如每个节点只有右孩子，所有操作的时间均为 $O(n)$。操作的复杂度与树高 $h$ 直接相关，因此需要通过 **平衡树** 维持树的高度。

## 平衡性的定义

不同平衡树对「平衡」的定义各不相同。本实现采用 AVL 树的定义：以 $T$ 为根节点的树，当且仅当左右子树也是 AVL 树，且：

$$
|h(T.\text{left}) - h(T.\text{right})| \le 1
$$

其中 $h$ 为子树高度。本实现用 **平衡因子** 表示这一差值：

$$
\text{bf}(v) = h(v.\text{left}) - h(v.\text{right})
$$

AVL 要求每个节点 $|\text{bf}(v)| \le 1$；当 $|\text{bf}| > 1$ 时触发旋转。

## 平衡的调整过程

对不满足平衡条件的搜索树进行调整，可以使其重新具有平衡性。二叉平衡树的调整操作只有 **左旋（Left Rotate / zag）** 和 **右旋（Right Rotate / zig）** 两种——两者均 **不改变中序遍历序列**，从而保持 BST 性质。

### 右旋

右旋也称为「右单旋转」或「LL 平衡旋转」。对于结点 $A$ 的右旋操作是指：将 $A$ 的左孩子 $B$ 向右上旋转，代替 $A$ 成为根节点；$A$ 向右下旋转成为 $B$ 的右子树的根结点；$B$ 原来的右子树 $T_2$ 变为 $A$ 的左子树。

![右旋（LL 平衡旋转）](https://oi-wiki.org/ds/images/bst-rotate.svg)

右旋只改变了三组结点关联，相当于对三组边进行循环置换，因此需要暂存一个结点再轮换更新。一般的更新顺序是：暂存 $B$（新根），让 $A$ 的左孩子指向 $B$ 的右子树 $T_2$，再让 $B$ 的右孩子指针指向 $A$，最后让 $A$ 的父结点指向暂存的 $B$。

### 左旋

左旋也称为「左单旋转」或「RR 平衡旋转」，与右旋互为镜像：将 $A$ 的右孩子 $B$ 向左上旋转成为新根，$A$ 成为 $B$ 的左子树的根，$B$ 原来的左子树变为 $A$ 的右子树。

本实现的旋转代码如下（调用方需接收返回的新根指针，因为旋转可能改变子树的根）：

```c
static TreeNode* rotate_left(TreeNode* root) {
    TreeNode* new_root = root->right;
    root->right = new_root->left;
    new_root->left = root;
    update_height(root);
    update_height(new_root);
    return new_root;
}

static TreeNode* rotate_right(TreeNode* root) {
    TreeNode* new_root = root->left;
    root->left = new_root->right;
    new_root->right = root;
    update_height(root);
    update_height(new_root);
    return new_root;
}
```

### 四种平衡性破坏的情况

虽然不同二叉平衡树的节点维护信息不同，但平衡性被破坏的情况只有以下四种，调整操作均只包括左旋和右旋。

**LL 型**：$T$ 的左孩子的左子树过长导致平衡性破坏。

调整方式：右旋节点 $T$。

![LL 型](https://oi-wiki.org/ds/images/bst-LL.svg)

**RR 型**：$T$ 的右孩子的右子树过长导致平衡性破坏。

调整方式：左旋节点 $T$。

![RR 型](https://oi-wiki.org/ds/images/bst-RR.svg)

**LR 型**：$T$ 的左孩子的右子树过长导致平衡性破坏。

调整方式：先左旋节点 $L$，成为 LL 型，再右旋节点 $T$。

![LR 型](https://oi-wiki.org/ds/images/bst-LR.svg)

**RL 型**：$T$ 的右孩子的左子树过长导致平衡性破坏。

调整方式：先右旋节点 $R$，成为 RR 型，再左旋节点 $T$。

![RL 型](https://oi-wiki.org/ds/images/bst-RL.svg)

本实现在 `rebalance` 中统一处理上述四种情况：

```c
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

## 数据结构

```c
typedef struct TreeNode {
    int value, height;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;
```

- 相比普通 BST，每个节点额外存储 `height`；空子树高度视为 0，单节点高度为 1。
- 无单独「树」结构体，以 `TreeNode*` 作为根指针；空树为 `NULL`。
- `insert`、`delete_value` 返回新的根指针（旋转可能改变根）。

## API

| 函数 | 说明 |
| :--- | :--- |
| `insert(root, value)` | 插入值并 rebalance，返回新根；已存在则不变 |
| `delete_value(root, value)` | 删除指定值并 rebalance，返回新根 |
| `delete_tree(root)` | 后序释放整棵树 |

### 插入

1. 按 BST 性质递归插入到叶子位置。
2. 回溯时 `update_height` 更新当前节点高度。
3. 若 `|balance_factor| > 1`，执行对应旋转后返回新根。

### 删除

删除逻辑与普通 BST 相同（叶子 / 单子 / 双子三种情况，双子节点用左子树最大值替换），区别在于每次递归返回后调用 `rebalance` 恢复平衡。详见 [二叉搜索树 - 删除的三种情况](/dsa/binary-search-trees#删除的三种情况)。

## 复杂度分析

AVL 树高度 $h \le 1.44 \log_2(n + 2) - 0.328$，因此所有沿树高路径的操作均为 $O(\log n)$。

| 操作 | 时间复杂度 | 说明 |
| :--- | :--- | :--- |
| **插入** | $O(\log n)$ | 递归下降 + 至多一次双旋 |
| **删除** | $O(\log n)$ | 递归下降 + 回溯 rebalance |
| **查找** | $O(\log n)$ | 沿高度路径（本实现未暴露查找 API） |
| **空间复杂度** | $O(n)$ | 节点存储；递归栈 $O(\log n)$ |

## 与普通 BST 的对比

| | 普通 BST | AVL 树（本实现） |
| :--- | :--- | :--- |
| 最坏插入 | $O(n)$（有序输入退化为链表） | $O(\log n)$ |
| 节点开销 | 值 + 左右指针 | 额外 `height` 字段 |
| 实现复杂度 | 低 | 需维护高度与旋转 |
| 适用场景 | 学习、随机数据 | 需要稳定性能的有序容器 |

## 测试

```bash
cd code/balanced_bst
gcc -Wall -Wextra -std=c11 -o main main.c
./main
```

测试覆盖：插入与重复值、删除（叶子 / 单子 / 双子 / 根）、升序 / 降序插入的平衡性、LR / RL 双旋场景、整树释放。测试中会校验 BST 性质、平衡因子以及 `height` 字段的正确性。

---

> 旋转图示来源：[OI Wiki - 二叉搜索树 & 平衡树](https://oi-wiki.org/ds/bst/)（CC BY-SA 4.0）
