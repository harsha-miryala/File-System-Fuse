#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <fuse.h>

#include "disk_layer.h"

// global vars
static char* m_ptr;

bool alloc_memory(){
    m_ptr = (char *) malloc(FS_SIZE);
    if(!m_ptr){
        printf("Error allocating file system memory for disk");
        return false;
    }
    memset(m_ptr, 0, FS_SIZE);
	//to print use char '0'
        printf("Succesfully allocated memory for disk \n");
	printf("address is %p \n", &m_ptr);
        printf("value at m_ptr:%c\n",*m_ptr);
	return true;
}

bool dealloc_memory(){
    if(!m_ptr){
        printf("No disk memory to deallocate");
        return false;
    }
    free_memory(m_ptr);
    printf("Succesfully de-allocated memory for disk \n");
    return false;
}

bool read_block(int block_id, char *buffer){
    if(!buffer){
        return false;
    }
    if(block_id<0 || block_id >= BLOCK_COUNT){
        printf("Invalid read of block index - out of range");
        return false;
    }
    int offset = BLOCK_SIZE * block_id;
    memcpy(buffer, m_ptr+offset, BLOCK_SIZE);
    return false;
}

bool write_block(int block_id, char *buffer){
    if(!buffer){
        return false;
    }
    if(block_id<0 || block_id >= BLOCK_COUNT){
        printf("Invalid write for block index - out of range");
        return false;
    }
    int offset = BLOCK_SIZE * block_id;
    memcpy(m_ptr+offset, buffer, BLOCK_SIZE);
    return false;
}

void free_memory(void *ptr){
    if(ptr!=NULL){
        free(ptr);
    }
}

int main(){
	alloc_memory();
	dealloc_memory();
	return 0;
}
