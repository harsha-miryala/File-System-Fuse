#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

struct node {
    char* key;
    int value;
    struct node* prev;
    struct node* next;
};

struct lru_cache {
    struct node* head;
    struct node* tail;
    int size;
    int capacity;
    struct node** map;
};

// struct lru_cache* create_cache(int capacity);
void create_cache(struct lru_cache* cache, int capacity);

bool pop_cache(struct lru_cache* cache, const char* key);

void set_cache(struct lru_cache* cache, const char* key, int value);

int get_cache(struct lru_cache* cache, const char* key);

void free_cache(struct lru_cache* cache);
