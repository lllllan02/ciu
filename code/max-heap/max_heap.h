#ifndef PROJECT_MAX_HEAP_H
#define PROJECT_MAX_HEAP_H

#include <stdbool.h>

typedef struct MaxHeap {
    int* data;
    int size;
    int capacity;
} MaxHeap;

// 创建一个最大堆
MaxHeap* create_max_heap(int capacity);

// 释放一个最大堆
void destroy_max_heap(MaxHeap* heap);

// 添加一个元素到最大堆
void push(MaxHeap* heap, int value);

// 删除一个元素从最大堆
int pop(MaxHeap* heap);

// 返回最大堆的顶部元素
int top(MaxHeap* heap);

// 返回最大堆的大小
int size(MaxHeap* heap);

// 返回最大堆是否为空
bool is_empty(MaxHeap* heap);

// 返回最大堆是否已满
bool is_full(MaxHeap* heap);

#endif