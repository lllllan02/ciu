#ifndef PROJECT_ARRAY_H
#define PROJECT_ARRAY_H

#include <stdbool.h>

const int MIN_CAPACITY = 16;    // 最小初始容量 16
const int GROWTH_FACTOR = 2;    // 扩容速度 2
const int SHRINK_FACTOR = 4;    // 缩容

typedef struct Array {
    int *data;
    int size;
    int capacity;
} Array;

// ========== use function ==========

// 创建一个新的动态数组，以容纳给定的初始容量。
Array *new_array(int capacity);

// 释放动态数组。
void destroy_array(Array *arr);

// 返回数组中管理的元素数量。
int size(Array *arr);

// 返回该数组实际能够容纳的容量。
int capacity(Array *arr);

// 如果数组为空，则返回 true。
bool is_empty(Array *arr);

// 返回存储在给定索引处的值
int at(Array *arr, int index);

// 返回给定值在数组中首次出现的索引，如果没有返回 -1。
int find(Array *arr, int value);

// 将给定的项追加到数组的末尾。
void push(Array *arr, int item);

// 在给定索引处插入给定值，将当前元素和后续元素向右移动。
void insert(Array *arr, int index, int value);

// 将给定值前置到数组开头，把后续元素向右移动。
void prepend(Array *arr, int value);

// 从数组中移除最后一个元素并返回其值。
int pop(Array *arr);

// 删除存储在给定索引处的元素，并将后续元素向左移动。
void remove_at(Array *arr, int index);

// 从数组中移除给定的值，即使该值出现多次。
void remove_value(Array *arr, int value);

// 出于调试目的，打印有关该数组的公开信息。
void print_array(Array *arr);

// ========== tool function ==========

// 确定为容纳给定容量所需的实际容量（就增长因子的幂而言）。
static int determine_capacity(int capacity);

// 报错并退出
static void exit_with_error(const char *msg);

// 校验索引在数组中是否合法。
static void validate_index(Array *arr, int index);

// 检查是否需要调整大小以支持候选大小，若需要则进行调整以容纳。
static void resize(Array *arr, int size);

#endif