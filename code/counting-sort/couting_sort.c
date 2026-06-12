#include "counting_sort.h"

void counting_sort(int* arr, int len, int min_val, int max_val) {
    if (len <= 1) {
        return;
    }

    int range = max_val - min_val + 1;
    int count[range];
    int output[len];

    for (int i = 0; i < range; i++) {
        count[i] = 0;
    }

    for (int i = 0; i < len; i++) {
        count[arr[i] - min_val]++;
    }

    for (int i = 1; i < range; i++) {
        count[i] += count[i - 1];
    }

    for (int i = len - 1; i >= 0; i--) {
        int idx = --count[arr[i] - min_val];
        output[idx] = arr[i];
    }

    for (int i = 0; i < len; i++) {
        arr[i] = output[i];
    }
}
