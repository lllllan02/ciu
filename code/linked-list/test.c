#include "test.h"
#include "linked_list.c"
#include "linked_list.h"
#include <assert.h>
#include <stdio.h>

void run_all_tests() {
    printf("test_size_init\n");
    test_size_init();
    printf("test_push_front\n");
    test_push_front();
    printf("test_push_back\n");
    test_push_back();
    printf("test_empty\n");
    test_empty();
    printf("test_front_back\n");
    test_front_back();
    printf("test_pop_front\n");
    test_pop_front();
    printf("test_pop_back\n");
    test_pop_back();
    printf("test_insert\n");
    test_insert();
    printf("test_value_at\n");
    test_value_at();
    printf("test_remove_at\n");
    test_remove_at();
    printf("test_remove_value\n");
    test_remove_value();
    printf("test_reverse\n");
    test_reverse();
}

void test_size_init() {
    List *list = new_list();
    assert(size(list) == 0);
    destory_list(list);
}

void test_push_front() {
    List *list = new_list();
    push_front(list, 1);
    push_front(list, 2);
    push_front(list, 3);
    
    assert(size(list) == 3);
    assert(front(list) == 3);
    assert(back(list) == 1);
    
    destory_list(list);
}

void test_push_back() {
    List *list = new_list();
    push_back(list, 1);
    push_back(list, 2);
    push_back(list, 3);
    
    assert(size(list) == 3);
    assert(front(list) == 1);
    assert(back(list) == 3);
    
    destory_list(list);
}

void test_empty() {
    List *list = new_list();
    assert(is_empty(list));
    push_back(list, 1);
    assert(!is_empty(list));
    destory_list(list);
}

void test_front_back() {
    List *list = new_list();
    push_back(list, 10);
    assert(front(list) == 10);
    assert(back(list) == 10);
    
    push_back(list, 20);
    assert(front(list) == 10);
    assert(back(list) == 20);
    
    destory_list(list);
}

void test_pop_front() {
    List *list = new_list();
    push_back(list, 1);
    push_back(list, 2);
    push_back(list, 3);
    
    assert(pop_front(list) == 1);
    assert(size(list) == 2);
    assert(front(list) == 2);
    
    assert(pop_front(list) == 2);
    assert(size(list) == 1);
    assert(front(list) == 3);
    
    assert(pop_front(list) == 3);
    assert(size(list) == 0);
    assert(is_empty(list));
    
    destory_list(list);
}

void test_pop_back() {
    List *list = new_list();
    push_back(list, 1);
    push_back(list, 2);
    push_back(list, 3);
    
    assert(pop_back(list) == 3);
    assert(size(list) == 2);
    assert(back(list) == 2);
    
    assert(pop_back(list) == 2);
    assert(size(list) == 1);
    assert(back(list) == 1);
    
    assert(pop_back(list) == 1);
    assert(size(list) == 0);
    assert(is_empty(list));
    
    destory_list(list);
}

void test_insert() {
    List *list = new_list();
    push_back(list, 1);
    push_back(list, 3);
    
    insert(list, 1, 2); // Insert 2 at index 1 -> 1, 2, 3
    assert(size(list) == 3);
    assert(value_at(list, 0) == 1);
    assert(value_at(list, 1) == 2);
    assert(value_at(list, 2) == 3);
    
    insert(list, 0, 0); // Insert 0 at index 0 -> 0, 1, 2, 3
    assert(value_at(list, 0) == 0);
    assert(front(list) == 0);
    
    insert(list, 4, 4); // Insert 4 at index 4 -> 0, 1, 2, 3, 4
    assert(value_at(list, 4) == 4);
    assert(back(list) == 4);
    
    destory_list(list);
}

void test_value_at() {
    List *list = new_list();
    push_back(list, 10);
    push_back(list, 20);
    push_back(list, 30);
    
    assert(value_at(list, 0) == 10);
    assert(value_at(list, 1) == 20);
    assert(value_at(list, 2) == 30);
    
    destory_list(list);
}

void test_remove_at() {
    List *list = new_list();
    push_back(list, 1);
    push_back(list, 2);
    push_back(list, 3);
    
    remove_at(list, 1); // Remove 2 -> 1, 3
    assert(size(list) == 2);
    assert(value_at(list, 0) == 1);
    assert(value_at(list, 1) == 3);
    
    remove_at(list, 0); // Remove 1 -> 3
    assert(size(list) == 1);
    assert(front(list) == 3);
    
    remove_at(list, 0); // Remove 3 -> empty
    assert(size(list) == 0);
    assert(is_empty(list));
    
    destory_list(list);
}

void test_remove_value() {
    List *list = new_list();
    push_back(list, 1);
    push_back(list, 2);
    push_back(list, 2);
    push_back(list, 3);
    
    remove_value(list, 2); // Should remove all 2s -> 1, 3
    
    // Note: The current implementation of remove_value might have a bug where it accesses freed memory.
    // If it crashes here, we know why.
    
    assert(size(list) == 2);
    assert(value_at(list, 0) == 1);
    assert(value_at(list, 1) == 3);
    
    remove_value(list, 1); // Remove 1 -> 3
    assert(size(list) == 1);
    assert(front(list) == 3);
    
    remove_value(list, 3); // Remove 3 -> empty
    assert(size(list) == 0);
    assert(is_empty(list));
    
    destory_list(list);
}

void test_reverse() {
    List *list = new_list();
    push_back(list, 1);
    push_back(list, 2);
    push_back(list, 3);
    
    reverse(list); // 3, 2, 1
    
    assert(value_at(list, 0) == 3);
    assert(value_at(list, 1) == 2);
    assert(value_at(list, 2) == 1);
    assert(front(list) == 3);
    assert(back(list) == 1);
    
    destory_list(list);
}
