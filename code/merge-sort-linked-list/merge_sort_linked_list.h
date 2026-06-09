#ifndef PROJECT_MERGE_SORT_LINKED_LIST_H
#define PROJECT_MERGE_SORT_LINKED_LIST_H

typedef struct Node {
    int data;
    struct Node *next;
} Node;

// 归并排序链表
Node* merge_sort(Node *head);

#endif