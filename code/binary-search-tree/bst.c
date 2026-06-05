#include "bst.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_va_list.h>

BSTNode* insert(BSTNode* node, const int value) {
    if (node == NULL) {
        node = malloc(sizeof(BSTNode));
        assert(node);

        node->value = value;
        node->left = NULL;
        node->right = NULL;
        return node;
    }

    if (node->value > value) {
        node->left = insert(node->left, value);
    } else if (node->value < value) {
        node->right = insert(node->right, value);
    }
    return node;
}

void delete_tree(BSTNode* node) {
    if (node == NULL) {
        return;
    }

    delete_tree(node->left);
    delete_tree(node->right);
    free(node);
}

void pre_order_traversal(BSTNode* node) {
    if (node == NULL) {
        return;
    }

    printf("%d\n", node->value);
    pre_order_traversal(node->left);
    pre_order_traversal(node->right);
}

void in_order_traversal(BSTNode* node) {
    if (node == NULL) {
        return;
    }

    in_order_traversal(node->left);
    printf("%d\n", node->value);
    in_order_traversal(node->right);
}

void post_order_traversal(BSTNode* node) {
    if (node == NULL) {
        return;
    }

    post_order_traversal(node->left);
    post_order_traversal(node->right);
    printf("%d\n", node->value);
}

void level_order_traversal(BSTNode* node) {
    if (node == NULL) {
        return;
    }
    
    int size = get_count(node);
    BSTNode** queue = malloc(size * sizeof(BSTNode));
    assert(queue);

    int head = 0, tail = 0;
    queue[tail++] = node;

    while(head != tail) {
        BSTNode* cur = queue[head++];

        printf("%d\n", cur->value);
        if (cur->left != NULL) {
            queue[tail++] = cur->left;
        }
        if (cur->right != NULL) {
            queue[tail++] = cur->right;
        }
    }

    free(queue);
}

int get_height(BSTNode* node) {
    if (node == NULL) {
        return 0;
    }

    int left = get_height(node->left);
    int right = get_height(node->right);
    return (left > right ? left : right) + 1;
}

int get_count(BSTNode* node) {
    if (node == NULL) {
        return 0;
    }

    int left = get_count(node->left);
    int right = get_count(node->right);
    return left + right + 1;
}

int get_min(BSTNode* node) {
    if (node == NULL) {
        return -1;
    } else if (node->left == NULL) {
        return node->value;
    }
    return get_min(node->left);
}

int get_max(BSTNode* node) {
    if (node == NULL) {
        return -1;
    } else if (node->right == NULL) {
        return node->value;
    }
    return get_max(node->right);
}

bool is_between(BSTNode* node, int min, int max) {
    if (node == NULL) {
        return true;
    }

    return min < node->value && node->value < max &&
        is_between(node->left, min, max) &&
        is_between(node->right, min, max);
}

bool is_in_tree(BSTNode* node, int value) {
    if (node == NULL) {
        return false;
    }

    return node->value == value ||
        is_in_tree(node->left, value) ||
        is_in_tree(node->right, value);
}

BSTNode* delete_value(BSTNode* node, int value) {
    if (node == NULL) {
        return NULL;
    }

    if (node->value > value) {
        node->left = delete_value(node->left, value);
    } else if (node->value < value) {
        node->right = delete_value(node->right, value);
    } else {
        if (node->left == NULL || node->right == NULL) {
            BSTNode* child = node->left ? node->left : node->right;
            free(node);
            return child;
        }

        node->value = get_max(node->left);
        node->left = delete_value(node->left, node->value);
    }

    return node;
}

int get_successor(BSTNode* node, int value) {
    int successor = -1;

    while (node != NULL) {
        if (node->value > value) {
            successor = node->value;
            node = node->left;
        } else if (node->value < value) {
            node = node->right;
        } else {
            if (node->right != NULL) {
                return get_min(node->right);
            }
            return successor;
        }
    }

    return successor;
}