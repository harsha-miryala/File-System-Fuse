# File System Implementation using FUSE

## Layers --

### Disk layer

### Block layer

### File layer

### Fuse layer

## Utils --

### Hash Table / LRU Cache (Yet to finalize)

## Tests

### Unit Tests

#### Disk layer

##### gcc -Wall test/layers/disk_layer_test.c lib/disk_layer.c -o obj/disk_test
##### ./obj/disk_test

#### Block layer

##### gcc -Wall test/layers/block_layer_test.c lib/block_layer.c lib/disk_layer.c -o obj/block_test
##### ./obj/block_test

#### File layer

##### gcc -Wall test/layers/file_layer_test.c lib/file_layer.c lib/block_layer.c lib/disk_layer.c lib/hash_table.c -o obj/file_test
##### ./obj/file_test