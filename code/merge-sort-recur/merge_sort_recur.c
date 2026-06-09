#include "merge_sort_recur.h"
#include <assert.h>
#include <stdlib.h>

static void merge_sort_recur(int *src, int* dst, int len) {
    if (len <= 1) {
        return;
    }

    int s1 = 0, e1 = len / 2;
    int s2 = e1, e2 = len;

    merge_sort_recur(src + s1, dst + s1, e1 - s1);
    merge_sort_recur(src + s2, dst + s2, e2 - s2);

    int index = 0;
    while (s1 < e1 && s2 < e2) {
        dst[index++] = src[s1] <= src[s2] ? src[s1++] : src[s2++];
    }
    while (s1 < e1) {
        dst[index++] = src[s1++];
    }
    while (s2 < e2) {
        dst[index++] = src[s2++];
    }

    for (int i = 0; i < len; i++) {
        src[i] = dst[i];
    }
}

void merge_sort(int *arr, int len) {
    if (len <= 1) {
        return;
    }

    int* buf = malloc(len * sizeof(int));
    assert(buf);

    int* src = arr;
    int* dst = buf;

    merge_sort_recur(src, dst, len);

    free(buf);
}