#include "test.h"
#include "array.c"
#include "array.h"
#include <assert.h>

void run_all_tests() {
	test_size_init();
	test_append();
	test_empty();
	test_resize();
	test_at();
	test_insert();
	test_prepend();
	test_pop();
	test_remove();
	test_find_exists();
	test_find_not_exists();
}

void test_size_init() {
	Array *arr = new_array(5);

	int initial_size = size(arr);
	assert(initial_size == 0);

	destroy_array(arr);
}

void test_append() {
	Array *arr = new_array(2);
	push(arr, 2);
	push(arr, 12);

	int new_size = size(arr);
	assert(new_size == 2);

	destroy_array(arr);
}

void test_resize() {
	Array *arr = new_array(2);

	int old_capacity = capacity(arr);
	assert(old_capacity == MIN_CAPACITY);

	// 添加元素到扩容
	for (int i = 0; i < 18; ++i) {
		push(arr, i + 1);
	}
	assert(capacity(arr) == 2 * MIN_CAPACITY);

	// 删除元素到缩容
	for (int j = 0; j < 15; ++j) {
		pop(arr);
	}
	assert(capacity(arr) == MIN_CAPACITY);

	destroy_array(arr);
}

void test_empty() {
	Array *arr = new_array(2);
	assert(is_empty(arr));

	push(arr, 1);
	assert(!is_empty(arr));

	destroy_array(arr);
}

void test_at() {
	Array *arr = new_array(12);
	
	for (int i = 0; i < 12; ++i) {
		push(arr, i + 3);
		assert(at(arr, i) == i + 3);
	}
	destroy_array(arr);
}

void test_insert() {
	Array *arr = new_array(5);

	for (int i = 0; i < 5; ++i) {
		push(arr, i + 5);
	}

	insert(arr, 2, 47);
	assert(at(arr, 2) == 47);
	assert(at(arr, 3) == 7);

	destroy_array(arr);
}

void test_prepend() {
	Array *arr = new_array(5);
	for (int i = 0; i < 3; ++i) {
		push(arr, i + 1);
	}

	prepend(arr, 15);
	assert(at(arr, 0) == 15);
	assert(at(arr, 1) == 1);

	destroy_array(arr);
}

void test_pop() {
	Array *arr = new_array(5);
	for (int i = 0; i < 3; ++i) {
		push(arr, i + 1);
	}
	assert(arr->size == 3);

	for (int j = 0; j < 3; ++j) {
		pop(arr);
	}
	assert(is_empty(arr));

	destroy_array(arr);
}

void test_remove() {
	Array *arr = new_array(5);

	push(arr, 12);
	push(arr, 3);
	push(arr, 41);
	push(arr, 12);
	push(arr, 12);
	remove_value(arr, 12);

	assert(size(arr) == 2);
	destroy_array(arr);
}

void test_find_exists() {
	Array *arr = new_array(5);
	push(arr, 1);
	push(arr, 2);
	push(arr, 3);
	push(arr, 4);
	push(arr, 5);
	assert(find(arr, 1) == 0);
	assert(find(arr, 4) == 3);
	assert(find(arr, 5) == 4);
	destroy_array(arr);
}

void test_find_not_exists() {
	Array *arr = new_array(3);
	push(arr, 1);
	push(arr, 2);
	push(arr, 3);

	assert(find(arr, 7) == -1);
	destroy_array(arr);
}