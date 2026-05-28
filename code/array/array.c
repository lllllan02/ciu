#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* ERROR_UNABLE_ALLOCATE_MEMORY = "Unable to allocate memory.";
const char* ERROR_OUT_OF_BOUNDS = "Index out of bounds.";

// ========== use function ==========

Array *new_array(int capacity) {
	// 申请地址
	Array *arr = malloc(sizeof(Array));
	if (arr == NULL) {
		exit_with_error(ERROR_UNABLE_ALLOCATE_MEMORY);
	}
	
	// 初始化
	arr->size = 0;
	arr->capacity = determine_capacity(capacity);
	arr->data = (int *) malloc(sizeof(int) * arr->capacity);
	if (arr->data == NULL) {
		free(arr);
		exit_with_error(ERROR_UNABLE_ALLOCATE_MEMORY);
	}

	return arr;
}

void destroy_array(Array *arr) {
	free(arr->data);
	free(arr);
}

int size(Array *arr) { return arr->size; }

int capacity(Array *arr) { return arr->capacity; }

bool is_empty(Array *arr) { return arr->size == 0; }

int at(Array *arr, int index) {
	// 检查索引是否合法
	validate_index(arr, index);
	return *(arr->data + index);
}

int find(Array *arr, int value) {
	for (int i = 0; i < arr->size; i++) {
		if (*(arr->data + i) == value) {
			return i;
		}
	}
	return -1;
}

void push(Array *arr, int item) {
	// 检查是否需要扩容
	resize(arr, arr->size + 1);

	*(arr->data + arr->size) = item;
	arr->size++;
}

void insert(Array *arr, int index, int value) {
	// 检查索引是否合法
	validate_index(arr, index);

	// 检查是否需要扩容
	resize(arr, arr->size + 1);

	// 后续元素向右移动
	memmove(arr->data + index + 1, arr->data + index, sizeof(int) * (arr->size - index));

	// 插入元素
	*(arr->data + index) = value;
	arr->size++;
}

void prepend(Array *arr, int value) {
	insert(arr, 0, value);
}

int pop(Array *arr) {
	// 检查是否还有元素
	if (is_empty(arr)) {
		exit_with_error(ERROR_OUT_OF_BOUNDS);
	}

	// 检查是否需要缩容
	resize(arr, arr->size - 1);

	// 缩减 size 并返回最有一个元素
	arr->size--;
	return *(arr->data + arr->size);;
}

void remove_at(Array *arr, int index) {
	// 检查所有是否合法
	validate_index(arr, index);

	// 检查是否需要缩容
	resize(arr, arr->size - 1);

	// 后续元素向左移动
	memmove(arr->data + index, arr->data + index + 1, sizeof(int) * (arr->size - index));
	
	// 缩减 size
	arr->size--;
}

void remove_value(Array *arr, int value) {
	for (int i = 0; i < arr->size; i++) {
		if (*(arr->data + i) == value) {
			remove_at(arr, i);
			i--;
		}
	}
}

void print_array(Array *arr) {
	printf("Size: %d\n", arr->size);
	printf("Capacity: %d\n", arr->capacity);
  
	printf("Items:\n");
	for (int i = 0; i < arr->size; ++i) {
		printf("%d: %d\n", i, *(arr->data + i));
	}
  
	printf("---------\n");  
}

// ========== tool function ==========

static int determine_capacity(int capacity) {
	// 只接受正整数
	if (capacity < 1) {
		exit(EXIT_FAILURE);
	}

	// 预申请容量为最小初始容量 16
	int true_capacity = MIN_CAPACITY;

	// 一直扩容到超过申请容量的两倍
	while (capacity * GROWTH_FACTOR > true_capacity) {
		true_capacity *= GROWTH_FACTOR;
	}

	return true_capacity;
}

static void exit_with_error(const char *msg) {
	printf("%s\n", msg);
	exit(EXIT_FAILURE);
}

static void validate_index(Array *arr, int index) {
	if (index < 0 || index >= arr->size) {
		exit_with_error(ERROR_OUT_OF_BOUNDS);
	}
}

static void resize(Array *arr, int size) {
	int old_capacity = arr->capacity;
	int new_capacity = old_capacity;

	// 检查是否需要调整大小
	if (size > arr->size) {
	// 添加元素并且 size == capacity 时进行扩容
		if (arr->size == arr->capacity) {
			new_capacity = arr->capacity * GROWTH_FACTOR;
		}
	} else if (size < arr->size) {
	// 删除元素并且 size < capacity / 4 时进行缩容
		if (arr->capacity > MIN_CAPACITY && size < arr->capacity / SHRINK_FACTOR) {
			new_capacity = arr->capacity / GROWTH_FACTOR;
		}
	}

	// 执行大小调整
	if (old_capacity != new_capacity) {
		int *new_data = (int *) realloc(arr->data, sizeof(int) * new_capacity);
		if (new_data == NULL) {
			exit_with_error(ERROR_UNABLE_ALLOCATE_MEMORY);
		}

		arr->data = new_data;
		arr->capacity = new_capacity;
	}
}