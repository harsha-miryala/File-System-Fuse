#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "../../include/disk_layer.h"

int main()
{
    char *block_buffer = (char *)malloc(BLOCK_SIZE);
    if(!alloc_memory())
    {
        printf("DISK_LAYER_TEST 1: Failed to allocate memory\n\n");
        return -1;
    }
    printf("DISK_LAYER_TEST 1: alloc_memory passed\n\n");

    // Read from an invalid block
    if (read_block(-1, block_buffer))
    {
        printf("DISK_LAYER_TEST 2: read_block (with invalid block-id) failed\n\n");
        return -1;
    }
    printf("DISK_LAYER_TEST 2: read_block (with invalid block-id) passed\n\n");

    // Read from block 10 -> should be all zeros
    if(!read_block(10, block_buffer))
    {
        printf("DISK_LAYER_TEST 3: read_block (zeroed out check) failed\n\n");
        return -1;
    }

    // Check whether everything is zero
    for (ssize_t i = 0; i < BLOCK_SIZE; i++)
    {
        if (block_buffer[i] != 0)
        {
            printf("DISK_LAYER_TEST 3: read_block (zeroed out check) failed : invalid content\n\n");
            return -1;
        }
    }
    printf("DISK_LAYER_TEST 3: read_block (zeroed out check) passed\n\n");

    // Write to an invalid block
    if (write_block(-1, block_buffer))
    {
        printf("DISK_LAYER_TEST 4: write_block (with invalid block-id) failed\n\n");
        return -1;
    }
    printf("DISK_LAYER_TEST 4: write_block (with invalid block-id) passed\n\n");

    // Write into a valid buffer
    char* test_str = "A TEST MESSAGE TO TEST THE CORRECTNESS OF DISK LAYER";
    memcpy(block_buffer, test_str, strlen(test_str));
    if(!write_block(19, block_buffer))
    {
        printf("DISK_LAYER_TEST 5: write_block failed\n\n");
        return -1;
    }
    printf("DISK_LAYER_TEST 5: write_block passed\n\n");

    // Read from the same block
    if (!read_block(19, block_buffer))
    {
        printf("DISK_LAYER_TEST 6: read_block failed\n\n");
        return -1;
    }
    if(strcmp(block_buffer, test_str) == 0) {
        printf("DISK_LAYER_TEST 6: read_block passed\n\n");
    }else {
        printf("DISK_LAYER_TEST 6: read_block failed\n\n");
        return -1;
    }

    // Write to all blocks and read from them
    for(ssize_t i=0; i<BLOCK_COUNT; i++)
    {
        memcpy(block_buffer, test_str, strlen(test_str));
        if(!write_block(i, block_buffer))
        {
            printf("DISK_LAYER_TEST 7: Write to block %ld failed\n\n", i);
            return -1;
        }
        if(!read_block(i, block_buffer))
        {
            printf("DISK_LAYER_TEST 7: Read from block %ld failed\n\n", i);
            return -1;
        }
        if(strcmp(block_buffer, test_str) != 0)
        {
            printf("DISK_LAYER_TEST 7: Compare data of block %ld failed\n\n", i);
        }
    }
    printf("DISK_LAYER_TEST 7: Write, read and data compare for all blocks passed\n");
    return 0;
}
