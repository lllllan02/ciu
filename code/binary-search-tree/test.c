#include "test.h"
#include "bst.c"
#include "bst.h"
#include <assert.h>
#include <stdio.h>

static BSTNode* build_tree(const int* values, int n) {
    BSTNode* root = NULL;
    for (int i = 0; i < n; i++) {
        root = insert(root, values[i]);
    }
    return root;
}

void run_all_tests() {
    printf("test_insert_and_count\n");
    test_insert_and_count();
    printf("test_get_min_max\n");
    test_get_min_max();
    printf("test_get_height\n");
    test_get_height();
    printf("test_is_in_tree\n");
    test_is_in_tree();
    printf("test_is_between\n");
    test_is_between();
    printf("test_delete_value\n");
    test_delete_value();
    printf("test_get_successor\n");
    test_get_successor();
    printf("test_delete_tree\n");
    test_delete_tree();
}

void test_insert_and_count() {
    BSTNode* root = NULL;
    assert(get_count(root) == 0);

    root = insert(root, 5);
    assert(get_count(root) == 1);

    int values[] = {5, 3, 7, 1, 4, 6, 8};
    root = build_tree(values, 7);
    assert(get_count(root) == 7);

    root = insert(root, 3);
    assert(get_count(root) == 7);

    delete_tree(root);
}

void test_get_min_max() {
    BSTNode* root = NULL;
    assert(get_min(root) == -1);
    assert(get_max(root) == -1);

    root = insert(root, 10);
    assert(get_min(root) == 10);
    assert(get_max(root) == 10);

    int values[] = {5, 3, 7, 1, 9};
    root = build_tree(values, 5);
    assert(get_min(root) == 1);
    assert(get_max(root) == 9);

    delete_tree(root);
}

void test_get_height() {
    BSTNode* root = NULL;
    assert(get_height(root) == 0);

    root = insert(root, 5);
    assert(get_height(root) == 1);

    root = insert(root, 3);
    root = insert(root, 7);
    assert(get_height(root) == 2);

    int values[] = {5, 3, 7, 1, 4, 6, 8};
    root = build_tree(values, 7);
    assert(get_height(root) == 3);

    delete_tree(root);
}

void test_is_in_tree() {
    BSTNode* root = NULL;
    assert(!is_in_tree(root, 5));

    int values[] = {5, 3, 7, 1, 4, 6, 8};
    root = build_tree(values, 7);

    assert(is_in_tree(root, 5));
    assert(is_in_tree(root, 1));
    assert(is_in_tree(root, 8));
    assert(!is_in_tree(root, 0));
    assert(!is_in_tree(root, 9));

    delete_tree(root);
}

void test_is_between() {
    BSTNode* root = NULL;
    assert(is_between(root, 0, 10));

    int values[] = {5, 3, 7, 1, 4, 6, 8};
    root = build_tree(values, 7);

    assert(is_between(root, 0, 10));
    assert(is_between(root, 0, 9));
    assert(!is_between(root, 0, 6));
    assert(!is_between(root, 2, 8));
    assert(!is_between(root, 0, 5));

    delete_tree(root);
}

void test_delete_value() {
    BSTNode* root = NULL;
    root = delete_value(root, 5);
    assert(root == NULL);

    int values[] = {5, 3, 7, 1, 4, 6, 8};
    root = build_tree(values, 7);

    root = delete_value(root, 1);
    assert(get_count(root) == 6);
    assert(!is_in_tree(root, 1));
    assert(is_in_tree(root, 3));

    root = delete_value(root, 5);
    assert(get_count(root) == 5);
    assert(!is_in_tree(root, 5));
    assert(is_in_tree(root, 6));

    root = delete_value(root, 7);
    assert(get_count(root) == 4);
    assert(!is_in_tree(root, 7));
    assert(is_in_tree(root, 8));

    while (root != NULL) {
        int min = get_min(root);
        root = delete_value(root, min);
    }
    assert(root == NULL);
}

void test_get_successor() {
    BSTNode* root = NULL;
    assert(get_successor(root, 5) == -1);

    int values[] = {5, 3, 7, 1, 4, 6, 8};
    root = build_tree(values, 7);

    assert(get_successor(root, 1) == 3);
    assert(get_successor(root, 4) == 5);
    assert(get_successor(root, 5) == 6);
    assert(get_successor(root, 6) == 7);
    assert(get_successor(root, 7) == 8);
    assert(get_successor(root, 8) == -1);
    assert(get_successor(root, 0) == 1);
    assert(get_successor(root, 9) == -1);

    delete_tree(root);
}

void test_delete_tree() {
    int values[] = {5, 3, 7, 1, 4, 6, 8};
    BSTNode* root = build_tree(values, 7);
    delete_tree(root);
}
