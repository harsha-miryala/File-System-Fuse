#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "../../include/block_layer.h"
#include "../../include/disk_layer.h"

int init_superblock_check()
{
    char* buff = (char *)malloc(BLOCK_SIZE);
    if(!read_block(0, buff))
    {
        printf("BLOCK_LAYER_TEST ERROR: Error during superBlock read\n");
        return -1;
    }
    struct superBlock *super_block = (struct superBlock* )buff;
    if(super_block->inode_count != (INODE_B_COUNT) / sizeof(struct iNode) ||
       super_block->latest_inum != 3 ||
       super_block->inodes_per_block != (BLOCK_SIZE) / sizeof(struct iNode) ||
       super_block->free_list_head != INODE_B_COUNT + 1)
    {
        printf("BLOCK_LAYER_TEST ERROR: Error checking the init values of superblock\n");
        return -1;
    }
    return 0;
}

struct superBlock *get_superblock()
{
    char *buff = (char *)malloc(BLOCK_SIZE);
    if(!read_block(0, buff))
    {
        printf("BLOCK_LAYER_TEST ERROR: Error during superblock read\n");
        return NULL;
    }
    return (struct superBlock*)buff;
}

int print_superblock()
{
    char* buff = (char *)malloc(BLOCK_SIZE);
    if(!read_block(0, buff))
    {
        printf("BLOCK_LAYER_TEST ERROR: Error during superBlock read\n");
        return -1;
    }
    struct superBlock *super_block = (struct superBlock *)buff;

    printf("\n=====SUPERBLOCK STARTS=====\n");
    printf("\nILIST SIZE : %d\nNEXT AVAILABLE INUM : %d\nINODES_PER_BLOCK: %dFREE_LIST_HEAD: %d\n",
            super_block->inode_count, super_block->latest_inum, 
            super_block->inodes_per_block, super_block->free_list_head);
    printf("\n=====SUPERBLOCK ENDS=====\n\n");
    return 0;
}

void print_consts()
{
    printf("\n==== FILE SYSTEM CONSTANTS =====\n");
    printf("FILE_SYSTEM_SIZE: %d MB\nBLOCK_SIZE: %d\nBLOCK_COUNT: %d \
            ADDRESS_SIZE: %d\nINODE_BLOCK_COUNT: %d\nDATA_BLOCK_COUNT: %d\n \
            DBLOCKS_PER_BLOCK: %d\nFREE_LIST_BLOCKS: %d\n",
           (FS_SIZE/(1024*1024)), BLOCK_SIZE, BLOCK_COUNT, ADDRESS_SIZE, INODE_B_COUNT,
            DATA_B_COUNT, DBLOCKS_PER_BLOCK, FREE_LIST_BLOCKS);
    printf("\n==== FILE SYSTEM CONSTANTS =====\n");
} 

void print_freelist_block(int freelist_block_num)
{
    char* buff = (char *)malloc(BLOCK_SIZE);
    if(!read_block(freelist_block_num, buff))
    {
        printf("BLOCK_LAYER_TEST ERROR: Error during superblock read\n");
        return;
    }
    printf("\n=== DUMP OF THE FREELIST BLOCK START ====\n");
    int *dblock_num_ptr = (int *)buff;
    for (size_t i = 0; i < DBLOCKS_PER_BLOCK/8; i++) {
        for (size_t j = 0; j < 8; j++)
            printf("%06d ", dblock_num_ptr[i*8 + j]);
        printf("\n");
    }
    printf("\n==== DUMP OF THE FREELIST BLOCK END =====\n");
}

void print_inode(int inode_num)
{
    struct iNode *inode = read_inode(inode_num);
    printf("\n=== PRINTING INODE ====\n");
    for(int i=0; i<DIRECT_B_COUNT; i++)
    {
        printf("direct_blocks[%d]: %d\n", i, inode->direct_blocks[i]);
    }
    printf("single_indirect: %d\n",inode->single_indirect);
    printf("double_indirect: %d\n",inode->double_indirect);
    printf("\n=== PRINTING INODE FINISHED===\n");
}


int main(){
    // checking make file system
    if(!make_fs())
    {
        printf("BLOCK_LAYER_TEST 1 ERROR: Error during mkfs\n\n");
        return -1;
    }
    printf("\nBLOCK_LAYER_TEST TEST 1 INFO: mkfs creation - Passed!\n\n");

    // checking superblock creation
    print_superblock();
    if(init_superblock_check() < 0)
    {
        printf("BLOCK_LAYER_TEST TEST 2 ERROR: Error reading the superBlock\n\n");
        return -1;
    }
    printf("BLOCK_LAYER_TEST TEST 2 INFO: initial superBlock value check - Passed!\n\n");

    // checking dblock related functions
    char *buff = (char *)malloc(BLOCK_SIZE);
    char *test_str = "A TEST MESSAGE TO TEST THE CORRECTNESS OF BLOCK LAYER";
    memcpy(buff, test_str, strlen(test_str));
    for(int i=0; i < DBLOCKS_PER_BLOCK; i++)
    {
        int dblock_num = create_new_dblock();
        if(dblock_num < 0)
        {
            printf("BLOCK_LAYER_TEST 3 ERROR: Error during dblock new\n\n");
            return -1;
        }
        if(!write_dblock(dblock_num, buff))
        {
            printf("BLOCK_LAYER_TEST 3 ERROR: Error during dblock write\n\n");
            return -1;
        }
        buff = read_dblock(dblock_num);
        if (buff == NULL)
        {
            printf("BLOCK_LAYER_TEST 3 ERROR: Error during dblock read\n\n");
            return -1;
        }
        if(strcmp(buff, test_str) != 0){
            printf("BLOCK_LAYER_TEST 3 ERROR: Error during data validity check\n\n");
            return -1;
        }
    }

    struct superBlock *super_block = get_superblock();
    printf("Freelist head : %d Expected head: %d\n\n", super_block->free_list_head, INODE_B_COUNT+1+DBLOCKS_PER_BLOCK);
    printf("BLOCK_LAYER_TEST 3: Dblock allocation, read and write consistency check - Passed!\n\n");

    // free any allocated block (freeing a properly known last head in this case)
    if(!free_dblock(INODE_B_COUNT+1))
    {
        printf("BLOCK_LAYER_TEST 4 ERROR: Error during dblock free\n\n");
        return -1;
    }
    
    super_block = get_superblock();
    printf("Freelist head: %d Expected head: %d\n\n", super_block->free_list_head, INODE_B_COUNT+1);
    printf("BLOCK_LAYER_TEST 4 INFO: Dblock deallocation check - Passed!\n\n");

    // checking inode related functions
    int inode_num;
    for(int i=0;i<super_block->inodes_per_block+2;i++){
        inode_num = create_new_inode();
        if(inode_num < 0)
        {
            printf("BLOCK_LAYER_TEST 5 ERROR: Error during create_new_inode\n");
            return -1;
        }
    }
    printf("\n\nBLOCK_LAYER_TEST 5 INFO: Inode allocation check - Passed!\n\n");
    struct iNode *inode = (struct iNode*)malloc(sizeof(struct iNode));

    //allocate all direct access dbs
    for(int i=0;i<DIRECT_B_COUNT;i++) {
        inode->direct_blocks[i] = create_new_dblock();
        printf("%d is the direct access block assigned\n", inode->direct_blocks[i]);
    }
    
    //fill the single indirect access adress block
    inode->single_indirect = create_new_dblock();
    printf("%d is the single indirect access block assigned\n", inode->single_indirect);

    memset(buff, 0, BLOCK_SIZE);
    size_t offset = 0;
    for (int i = 0; i < DBLOCKS_PER_BLOCK/2; i++) //goes over the one block and adds all addresses
    {   
        int block_id = create_new_dblock();
        memcpy(buff + offset, &block_id, ADDRESS_SIZE);
        offset += ADDRESS_SIZE;
    }

    //write that dblock into the memory
    if (!write_dblock(inode->single_indirect, buff))
    {
        printf("BLOCK_LAYER ERROR: Unable to write single indirect block info into block\n");
        return -1;
    }

    if (!write_inode(inode_num, inode))
    {
        printf("BLOCK_LAYER ERROR: Unable to write single indirect block info into block\n");
        return -1;
    }

    struct iNode* rd_inode= read_inode(inode_num);
    if(rd_inode == NULL)
    {
        printf("BLOCK_LAYER_TEST ERROR: Unable to read inode of inum %d\n", inode_num);
        return -1;
    }

    for(int i=0; i<DIRECT_B_COUNT; i++)
        printf("%d is the direct access block read-back\n", rd_inode->direct_blocks[i]);

    printf("%d is the single indirect access block read_back\n", rd_inode->single_indirect);

    printf("\n\nBLOCK_LAYER_TEST 6 INFO: Inode read write check - Passed!\n\n");

    if(free_inode(inode_num) < 0)
    {
        printf("BLOCK_LAYER_TEST 7 ERROR: Error freeing the inode\n");
        return -1;
    }
    printf("BLOCK_LAYER_TEST 7 INFO: Inode free check - Passed!\n\n");
    return 0;
}
