#include "selection_sort.h"
#include <stdlib.h>

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void selection_sort(int* arr, int len) {
    for (int i = 0; i < len; i++) {
        int min = i;
        for (int j = i + 1; j < len; j++) {
            if (arr[j] < arr[min]) min = j;
        }

        if (i != min) swap(arr + i, arr + min);
    }
}