#include "test.h"
#include "couting_sort.c"
#include "counting_sort.h"
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
    printf("test_counting_sort_basic\n");
    test_counting_sort_basic();
    printf("test_counting_sort_already_sorted\n");
    test_counting_sort_already_sorted();
    printf("test_counting_sort_reverse_sorted\n");
    test_counting_sort_reverse_sorted();
    printf("test_counting_sort_duplicates\n");
    test_counting_sort_duplicates();
    printf("test_counting_sort_edge_cases\n");
    test_counting_sort_edge_cases();
}

void test_counting_sort_basic() {
    int arr[] = {64, 34, 25, 12, 22, 11, 90};
    int expected[] = {11, 12, 22, 25, 34, 64, 90};

    counting_sort(arr, 7, 100);

    assert(is_sorted(arr, 7));
    for (int i = 0; i < 7; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_counting_sort_already_sorted() {
    int arr[] = {1, 2, 3, 4, 5};

    counting_sort(arr, 5, 6);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_counting_sort_reverse_sorted() {
    int arr[] = {5, 4, 3, 2, 1};

    counting_sort(arr, 5, 6);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_counting_sort_duplicates() {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    int expected[] = {1, 1, 2, 3, 3, 4, 5, 5, 6, 9};

    counting_sort(arr, 10, 10);

    assert(is_sorted(arr, 10));
    for (int i = 0; i < 10; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_counting_sort_edge_cases() {
    int empty[] = {42};
    counting_sort(empty, 0, 100);
    assert(empty[0] == 42);

    int single[] = {7};
    counting_sort(single, 1, 8);
    assert(single[0] == 7);

    int zeros[] = {0, 0, 0};
    counting_sort(zeros, 3, 1);
    assert(zeros[0] == 0);
    assert(zeros[1] == 0);
    assert(zeros[2] == 0);

    int bounds[] = {99, 0, 99, 1, 0};
    counting_sort(bounds, 5, 100);
    assert(is_sorted(bounds, 5));
    assert(bounds[0] == 0);
    assert(bounds[1] == 0);
    assert(bounds[2] == 1);
    assert(bounds[3] == 99);
    assert(bounds[4] == 99);
}
