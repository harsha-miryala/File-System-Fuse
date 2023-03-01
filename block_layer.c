#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "disk_layer.h"
#include "block_layer.h"
#include "debug.h"

static struct superBlock* super_block = NULL;

bool write_superblock(){
    char buff[BLOCK_SIZE];
    memset(buff, 0, BLOCK_SIZE);
    memcpy(buff, super_block, sizeof(struct superBlock));
    if(!write_block(0, buff)){
        printf("Could not write superblock into memory");
        return false;
    }
    return true;
}

bool init_superblock(){
    // assign memory
    super_block = (struct superBlock*) malloc(sizeof(struct superBlock));
    // initialize values
    super_block->latest_inum = 3; // It means this is the first free inode, need 1 and 2 for file level op
    super_block->inodes_per_block = (BLOCK_SIZE) / sizeof(struct iNode);
    super_block->inode_count = (INODE_B_COUNT * super_block->inodes_per_block);
    super_block->free_list_head = INODE_B_COUNT + 1;
    return write_superblock();
}

bool init_inode_list(){
    // TODO : Special place for indirect blocks needs to be added
    struct iNode inode[sizeof(struct iNode)];
    for(size_t i=0; i<DIRECT_B_COUNT; i++){
        inode->direct_blocks[i] = 0; // using 0 because 0 can only be used by superblock
    }
    inode->single_indirect = 0;
    inode->double_indirect = 0;
    inode->link_count = 0;
    inode->file_size = 0;
    inode->num_blocks = 0;
    inode->allocated = false;
    char buff[BLOCK_SIZE];
    int dummy_value = 0;
    for(size_t block_id=1; block_id<=INODE_B_COUNT; block_id++){
        size_t offset = 0;
        memset(buff, dummy_value, BLOCK_SIZE);
        for(size_t i=0; i<super_block->inodes_per_block; i++){
            memcpy(buff+offset, inode, sizeof(struct iNode));
            offset += sizeof(struct iNode);
        }
        if(!write_block(block_id, buff)){    
            printf("Could not create inode at block - %d", &block_id);
            return false;
        }
    }
    return true;
}

bool init_freelist(){
    char buff[BLOCK_SIZE];
    size_t block_id = super_block->free_list_head;
    size_t dummy_value = 0;
    // 1 will have details of 2 - 512
    // 514 ...........        515 - 1026
    for(size_t i=0; i<FREE_LIST_BLOCKS; i++){
        size_t curr_block_id = block_id;
        memset(buff, dummy_value, BLOCK_SIZE);
        size_t offset = 0;
        for(size_t j=1; j<=DBLOCKS_PER_BLOCK; j++){
            // not utilising the first available space in a freelist
            // TODO : revisit
            offset += ADDRESS_SIZE;
            block_id++;
            if(block_id >= BLOCK_COUNT){
                memcpy(buff+offset, &dummy_value, ADDRESS_SIZE);
            } else{
                memcpy(buff+offset, &block_id, ADDRESS_SIZE);
            }
        }
        block_id++;
        // assigning next free_block_id at the first position
        if(block_id >= BLOCK_COUNT){
            memcpy(buff, &dummy_value, ADDRESS_SIZE);
        } else{
            memcpy(buff, &block_id, ADDRESS_SIZE);
        }
        if(!write_block(curr_block_id, buff)){
            printf("Unable to write in the free list");
            return false;
        }
        if(block_id >= BLOCK_COUNT){
            break; // this wont happen but safe
        }
    }
    return true;
}

int create_new_dblock(){
    if(super_block->free_list_head == 0)
    {
        printf("No more blocks left\n");
        return -1;
    }
    char buff[BLOCK_SIZE];
    if(!read_block(super_block->free_list_head, buff)){
        return -1;
    }
    size_t dblock_num;
    size_t *dblock_num_ptr = (size_t *)buff;
    size_t ans = 0;
    for(size_t i=1; i<DBLOCKS_PER_BLOCK; i++){
        if(dblock_num_ptr[i]!=0){
            // the dblock is free and we are allotting this
            dblock_num_ptr[i]=0;
            ans = dblock_num;
            break;
        }
    }
    size_t temp = super_block->free_list_head;
    if(ans == 0){
        //freeList node itself is allocated as DataBlock
        ans = super_block->free_list_head;
        // move free_list_head to the next free_list_block
        super_block->free_list_head = dblock_num_ptr[0];
        dblock_num_ptr[0] = 0;
        printf("Freelist Node %d is now a dblock\n", ans);
        if(!write_superblock()){
            return -1;
        }
    }
    if(!write_block(temp, buff)){
        printf("Could not write free_list block\n");
        return -1;
    }
    return ans;
}

char *read_dblock(int dblock_num){
    if(dblock_num <= INODE_B_COUNT || dblock_num > BLOCK_COUNT){
        printf("Invalid data block number %d provided", dblock_num);
        return NULL;
    }
    char *buff= (char *)malloc(BLOCK_SIZE);
    if(read_block(dblock_num, buff)){
        return buff;
    }
    return NULL;
}

bool write_dblock(int dblock_num, char *buff){
    if(dblock_num <= INODE_B_COUNT || dblock_num > BLOCK_COUNT){
        printf("Invalid data block number %d provided\n", dblock_num);
        return false;
    }
    return write_block(dblock_num, buff);
}

bool free_dblock(int dblock_num){
    if(dblock_num <= INODE_B_COUNT || dblock_num > BLOCK_COUNT){
        printf("Invalid data block number %d provided\n", dblock_num);
        return false;
    }
    char buff[BLOCK_SIZE];
    memset(buff, 0, BLOCK_SIZE);
    // flushing the data (might be doing two times) TODO
    write_block(dblock_num, buff);
    if(super_block->free_list_head==0){
        super_block->free_list_head = dblock_num;
        if(!write_superblock()){
            return false;
        }
    }
    if(!read_block(super_block->free_list_head, buff)){
        return false;
    }
    size_t* dblock_num_ptr = (size_t*) buff;
    int ans = 0;
    for(size_t i=1; i<DBLOCKS_PER_BLOCK; i++){
        if(dblock_num_ptr[i]==0){
            dblock_num_ptr[i] = dblock_num;
            ans = 1;
            break;
        }
    }
    if(ans==0){
        // dblock will be made the new free list which points to old one
        size_t temp = super_block->free_list_head;
        super_block->free_list_head = dblock_num;
        printf("Dblock %d is named as the free list head\n", dblock_num);
        if(!write_superblock()){
            return false;
        }
        memset(buff, 0, BLOCK_SIZE);
        memcpy(buff, &temp, ADDRESS_SIZE);
        if(!write_block(dblock_num, buff)){
            return false;
        }
    }
    else{
        if(!write_block(super_block->free_list_head, buff)){
            return false;
        }
    }
    return true;
}

bool is_valid_inum(size_t inode_num){
    return inode_num>0 && inode_num<super_block->inode_count;
}

size_t inode_num_to_block_id(int inode_num){
    return 1 + (inode_num/super_block->inodes_per_block);
}

size_t inode_num_to_offset(int inode_num){
    return inode_num % super_block->inodes_per_block;
}

bool create_new_inode(){
    // TODO: Worst case - O(N) for finding one even with use of latest inum
    if(!is_valid_inum(super_block->latest_inum)){
        printf("Invalid inode num being accessed or out of range");
        return false;
    }
    size_t block_id = inode_num_to_block_id(super_block->latest_inum);
    size_t offset = inode_num_to_offset(super_block->latest_inum);
    char buff[BLOCK_SIZE];
    struct iNode* temp = NULL;
    struct iNode* inode = NULL;
    size_t visit_count = 0;
    while(visit_count != super_block->inode_count){
        // reading block
        if(read_block(block_id, buff)){
            return false;
        }
        temp = (struct iNode* ) buff;
        for(; offset<super_block->inodes_per_block; offset++){
            inode = temp+offset;
            visit_count++;
            if(inode->allocated==false){
                size_t ans = super_block->latest_inum;
                // getting the new latest inum from block_id and offset
                super_block->latest_inum += ((block_id-1)*super_block->inodes_per_block)+offset;
                inode->allocated = true;
                DEBUG_PRINTF("Inum %03d Block %03d Offset %03d allocated\n", ans, block_id, offset);
                return ans;
            }
        }
        offset = 0;
        block_id = (block_id%INODE_B_COUNT) + 1;
    }
    printf("Unable to find memory for inode\n");
    return false;
}

struct iNode* read_inode(int inode_num){
    if(!is_valid_inum(inode_num)){
        printf("Invalid inode num being accessed or out of range");
        return false;
    }
    size_t block_id = inode_num_to_block_id(inode_num);
    size_t offset = inode_num_to_offset(inode_num);
    char buff[BLOCK_SIZE];
    if(!read_block(block_id, buff)){
        return false;
    }
    struct iNode* inode = (struct iNode*)buff;
    inode = inode + offset;
    struct iNode* ans = (struct iNode*)malloc(sizeof(struct iNode));
    memcpy(ans, inode, sizeof(struct iNode));
    return ans;
}

bool write_inode(int inode_num, struct iNode* inode){
    if(!is_valid_inum(inode_num)){
        printf("Invalid inode num being accessed or out of range");
        return false;
    }
    size_t block_id = inode_num_to_block_id(inode_num);
    size_t offset = inode_num_to_offset(inode_num);
    char buff[BLOCK_SIZE];
    if(!read_block(block_id, buff)){
        return false;
    }
    struct iNode* temp = (struct iNode*)buff;
    temp = temp + offset;
    memcpy(temp, inode, sizeof(struct iNode));
    if(write_block(block_id, buff)){
        return false;
    }
    DEBUG_PRINTF("Wrote inode %d to block %d offset %d", inode_num, block_id, offset);
    return true;
}

bool free_dblocks_from_inode(struct iNode* inode){
    bool done = false;
    char dblock_list_buff[BLOCK_SIZE];
    char single_indirect_block_buff[BLOCK_SIZE];
    size_t* dblock_list_ptr;
    size_t* single_indirect_block_ptr;
    // freeing direct block
    for(size_t i=0; i<DIRECT_B_COUNT; i++){
        if(inode->direct_blocks[i]==0){
            done = true;
            break;
        }
        if(!free_dblock(inode->direct_blocks[i])){
            return false;
        }
    }
    if(done || inode->single_indirect==0){
        return true;
    }
    // freeing single indirect
    if(!read_block(inode->single_indirect, dblock_list_buff)){
        return false;
    }
    // store dblock number each by size_t
    dblock_list_ptr = (size_t* )dblock_list_buff;
    for(size_t i=0; i<DBLOCKS_PER_BLOCK; i++){
        if(dblock_list_ptr[i]==0){
            done = true;
            break;
        }
        if(!free_dblock(dblock_list_ptr[i])){
            return false;
        }
    }
    if(!free_dblock(inode->single_indirect)){
        return false;
    }
    if(done || inode->double_indirect==0){
        return true;
    }
    // freeing double indirect
    if(!read_block(inode->double_indirect, single_indirect_block_buff)){
        return false;
    }
    single_indirect_block_ptr = (size_t* )single_indirect_block_buff;
    for(size_t i=0; i<DBLOCKS_PER_BLOCK; i++){
        if(single_indirect_block_ptr[i]==0){
            done = true;
            break;
        }
        if(!read_block(single_indirect_block_ptr[i], dblock_list_buff)){
            return false;
        }
        dblock_list_ptr = (size_t* )dblock_list_buff;
        for(size_t j=0; j<DBLOCKS_PER_BLOCK; j++){
            if(dblock_list_ptr[j]==0){
                done = true;
                break;
            }
            if(!free_dblock(dblock_list_ptr[j])){
                return false;
            }
        }
        if(!free_dblock(single_indirect_block_ptr[i])){
            return false;
        }
    }
    if(!free_dblock(inode->double_indirect)){
        return false;
    }
    return true;
}

bool free_inode(int inode_num){
    if(!is_valid_inum(inode_num)){
        printf("Invalid inode num being accessed or out of range");
        return false;
    }
    size_t block_id = inode_num_to_block_id(inode_num);
    size_t offset = inode_num_to_offset(inode_num);
    char buff[BLOCK_SIZE];
    if(!read_block(block_id, buff)){
        return false;
    }
    struct iNode* temp = (struct iNode*)buff;
    temp = temp + offset;
    if(!free_dblocks_from_inode(temp)){
        return false;
    }
    temp->allocated = false;
    if(!write_block(block_id, buff)){
        return false;
    }
    DEBUG_PRINTF("Inode %d has been freed \n", inode_num);
    return true;
}

bool make_fs(){
    // this calls disk layer
    if(allocate_memory()){
        printf("Memory allocation for block failed \n");
        return false;
    }
    DEBUG_PRINTF("Memory Allocated for block \n");

    if(init_superblock()){
        printf("Superblock allocation failed \n");
        return false;
    }
    DEBUG_PRINTF("Superblock created \n");

    if(init_inode_list()){
        printf("Inode List allocation failed \n");
        return false;
    }
    DEBUG_PRINTF("Inode List created \n");

    if(init_freelist()){
        printf("Free List allocation failed \n");
        return false;
    }
    DEBUG_PRINTF("Free List created \n");
    return true;
}