#include <stdio.h>
#include <assert.h>
#include "linked_list_queue.h"
#include "linked_list_queue.c"
#include "test.h"

void run_all_tests() {
    test_new_queue();
    test_enqueue();
    test_dequeue();
    test_is_empty();
}

void test_new_queue() {
    Queue *queue = new_queue();
    assert(queue != NULL);
    assert(queue->head == NULL);
    assert(queue->tail == NULL);
    assert(is_empty(queue) == true);
    destory_queue(queue);
}

void test_enqueue() {
    Queue *queue = new_queue();
    enqueue(queue, 10);
    assert(is_empty(queue) == false);
    assert(queue->head->data == 10);
    assert(queue->tail->data == 10);
    
    enqueue(queue, 20);
    assert(queue->head->data == 10);
    assert(queue->tail->data == 20);
    
    enqueue(queue, 30);
    assert(queue->head->data == 10);
    assert(queue->head->next->data == 20);
    assert(queue->tail->data == 30);
    
    destory_queue(queue);
}

void test_dequeue() {
    Queue *queue = new_queue();
    enqueue(queue, 10);
    enqueue(queue, 20);
    enqueue(queue, 30);
    
    int val = dequeue(queue);
    assert(val == 10);
    assert(queue->head->data == 20);
    
    val = dequeue(queue);
    assert(val == 20);
    assert(queue->head->data == 30);
    
    val = dequeue(queue);
    assert(val == 30);
    assert(is_empty(queue) == true);
    
    destory_queue(queue);
}

void test_is_empty() {
    Queue *queue = new_queue();
    assert(is_empty(queue) == true);
    
    enqueue(queue, 10);
    assert(is_empty(queue) == false);
    
    dequeue(queue);
    assert(is_empty(queue) == true);
    
    destory_queue(queue);
}
