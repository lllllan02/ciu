#include "hash_table.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

HashTable* create_table(int size) {
    HashTable* fresh = malloc(sizeof(HashTable));
    assert(fresh != NULL);

    fresh->data = calloc((size_t)size, sizeof(KeyValue*));
    assert(fresh->data != NULL);

    fresh->size = size;

    return fresh;
}

void destroy_table(HashTable* table) {
    for (int i = 0; i < table->size; i++) {
        KeyValue* kv = table->data[i];
        for ( ; kv != NULL; ) {
            KeyValue* next = kv->next;
            free(kv->key);
            free(kv->value);
            free(kv);
            kv = next;
        }
    }

    free(table->data);
    free(table);
}

int hash(const char* key, const int m) {
    int hash = 0;

    for (int i = 0; key[i] != '\0'; ++i) {
        hash = hash * 31 + key[i];
    }
  
    return abs(hash % m);
}

KeyValue* find(const HashTable* table, const char* key) {
    int index = hash(key, table->size);

    KeyValue* kv = table->data[index];
    for ( ; kv != NULL; ) {
        if (strcmp(kv->key, key) == 0) {
            return kv;
        }

        kv = kv->next;
    }

    return NULL;
}

char* get(const HashTable* table, const char* key) {
    KeyValue* kv = find(table, key);
    if (kv != NULL) {
        return kv->value;
    }
    return NULL;
}

bool exists(const HashTable* table, const char* key) {
    KeyValue* kv = find(table, key);
    return kv != NULL;
}

void add(HashTable* table, const char* key, const char* value) {
    int index = hash(key, table->size);

    KeyValue* kv = find(table, key);
    if (kv != NULL) {
        free(kv->value);
        kv->value = strdup(value);
        return;
    }

    kv = malloc(sizeof(KeyValue));
    kv->key = strdup(key);
    kv->value = strdup(value);
    kv->next = table->data[index];
    table->data[index] = kv;
}

void remove_key(HashTable* table, const char* key) {
    int index = hash(key, table->size);

    KeyValue* pre = NULL;
    KeyValue* kv = table->data[index];
    for (; kv != NULL; kv = kv->next) {
        if (strcmp(kv->key, key) == 0) {
            if (pre == NULL) {
                table->data[index] = kv->next;
            } else {
                pre->next = kv->next;
            }

            free(kv->key);
            free(kv->value);
            free(kv);
            return;
        }

        pre = kv;
    }
}