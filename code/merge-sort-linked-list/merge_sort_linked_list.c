#include "merge_sort_linked_list.h"
#include <stddef.h>

static Node *merge(Node *left, Node *right) {
    Node dummy = {0, NULL};
    Node *tail = &dummy;

    while (left && right) {
        if (left->data <= right->data) {
            tail->next = left;
            left = left->next;
        } else {
            tail->next = right;
            right = right->next;
        }
        tail = tail->next;
    }
    tail->next = left ? left : right;
    return dummy.next;
}

static Node *split(Node *head) {
    Node *slow = head;
    Node *fast = head;
    Node *prev = NULL;

    while (fast && fast->next) {
        prev = slow;
        slow = slow->next;
        fast = fast->next->next;
    }
    prev->next = NULL;
    return slow;
}

Node *merge_sort(Node *head) {
    if (!head || !head->next) {
        return head;
    }

    Node *second = split(head);
    head = merge_sort(head);
    second = merge_sort(second);
    return merge(head, second);
}
