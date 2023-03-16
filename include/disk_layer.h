#ifndef __DISK_LAYER_H__
#define __DISK_LAYER_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//volume device has to be defined here
#ifdef DISK
#define BLOCK_DEVICE "/dev/vdc"
// #define FS_SIZE 1073741824 // 1GB - taking 2 sec
// #define FS_SIZE 10737418240 // 10GB - taking 1 min
#define FS_SIZE 32212254720 // 30GB - taking 4:30 min
#else
//currently implementing in memory FS
#define FS_SIZE 104857600
//100 MB
#endif

#define BLOCK_SIZE 4096
//4 KB
#define BLOCK_COUNT (FS_SIZE/BLOCK_SIZE)

// this allocated memory, true/false for success
bool alloc_memory();
// deallocates memory, true/false
bool dealloc_memory();
// reads data from block_id into buffer
bool read_block(ssize_t block_id, char *buffer);
// writes data into block_id from buffer
bool write_block(ssize_t block_id, char *buffer);
// de-reference the pointer to nothing
void free_memory(void *ptr);

#endif
