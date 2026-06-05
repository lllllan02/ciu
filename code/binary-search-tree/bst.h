#ifndef PROJECT_BST_H
#define PROJECT_BST_H

#include <stdbool.h>

typedef struct BSTNode {
    int value;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

// 插入一个值到二叉搜索树中（返回值为根节点）
BSTNode* insert(BSTNode* node, const int value);

// 删除二叉搜索树中的所有节点
void delete_tree(BSTNode* node);

// 先序遍历打印二叉搜索树中的值
void pre_order_traversal(BSTNode* node);

// 中序遍历打印二叉搜索树中的值
void in_order_traversal(BSTNode* node);

// 后序遍历打印二叉搜索树中的值
void post_order_traversal(BSTNode* node);

// 层序遍历打印二叉搜索树中的值
void level_order_traversal(BSTNode* node);

// 返回二叉搜索树的高度
int get_height(BSTNode* node);

// 返回二叉搜索树中的节点数
int get_count(BSTNode* node);

// 返回二叉搜索树中的最小值
int get_min(BSTNode* node);

// 返回二叉搜索树中的最大值
int get_max(BSTNode* node);

// 返回给定树是否在给定最小值和最大值之间
bool is_between(BSTNode* node, int min, int max);

// 返回给定值是否在二叉搜索树中
bool is_in_tree(BSTNode* node, int value);

// 删除给定值从二叉搜索树中
BSTNode* delete_value(BSTNode* node, int value);

// 返回给定值在二叉搜索树中的后继者，若没有则返回-1
int get_successor(BSTNode* node, int value);

#endif  // PROJECT_BST_H