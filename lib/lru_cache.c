#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the node structure for doubly linked list
typedef struct Node {
    char* key;
    int value;
    struct Node* prev;
    struct Node* next;
} Node;

// Define the LRU Cache structure
typedef struct LRUCache {
    int capacity;
    int size;
    Node* head;
    Node* tail;
    Node** map; // Hash Map to store the keys and corresponding Node pointers
} LRUCache;

// Initialize a new node with given key and value
Node* newNode(char* key, int value) {
    Node* node = (Node*) malloc(sizeof(Node));
    node->key = strdup(key);
    node->value = value;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

// Initialize the LRU Cache with given capacity
LRUCache* LRUCacheCreate(int capacity) {
    LRUCache* cache = (LRUCache*) malloc(sizeof(LRUCache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->head = NULL;
    cache->tail = NULL;
    cache->map = (Node**) calloc(capacity, sizeof(Node*)); // Initialize hash map with NULL pointers
    return cache;
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
    cache->map[hash(node->key, cache->capacity)] = NULL;
    free(node->key);
    free(node);
    cache->size--;
}

// Get the value associated with the given key from the cache
// Returns -1 if key not found in cache
int LRUCacheGet(LRUCache* cache, char* key) {
    int index = hash(key, cache->capacity);
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
void LRUCachePut(LRUCache* cache, char* key, int value) {
    int index = hash(key, cache->capacity); // this uses djb2 hashing
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
}
