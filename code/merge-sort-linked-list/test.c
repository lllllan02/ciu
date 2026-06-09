#include "test.h"
#include "merge_sort_linked_list.c"
#include "merge_sort_linked_list.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static Node *from_array(int *arr, int len) {
    if (len == 0) {
        return NULL;
    }

    Node *head = malloc(sizeof(Node));
    head->data = arr[0];
    head->next = NULL;

    Node *tail = head;
    for (int i = 1; i < len; i++) {
        Node *node = malloc(sizeof(Node));
        node->data = arr[i];
        node->next = NULL;
        tail->next = node;
        tail = node;
    }
    return head;
}

static void free_list(Node *head) {
    while (head) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}

static bool is_sorted(Node *head) {
    while (head && head->next) {
        if (head->data > head->next->data) {
            return false;
        }
        head = head->next;
    }
    return true;
}

static void assert_list_eq(Node *head, int *expected, int len) {
    for (int i = 0; i < len; i++) {
        assert(head != NULL);
        assert(head->data == expected[i]);
        head = head->next;
    }
    assert(head == NULL);
}

void run_all_tests() {
    printf("test_merge_sort_linked_list_basic\n");
    test_merge_sort_linked_list_basic();
    printf("test_merge_sort_linked_list_already_sorted\n");
    test_merge_sort_linked_list_already_sorted();
    printf("test_merge_sort_linked_list_reverse_sorted\n");
    test_merge_sort_linked_list_reverse_sorted();
    printf("test_merge_sort_linked_list_duplicates\n");
    test_merge_sort_linked_list_duplicates();
    printf("test_merge_sort_linked_list_edge_cases\n");
    test_merge_sort_linked_list_edge_cases();
}

void test_merge_sort_linked_list_basic() {
    int arr[] = {64, 34, 25, 12, 22, 11, 90};
    int expected[] = {11, 12, 22, 25, 34, 64, 90};

    Node *head = from_array(arr, 7);
    head = merge_sort(head);

    assert(is_sorted(head));
    assert_list_eq(head, expected, 7);
    free_list(head);
}

void test_merge_sort_linked_list_already_sorted() {
    int arr[] = {1, 2, 3, 4, 5};
    int expected[] = {1, 2, 3, 4, 5};

    Node *head = from_array(arr, 5);
    head = merge_sort(head);

    assert(is_sorted(head));
    assert_list_eq(head, expected, 5);
    free_list(head);
}

void test_merge_sort_linked_list_reverse_sorted() {
    int arr[] = {5, 4, 3, 2, 1};
    int expected[] = {1, 2, 3, 4, 5};

    Node *head = from_array(arr, 5);
    head = merge_sort(head);

    assert(is_sorted(head));
    assert_list_eq(head, expected, 5);
    free_list(head);
}

void test_merge_sort_linked_list_duplicates() {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    int expected[] = {1, 1, 2, 3, 3, 4, 5, 5, 6, 9};

    Node *head = from_array(arr, 10);
    head = merge_sort(head);

    assert(is_sorted(head));
    assert_list_eq(head, expected, 10);
    free_list(head);
}

void test_merge_sort_linked_list_edge_cases() {
    assert(merge_sort(NULL) == NULL);

    int single[] = {7};
    Node *head = from_array(single, 1);
    head = merge_sort(head);
    assert(head->data == 7);
    assert(head->next == NULL);
    free_list(head);

    int negatives[] = {0, -3, 5, -1, 2};
    int expected[] = {-3, -1, 0, 2, 5};
    head = from_array(negatives, 5);
    head = merge_sort(head);
    assert(is_sorted(head));
    assert_list_eq(head, expected, 5);
    free_list(head);
}
