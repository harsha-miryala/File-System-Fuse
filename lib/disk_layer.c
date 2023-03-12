#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <fcntl.h>
#include "../include/disk_layer.h"

// global vars
#ifdef DISK
static size_t m_ptr;
#else
static char* m_ptr;
#endif

bool alloc_memory(){
#ifdef DISK
    // TODO : Instead of re-starting everything, find out the first address
    printf("Location where FS is mounted - %s\n", BLOCK_DEVICE);
    m_ptr = open(BLOCK_DEVICE, O_RDWR);
    // printf("File Desc of directory - %d\n", dirfd(opendir(BLOCK_DEVICE)));
    if(m_ptr==-1){
        return false;
    }
    printf("address is %p \n", &m_ptr);
    char buff[BLOCK_SIZE];
    memset(&buff, 0, BLOCK_SIZE);
    for(int i=0; i<BLOCK_COUNT; i++){
        if(!write_block(i, buff)){
            return false;
        }
    }
#else
    m_ptr = (char *) malloc(FS_SIZE);
    if(!m_ptr){
        printf("Error allocating file system memory for disk \n");
        return false;
    }
    memset(m_ptr, 0, FS_SIZE);
    printf("Succesfully allocated memory for disk \n");
	printf("address is %p \n", &m_ptr);
#endif
	//printf("memory successfully allocated\n");
	return true;
}

bool dealloc_memory(){
#ifdef DISK
   printf("memory deallocation starting\n");
    if(close(m_ptr)!=0){
        return false;
    }
#else
    if(!m_ptr){
        printf("No disk memory to deallocate \n");
        return false;
    }
    free_memory(m_ptr);
#endif
    printf("Succesfully de-allocated memory for disk \n");
    return true;
}

bool read_block(int block_id, char *buffer){
    if(!buffer){
        return false;
    }
    if(block_id<0 || block_id >= BLOCK_COUNT){
        printf("Invalid read of block index - out of range\n");
        return false;
    }
#ifdef DISK
    off_t offset = (unsigned long) BLOCK_SIZE * block_id;
    off_t status = lseek(m_ptr, offset, SEEK_SET);
    if(status == -1){
        return false;
    }
    if(read(m_ptr, buffer, BLOCK_SIZE) != BLOCK_SIZE){
        return false;
    }
#else
    int offset = BLOCK_SIZE * block_id;
    memcpy(buffer, m_ptr+offset, BLOCK_SIZE);
#endif
    //printf("successfully read block %d\n",block_id);
    return true;
}

bool write_block(int block_id, char *buffer){
    if(!buffer){
        return false;
    }
    if(block_id<0 || block_id >= BLOCK_COUNT){
        printf("Invalid write for block index - out of range\n");
        return false;
    }
#ifdef DISK
    off_t offset = (unsigned long) BLOCK_SIZE * block_id;
    off_t status = lseek(m_ptr, offset, SEEK_SET);
    if(status == -1){
        return false;
    }
    if(write(m_ptr, buffer, BLOCK_SIZE) != BLOCK_SIZE){
        return false;
    }
#else
    int offset = BLOCK_SIZE * block_id;
    memcpy(m_ptr+offset, buffer, BLOCK_SIZE);
#endif
    //printf("successfully wrote block %d\n", block_id);
    return true;
}

void free_memory(void *ptr){
    if(ptr!=NULL){
        free(ptr);
    }
}
