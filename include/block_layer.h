#ifndef __BLOCK_LAYER_H__
#define __BLOCK_LAYER_H__

#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "disk_layer.h"

#define ADDRESS_SIZE ((ssize_t) 8)
#define INODE_B_COUNT ((ssize_t) (BLOCK_COUNT/10)) // blocks being allocated for inodes
#define DATA_B_COUNT ((ssize_t) (BLOCK_COUNT - INODE_B_COUNT - 1)) // blocks allocated for storing data
#define DIRECT_B_COUNT ((ssize_t) 10) // number of direct blocks per inode
#define DBLOCKS_PER_BLOCK ((ssize_t) (BLOCK_SIZE / ADDRESS_SIZE)) // number of block addresses storable by a block i.e. 4K/4 = 0.5 KB
#define FREE_LIST_BLOCKS ((ssize_t) ((DATA_B_COUNT/DBLOCKS_PER_BLOCK) + 1)) // number of freelist blocks needed to store the information of data blocks
// 10 direct blocks = 20KB
// single indirect stores BLOCK_SIZE/ADDRESS_SIZE = 4KB/8 = 0.5K * 4K = 2MB
// double indirect stores 0.5K * 0.5K * 4K = 1GB
// triple indirect stores 0.5K * 0.5K * 0.5K * 4K = 500GB

struct superBlock {
    ssize_t inode_count; // total number of inodes we can have
    ssize_t latest_inum; // latest inode that is free
    ssize_t inodes_per_block; // how many inodes per block
    ssize_t free_list_head; // block containing addresses of free dblocks, when 0 that means fs is full
};

struct iNode {
    ssize_t direct_blocks[DIRECT_B_COUNT]; // direct block numbers
    ssize_t single_indirect; // single indirection -> this is a block number which contains the block numbers
    ssize_t double_indirect;
    ssize_t triple_indirect;
    ssize_t link_count; // how many links an inode has
    ssize_t file_size; // size of file
    ssize_t num_blocks; // number of blocks a file has
    bool allocated; // boolean value to track allocation
    ssize_t user_id;
    ssize_t group_id;
    mode_t mode; // 664 etc - rwx rwx rwx (user, group, global) - 9 bits
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
    inode num / -1;
*/
ssize_t create_new_inode();

/*
returns the inode associated with inode_num
Inputs:
    inode_num: the num associated with the inode
Returns:
    inode struct if success; NULL if failure
*/
struct iNode* read_inode(ssize_t inode_num);

/*
writes the inode struct into the inode_num
Inputs:
    inode_num: the num associated with the inode
    i_node : the inode struct to write into the inodenum
Returns:
    true / false
*/
bool write_inode(ssize_t inode_num, struct iNode* inode);

/*
frees the inode and the data dblocks with the inode
Inputs:
    inode_num: the num associated with the inode
Returns:
    true / false
*/
bool free_inode(ssize_t inode_num);

/*
assigns a new dblock and returns the dblock_num
Inputs: 
    None
Returns:
    dblocknum on success; -1 on failure
*/
ssize_t create_new_dblock();

/*
returns the read info buf from the dblock given by dblock_num
Inputs:
    dblock_num: the dblock number
Returns:
    buf which contains the read on success and NULL on failure
*/
char* read_dblock(ssize_t dblock_num);

/*
writes the info in buf to the dblock given by dblock_num
Inputs:
    dblock_num: the dblock number
    buf: the buffer which contains data to write
Returns:
    true / false
*/
bool write_dblock(ssize_t dblock_num, char* buff);

/*
Frees the dblock given by dblock_num
Inputs:
    dblock_num: the dblock number
Returns:
    true / false
*/
bool free_dblock(ssize_t dblock_num);

bool is_valid_inum(ssize_t inode_num);

#endif

