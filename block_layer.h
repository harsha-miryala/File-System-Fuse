#ifndef __BLOCK_LAYER_H__
#define __BLOCK_LAYER_H__

#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#include "disk_layer.h"

#define ADDRESS_SIZE 8
#define INODE_B_COUNT (BLOCK_COUNT/10)
#define DATA_B_COUNT (BLOCK_COUNT - INODE_B_COUNT - 1)
#define DIRECT_B_COUNT 10
#define DBLOCKS_PER_BLOCK (BLOCK_SIZE / ADDRESS_SIZE)
#define FREE_LIST_BLOCKS ((DATA_B_COUNT/DBLOCKS_PER_BLOCK) + 1)

struct superBlock {
    size_t inode_count;
    size_t latest_inum;
    size_t inodes_per_block;
    size_t free_list_head;
};

struct iNode {
    size_t direct_blocks[DIRECT_B_COUNT];
    size_t single_indirect;
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

// TODO : make the output of operation status as true or false instead of 0/-1

bool make_fs();

bool create_new_inode();

struct iNode* read_inode(int inode_num);

bool write_inode(int inode_num, struct iNode* inode);

bool free_inode(int inode_num);

bool create_new_dblock();

char* read_dblock(int dblock_num);

bool write_dblock(int dblock_num, char* buff);

bool free_dblock(int dblock_num);

#endif
