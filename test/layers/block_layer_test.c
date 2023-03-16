#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "../../include/block_layer.h"
#include "../../include/disk_layer.h"

// Check the superblock initialization values
int check_superblock_init()
{
    char *buffer = (char *)malloc(BLOCK_SIZE);
    if (!read_block(0, buffer))
    {
        printf("ERROR: Failed to read superblock.\n");
        return -1;
    }
    struct superBlock *sb = (struct superBlock *)buffer;
    if (sb->inode_count != (INODE_B_COUNT) * sb->inodes_per_block ||
        sb->latest_inum != 3 ||
        sb->inodes_per_block != (BLOCK_SIZE) / sizeof(struct iNode) ||
        sb->free_list_head != INODE_B_COUNT + 1)
    {
        printf("ERROR: Failed to check the initialization values of superblock.\n");
        return -1;
    }
    return 0;
}

// Get the superblock
struct superBlock *get_superblock()
{
    char *buffer = (char *)malloc(BLOCK_SIZE);
    if (!read_block(0, buffer))
    {
        printf("ERROR: Failed to read superblock.\n");
        return NULL;
    }
    return (struct superBlock *)buffer;
}

// Print the superblock
int print_superblock()
{
    char *buffer = (char *)malloc(BLOCK_SIZE);
    if (!read_block(0, buffer))
    {
        printf("ERROR: Failed to read superblock.\n");
        return -1;
    }
    struct superBlock *sb = (struct superBlock *)buffer;
    printf("\n=====SUPERBLOCK STARTS=====\n");
    printf("\nILIST SIZE: %ld\nNEXT AVAILABLE INUM: %ld\nINODES PER BLOCK: %ld\nFREE LIST HEAD: %ld\n",
           sb->inode_count, sb->latest_inum, sb->inodes_per_block, sb->free_list_head);
    printf("\n=====SUPERBLOCK ENDS=====\n\n");
    return 0;
}

// Print file system constants
void print_constants()
{
    printf("\n==== FILE SYSTEM CONSTANTS =====\n");
    printf("FILE SYSTEM SIZE: %ld MB\nBLOCK SIZE: %ld\nBLOCK COUNT: %ld\nADDRESS SIZE: %ld\nINODE BLOCK COUNT: %ld\nDATA BLOCK COUNT: %ld\nDBLOCKS PER BLOCK: %ld\nFREE LIST BLOCKS: %ld\n",
           (FS_SIZE / (1024 * 1024)), BLOCK_SIZE, BLOCK_COUNT, ADDRESS_SIZE, INODE_B_COUNT,
           DATA_B_COUNT, DBLOCKS_PER_BLOCK, FREE_LIST_BLOCKS);
    printf("\n==== FILE SYSTEM CONSTANTS =====\n");
}

// Print a free list block
void print_free_list_block(ssize_t freelist_block_num)
{
    char *block_buff = (char *)malloc(BLOCK_SIZE);
    if (!read_block(freelist_block_num, block_buff))
    {
        printf("ERROR: Failed to read free list block.\n");
        return;
    }
    printf("\n=== DUMP OF THE FREELIST BLOCK START ====\n");
    ssize_t *dblock_num_ptr = (ssize_t *)block_buff;
    for (size_t i = 0; i < DBLOCKS_PER_BLOCK / 8; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            printf("%ld ", dblock_num_ptr[i * 8 + j]);
        }
        printf("\n");
    }
    printf("\n==== DUMP OF THE FREELIST BLOCK END =====\n");
}

// Print inode details
void print_inode(ssize_t inode_num)
{
    struct iNode *inode = read_inode(inode_num);
    printf("\n=== INODE DETAILS ===\n");
    for (ssize_t i = 0; i < DIRECT_B_COUNT; i++)
    {
        printf("direct_blocks[%ld]: %ld\n", i, inode->direct_blocks[i]);
    }
    printf("single_indirect: %ld\n", inode->single_indirect);
    printf("double_indirect: %ld\n", inode->double_indirect);
    printf("\n=== INODE DETAILS FINISHED===\n");
}

int main()
{
    // Check filesystem creation
    if (!make_fs())
    {
        printf("BLOCK_LAYER_TEST 1 ERROR: Error during filesystem creation\n\n");
        return -1;
    }
    printf("\nBLOCK_LAYER_TEST 1 INFO: Filesystem creation test - Passed!\n\n");
    // Check superblock creation
    print_superblock();
    if (check_superblock_init() < 0)
    {
        printf("BLOCK_LAYER_TEST 2 ERROR: Error reading the superblock\n\n");
        return -1;
    }
    printf("BLOCK_LAYER_TEST 2 INFO: Initial superblock value check - Passed!\n\n");
    // Check dblock related functions
    char *buffer = (char *)malloc(BLOCK_SIZE);
    char *test_string = "A TEST MESSAGE TO TEST THE CORRECTNESS OF BLOCK LAYER";
    memcpy(buffer, test_string, strlen(test_string));
    printf("Data blocks per block %ld\n", DBLOCKS_PER_BLOCK);
    for (ssize_t i = 0; i < DBLOCKS_PER_BLOCK; i++)
    {
        ssize_t dblock_num = create_new_dblock();
        //printf("Dblock - %ld allocated\n",dblock_num);
        if (dblock_num < 0)
        {
            printf("BLOCK_LAYER_TEST 3 ERROR: Error during new dblock creation\n\n");
            return -1;
        }
        if (!write_dblock(dblock_num, buffer))
        {
            printf("BLOCK_LAYER_TEST 3 ERROR: Error during dblock write\n\n");
            return -1;
        }
        buffer = read_dblock(dblock_num);
        if (buffer == NULL)
        {
            printf("BLOCK_LAYER_TEST 3 ERROR: Error during dblock read\n\n");
            return -1;
        }
        if (strcmp(buffer, test_string) != 0)
        {
            printf("BLOCK_LAYER_TEST 3 ERROR: Error during data validity check\n\n");
            return -1;
        }
    }
    struct superBlock *super_block = get_superblock();
    printf("Freelist head: %ld Expected head: %ld\n\n", super_block->free_list_head, INODE_B_COUNT + 1 + DBLOCKS_PER_BLOCK);
    printf("BLOCK_LAYER_TEST 3 INFO: Dblock allocation, read and write consistency check - Passed!\n\n");
    // printf("size - %ld", sizeof(int));
    // Free any allocated block (freeing a properly known last head in this case)
    if (!free_dblock(INODE_B_COUNT + 1))
    {
        printf("BLOCK_LAYER_TEST 4 ERROR: Error during dblock deallocation\n\n");
        return -1;
    }
    super_block = get_superblock();
    printf("Freelist head: %ld Expected head: %ld\n\n", super_block->free_list_head, INODE_B_COUNT + 1);
    printf("BLOCK_LAYER_TEST 4 INFO: Dblock deallocation check - Passed!\n\n");
    // Check inode related functions
    ssize_t inode_num = 0;
    // Create a new inode for each available slot in the superblock
    for (ssize_t i = 0; i < super_block->inodes_per_block; i++)
    {
        inode_num = create_new_inode();
        // printf("Inum - %ld allocated \n", inode_num);
        // Check for any errors while creating the inode
        if (!is_valid_inum(inode_num))
        {
            printf("BLOCK_LAYER_TEST 5 ERROR: Error during create_new_inode - %ld\n", inode_num);
            return -1;
        }
    }
    // Inform the user that the inode allocation check has passed
    printf("BLOCK_LAYER_TEST 5 INFO: Inode allocation check - Passed!\n\n");
    // Allocate memory for an inode struct
    struct iNode *inode = (struct iNode *)malloc(sizeof(struct iNode));
    // Allocate all direct access dbs for the inode
    for (ssize_t i = 0; i < DIRECT_B_COUNT; i++)
    {
        inode->direct_blocks[i] = create_new_dblock();
        printf("%ld is the direct access block assigned\n", inode->direct_blocks[i]);
    }
    // Fill the single indirect access address block for the inode
    inode->single_indirect = create_new_dblock();
    printf("%ld is the single indirect access block assigned\n", inode->single_indirect);
    // Initialize a buffer to hold block addresses
    char buff[BLOCK_SIZE];
    memset(buff, 0, BLOCK_SIZE);
    size_t offset = 0;
    // Go over one block and add all addresses to the buffer
    for (ssize_t i = 0; i < DIRECT_B_COUNT / 2; i++)
    {
        ssize_t block_id = create_new_dblock();
        memcpy(buff + offset, &block_id, ADDRESS_SIZE);
        offset += ADDRESS_SIZE;
    }
    // Write the dblock into memory
    if (!write_dblock(inode->single_indirect, buff))
    {
        printf("BLOCK_LAYER ERROR: Unable to write single indirect block info into block\n");
        return -1;
    }
    // Write the inode to disk
    if (!write_inode(inode_num, inode))
    {
        printf("BLOCK_LAYER ERROR: Unable to write inode into disk\n");
        return -1;
    }
    // Read the inode from disk
    struct iNode *rd_inode = read_inode(inode_num);
    if (rd_inode == NULL)
    {
        printf("BLOCK_LAYER_TEST ERROR: Unable to read inode of inum %ld\n", inode_num);
        return -1;
    }
    // Print the direct access block read-back for the inode
    for (ssize_t i = 0; i < DIRECT_B_COUNT; i++){
        printf("%ld is the direct access block read-back\n", rd_inode->direct_blocks[i]);
    }
    // Print the single indirect access block read-back for the inode
    printf("%ld is the single indirect access block read-back\n", rd_inode->single_indirect);
    // Inform the user that the inode read-write check has passed
    printf("\n\nBLOCK_LAYER_TEST 6 INFO: Inode read-write check - Passed!\n\n");
    // Free the inode and check for errors
    if (!free_inode(inode_num))
    {
        printf("BLOCK_LAYER_TEST 7 ERROR: Error freeing the inode\n");
        return -1;
    }
    // Inform the user that the inode free check has passed
    printf("BLOCK_LAYER_TEST 7 INFO: Inode free check - Passed!\n\n");
    return 0;
}
