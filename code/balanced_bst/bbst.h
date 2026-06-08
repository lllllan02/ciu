#ifndef PROJECT_BBST_H
#define PROJECT_BBST_H

#include <stdbool.h>

typedef struct TreeNode {
    int value, height;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// 插入一个值到平衡二叉搜索树中
TreeNode* insert(TreeNode* root, const int value);

// 删除一个值从平衡二叉搜索树中
TreeNode* delete_value(TreeNode* root, const int value);

// 删除一棵平衡二叉搜索树
void delete_tree(TreeNode* root);

#endif