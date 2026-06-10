#include "test.h"
#include "insertion_sort.c"
#include "insertion_sort.h"
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
    printf("test_insertion_sort_basic\n");
    test_insertion_sort_basic();
    printf("test_insertion_sort_already_sorted\n");
    test_insertion_sort_already_sorted();
    printf("test_insertion_sort_reverse_sorted\n");
    test_insertion_sort_reverse_sorted();
    printf("test_insertion_sort_duplicates\n");
    test_insertion_sort_duplicates();
    printf("test_insertion_sort_edge_cases\n");
    test_insertion_sort_edge_cases();
}

void test_insertion_sort_basic() {
    int arr[] = {64, 34, 25, 12, 22, 11, 90};
    int expected[] = {11, 12, 22, 25, 34, 64, 90};

    insertion_sort(arr, 7);

    assert(is_sorted(arr, 7));
    for (int i = 0; i < 7; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_insertion_sort_already_sorted() {
    int arr[] = {1, 2, 3, 4, 5};

    insertion_sort(arr, 5);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_insertion_sort_reverse_sorted() {
    int arr[] = {5, 4, 3, 2, 1};

    insertion_sort(arr, 5);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_insertion_sort_duplicates() {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    int expected[] = {1, 1, 2, 3, 3, 4, 5, 5, 6, 9};

    insertion_sort(arr, 10);

    assert(is_sorted(arr, 10));
    for (int i = 0; i < 10; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_insertion_sort_edge_cases() {
    int empty[] = {42};
    insertion_sort(empty, 0);
    assert(empty[0] == 42);

    int single[] = {7};
    insertion_sort(single, 1);
    assert(single[0] == 7);

    int negatives[] = {0, -3, 5, -1, 2};
    insertion_sort(negatives, 5);
    assert(is_sorted(negatives, 5));
    assert(negatives[0] == -3);
    assert(negatives[4] == 5);
}
