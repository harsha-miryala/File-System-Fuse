#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "../../include/disk_layer.h"

int main()
{
    char *buff = (char *)malloc(BLOCK_SIZE);
    if(!alloc_memory())
    {
        printf("Alloc Memory failed\n\n");
        return -1;
    }
    printf("DISK_LAYER_TEST 1: alloc_memory passed\n\n");

    //read from invalid block
    if (read_block(-1 , buff))
    {
        printf("DISK_LAYER_TEST 2: read_block (with invalid block-id) failed\n\n");
        return -1;
    }
    printf("DISK_LAYER_TEST 2: read_block (with invalid block-id) passed\n\n");

    //read from block x -> should be 0
    if(!read_block(10, buff))
    {
        printf("DISK_LAYER_TEST 3: read_block (zeroed out check) failed\n\n");
        return -1;
    }

    // checking whether everything is zero
    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        if (buff[i] != 0)
        {
            printf("DISK_LAYER_TEST 3: read_block (zeroed out check) failed : invalid content\n\n");
            return -1;
        }
    }
    printf("DISK_LAYER_TEST 3: read_block (zeroed out check) passed\n\n");

    //write to invalid block
    if (!write_block(-1, buff))
    {
        printf("DISK_LAYER_TEST 4: write_block (with invalid block-id) failed\n\n");
        return -1;
    }
    printf("DISK_LAYER_TEST 4: write_block (with invalid block-id) passed\n\n");

    //write into a valid buffer
    char* test_str = "A TEST MESSAGE TO TEST THE CORRECTNESS OF DISK LAYER";
    memcpy(buff, test_str, strlen(test_str));
    if(!write_block(19, buff))
    {
        printf("DISK_LAYER_TEST 5: write_block failed\n\n");
        return -1;
    }
    printf("DISK_LAYER_TEST 5: write_block passed\n\n");

    //read from the same block
    if (!read_block(19, buff))
    {
        printf("DISK_LAYER_TEST 6: read_block failed\n\n");
        return -1;
    }
    if(strcmp(buff, test_str) == 0) {
        printf("DISK_LAYER_TEST 6: read_block passed\n\n");
    }else {
        printf("DISK_LAYER_TEST 6: read_block failed\n\n");
        return -1;
    }

    // write to all blocks and correspondingly read from it
    for(int i=0;i<BLOCK_COUNT;i++)
    {
        memcpy(buff, test_str, strlen(test_str));
        if(!write_block(i, buff))
        {
            printf("DISK_LAYER_TEST 7: all_block write to %d block failed\n\n", i);
            return -1;
        }
        if(!read_block(i, buff))
        {
            printf("DISK_LAYER_TEST 7: all_block read from %d block failed\n\n", i);
            return -1;
        }
        if(strcmp(buff, test_str) != 0)
        {
            printf("DISK_LAYER_TEST 7: all_block compare data %d block failed\n\n", i);
        }
    }
    printf("DISK_LAYER_TEST 7: all block write, read, data compare passed\n");
    return 0;
}
