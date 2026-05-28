#ifndef PROJECT_ARRAY_QUEUE_H
#define PROJECT_ARRAY_QUEUE_H

#include <stdbool.h>

typedef struct Queue {
    int *data;
    int head, tail, capcity;
} Queue;

// 创建一个指定容量的队列
Queue *create_queue(int capacity);

// 释放队列
void destory_queue(Queue *queue);

// 添加元素到队列末尾
void enqueue(Queue *queue, int value);

// 返回并删除队列的第一个元素
int dequeue(Queue *queue);

// 返回队列是否为空
bool is_empty(Queue *queue);

// 返回队列是否已满
bool is_full(Queue *queue);

#endif