#include "test.h"
#include "shell_sort.c"
#include "shell_sort.h"
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
    printf("test_shell_sort_basic\n");
    test_shell_sort_basic();
    printf("test_shell_sort_already_sorted\n");
    test_shell_sort_already_sorted();
    printf("test_shell_sort_reverse_sorted\n");
    test_shell_sort_reverse_sorted();
    printf("test_shell_sort_duplicates\n");
    test_shell_sort_duplicates();
    printf("test_shell_sort_edge_cases\n");
    test_shell_sort_edge_cases();
}

void test_shell_sort_basic() {
    int arr[] = {64, 34, 25, 12, 22, 11, 90};
    int expected[] = {11, 12, 22, 25, 34, 64, 90};

    shell_sort(arr, 7);

    assert(is_sorted(arr, 7));
    for (int i = 0; i < 7; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_shell_sort_already_sorted() {
    int arr[] = {1, 2, 3, 4, 5};

    shell_sort(arr, 5);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_shell_sort_reverse_sorted() {
    int arr[] = {5, 4, 3, 2, 1};

    shell_sort(arr, 5);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_shell_sort_duplicates() {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    int expected[] = {1, 1, 2, 3, 3, 4, 5, 5, 6, 9};

    shell_sort(arr, 10);

    assert(is_sorted(arr, 10));
    for (int i = 0; i < 10; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_shell_sort_edge_cases() {
    int empty[] = {42};
    shell_sort(empty, 0);
    assert(empty[0] == 42);

    int single[] = {7};
    shell_sort(single, 1);
    assert(single[0] == 7);

    int negatives[] = {0, -3, 5, -1, 2};
    shell_sort(negatives, 5);
    assert(is_sorted(negatives, 5));
    assert(negatives[0] == -3);
    assert(negatives[4] == 5);
}
