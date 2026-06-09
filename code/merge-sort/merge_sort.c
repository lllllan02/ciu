#include "merge_sort.h"
#include <assert.h>
#include <stdlib.h>

static int min(int a, int b) {
    return a < b ? a : b;
}

void merge_sort(int *arr, int len) {
    if (len <= 1) {
        return;
    }

    int *buf = malloc(len * sizeof(int));
    assert(buf);

    int *src = arr;
    int *dst = buf;

    for (int seg = 1; seg < len; seg <<= 1) {
        for (int start = 0; start < len; start += 2 * seg) {
            int s1 = start, e1 = min(start + seg, len);
            int s2 = e1, e2 = min(start + 2 * seg, len);
            int index = start;

            while (s1 < e1 && s2 < e2) {
                dst[index++] = src[s1] <= src[s2] ? src[s1++] : src[s2++];
            }
            while (s1 < e1) {
                dst[index++] = src[s1++];
            }
            while (s2 < e2) {
                dst[index++] = src[s2++];
            }
        }

        int *tmp = src;
        src = dst;
        dst = tmp;
    }

    if (src != arr) {
        for (int i = 0; i < len; i++) {
            arr[i] = src[i];
        }
    }

    free(buf);
}
