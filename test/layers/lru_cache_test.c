#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../include/lru_cache.h"

int main() {
    // Create a cache with capacity 3
    ssize_t test_size = 3;
    static struct lru_cache cache;
    create_cache(&cache, test_size);
    assert(cache.capacity==test_size);
    assert(cache.size==0);
    printf("LRU Cache Creation Successful\n");
    // Set values for keys "A", "B", and "C"
    set_cache(&cache, "A", 1);
    set_cache(&cache, "B", 2);
    set_cache(&cache, "C", 3);
    // Get values for keys "A", "B", and "C"
    assert(get_cache(&cache, "A")==1); // Output: 1
    assert(get_cache(&cache, "B")==2); // Output: 2
    assert(get_cache(&cache, "C")==3); // Output: 3
    printf("LRU Cache Read, Write Successful\n");
    // Set a value for key "D", causing key "A" to be removed from the cache
    set_cache(&cache, "D", 4);
    // Get values for keys "A", "B", "C", and "D"
    assert(get_cache(&cache, "A")==-1); // Output: -1
    assert(get_cache(&cache, "B")==2); // Output: 2
    assert(get_cache(&cache, "C")==3); // Output: 3
    assert(get_cache(&cache, "D")==4); // Output: 4
    printf("LRU Cache least recently used query remove Successful\n");
    // Set a value for key "E", causing key "B" to be removed from the cache
    set_cache(&cache, "E", 5);
    // Remove D from cache
    pop_cache(&cache, "D");
    // Get values for keys "B", "C", "D", and "E"
    assert(get_cache(&cache, "B")==-1); // Output: -1
    assert(get_cache(&cache, "C")==3); // Output: 3
    assert(get_cache(&cache, "D")==-1); // Output: -1
    assert(get_cache(&cache, "E")==5); // Output: 5
    printf("LRU Cache pop cache Successful\n");
    // Check for null values
    set_cache(&cache, NULL, 6); // Should not change cache
    assert(get_cache(&cache, NULL)==-1); // Output: -1
    printf("LRU Cache corner case Successful\n");
    printf("All test cases for LRU Cache Passed!!\n");
    return 0;
}