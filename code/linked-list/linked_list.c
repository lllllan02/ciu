#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* ERROR_UNABLE_ALLOCATE_MEMORY = "Unable to allocate memory.";
const char* ERROR_OUT_OF_BOUNDS = "Index out of bounds.";
const char* ERROR_EMPTY_LIST = "Cannot get elem from empty list.";

static void exit_with_error(const char *msg) {
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
}

static void validate_index(List *list, int index) {
	if (index < 0 || index >= list->size) {
        destory_list(list);
		exit_with_error(ERROR_OUT_OF_BOUNDS);
	}
}

static Node *new_node(List *list, int value) {
    // 申请地址并初始化
    Node *node = (Node *) malloc(sizeof(Node));
    if (node == NULL) {
        destory_list(list);
        exit_with_error(ERROR_UNABLE_ALLOCATE_MEMORY);
    }
    node->data = value;
    node->next = NULL;

    return node;
}

List *new_list() {
    // 申请地址
    List *list = (List *) malloc(sizeof(List));
    if (list == NULL) {
        exit_with_error(ERROR_UNABLE_ALLOCATE_MEMORY);
    }

    // 初始化
    list->size = 0;
    list->head = list->tail = NULL;
    return list;
}

void destory_list(List *list) {
    // 逐个释放节点
    Node *node = list->head;
    for (; node != NULL;) {
        Node *next = node->next;
        free(node);
        node = next;
    }

    // 释放列表
    free(list);
}

int size(List *list) { return list->size; }

bool is_empty(List *list) { return list->size == 0; }

void push_front(List *list, int value) {
    // 申请地址并初始化
    Node *new_head = new_node(list, value);

    // 更新列表头部
    list->size++;
    list->head = new_head;
    if (list->tail == NULL) {
        list->tail = new_head;
    }
}

void push_back(List *list, int value) {
    // 申请地址并初始化
    Node *node = new_node(list, value);

    // 更新列表尾部
    list->size++;
    if (list->tail == NULL) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
}

int front(List *list) {
    // 检查列表头部不为空
    if (list->head == NULL) {
        exit_with_error(ERROR_EMPTY_LIST);
    }

    return list->head->data;
}

int back(List *list) {
    // 检查列表尾部不为空
    if (list->tail == NULL) {
        exit_with_error(ERROR_EMPTY_LIST);
    }

    return list->tail->data;
}

int pop_front(List *list) {
    // 检查列表头部不为空
    if (list->head == NULL) {
        exit_with_error(ERROR_EMPTY_LIST);
    }

    // 暂存头部和返回值
    Node *head = list->head;
    int value = head->data;

    // 更新列表头部
    list->size--;
    list->head = head->next;
    if (list->size == 0) {
        list->tail = NULL;
    }

    // 释放旧头部并返回
    free(head);
    return value;
}

int pop_back(List *list) {
    // 检查列表尾部不为空
    if (list->tail == NULL) {
        exit_with_error(ERROR_EMPTY_LIST);
    }
    
    int value = list->tail->data;
    
    // 如果只有一个节点
    if (list->head == list->tail) {
        free(list->head);
        list->head = list->tail = NULL;
        list->size = 0;
        return value;
    }
    
    // 遍历找到尾部的前一个节点
    Node *current = list->head;
    while (current->next != list->tail) {
        current = current->next;
    }

    // 更新列表尾部
    free(list->tail);
    list->tail = current;
    list->tail->next = NULL;
    list->size--;
    
    return value;
}

void insert(List *list, int index, int value) {
    // 检查索引是否合法
    if (index < 0 || index > list->size) {
        destory_list(list);
        exit_with_error(ERROR_OUT_OF_BOUNDS);
    }

    // 申请地址并初始化
    Node *node = new_node(list, value);

    list->size++;

    // 如果索引为零，将节点添加到头部
    if (index == 0) {
        node->next = list->head;
        list->head = node;
        if (list->tail == NULL) {
            list->tail = list->head;
        }
        return;
    }

    // 遍历列表找到索引处的前一个节点
    Node *current = list->head;
    for (; index > 1; index--) {
        current = current->next;
    }
    // 插入新节点
    node->next = current->next;
    current->next = node;

    if (node->next == NULL) {
        list->tail = node;
    }
}

int value_at(List *list, int index) {
    // 检查索引是否合法
    validate_index(list, index);

    // 遍历到索引处
    Node *current = list->head;
    for (; index > 0; index--) {
        current = current->next;
    }
    return current->data;
}

void remove_at(List *list, int index) {
    // 检查索引是否合法
    validate_index(list, index);

    list->size--;

    // 如果索引为零，直接删除头部
    if (index == 0) {
        Node *node = list->head;
        list->head = node->next;
        if (list->head == NULL) {
            list->tail = NULL;
        }
        free(node);
        return;
    }
    
    // 遍历列表找到索引处的前一个节点
    Node *current = list->head;
    for (; index > 1; index--) {
        current = current->next;
    }

    // 删除并释放节点
    Node *next = current->next;
    if (list->tail == next) {
        list->tail = current;
    }
    current->next = next->next;
    free(next);
}

void remove_value(List *list, int value) {
    Node *current = list->head;
    Node *prev = NULL;

    while (current) {
        Node *next = current->next;

        if (current->data == value) {
            list->size--;

            if (prev) {
                prev->next = next;
                if (list->tail == current) {
                    list->tail = prev;
                }
            } else {
                list->head = next;
                if (list->tail == current) {
                    list->tail = NULL;
                }
            }

            free(current);
            current = next;
        } else {
            prev = current;
            current = next;
        }
    }
}

void reverse(List *list) {
    Node *prev = NULL;
    Node *current = list->head;
    Node *next = NULL;

    list->tail = list->head;

    while (current) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    list->head = prev;
}