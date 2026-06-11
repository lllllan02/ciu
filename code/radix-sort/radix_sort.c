#include "radix_sort.h"
#include <stdlib.h>

#define BASE 10

static int bucket_index(int value, int exp) {
    return (value / exp) % BASE;
}

void radix_sort(int* arr, int len) {
    if (len <= 1) return;

    int max = arr[0];
    for (int i = 1; i < len; i++) {
        if (arr[i] > max) max = arr[i];
    }

    int exp = 1;
    int* output = malloc(len * sizeof(int));
    if (!output) return;

    while (max / exp > 0) {
        int buckets[BASE] = {0};

        for (int i = 0; i < len; i++) {
            int index = bucket_index(arr[i], exp);
            buckets[index]++;
        }

        for (int i = 1; i < BASE; i++) {
            buckets[i] += buckets[i - 1];
        }

        for (int i = len - 1; i >= 0; i--) {
            int index = bucket_index(arr[i], exp);
            int pos = --buckets[index];
            output[pos] = arr[i];
        }

        for (int i = 0; i < len; i++) {
            arr[i] = output[i];
        }

        exp *= BASE;
    }

    free(output);
}