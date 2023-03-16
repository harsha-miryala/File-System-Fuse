CC=gcc

SHELL = /bin/sh
PKGFLAGS = `pkg-config fuse --cflags --libs`

# CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -D_FILE_OFFSET_BITS=64

# Uncomment line below for disk layer to read from disk
CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -DDISK -D_FILE_OFFSET_BITS=64

# Uncomment line below for more verbose debug info
# CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -DDEBUG -D_FILE_OFFSET_BITS=64

test: basic_api obj/large_file_test obj/extra_large_file_test obj/nested_dir_test

basic_api: obj/open_create_write_test obj/lseek_read_write_test obj/unlink_test

obj/open_create_write_test: basic_api/open_create_write_test.c
	$(CC) -o $@ $^ $(CFLAGS)

obj/lseek_read_write_test: basic_api/lseek_read_write_test.c
	$(CC) -o $@ $^ $(CFLAGS)

obj/unlink_test: basic_api/unlink_test.c
	$(CC) -o $@ $^ $(CFLAGS)

obj/large_file_test: large_file_test.c
	$(CC) -o $@ $^ $(CFLAGS)

obj/extra_large_file_test: extra_large_file_test.c
	$(CC) -o $@ $^ $(CFLAGS)

obj/nested_dir_test: nested_dir_test.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf obj/*