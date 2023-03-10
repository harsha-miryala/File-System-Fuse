CC=gcc

SHELL = /bin/sh
PKGFLAGS = `pkg-config fuse --cflags --libs`

#CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -D_FILE_OFFSET_BITS=64

# Uncomment line below for disk layer to read from disk
CFLAGS = -g  -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -DDISK -D_FILE_OFFSET_BITS=64

#Uncomment line below for more verbose debug info
# CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -DDEBUG -D_FILE_OFFSET_BITS=64

init: lib/fuse_layer.c lib/file_layer.c lib/block_layer.c lib/disk_layer.c lib/lru_cache.c
	$(CC) -o $@ $^ $(CFLAGS)

test: obj/block_layer_test obj/disk_layer_test obj/file_layer_test obj/lru_cache_test

obj/file_layer_test: test/layers/file_layer_test.c lib/file_layer.c lib/disk_layer.c lib/block_layer.c lib/lru_cache.c
	$(CC) -o $@ $^ $(CFLAGS)

obj/block_layer_test: test/layers/block_layer_test.c lib/disk_layer.c lib/block_layer.c 
	$(CC) -o $@ $^ $(CFLAGS)

obj/disk_layer_test: test/layers/disk_layer_test.c lib/disk_layer.c 
	$(CC) -o $@ $^ $(CFLAGS)

obj/lru_cache_test: test/layers/lru_cache_test.c lib/lru_cache.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf obj/*
