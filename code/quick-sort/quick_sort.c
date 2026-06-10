#include "quick_sort.h"
#include <assert.h>
#include <stdlib.h>

typedef struct Range {
    int start, end;
} Range;

static Range new_range(int start, int end) {
    Range range;
    range.start = start;
    range.end = end;
    return range;
}

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void quick_sort(int *arr, int len) {
    if (len <= 0) {
        return;
    }
    
    Range* ranges = malloc(len * sizeof(Range));
    assert(ranges);

    int index = 0;
    ranges[index++] = new_range(0, len - 1);
    while (index) {
        Range range = ranges[--index];
        if (range.start >= range.end) {
            continue;
        }

        int left = range.start, right = range.end;
        int mid = arr[(range.start + range.end) / 2];
        while (left <= right) {
            while (arr[left] < mid) left++;
            while (arr[right] > mid) right--;

            if (left <= right) {
                swap(arr + left, arr + right);
                left++, right--;
            }
        }

        if (left < range.end) ranges[index++] = new_range(left, range.end);
        if (right > range.start) ranges[index++] = new_range(range.start, right);
    }
}