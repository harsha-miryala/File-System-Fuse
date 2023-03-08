# File System Implementation using FUSE

## Packages to download

### sudo apt-get install libfuse-dev
### sudo apt-get install fuse fuse3
### sudo apt-get install make automake autoconf

## Layers --

### Disk layer

### Block layer

### File layer

### Fuse layer

## Utils --

### Hash Table / LRU Cache (Yet to finalize)

## Tests

### Unit Tests

#### LRU Cache layer

##### gcc -Wall test/layers/lru_cache_test.c lib/lru_cache.c -o obj/lru_cache
##### ./obj/lru_cache_test

#### Disk layer

##### gcc -Wall test/layers/disk_layer_test.c lib/disk_layer.c -o obj/disk_test
##### ./obj/disk_test

#### Block layer

##### gcc -Wall test/layers/block_layer_test.c lib/block_layer.c lib/disk_layer.c -o obj/block_test
##### ./obj/block_test

#### File layer

##### gcc -Wall test/layers/file_layer_test.c lib/file_layer.c lib/block_layer.c lib/disk_layer.c lib/lru_cache.c -o obj/file_test
##### ./obj/file_test