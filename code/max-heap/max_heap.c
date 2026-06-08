#include "max_heap.h"
#include <stdlib.h>
#include <assert.h>

static int left(int i) {
    return i * 2 + 1;
}

static int right(int i) {
    return i * 2 + 2;
}

static int parent(int i) {
    return (i - 1) / 2;
}

static void swap(MaxHeap* h, int a, int b) {
    int tmp = h->data[a];
    h->data[a] = h->data[b];
    h->data[b] = tmp;
}

static void sift_up(MaxHeap* h, int i) {
    while (i > 0) {
        int p = parent(i);
        if (h->data[i] <= h->data[p]) {
            break;
        }
        swap(h, i, p);
        i = p;
    }
}

static void sift_down(MaxHeap* h, int i) {
    while (true) {
        int l = left(i), r = right(i), down = i;

        if (l < h->size && h->data[l] > h->data[down]) down = l;
        if (r < h->size && h->data[r] > h->data[down]) down = r;
        if (down == i) break;

        swap(h, i, down);
        i = down;
    }
}

MaxHeap* create_max_heap(int capacity) {
    MaxHeap* h = malloc(sizeof(MaxHeap));
    assert(h);

    h->size = 0;
    h->capacity = capacity;
    h->data = malloc(capacity * sizeof(int));
    assert(h->data);

    return h;
}

void destroy_max_heap(MaxHeap* heap) {
    free(heap->data);
    free(heap);
}

void push(MaxHeap* heap, int value) {
    assert(!is_full(heap));

    heap->data[heap->size++] = value;
    sift_up(heap, heap->size - 1);
}

int pop(MaxHeap* heap) {
    int t = top(heap);

    // 将最后一个元素置于栈顶然后下推
    heap->data[0] = heap->data[--heap->size];
    sift_down(heap, 0);

    return t;
}

int top(MaxHeap* heap) {
    assert(!is_empty(heap));

    return heap->data[0];
}

int size(MaxHeap* heap) {
    assert(heap);
    return heap->size;
}

bool is_empty(MaxHeap* heap) {
    assert(heap);
    return heap->size == 0;
}

bool is_full(MaxHeap* heap) {
    assert(heap);
    return heap->size >= heap->capacity;
}