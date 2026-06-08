#include "test.h"
#include "max_heap.c"
#include "max_heap.h"
#include <assert.h>
#include <stdio.h>

static bool is_max_heap(MaxHeap* heap) {
    for (int i = 0; i < heap->size; i++) {
        int l = i * 2 + 1;
        int r = i * 2 + 2;
        if (l < heap->size && heap->data[i] < heap->data[l]) {
            return false;
        }
        if (r < heap->size && heap->data[i] < heap->data[r]) {
            return false;
        }
    }
    return true;
}

void run_all_tests() {
    printf("test_create_max_heap\n");
    test_create_max_heap();
    printf("test_push\n");
    test_push();
    printf("test_pop\n");
    test_pop();
    printf("test_top\n");
    test_top();
    printf("test_is_empty\n");
    test_is_empty();
    printf("test_is_full\n");
    test_is_full();
    printf("test_size\n");
    test_size();
    printf("test_heap_property\n");
    test_heap_property();
}

void test_create_max_heap() {
    MaxHeap* heap = create_max_heap(5);
    assert(heap != NULL);
    assert(heap->data != NULL);
    assert(heap->capacity == 5);
    assert(size(heap) == 0);
    assert(is_empty(heap) == true);
    assert(is_full(heap) == false);
    destroy_max_heap(heap);
}

void test_push() {
    MaxHeap* heap = create_max_heap(5);

    push(heap, 3);
    assert(top(heap) == 3);
    assert(is_max_heap(heap));

    push(heap, 7);
    assert(top(heap) == 7);
    assert(is_max_heap(heap));

    push(heap, 1);
    assert(top(heap) == 7);
    assert(is_max_heap(heap));

    push(heap, 9);
    assert(top(heap) == 9);
    assert(is_max_heap(heap));

    destroy_max_heap(heap);
}

void test_pop() {
    MaxHeap* heap = create_max_heap(5);
    int values[] = {3, 7, 1, 9, 4};
    for (int i = 0; i < 5; i++) {
        push(heap, values[i]);
    }

    assert(pop(heap) == 9);
    assert(pop(heap) == 7);
    assert(pop(heap) == 4);
    assert(pop(heap) == 3);
    assert(pop(heap) == 1);
    assert(is_empty(heap));

    destroy_max_heap(heap);
}

void test_top() {
    MaxHeap* heap = create_max_heap(3);

    push(heap, 10);
    assert(top(heap) == 10);

    push(heap, 5);
    assert(top(heap) == 10);

    push(heap, 20);
    assert(top(heap) == 20);

    destroy_max_heap(heap);
}

void test_is_empty() {
    MaxHeap* heap = create_max_heap(3);
    assert(is_empty(heap) == true);

    push(heap, 1);
    assert(is_empty(heap) == false);

    pop(heap);
    assert(is_empty(heap) == true);

    destroy_max_heap(heap);
}

void test_is_full() {
    MaxHeap* heap = create_max_heap(3);
    assert(is_full(heap) == false);

    push(heap, 1);
    assert(is_full(heap) == false);

    push(heap, 2);
    assert(is_full(heap) == false);

    push(heap, 3);
    assert(is_full(heap) == true);

    pop(heap);
    assert(is_full(heap) == false);

    destroy_max_heap(heap);
}

void test_size() {
    MaxHeap* heap = create_max_heap(5);
    assert(size(heap) == 0);

    push(heap, 1);
    push(heap, 2);
    push(heap, 3);
    assert(size(heap) == 3);

    pop(heap);
    assert(size(heap) == 2);

    destroy_max_heap(heap);
}

void test_heap_property() {
    MaxHeap* heap = create_max_heap(10);
    int values[] = {15, 10, 20, 8, 25, 16, 5, 30, 12, 3};

    for (int i = 0; i < 10; i++) {
        push(heap, values[i]);
        assert(is_max_heap(heap));
    }

    while (!is_empty(heap)) {
        pop(heap);
        assert(is_max_heap(heap));
    }

    destroy_max_heap(heap);
}
