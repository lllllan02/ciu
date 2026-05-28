#include "array_queue.h"
#include <stdlib.h>
#include <stdio.h>

const char* ERROR_UNABLE_ALLOCATE_MEMORY = "Unable to allocate memory.";
const char* ERROR_QUEUE_IS_FULL = "Cannot add item to a full queue.";
const char* ERROR_QUEUE_IS_EMPTY = "Queue is empty.";

static void exit_with_error(const char *msg) {
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
}

Queue *create_queue(int capacity) {
	// 申请队列的地址
	Queue *queue = (Queue *) malloc(sizeof(Queue));
	if (queue == NULL) {
		exit_with_error(ERROR_UNABLE_ALLOCATE_MEMORY);
	}

	// 申请队列中元素的具体存放地址
	int *data = (int *) malloc(sizeof(int) * capacity + 1);
	if (data == NULL) {
		free(queue);
		exit_with_error(ERROR_UNABLE_ALLOCATE_MEMORY);
	}

	// 初始化并返回
	queue->data = data;
	queue->capcity = capacity + 1; // capacity 为数组实际大小，是申请容量 + 1
	queue->head = queue->tail = 0;
	return queue;
}

void destory_queue(Queue *queue) {
	free(queue->data);
	free(queue);
}

void enqueue(Queue *queue, int value) {
	// 检查队列是否已满
	if (is_full(queue)) {
		destory_queue(queue);
		exit_with_error(ERROR_QUEUE_IS_FULL);
	}

	// 添加元素到末尾并后移队列的尾部索引
	queue->data[queue->tail] = value;
	queue->tail = (queue->tail + 1) % queue->capcity;
}

int dequeue(Queue *queue) {
	// 检查队列是否为空
	if (is_empty(queue)) {
		destory_queue(queue);
		exit_with_error(ERROR_QUEUE_IS_EMPTY);
	}

	// 后移队列的头部索引
	int value = queue->data[queue->head];
	queue->head = (queue->head + 1) % queue->capcity;
	return value;
}

bool is_empty(Queue *queue) { return queue->head == queue->tail; }

bool is_full(Queue *queue) { 
	return queue->head == (queue->tail + 1) % queue->capcity;
}