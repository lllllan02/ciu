#ifndef PROJECT_HASH_TABLE_H
#define PROJECT_HASH_TABLE_H

#include <stdbool.h>

typedef struct KeyValue {
    char* key;
    char* value;
    struct KeyValue* next;
} KeyValue;

typedef struct HashTable {
    KeyValue** data;
    int size;
} HashTable;

// 创建一个哈希表
HashTable* create_table(int size);

// 释放哈希表
void destroy_table(HashTable* table);

// 哈希函数
int hash(const char* key, const int m);

// 查找哈希表中某个键对应的键值对
KeyValue* find(const HashTable* table, const char* key);

// 获取哈希表中某个键对应的值
char* get(const HashTable* table, const char* key);

// 检查哈希表中是否存在某个键
bool exists(const HashTable* table, const char* key);

// 添加键值对到哈希表
void add(HashTable* table, const char* key, const char* value);

// 删除哈希表中某个键的值
void remove_key(HashTable* table, const char* key);

#endif