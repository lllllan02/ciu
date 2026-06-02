#include "test.h"
#include "binary_search.c"
#include "binary_search.h"
#include <assert.h>
#include <stdio.h>

static int sorted[] = {1, 3, 5, 7, 9, 11, 13};
static const int sorted_len = 7;

void run_all_tests() {
    printf("test_binary_search_found\n");
    test_binary_search_found();
    printf("test_binary_search_not_found\n");
    test_binary_search_not_found();
    printf("test_binary_search_edge_cases\n");
    test_binary_search_edge_cases();
    printf("test_binary_search_recur_found\n");
    test_binary_search_recur_found();
    printf("test_binary_search_recur_not_found\n");
    test_binary_search_recur_not_found();
    printf("test_binary_search_recur_edge_cases\n");
    test_binary_search_recur_edge_cases();
}

void test_binary_search_found() {
    assert(binary_search(sorted, sorted_len, 1) == 0);
    assert(binary_search(sorted, sorted_len, 7) == 3);
    assert(binary_search(sorted, sorted_len, 13) == 6);
}

void test_binary_search_not_found() {
    assert(binary_search(sorted, sorted_len, 4) == -1);
    assert(binary_search(sorted, sorted_len, 0) == -1);
    assert(binary_search(sorted, sorted_len, 100) == -1);
}

void test_binary_search_edge_cases() {
    int empty[] = {1};
    assert(binary_search(empty, 0, 1) == -1);

    int single[] = {5};
    assert(binary_search(single, 1, 5) == 0);
    assert(binary_search(single, 1, 3) == -1);
}

void test_binary_search_recur_found() {
    assert(binary_search_recur(sorted, 0, sorted_len - 1, 1) == 0);
    assert(binary_search_recur(sorted, 0, sorted_len - 1, 7) == 3);
    assert(binary_search_recur(sorted, 0, sorted_len - 1, 13) == 6);
}

void test_binary_search_recur_not_found() {
    assert(binary_search_recur(sorted, 0, sorted_len - 1, 4) == -1);
    assert(binary_search_recur(sorted, 0, sorted_len - 1, 0) == -1);
    assert(binary_search_recur(sorted, 0, sorted_len - 1, 100) == -1);
}

void test_binary_search_recur_edge_cases() {
    int empty[] = {1};
    assert(binary_search_recur(empty, 0, -1, 1) == -1);

    int single[] = {5};
    assert(binary_search_recur(single, 0, 0, 5) == 0);
    assert(binary_search_recur(single, 0, 0, 3) == -1);
}
