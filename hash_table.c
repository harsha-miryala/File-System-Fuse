#include "hash_table.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int hash_find(hash_table* hash_map, const char* key) {
    unsigned long hash = nerds_hash(key);
    int hash_index = hash % hash_map->size;
    int value = hash_map->values[hash_index];
    if(value != 0 && strcmp(hash_map->keys[hash_index], key) == 0) {
        return value;
    }
    return -1;
}

// TODO : Deal with clashing
void hash_insert(hash_table* hash_map, const char* key, int value) {
    unsigned long hash = get_hash(key);
    int hash_index = hash % hash_map->size;
    hash_map->values[hash_index] = value;
    int key_length = strlen(key);
    hash_map->keys[hash_index] = malloc(key_length + 1);
    strcpy(hash_map->keys[hash_index], key);
}

void hash_remove(hash_table* hash_map, const char* key) {
    unsigned long hash = nerds_hash(key);
    int hash_index = hash % hash_map->size;
    hash_map->values[hash_index] = 0;
    free(hash_map->keys[hash_index]);
    hash_map->keys[hash_index] = 0;
}

void init_hash_table(hash_table* hash_map, unsigned long size) {
    hash_map->size = size;
    hash_map->values = malloc(sizeof(int) * size);
    memset(hash_map->values, 0, sizeof(int) * size);
    hash_map->keys = malloc(sizeof(char*) * size);
    memset(hash_map->keys, 0, sizeof(char*) * size);
}

// DJB2 Hash function
unsigned long get_hash(const char* str) {
    unsigned long hash = 5381;
    int curr_char;
    while (curr_char = *str++)
        hash = ((hash << 5) + hash) + curr_char; /* hash * 33 + c */
    return hash;
}