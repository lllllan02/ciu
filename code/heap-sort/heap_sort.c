#include "heap_sort.h"
#include <stdlib.h>

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static int left(int i) {
    return i * 2 + 1;
}

static int right(int i) {
    return i * 2 + 2;
}

static int parent(int i) {
    return (i - 1) / 2;
}

static void heapify(int *arr, int start, int end) {
    while (start < end) {
        int down = start;
        
        int l = left(down), r = right(down);
        if (l <= end && arr[down] < arr[l]) {
            down = l;
        }
        if (r <= end && arr[down] < arr[r]) {
            down = r;
        }

        if (down == start) {
            break;
        }

        swap(arr + start, arr + down);
        start = down;
    }
}

void heap_sort(int *arr, int len) {
     // 初始化，从最后一个父节点开始调整
    int end = parent(len - 1);
    for (int i = end; i >= 0; i--) {
        heapify(arr, i, len - 1);
    }

    // 将确定的最大元素交换到末尾，然后重新调整 [0, i - 1] 的大顶堆
    for (int i = len - 1; i > 0; i--) {
        swap(arr, arr + i);
        heapify(arr, 0, i - 1);
    }
}