CC=gcc

SHELL = /bin/sh
PKGFLAGS = `pkg-config fuse --cflags --libs`

CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -D_FILE_OFFSET_BITS=64

# Uncomment line below for disk layer to read from disk
# CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -DDISK -D_FILE_OFFSET_BITS=64

# Uncomment line below for more verbose debug info
#CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -DDEBUG -D_FILE_OFFSET_BITS=64

init: lib/fuse_layer.c lib/file_layer.c lib/block_layer.c lib/disk_layer.c lib/hash_table.c
	$(CC) -o $@ $^ $(CFLAGS)

# test: obj/block_layer_test obj/disk_layer_test obj/file_layer_test obj/fuse_layer_test

# obj/fuse_layer_test: test/fuse_layer_test.c lib/fuse_layer.c lib/file_layer.c lib/disk_layer.c lib/block_layer.c
# 	$(CC) -o $@ $^ $(CFLAGS)

# obj/file_layer_test: test/file_layer_test.c lib/file_layer.c lib/disk_layer.c lib/block_layer.c lib/hash_table.c
# 	$(CC) -o $@ $^ $(CFLAGS)

# obj/block_layer_test: test/block_layer_test.c lib/disk_layer.c lib/block_layer.c 
# 	$(CC) -o $@ $^ $(CFLAGS)

obj/disk_layer_test: test/disk_layer_test.c lib/disk_layer.c 
	$(CC) -o $@ $^ $(CFLAGS)

# obj/hash_table_test: test/hash_table_test.c lib/hash_table.c
# 	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf obj/*
