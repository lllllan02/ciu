#include "test.h"
#include "radix_sort.c"
#include "radix_sort.h"
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
    printf("test_radix_sort_basic\n");
    test_radix_sort_basic();
    printf("test_radix_sort_already_sorted\n");
    test_radix_sort_already_sorted();
    printf("test_radix_sort_reverse_sorted\n");
    test_radix_sort_reverse_sorted();
    printf("test_radix_sort_duplicates\n");
    test_radix_sort_duplicates();
    printf("test_radix_sort_edge_cases\n");
    test_radix_sort_edge_cases();
}

void test_radix_sort_basic() {
    int arr[] = {170, 45, 75, 90, 802, 24, 2, 66};
    int expected[] = {2, 24, 45, 66, 75, 90, 170, 802};

    radix_sort(arr, 8);

    assert(is_sorted(arr, 8));
    for (int i = 0; i < 8; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_radix_sort_already_sorted() {
    int arr[] = {1, 2, 3, 4, 5};

    radix_sort(arr, 5);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_radix_sort_reverse_sorted() {
    int arr[] = {5, 4, 3, 2, 1};

    radix_sort(arr, 5);

    assert(is_sorted(arr, 5));
    for (int i = 0; i < 5; i++) {
        assert(arr[i] == i + 1);
    }
}

void test_radix_sort_duplicates() {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    int expected[] = {1, 1, 2, 3, 3, 4, 5, 5, 6, 9};

    radix_sort(arr, 10);

    assert(is_sorted(arr, 10));
    for (int i = 0; i < 10; i++) {
        assert(arr[i] == expected[i]);
    }
}

void test_radix_sort_edge_cases() {
    int empty[] = {42};
    radix_sort(empty, 0);
    assert(empty[0] == 42);

    int single[] = {7};
    radix_sort(single, 1);
    assert(single[0] == 7);

    int zeros[] = {0, 0, 0};
    radix_sort(zeros, 3);
    assert(zeros[0] == 0);
    assert(zeros[1] == 0);
    assert(zeros[2] == 0);

    int same_digits[] = {11, 12, 13, 21, 22, 23};
    radix_sort(same_digits, 6);
    assert(is_sorted(same_digits, 6));
    assert(same_digits[0] == 11);
    assert(same_digits[5] == 23);

    int large[] = {1000, 1, 100, 10};
    radix_sort(large, 4);
    assert(is_sorted(large, 4));
    assert(large[0] == 1);
    assert(large[1] == 10);
    assert(large[2] == 100);
    assert(large[3] == 1000);
}
