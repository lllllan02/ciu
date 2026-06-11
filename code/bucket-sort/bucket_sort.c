#include "bucket_sort.h"
#include <assert.h>
#include <stdlib.h>

enum { BUCKET_NUM = 10 };

static void insertion_sort(int* arr, int len) {
    for (int i = 1; i < len; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

static int bucket_index(int value) {
    int index = value / BUCKET_NUM;
    return index >= BUCKET_NUM ? BUCKET_NUM - 1 : index;
}

void bucket_sort(int* arr, int len) {
    if (len <= 0) return;

    int counts[BUCKET_NUM] = {0};

    for (int i = 0; i < len; i++) {
        counts[bucket_index(arr[i])]++;
    }

    int** buckets = malloc(BUCKET_NUM * sizeof(int*));
    assert(buckets);

    int* pos = malloc(BUCKET_NUM * sizeof(int));
    assert(pos);

    for (int i = 0; i < BUCKET_NUM; i++) {
        buckets[i] = counts[i] ? malloc(counts[i] * sizeof(int)) : NULL;
        pos[i] = 0;
    }

    for (int i = 0; i < len; i++) {
        int index = bucket_index(arr[i]);
        buckets[index][pos[index]++] = arr[i];
    }

    int idx = 0;
    for (int i = 0; i < BUCKET_NUM; i++) {
        if (counts[i] == 0) continue;

        insertion_sort(buckets[i], counts[i]);
        for (int j = 0; j < counts[i]; j++) {
            arr[idx++] = buckets[i][j];
        }
        free(buckets[i]);
    }

    free(buckets);
    free(pos);
}
