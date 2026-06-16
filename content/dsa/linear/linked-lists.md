---
title: 链表
order: 2
---

链表（Linked List）由节点串成，每个节点存数据和指向下一个节点的指针；元素在内存中不必连续。本实现为 **单向链表**，维护 `head` 和 `tail` 指针，使头尾插入均为 $O(1)$。

**特性**：头尾插入 $O(1)$ · 按索引访问 $O(n)$ · 中间增删 $O(n)$ · 空间 $O(n)$

> 源码: https://github.com/lllllan02/ciu/tree/master/code/linked-list

## 实现要求

- **节点结构**：每个 `Node` 含 `data`（`int`）和 `next` 指针。
- **头尾指针**：`List` 维护 `head`、`tail` 和 `size`；空链表时 `head == tail == NULL`。
- **内存管理**：节点用 `malloc` 分配，删除时 `free`；销毁链表时逐个释放节点再释放 `List` 本身。
- **边界处理**：空链表上调用 `front`/`back`/`pop_front`/`pop_back` 报错退出；分配失败时先销毁已有结构再退出。
- **按值删除**：`remove_value` 需删除链表中 **所有** 等于给定值的节点，单次遍历完成。

## 数据结构

```c
typedef struct Node {
    int data;
    struct Node *next;
} Node;

typedef struct List {
    int size;
    Node *head, *tail;
} List;
```

## 关键实现

### 头尾插入

`push_front` 新建节点后直接设为 `head`；若链表原本为空，同时更新 `tail`：

```c
void push_front(List *list, int value) {
    Node *new_head = new_node(list, value);
    list->size++;
    list->head = new_head;
    if (list->tail == NULL) {
        list->tail = new_head;
    }
}
```

`push_back` 将新节点挂到 `tail->next`，再移动 `tail`；空链表时 `head` 和 `tail` 同时指向新节点。

### 尾部删除

`pop_back` 需要找到倒数第二个节点。单节点时直接释放并置空；多节点时从头遍历直到 `current->next == tail`，再释放 `tail` 并回退 `tail` 指针：

```c
if (list->head == list->tail) {
    free(list->head);
    list->head = list->tail = NULL;
} else {
    Node *current = list->head;
    while (current->next != list->tail) {
        current = current->next;
    }
    free(list->tail);
    list->tail = current;
    list->tail->next = NULL;
}
```

尾部删除因此是 $O(n)$；若需要 $O(1)$ 尾部删除，应改用双向链表。

### 按索引插入与删除

插入允许 `index == size`（追加到尾部）。找到 `index` 的前驱节点：从 `head` 出发走 `index - 1` 步（`index == 0` 时直接改 `head`），将新节点插入 `current` 与 `current->next` 之间；若新节点成为最后一个，更新 `tail`。

删除时 `index == 0` 单独处理（改 `head`，必要时清空 `tail`）；否则同样找前驱，跳过目标节点并释放，若删的是尾节点则回退 `tail`。

### 按值删除

维护 `prev` 和 `current` 双指针遍历：匹配时断开链接并释放，不移动 `prev`（可能连续多个相同值）；不匹配时 `prev` 前进一步。

### 反转

三指针迭代：`prev`、`current`、`next`。每次将 `current->next` 指向前驱，然后三个指针各前进一步。循环结束后 `head` 指向原尾节点（新的头），`tail` 指向原头节点。

```c
void reverse(List *list) {
    Node *prev = NULL, *current = list->head, *next;
    list->tail = list->head;

    while (current) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    list->head = prev;
}
```

## 复杂度分析

| 操作 | 时间复杂度 | 说明 |
| :--- | :--- | :--- |
| **按索引访问** | $O(n)$ | 从头遍历到目标位置 |
| **头尾插入** | $O(1)$ | 维护 `tail` 指针 |
| **头部删除** | $O(1)$ | 直接改 `head` |
| **尾部删除** | $O(n)$ | 需遍历找前驱 |
| **中间插入/删除** | $O(n)$ | 先遍历定位 |
| **反转** | $O(n)$ | 单次遍历 |
| **空间复杂度** | $O(n)$ | 每个节点额外存一个指针 |
