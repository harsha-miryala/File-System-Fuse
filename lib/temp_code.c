#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

unsigned long djb2_hash(const char *str)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

struct lru_cache* create_cache(int capacity) {
    struct lru_cache* cache = (struct lru_cache*) malloc(sizeof(struct lru_cache));
    cache->head = NULL;
    cache->tail = NULL;
    cache->size = 0;
    cache->capacity = capacity;
    cache->map = (struct node**) calloc(capacity, sizeof(struct node**));
    return cache;
}

// Free the given node and its key
void free_node(struct node* node) {
    free(node->key);
    free(node);
}

// Free the entire doubly linked list
void free_list(struct node* node) {
    while (node) {
        struct node* next = node->next;
        free_node(node);
        node = next;
    }
}

// Free the LRU Cache and its hash map
void free_cache(struct lru_cache* cache) {
    free_list(cache->head);
    free(cache->map);
    free(cache);
}

void delete_node(struct lru_cache* cache, struct node* node) {
    if (node == cache->head) {
        cache->head = node->next;
    } else if (node == cache->tail) {
        cache->tail = node->prev;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    free_node(node);
}

bool pop_cache(struct lru_cache* cache, const char* key){
    // Check for null values
    if (cache == NULL || key == NULL) {
        return -1;
    }
    // Find the node with the given key
    unsigned long hash = djb2_hash(key);
    int hash_index = hash % cache->capacity;
    struct node* curr = cache->map[hash_index];
    while (curr != NULL) {
        if (strcmp(curr->key, key) == 0) {
            // remove node from the cache
            delete_node(cache, curr);
            cache->map[hash_index] = NULL;
            return true;
        }
        curr = curr->next;
    }
    return false; // If the key is not found in the cache
}

void set_cache(struct lru_cache* cache, const char* key, int value) {
    // Check for null values
    if (cache == NULL || key == NULL) {
        return;
    }
    // Check if key already exists in cache
    unsigned long hash = djb2_hash(key);
    int hash_index = hash % cache->capacity;
    struct node* curr = cache->map[hash_index];
    while (curr != NULL) {
        if (strcmp(curr->key, key) == 0) {
            curr->value = value;
            // Move the node to the front of the cache
            if (curr != cache->head) {
                curr->prev->next = curr->next;
                if (curr == cache->tail) {
                    cache->tail = curr->prev;
                } else {
                    curr->next->prev = curr->prev;
                }
                curr->prev = NULL;
                curr->next = cache->head;
                cache->head->prev = curr;
                cache->head = curr;
            }
            return;
        }
        curr = curr->next;
    }
    // Create a new node and add it to the front of the cache
    struct node* node = (struct node*) malloc(sizeof(struct node));
    node->key = (char*) malloc(strlen(key) + 1);
    strcpy(node->key, key);
    node->value = value;
    node->prev = NULL;
    node->next = cache->head;
    if (cache->head != NULL) {
        cache->head->prev = node;
    }
    cache->head = node;
    if (cache->tail == NULL) {
        cache->tail = node;
    }
    cache->size++;
    // If the cache is full, remove the least recently used node
    if (cache->size > cache->capacity) {
        delete_node(cache, cache->tail);
        cache->size--;
    }
    cache->map[hash_index] = node;
}

int get_cache(struct lru_cache* cache, const char* key){
    // Check for null values
    if (cache == NULL || key == NULL) {
        return -1;
    }
    // Find the node with the given key
    unsigned long hash = djb2_hash(key);
    int hash_index = hash % cache->capacity;
    struct node* curr = cache->map[hash_index];
    while (curr != NULL) {
        if (strcmp(curr->key, key) == 0) {
            // Move the node to the front of the cache
            if (curr != cache->head) {
                curr->prev->next = curr->next;
                if (curr == cache->tail) {
                    cache->tail = curr->prev;
                } else {
                    curr->next->prev = curr->prev;
                }
                curr->prev = NULL;
                curr->next = cache->head;
                cache->head->prev = curr;
                cache->head = curr;
            }
            return curr->value;
        }
        curr = curr->next;
    }
    return -1; // If the key is not found in the cache
}
