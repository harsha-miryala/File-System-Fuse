#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../include/lru_cache.h"

// FNV-1a constants
#define FNV_PRIME 0x1000193
#define FNV_OFFSET 0x811C9DC5

uint32_t fnv1a_hash(const char* str)
{
    uint32_t hash = FNV_OFFSET;
    size_t len = strlen(str);

    for (size_t i = 0; i < len; ++i)
    {
        hash ^= (uint32_t)str[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

// Initialize a new node with given key and value
Node* newNode(const char* key, int value) {
    Node* node = (Node*) malloc(sizeof(Node));
    node->key = strdup(key);
    node->value = value;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

// Initialize the LRU Cache with given capacity
void LRUCacheCreate(LRUCache* cache, int capacity) {
    cache = (LRUCache*) malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;
    cache->map = (Node**) calloc(capacity, sizeof(Node*)); // Initialize hash map with NULL pointers
}

// Remove the given node from the doubly linked list
void removeNode(LRUCache* cache, Node* node) {
    if (node == cache->head) {
        cache->head = node->next;
    } else {
        node->prev->next = node->next;
    }
    if (node == cache->tail) {
        cache->tail = node->prev;
    } else {
        node->next->prev = node->prev;
    }
}

// Move the given node to the front of the doubly linked list
void moveToFront(LRUCache* cache, Node* node) {
    if (node == cache->head) {
        return;
    }
    removeNode(cache, node);
    node->next = cache->head;
    if (cache->head) {
        cache->head->prev = node;
    }
    cache->head = node;
    node->prev = NULL;
    if (!cache->tail) {
        cache->tail = node;
    }
}

// Free the given node and its key
void freeNode(Node* node) {
    free(node->key);
    free(node);
}

// Free the entire doubly linked list
void freeList(Node* node) {
    while (node) {
        Node* next = node->next;
        freeNode(node);
        node = next;
    }
}

// Free the LRU Cache and its hash map
void LRUCacheFree(LRUCache* cache) {
    freeList(cache->head);
    free(cache->map);
    free(cache);
}

// Remove and free the least recently used node from the cache
void removeLRUNode(LRUCache* cache) {
    Node* node = cache->tail;
    removeNode(cache, node);
    cache->map[fnv1a_hash(node->key)%cache->capacity] = NULL;
    free(node->key);
    free(node);
    cache->size--;
}

bool LRUCacheRemove(LRUCache* cache, const char* key) {
    int index = fnv1a_hash(key) % cache->capacity;
    Node* node = cache->map[index];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            // Move the accessed node to the front of the list
            removeNode(cache, node);
            return true;
        }
        node = node->next;
    }
    // TODO: Potential key not found takes O(N)
    return false;
}

// Get the value associated with the given key from the cache
// Returns -1 if key not found in cache
int LRUCacheGet(LRUCache* cache, const char* key) {
    int index = fnv1a_hash(key) % cache->capacity;
    Node* node = cache->map[index];
    while (node) {
        if (strcmp(node->key, key) == 0) {
            // Move the accessed node to the front of the list
            removeNode(cache, node);
            moveToFront(cache, node);
            return node->value;
        }
        node = node->next;
    }
    // TODO: Potentiall key not found takes O(N)
    return -1;
}

// Put the given key-value pair in the cache
void LRUCachePut(LRUCache* cache, const char* key, int value) {
    int index = fnv1a_hash(key) % cache->capacity;
    Node* node = cache->map[index];
    while (node) {
        if(strcmp(node->key, key) == 0){
            // Update the value of the existing node and move it to the front of the list
            node->value = value;
            removeNode(cache, node);
            moveToFront(cache, node);
            return;
        }
        node = node->next;
    }
    // TODO: Potentiall key not found takes O(N)
    // If key not found, create a new node and add it to the front of the list
    Node* new_node = newNode(key, value);
    moveToFront(cache, new_node);
    cache->map[index] = new_node;
    cache->size++;
    // If cache size exceeds the capacity, remove the least recently used node
    if (cache->size > cache->capacity) {
        removeLRUNode(cache);
    }
    return;
}
