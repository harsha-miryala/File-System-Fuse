#ifdef __BLOCK_LAYER_H__
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
    int inode_sz;
    int latest_inum;
    int inodes_per_block;
    int free_list_head;
}

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
}

int make_fs();

int create_new_inode();

struct iNode* read_inode(int inode_num);

int write_inode(int inode_num, struct iNode* inode);

int free_inode(int inode_num);

int create_new_dblock();

char* read_dblock(int dblock_num);

int write_dblock(int dblock_num, char* buff);

int free_dblock(int dblock_num);

#endif
