#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct {
    char** keys;
    int* values;
    unsigned long size;
} hash_table;

// gives the corresponding value for a given string
int hash_find(hash_table* hash_map, const char* key);

void hash_insert(hash_table* hash_map, const char* key, int value);

void hash_remove(hash_table* hash_map, const char *key);

void init_hash_table(hash_table* hash_map, unsigned long size);

unsigned long get_hash(const char* str);

#endif