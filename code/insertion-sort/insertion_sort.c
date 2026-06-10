#include "insertion_sort.h"

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void insertion_sort(int* arr, int len) {
    for (int i = 0; i < len; i++) {
        for (int j = i - 1; j >= 0; j--) {
            if (arr[j + 1] >= arr[j]) break;
            
            swap(arr + j, arr + j + 1);
        }
    }
}