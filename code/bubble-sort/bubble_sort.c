#include "bubble_sort.h"
#include <stdbool.h>

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void bubble_sort(int* arr, int len) {
    if (len <= 1) return;

    bool flag = true;
    while (flag) {
        flag = false;
        for (int i = 0; i < len - 1; i++) {
            if (arr[i] > arr[i + 1]) {
                flag = true;
                swap(arr + i, arr + i + 1);
            }
        }
    }
}
