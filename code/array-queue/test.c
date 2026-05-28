#include <stdio.h>
#include <assert.h>
#include "array_queue.h"
#include "array_queue.c"
#include "test.h"

void run_all_tests() {
    test_create_queue();
    test_enqueue();
    test_dequeue();
    test_is_empty();
    test_is_full();
    test_circular_buffer();
}

void test_create_queue() {
    int capacity = 5;
    Queue *queue = create_queue(capacity);
    assert(queue != NULL);
    assert(queue->data != NULL);
    // The implementation allocates capacity + 1
    assert(queue->capcity == capacity + 1);
    assert(queue->head == 0);
    assert(queue->tail == 0);
    assert(is_empty(queue) == true);
    assert(is_full(queue) == false);
    destory_queue(queue);
}

void test_enqueue() {
    int capacity = 3;
    Queue *queue = create_queue(capacity);
    
    enqueue(queue, 10);
    assert(is_empty(queue) == false);
    assert(queue->data[queue->head] == 10);
    
    enqueue(queue, 20);
    assert(queue->data[(queue->head + 1) % queue->capcity] == 20);
    
    enqueue(queue, 30);
    assert(is_full(queue) == true);
    
    destory_queue(queue);
}

void test_dequeue() {
    int capacity = 3;
    Queue *queue = create_queue(capacity);
    
    enqueue(queue, 10);
    enqueue(queue, 20);
    enqueue(queue, 30);
    
    int val = dequeue(queue);
    assert(val == 10);
    assert(is_full(queue) == false);
    
    val = dequeue(queue);
    assert(val == 20);
    
    val = dequeue(queue);
    assert(val == 30);
    assert(is_empty(queue) == true);
    
    destory_queue(queue);
}

void test_is_empty() {
    int capacity = 2;
    Queue *queue = create_queue(capacity);
    assert(is_empty(queue) == true);
    
    enqueue(queue, 10);
    assert(is_empty(queue) == false);
    
    dequeue(queue);
    assert(is_empty(queue) == true);
    
    destory_queue(queue);
}

void test_is_full() {
    int capacity = 2;
    Queue *queue = create_queue(capacity);
    assert(is_full(queue) == false);
    
    enqueue(queue, 10);
    assert(is_full(queue) == false);
    
    enqueue(queue, 20);
    assert(is_full(queue) == true);
    
    dequeue(queue);
    assert(is_full(queue) == false);
    
    destory_queue(queue);
}

void test_circular_buffer() {
    int capacity = 3;
    Queue *queue = create_queue(capacity);
    
    // Fill the queue
    enqueue(queue, 1);
    enqueue(queue, 2);
    enqueue(queue, 3);
    assert(is_full(queue) == true);
    
    // Remove one to make space at the beginning
    int val = dequeue(queue);
    assert(val == 1);
    assert(is_full(queue) == false);
    
    // Add one, which should wrap around to the beginning
    enqueue(queue, 4);
    assert(is_full(queue) == true);
    
    // Check values
    assert(dequeue(queue) == 2);
    assert(dequeue(queue) == 3);
    assert(dequeue(queue) == 4);
    assert(is_empty(queue) == true);
    
    destory_queue(queue);
}
