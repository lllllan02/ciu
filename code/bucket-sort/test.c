#include "test.h"
#include "bucket_sort.c"
#include "bucket_sort.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static bool is_sorted(int *arr, int len) {
    for (int i = 1; i < len; i++) {
        if (arr[i - 1] > arr[i]) {
            return false;
        }
    }
    return true;
}

void run_all_tests() {
    printf("test_bucket_sort_basic\n");
    test_bucket_sort_basic();
    printf("test_bucket_sort_already_sorted\n");
    test_bucket_sort_already_sorted();
    printf("test_bucket_sort_reverse_sorted\n");
    test_bucket_sort_reverse_sorted();
    printf("test_bucket_sort_duplicates\n");
    test_bucket_sort_duplicates();
    printf("test_bucket_sort_edge_cases\n");
    test_bucket_sort_edge_cases();
}

void test_bucket_sort_basic() {
    int arr[] = {64, 34, 25, 12, 22, 11, 90};
    int expected[] = {11, 12, 22, 25, 34, 64, 90};

    bucket_sort(arr, 7);

    assert(is_sorted(arr, 7));
    for (int i = 0; i < 7; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_bucket_sort_already_sorted() {
    int arr[] = {1, 2, 3, 4, 5};

    bucket_sort(arr, 5);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_bucket_sort_reverse_sorted() {
    int arr[] = {5, 4, 3, 2, 1};

    bucket_sort(arr, 5);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_bucket_sort_duplicates() {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    int expected[] = {1, 1, 2, 3, 3, 4, 5, 5, 6, 9};

    bucket_sort(arr, 10);

    assert(is_sorted(arr, 10));
    for (int i = 0; i < 10; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_bucket_sort_edge_cases() {
    int empty[] = {42};
    bucket_sort(empty, 0);
    assert(empty[0] == 42);

    int single[] = {7};
    bucket_sort(single, 1);
    assert(single[0] == 7);

    int zeros[] = {0, 0, 0};
    bucket_sort(zeros, 3);
    assert(zeros[0] == 0);
    assert(zeros[1] == 0);
    assert(zeros[2] == 0);

    int bounds[] = {99, 0, 99, 1, 0};
    bucket_sort(bounds, 5);
    assert(is_sorted(bounds, 5));
    assert(bounds[0] == 0);
    assert(bounds[1] == 0);
    assert(bounds[2] == 1);
    assert(bounds[3] == 99);
    assert(bounds[4] == 99);

    int same_bucket[] = {8, 3, 9, 1, 5, 0, 7, 2, 6, 4};
    bucket_sort(same_bucket, 10);
    assert(is_sorted(same_bucket, 10));
    for (int i = 0; i < 10; i++) {
        assert(same_bucket[i] == i);
    }
}
