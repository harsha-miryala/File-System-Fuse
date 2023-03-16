#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

struct node {
    char* key;
    ssize_t value;
    struct node* prev;
    struct node* next;
};

struct lru_cache {
    struct node* head;
    struct node* tail;
    ssize_t size;
    ssize_t capacity;
    struct node** map;
};

void create_cache(struct lru_cache* cache, ssize_t capacity);

bool pop_cache(struct lru_cache* cache, const char* key);

void set_cache(struct lru_cache* cache, const char* key, ssize_t value);

ssize_t get_cache(struct lru_cache* cache, const char* key);

void free_cache(struct lru_cache* cache);
