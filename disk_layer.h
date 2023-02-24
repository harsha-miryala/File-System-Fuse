#ifndef __DISK_LAYER_H__
#define __DISK_LAYER_H__

#include <stdlib.h>
#include <stdio.h>

//volume device has to be defined here
//currently implementing in memory FS

#define FS_SIZE 104857600 //100 MB
#define BLOCK_SIZE 1024
#define BLOCK_COUNT (FS_SIZE/BLOCK_SIZE)

int alloc_memory();
int dealloc_memory();
int read_block(int block_id, char *buffer);
int write_block(int block_id, char *buffer);
void free_memory(void *ptr);
#endif
