#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

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

void LRUCacheCreate(LRUCache* cache, int capacity);

bool LRUCacheRemove(LRUCache* cache, const char* key);

int LRUCacheGet(LRUCache* cache, const char* key);

void LRUCachePut(LRUCache* cache, const char* key, int value);
