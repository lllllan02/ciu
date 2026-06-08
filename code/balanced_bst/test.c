#include "test.h"
#include "bbst.c"
#include "bbst.h"
#include <assert.h>
#include <stdio.h>

static TreeNode* build_tree(const int* values, int n) {
    TreeNode* root = NULL;
    for (int i = 0; i < n; i++) {
        root = insert(root, values[i]);
    }
    return root;
}

static int get_count(TreeNode* root) {
    if (root == NULL) {
        return 0;
    }
    return 1 + get_count(root->left) + get_count(root->right);
}

static bool is_in_tree(TreeNode* root, const int value) {
    if (root == NULL) {
        return false;
    }
    if (root->value == value) {
        return true;
    }
    if (value < root->value) {
        return is_in_tree(root->left, value);
    }
    return is_in_tree(root->right, value);
}

static void assert_balanced(TreeNode* root) {
    if (root == NULL) {
        return;
    }
    int lh = root->left ? root->left->height : 0;
    int rh = root->right ? root->right->height : 0;
    int bf = lh - rh;
    assert(bf >= -1 && bf <= 1);
    assert_balanced(root->left);
    assert_balanced(root->right);
}

static bool is_bst(TreeNode* root, int min, int max) {
    if (root == NULL) {
        return true;
    }
    if (root->value <= min || root->value >= max) {
        return false;
    }
    return is_bst(root->left, min, root->value) &&
           is_bst(root->right, root->value, max);
}

static int actual_height(TreeNode* root) {
    if (root == NULL) {
        return 0;
    }
    int lh = actual_height(root->left);
    int rh = actual_height(root->right);
    int h = (lh > rh ? lh : rh) + 1;
    assert(root->height == h);
    return h;
}

static void assert_valid_tree(TreeNode* root) {
    assert(is_bst(root, -2147483647, 2147483647));
    assert_balanced(root);
    actual_height(root);
}

void run_all_tests() {
    printf("test_insert\n");
    test_insert();
    printf("test_delete_value\n");
    test_delete_value();
    printf("test_avl_balance\n");
    test_avl_balance();
    printf("test_delete_tree\n");
    test_delete_tree();
}

void test_insert() {
    TreeNode* root = NULL;
    assert(get_count(root) == 0);

    root = insert(root, 5);
    assert(get_count(root) == 1);
    assert(is_in_tree(root, 5));
    assert_valid_tree(root);

    int values[] = {5, 3, 7, 1, 4, 6, 8};
    root = build_tree(values, 7);
    assert(get_count(root) == 7);
    assert_valid_tree(root);

    root = insert(root, 3);
    assert(get_count(root) == 7);
    assert_valid_tree(root);

    delete_tree(root);
}

void test_delete_value() {
    TreeNode* root = NULL;
    root = delete_value(root, 5);
    assert(root == NULL);

    int values[] = {5, 3, 7, 1, 4, 6, 8};
    root = build_tree(values, 7);

    root = delete_value(root, 1);
    assert(get_count(root) == 6);
    assert(!is_in_tree(root, 1));
    assert(is_in_tree(root, 3));
    assert_valid_tree(root);

    root = delete_value(root, 5);
    assert(get_count(root) == 5);
    assert(!is_in_tree(root, 5));
    assert(is_in_tree(root, 6));
    assert_valid_tree(root);

    root = delete_value(root, 7);
    assert(get_count(root) == 4);
    assert(!is_in_tree(root, 7));
    assert(is_in_tree(root, 8));
    assert_valid_tree(root);

    while (root != NULL) {
        int min = root->value;
        TreeNode* left = root->left;
        while (left != NULL) {
            min = left->value;
            left = left->left;
        }
        root = delete_value(root, min);
        assert_valid_tree(root);
    }
    assert(root == NULL);
}

void test_avl_balance() {
    int ascending[] = {1, 2, 3, 4, 5, 6, 7};
    TreeNode* root = build_tree(ascending, 7);
    assert_valid_tree(root);
    assert(actual_height(root) <= 3);
    delete_tree(root);

    int descending[] = {7, 6, 5, 4, 3, 2, 1};
    root = build_tree(descending, 7);
    assert_valid_tree(root);
    assert(actual_height(root) <= 3);
    delete_tree(root);

    int lr[] = {3, 1, 2};
    root = build_tree(lr, 3);
    assert_valid_tree(root);
    assert(is_in_tree(root, 2));
    assert(actual_height(root) == 2);
    delete_tree(root);

    int rl[] = {1, 3, 2};
    root = build_tree(rl, 3);
    assert_valid_tree(root);
    assert(is_in_tree(root, 2));
    assert(actual_height(root) == 2);
    delete_tree(root);

    int many[] = {50, 25, 75, 10, 30, 60, 80, 5, 15, 27, 40};
    root = build_tree(many, 11);
    assert_valid_tree(root);
    assert(get_count(root) == 11);
    delete_tree(root);
}

void test_delete_tree() {
    int values[] = {5, 3, 7, 1, 4, 6, 8};
    TreeNode* root = build_tree(values, 7);
    delete_tree(root);
}
