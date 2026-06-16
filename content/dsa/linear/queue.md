---
title: 队列
order: 4
---

队列（Queue）遵循 **先进先出（FIFO）** 原则：从队尾入队（enqueue），从队头出队（dequeue）。本仓库提供两种实现——**循环数组** 和 **链表**，均保证入队、出队为 $O(1)$。

**特性**：入队/出队 $O(1)$ · 空间 $O(n)$

> 源码: [数组实现](https://github.com/lllllan02/ciu/tree/master/code/array-queue)、[链表实现](https://github.com/lllllan02/ciu/tree/master/code/linked-list-queue)

## 实现要求

两种实现均需支持：

- **enqueue**：将元素加到队尾
- **dequeue**：移除并返回队头元素
- **is_empty**：判断队列是否为空

数组实现额外要求：

- **定容**：创建时指定最大元素个数，满时拒绝入队
- **is_full**：判断队列是否已满
- **循环缓冲区**：入队出队均 $O(1)$，无需搬移元素

## 数组实现：循环队列

### 数据结构

```c
typedef struct Queue {
    int *data;
    int head, tail, capcity;  // capcity 为数组实际长度
} Queue;
```

用户传入 `capacity` 表示最多存放的元素个数；内部数组长度为 `capacity + 1`，多出的一个槽位用于区分「空」与「满」。

### 空满判定

维护 `head`（队头，出队位置）和 `tail`（队尾，下一个入队位置）两个索引，到达数组末尾时用取模回到开头：

```
空：head == tail
满：(tail + 1) % capcity == head
```

浪费一个空间是经典做法：若不加这一格，`head == tail` 既可能是空队列，也可能是满队列，无法区分。

### 入队与出队

```c
void enqueue(Queue *queue, int value) {
    queue->data[queue->tail] = value;
    queue->tail = (queue->tail + 1) % queue->capcity;
}

int dequeue(Queue *queue) {
    int value = queue->data[queue->head];
    queue->head = (queue->head + 1) % queue->capcity;
    return value;
}
```

满队列入队、空队列出队时打印错误并销毁队列后退出。

## 链表实现

### 数据结构

```c
typedef struct Node {
    int data;
    struct Node *next;
} Node;

typedef struct Queue {
    Node *head, *tail;
} Queue;
```

`head` 指向队头（出队端），`tail` 指向队尾（入队端）。

### 入队与出队

入队时在尾部 `malloc` 新节点：空队列时 `head` 和 `tail` 都指向新节点；否则挂到 `tail->next` 并移动 `tail`。

出队时取 `head` 的值，将 `head` 后移；若出队后为空，同时将 `tail` 置 `NULL`，然后释放旧头节点。

链表实现没有「满」的概念（除非内存耗尽），也不需要取模或浪费空间，但每个元素多一个指针开销。

## 两种实现对比

| | 循环数组 | 链表 |
| :--- | :--- | :--- |
| **入队** | $O(1)$ | $O(1)$ |
| **出队** | $O(1)$ | $O(1)$ |
| **容量** | 创建时固定 | 按需增长 |
| **额外空间** | 浪费 1 个槽位 | 每节点一个指针 |
| **缓存友好性** | 连续内存，更好 | 节点分散，较差 |

实际选型时：已知上限、追求性能用循环数组；元素数量不确定、不想处理扩容逻辑用链表。
