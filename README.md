# File System Implementation using FUSE

## Packages to download

### sudo apt-get install libfuse-dev
### sudo apt-get install fuse fuse3
### sudo apt-get install make automake autoconf

## How to mount external partition for fuse
- Add the block device name in disk_layer.h
- ``` sudo su ``` to login as root user
- Open two terminals and create a directory to intercept sys calls through fuse in that area (this is to unmount if already mounted) - ``` fusermount -u mpoint ```
- In one of the terminals launch the fuse layer instance through - ``` make init ``` and ``` ./init -d mpoint ```
- In the other terminal, cd into the mpoint directory and anything you do in through that dir will be intercepted by fuse
- Note: mount directory should be empty before launching fuse 
- You can copy the test cases using cp into the mpoint e.g. ``` cp -r ../../File-System-Fuse/test/fuse . ```

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

##### gcc -Wall test/layers/file_layer_test.c lib/file_layer.c lib/block_layer.c lib/disk_layer.c lib/lru_cache.c -o obj/file_test `pkg-config fuse --cflags --libs`
##### ./obj/file_test