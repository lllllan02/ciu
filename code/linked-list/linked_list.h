#ifndef PROJECT_LINKED_LIST_H
#define PROJECT_LINKED_LIST_H

#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

typedef struct List {
    int size;
    Node *head, *tail;
} List;

// 创建一个链表
List *new_list();

// 释放链表
void destory_list(List *list);

// 返回链表中的节点数量
int size(List *list);

// 如果链表为空，返回 true
bool is_empty(List *list);

// 添加元素到列表前端
void push_front(List *list, int value);

// 添加元素到列表后段
void push_back(List *list, int value);

// 返回列表前端的元素
int front(List *list);

// 返回列表后段的元素
int back(List *list);

// 返回并删除列表前端的元素
int pop_front(List *list);

// 返回并删除列表后段的元素
int pop_back(List *list);

// 在索引出添加元素
void insert(List *list, int index, int value);

// 返回索引处的元素
int value_at(List *list, int index);

// 删除索引处的元素
void remove_at(List *list, int index);

// 删除链表中所有值为 value 的元素
void remove_value(List *list, int value);

// 翻转链表
void reverse(List *list);

#endif