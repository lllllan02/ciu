#include "counting_sort.h"

void counting_sort(int* arr, int len, int n) {
    int count[n];
    for (int i = 0; i < n; i++) {
        count[i] = 0;
    }

    for (int i = 0; i < len; i++) {
        count[ arr[i] ] += 1;
    }

    int index = 0;
    for (int i = 0; i < n; i++) {
        while (count[i]--) {
            arr[index++] = i;
        }
    }
}