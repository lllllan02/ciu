#include "shell_sort.h"

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void shell_sort(int* arr, int len) {
    for (int gap = len >> 1; gap > 0; gap >>= 1) {
        for (int i = gap; i < len; i++) {
            for (int j = i - gap; j >= 0; j -= gap) {
                if (arr[j + gap] >= arr[j]) break;

                swap(arr + j, arr + j + gap);
            }
        }
    }
}