CC=gcc

SHELL = /bin/sh
PKGFLAGS = `pkg-config fuse --cflags --libs`

CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -D_FILE_OFFSET_BITS=64

# Uncomment line below for disk layer to read from disk
# CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -DDISK -D_FILE_OFFSET_BITS=64

# Uncomment line below for more verbose debug info
# CFLAGS = -g -Og -I./include -Wall -std=gnu11 $(PKGFLAGS) -DDEBUG -D_FILE_OFFSET_BITS=64

test: basic_api large_file_test

basic_api: open_create_write_test lseek_read_write_test unlink_test

open_create_write_test: basic_api/open_create_write_test.c helpers.c
	$(CC) -o $@ $^ $(CFLAGS)

lseek_read_write_test: basic_api/lseek_read_write_test.c helpers.c
	$(CC) -o $@ $^ $(CFLAGS)

unlink_test: basic_api/unlink_test.c helpers.c
	$(CC) -o $@ $^ $(CFLAGS)

large_file_test: large_file_test.c helpers.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf *.o