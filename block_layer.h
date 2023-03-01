#ifndef __BLOCK_LAYER_H__
#define __BLOCK_LAYER_H__

#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#include "disk_layer.h"

#define ADDRESS_SIZE 8 // we need 8 bytes to tag a block number
#define INODE_B_COUNT (BLOCK_COUNT/10)
#define DATA_B_COUNT (BLOCK_COUNT - INODE_B_COUNT - 1)
#define DIRECT_B_COUNT 10
#define DBLOCKS_PER_BLOCK (BLOCK_SIZE / ADDRESS_SIZE)
#define FREE_LIST_BLOCKS ((DATA_B_COUNT/DBLOCKS_PER_BLOCK) + 1)

struct superBlock {
    size_t inode_count; // total number of inodes we can have
    size_t latest_inum; // latest inode that has been assigned
    size_t inodes_per_block; // how many inodes per block
    size_t free_list_head; // block containing addresses of free dblocks, when 0 that means fs is full
};

struct iNode {
    size_t direct_blocks[DIRECT_B_COUNT]; // direct block numbers
    size_t single_indirect; // 
    size_t double_indirect;
    size_t link_count;
    size_t file_size;
    size_t num_blocks;
    size_t allocated;
    size_t user_id;
    size_t group_id;
    mode_t mode;
    time_t access_time;
    time_t creation_time;
    time_t modification_time;
    time_t status_change_time;
};

// init the super block, ilist and free list
bool make_fs();

/*
allocates a new inode and returns the inode_num
Inputs:
    inode_num: the num associated with the inode
Returns:
    true / false
*/
bool create_new_inode();

/*
returns the inode associated with inode_num
Inputs:
    inode_num: the num associated with the inode
Returns:
    inode struct if success; NULL if failure
*/
struct iNode* read_inode(int inode_num);

/*
writes the inode struct into the inode_num
Inputs:
    inode_num: the num associated with the inode
    i_node : the inode struct to write into the inodenum
Returns:
    true / false
*/
bool write_inode(int inode_num, struct iNode* inode);

/*
frees the inode and the data dblocks with the inode
Inputs:
    inode_num: the num associated with the inode
Returns:
    true / false
*/
bool free_inode(int inode_num);

/*
assigns a new dblock and returns the dblock_num
Inputs: 
    None
Returns:
    dblocknum on success; -1 on failure
*/
int create_new_dblock();

/*
returns the read info buf from the dblock given by dblock_num
Inputs:
    dblock_num: the dblock number
Returns:
    buf which contains the read on success and NULL on failure
*/
char* read_dblock(int dblock_num);

/*
writes the info in buf to the dblock given by dblock_num
Inputs:
    dblock_num: the dblock number
    buf: the buffer which contains data to write
Returns:
    true / false
*/
bool write_dblock(int dblock_num, char* buff);

/*
Frees the dblock given by dblock_num
Inputs:
    dblock_num: the dblock number
Returns:
    true / false
*/
bool free_dblock(int dblock_num);

#endif
