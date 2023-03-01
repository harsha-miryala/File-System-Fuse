#ifndef __DISK_LAYER_H__
#define __DISK_LAYER_H__

#include <stdlib.h>
#include <stdio.h>

//volume device has to be defined here
//currently implementing in memory FS

#define FS_SIZE 104857600
//100 MB
#define BLOCK_SIZE 4096
//4 KB
#define BLOCK_COUNT (FS_SIZE/BLOCK_SIZE)
// Total number of blocks ... in this case - 25000 (less than max size of int)

bool alloc_memory();
bool dealloc_memory();
bool read_block(int block_id, char *buffer);
bool write_block(int block_id, char *buffer);
void free_memory(void *ptr);
#endif
