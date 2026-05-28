#ifndef PROJECT_LINKED_LIST_QUEUE_H
#define PROJECT_LINKED_LIST_QUEUE_H

#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

typedef struct Queue {
    Node *head, *tail;
} Queue;

// 创建一个队列
Queue *new_queue();

// 释放一个队列
void destory_queue(Queue *queue);

// 添加元素到队列末尾
void enqueue(Queue *queue, int value);

// 返回并删除队列头部元素
int dequeue(Queue *queue);

// 返回队列是否为空
bool is_empty(Queue *queue);

#endif