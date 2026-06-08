#include "bbst.h"
#include <assert.h>
#include <stdlib.h>

static int get_max(TreeNode* root) {
    assert(root != NULL);
    
    if (root->right == NULL) {
        return root->value;
    }
    return get_max(root->right);
}

static void update_height(TreeNode* root) {
    if (root == NULL) {
        return;
    }

    int lheight = root->left ? root->left->height : 0;
    int rheight = root->right ? root->right->height : 0;

    root->height = (lheight > rheight ? lheight : rheight) + 1;
}

static TreeNode* rotate_left(TreeNode* root) {
    if (root->right == NULL) {
        return root;
    }
    
    TreeNode* new_root = root->right;
    root->right = new_root->left;
    new_root->left = root;

    update_height(root);
    update_height(new_root);

    return new_root;
}

static TreeNode* rotate_right(TreeNode* root) {
    if (root->left == NULL) {
        return root;
    }

    TreeNode* new_root = root->left;
    root->left = new_root->right;
    new_root->right = root;

    update_height(root);
    update_height(new_root);

    return new_root;
}

static int balance_factor(TreeNode* node) {
    if (node == NULL) {
        return 0;
    }

    int lh = node->left ? node->left->height : 0;
    int rh = node->right ? node->right->height : 0;
    return lh - rh;
}

static TreeNode* rebalance(TreeNode* root) {
    update_height(root);

    int bf = balance_factor(root);
    if (bf > 1) {
        if (balance_factor(root->left) < 0) {  // LR
            root->left = rotate_left(root->left);
        }
        return rotate_right(root);
    } else if (bf < -1) {
        if (balance_factor(root->right) > 0) {  // RL
            root->right = rotate_right(root->right);
        }
        return rotate_left(root);
    }

    return root;
}

TreeNode* insert(TreeNode* root, const int value) {
    if (root == NULL) {
        root = malloc(sizeof(TreeNode));
        assert(root);

        root->value = value;
        root->height = 1;
        root->left = NULL;
        root->right = NULL;

        return root;
    }

    // 不重复添加
    if (root->value == value) {
        return root;
    }

    if (root->value > value) {
        root->left = insert(root->left, value);
    } else {
        root->right = insert(root->right, value);
    }

    // 平衡二叉树
    return rebalance(root);
}

TreeNode* delete_value(TreeNode* root, const int value) {
    if (root == NULL) {
        return NULL;
    }

    if (root->value > value) {
        // 1. 在左子树中删除目标值
        root->left = delete_value(root->left, value);
    } else if (root->value < value) {
        // 2. 在右子树中删除目标值
        root->right = delete_value(root->right, value);
    } else {
        // 3.0 当前节点就是目标值：

        if (root->left == NULL || root->right == NULL) {
            // 3.1 其中一个子树为空
            // 使用另一个子树，并释放当前节点
            TreeNode* new_root = root->left ? root->left : root->right;
            free(root);
            root = new_root;
        } else {
            // 3.2 左右子树都不为空
            // 将当前节点值设置为左子树中的最大值，并去左子树中删除该值
            root->value = get_max(root->left);
            root->left = delete_value(root->left, root->value);
        }
    }

    return rebalance(root);
}

void delete_tree(TreeNode* node) {
    if (node == NULL) {
        return;
    }

    delete_tree(node->left);
    delete_tree(node->right);
    free(node);
}
