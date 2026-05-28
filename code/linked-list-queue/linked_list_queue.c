#include "linked_list_queue.h"
#include <stdlib.h>
#include <stdio.h>

const char* ERROR_UNABLE_ALLOCATE_MEMORY = "Unable to allocate memory.";
const char* ERROR_EMPTY_Queue = "Cannot get elem from empty queue.";

static void exit_with_error(const char *msg) {
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
}

Queue *new_queue() {
	// 申请地址
	Queue *queue = (Queue *) malloc(sizeof(Queue));
	if (queue == NULL) {
		exit_with_error(ERROR_UNABLE_ALLOCATE_MEMORY);
	}

	// 初始化
	queue->head = queue->tail = NULL;
	return queue;
}

void destory_queue(Queue *queue) {
	// 逐个释放队列中的节点
	Node *node = queue->head;
	while (node) {
		Node *next = node->next;
		free(node);
		node = next;
	}

	// 释放队列
	free(queue);
}

void enqueue(Queue *queue, int value) {
	// 申请地址并初始化
	Node *node = (Node *) malloc(sizeof(Node));
	if (node == NULL) {
		destory_queue(queue);
		exit_with_error(ERROR_UNABLE_ALLOCATE_MEMORY);
	}
	node->data = value;
	node->next = NULL;

	// 添加节点到队列末尾
	if (queue->head == NULL) {
		queue->head = queue->tail = node;
	} else {
		queue->tail->next = node;
		queue->tail = node;
	}
}

int dequeue(Queue *queue) {
	// 检查队列是否为空
	if (queue->head == NULL) {
		destory_queue(queue);
		exit_with_error(ERROR_EMPTY_Queue);
	}

	// 获取队列中的第一个元素
	Node *head = queue->head;
	int value = head->data;

	// 替换队列头部
	queue->head = head->next;
	if (queue->head == NULL) {
		queue->tail = NULL;
	}

	// 释放地址并返回
	free(head);
	return value;
}

bool is_empty(Queue *queue) { return queue->head == NULL; }