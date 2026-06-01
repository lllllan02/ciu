#include "test.h"
#include "hash_table.c"
#include "hash_table.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void run_all_tests() {
    printf("test_create_table\n");
    test_create_table();
    printf("test_hash\n");
    test_hash();
    printf("test_add_and_get\n");
    test_add_and_get();
    printf("test_exists\n");
    test_exists();
    printf("test_add_update\n");
    test_add_update();
    printf("test_remove\n");
    test_remove();
    printf("test_collision\n");
    test_collision();
    printf("test_remove_chain\n");
    test_remove_chain();
}

void test_create_table() {
    HashTable* table = create_table(8);
    assert(table->size == 8);
    for (int i = 0; i < table->size; i++) {
        assert(table->data[i] == NULL);
    }
    destroy_table(table);
}

void test_hash() {
    const int m = 16;
    assert(hash("a", m) >= 0 && hash("a", m) < m);
    assert(hash("hello", m) >= 0 && hash("hello", m) < m);
    assert(hash("", m) >= 0 && hash("", m) < m);
    assert(hash("a", m) == hash("a", m));
}

void test_add_and_get() {
    HashTable* table = create_table(8);
    add(table, "name", "alice");
    add(table, "city", "beijing");

    assert(strcmp(get(table, "name"), "alice") == 0);
    assert(strcmp(get(table, "city"), "beijing") == 0);
    assert(get(table, "missing") == NULL);

    destroy_table(table);
}

void test_exists() {
    HashTable* table = create_table(8);
    assert(!exists(table, "key"));
    add(table, "key", "value");
    assert(exists(table, "key"));
    assert(!exists(table, "other"));
    destroy_table(table);
}

void test_add_update() {
    HashTable* table = create_table(8);
    add(table, "count", "1");
    add(table, "count", "2");

    assert(strcmp(get(table, "count"), "2") == 0);
    assert(find(table, "count") != NULL);

    destroy_table(table);
}

void test_remove() {
    HashTable* table = create_table(8);
    add(table, "a", "1");
    add(table, "b", "2");

    remove_key(table, "a");
    assert(!exists(table, "a"));
    assert(get(table, "a") == NULL);
    assert(exists(table, "b"));
    assert(strcmp(get(table, "b"), "2") == 0);

    remove_key(table, "b");
    assert(!exists(table, "b"));

    destroy_table(table);
}

void test_collision() {
    HashTable* table = create_table(1);
    add(table, "foo", "111");
    add(table, "bar", "222");
    add(table, "baz", "333");

    assert(strcmp(get(table, "foo"), "111") == 0);
    assert(strcmp(get(table, "bar"), "222") == 0);
    assert(strcmp(get(table, "baz"), "333") == 0);

    destroy_table(table);
}

void test_remove_chain() {
    HashTable* table = create_table(1);
    add(table, "a", "1");
    add(table, "b", "2");
    add(table, "c", "3");

    remove_key(table, "b");
    assert(!exists(table, "b"));
    assert(strcmp(get(table, "a"), "1") == 0);
    assert(strcmp(get(table, "c"), "3") == 0);

    remove_key(table, "a");
    assert(!exists(table, "a"));
    assert(strcmp(get(table, "c"), "3") == 0);

    remove_key(table, "c");
    assert(!exists(table, "c"));

    destroy_table(table);
}
