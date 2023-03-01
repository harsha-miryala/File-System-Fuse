#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "disk_layer.h"
#include "file_layer.h"
#include "hash_table.h"

static hash_table iName_cache;

// Returns char array index of the last slash of parent's path
int get_parent_id(const char* const path, int path_len){
    for(int id=path_len-1; id>=0; id--){
        if(path[id]=='/' && id!=(path_len-1)){
            return id;
        }
    }
    return -1;
}

// copy parent path into a given buffer
bool get_parent_path(char* const buff, const char* const path, int path_len){
    int pos = get_parent_id(path, path_len);
    if(pos==-1){
        return false;
    }
    memcpy(buff, path, pos+1);
    buff[pos+1] = '\0';
    return true;
}

// copy child file into a given buffer
bool get_child_name(char* const buff, const char* const path, int path_len){
    int start = get_parent_id(path, path_len);
    if(start==-1){
        return false;
    }
    start++;
    int end = path_len;
    if(path[end-1]=='/'){
        // removing trailing slash
        end--;
    }
    memcpy(buff, path+start, end-start);
    buff[end-start]='\0';
    return true;
}

// get dblock num corr to file block number
int fblock_num_to_dblock_num(const struct iNode* const inode, int fblock_num){
    int dblock_num = -1;
    if((size_t)fblock_num > inode->num_blocks){
        printf("invalid file block num");
        return dblock_num;
    }
    // if its part of direct block
    if(fblock_num<DIRECT_B_COUNT){
        dblock_num = inode->direct_blocks[fblock_num];
    }
    // single indirect
    else if(fblock_num < DIRECT_B_COUNT + SINGLE_INDIRECT_BLOCK_COUNT){
        if(inode->single_indirect==0){
            return dblock_num;
        }
        int* single_indirect_buff = (int*) read_dblock(inode->single_indirect);
        dblock_num = single_indirect_buff[fblock_num-DIRECT_B_COUNT];
        free_memory(single_indirect_buff);
    }
    // double indirect
    else{
        if(inode->double_indirect==0){
            return dblock_num;
        }
        int* double_indirect_buff = (int*) read_dblock(inode->double_indirect);
        int offset = fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT;
        dblock_num = double_indirect_buff[offset/SINGLE_INDIRECT_BLOCK_COUNT];
        free_memory(double_indirect_buff);
        if(dblock_num<=0){
            return -1;
        }
        int* single_indirect_buff = (int*) read_dblock(dblock_num);
        dblock_num = single_indirect_buff[offset%SINGLE_INDIRECT_BLOCK_COUNT];
        free_memory(single_indirect_buff);
    }
    return dblock_num;
}

bool write_dblock_to_inode(struct iNode* inode, int fblock_num, int dblock_num){
    if((size_t)fblock_num > inode->num_blocks){
        printf("invalid file block num");
        return false;
    }
    // if direct block
    if(fblock_num < DIRECT_B_COUNT){
        inode->direct_blocks[fblock_num] = dblock_num;
    }
    // single indirect
    else if(fblock_num<DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT){
        if(inode->single_indirect==0){
            return false;
        }
        int* single_indirect_buff = (int*) read_dblock(inode->single_indirect);
        single_indirect_buff[fblock_num-DIRECT_B_COUNT] = dblock_num;
        write_dblock(inode->single_indirect, (char*)single_indirect_buff);
        free_memory(single_indirect_buff);
    }
    else{
        if(inode->double_indirect==0){
            return false;
        }
        int* double_indirect_buff = (int*) read_dblock(inode->double_indirect);
        int offset = fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT;
        int single_dblock_num = double_indirect_buff[offset/SINGLE_INDIRECT_BLOCK_COUNT];
        free_memory(double_indirect_buff);
        if(single_dblock_num<=0){
            return false;
        }
        int* single_indirect_buff = (int*) read_dblock(single_dblock_num);
        single_indirect_buff[offset%SINGLE_INDIRECT_BLOCK_COUNT] = dblock_num;
        write_dblock(single_dblock_num, (char*)single_indirect_buff);
        free_memory(single_indirect_buff);
    }
    return true;
}

struct in_core_dir find_file(const char *const name, const struct iNode* const parent_inode){}

bool add_dblock_to_inode(struct iNode* inode, const int dblock_num){
    int fblock_num = inode->num_blocks;
    // if direct block
    if(fblock_num < DIRECT_B_COUNT){
        inode->direct_blocks[fblock_num] = dblock_num;
    }
    // single indirect
    else if(fblock_num<DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT){
        if(fblock_num == DIRECT_B_COUNT){
            int single_dblock_num = create_new_dblock();
            if(single_dblock_num==-1){
                return false;
            }
            inode->single_indirect = single_dblock_num;
        }
        int* single_indirect_buff = (int*) read_dblock(inode->single_indirect);
        single_indirect_buff[fblock_num-DIRECT_B_COUNT] = dblock_num;
        if(!write_dblock(inode->single_indirect, (char*)single_indirect_buff)){
            return false;
        }
        free_memory(single_indirect_buff);
    }
    else{
        if(fblock_num==DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT){
            int double_dblock_num = create_new_dblock();
            if(double_dblock_num==-1){
                return false;
            }
            inode->double_indirect = double_dblock_num;
        }
        int* double_indirect_buff = (int*) read_dblock(inode->double_indirect);
        int offset = fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT;
        if(offset%SINGLE_INDIRECT_BLOCK_COUNT == 0){
            int single_dblock_num = create_new_dblock();
            if(single_dblock_num==-1){
                return false;
            }
            double_indirect_buff[offset/SINGLE_INDIRECT_BLOCK_COUNT] = single_dblock_num;
            if(!write_dblock(inode->double_indirect, (char*)double_indirect_buff)){
                return false;
            }
        }
        int single_dblock_num = double_indirect_buff[offset/SINGLE_INDIRECT_BLOCK_COUNT];
        if(single_dblock_num<=0){
            return false;
        }
        int* single_indirect_buff = (int*) read_dblock(single_dblock_num);
        single_indirect_buff[offset%SINGLE_INDIRECT_BLOCK_COUNT] = dblock_num;
        if(!write_dblock(single_dblock_num, (char*)single_indirect_buff)){
            return false;
        }
        free_memory(double_indirect_buff);
        free_memory(single_indirect_buff);
    }
    return true;
}

// free the blocks inside single indirect
bool remove_single_indirect(int dblock_num){
    if(dblock_num<=0){
        printf("Invalid dblock num for single indirection removal");
        return true;
    }
    int* single_indirect_buff = (int*) read_dblock(dblock_num);
    for(int i=0; i<BLOCK_SIZE/ADDRESS_SIZE; i++){
        if(single_indirect_buff[i]==0){
            break;
        }
        free_dblock(single_indirect_buff[i]);
    }
    free_dblock(dblock_num);
    free_memory(single_indirect_buff);
    return true;
}

// free the blocks inside double indirect
bool remove_double_indirect(int dblock_num){
    if(dblock_num<=0){
        printf("Invalid dblock num for double indirection removal");
        return true;
    }
    int* double_indirect_buff = (int*) read_dblock(dblock_num);
    for(int i=0; i<BLOCK_SIZE/ADDRESS_SIZE; i++){
        if(double_indirect_buff[i]==0){
            break;
        }
        remove_single_indirect(double_indirect_buff[i]);
    }
    free_dblock(dblock_num);
    free_memory(double_indirect_buff);
    return true;
}

bool remove_dblocks_from_inode(struct iNode* inode, int fblock_num){
    if(fblock_num >= inode->num_blocks){
        printf("Can't remove block that doesn't exist");
        return false;
    }
    int curr_blocks = inode->num_blocks;
    inode->num_blocks = fblock_num;
    // remove direct blocks
    if(fblock_num<=DIRECT_B_COUNT){
        for(int i=fblock_num; i<curr_blocks && i<DIRECT_B_COUNT; i++){
            free_dblock(inode->direct_blocks[i]);
            inode->direct_blocks[i] = 0;
        }
        if(curr_blocks<=DIRECT_B_COUNT){
            return true;
        }
        remove_single_indirect(inode->single_indirect);
        inode->single_indirect = 0;
        if(curr_blocks <= DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT){
            return true;
        }
        remove_double_indirect(inode->double_indirect);
        inode->double_indirect = 0;
    }
    // remove single indirect blocks
    else if(fblock_num<=DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT){
        int* single_indirect_buff = (int*) read_dblock(inode->single_indirect);
        int single_indirect_offset = fblock_num - DIRECT_B_COUNT;
        int single_indirect_total = curr_blocks - DIRECT_B_COUNT;
        int blocks_to_free = SINGLE_INDIRECT_BLOCK_COUNT - single_indirect_offset;
        if(single_indirect_total < blocks_to_free){
            blocks_to_free = single_indirect_total - single_indirect_offset;
        }
        for(int i=single_indirect_offset; i<single_indirect_offset+blocks_to_free; i++){
            free_dblock(single_indirect_buff[i]);
            single_indirect_buff[i] = 0;
        }
        free_memory(single_indirect_buff);
        if(curr_blocks <= DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT){
            return true;
        }
        remove_double_indirect(inode->double_indirect);
        inode->double_indirect = 0;
    }
    // remove double indirect blocks
    else{
        int* double_indirect_buff = (int*) read_dblock(inode->double_indirect);
        int double_indirect_offset = (fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT) / SINGLE_INDIRECT_BLOCK_COUNT;
        int single_indirect_offset = (fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT) % SINGLE_INDIRECT_BLOCK_COUNT;
        // either single or partial single indirect to be removed
        if(single_indirect_offset==0){
            remove_single_indirect(double_indirect_buff[double_indirect_offset]);
            double_indirect_buff[double_indirect_offset] = 0;
        } else{
            int* single_indirect_buff = (int*) read_dblock(double_indirect_buff[double_indirect_offset]);
            for(int i=single_indirect_offset; i<SINGLE_INDIRECT_BLOCK_COUNT; i++){
                free_dblock(single_indirect_buff[i]);
                single_indirect_buff[i] = 0;
            }
            free_memory(single_indirect_buff);
        }
        // removing remaining single indirect
        int double_indirect_total = curr_blocks - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT;
        int single_blocks_to_free = (BLOCK_COUNT/ADDRESS_SIZE) - double_indirect_offset;
        if(double_indirect_total/(BLOCK_COUNT/ADDRESS_SIZE) < single_blocks_to_free){
            single_blocks_to_free = double_indirect_total - double_indirect_offset;
        }
        for(int i=double_indirect_offset+1; i<double_indirect_offset+single_blocks_to_free; i++){
            remove_single_indirect(double_indirect_buff[i]);
            double_indirect_buff[i] = 0;
        }
        free_memory(double_indirect_buff);
    }
    return true;
}

int get_inode_num_from_path(const char* const path){
    int path_len = strlen(path);
    if(path_len==1 && path[0]=='/'){
        return ROOT_INODE;
    }
    // check if present in cache
    int cached_inode_num = hash_find(&iName_cache, path);
    if(cached_inode_num>0){
        return cached_inode_num;
    }
    // get parent path
    char parent_path[path_len+1];
    if(!get_parent_path(parent_path, path, path_len)){
        return -1;
    }
    int parent_inode_num = get_inode_num_from_path(parent_path);
    if(parent_inode_num==-1){
        return -1;
    }
    struct iNode* inode = read_inode(parent_inode_num);
    char child_name[path_len+1];
    if(!get_child_name(child_name, path, path_len)){
        free_memory(inode);
        return -1;
    }
    // find the file
    struct in_core_dir file = find_file(child_name, inode);
    if(file.start_pos==-1){
        free_memory(inode);
        return -1;
    }
    int inode_num = ((int*) (file.dblock + file.start_pos))[0];
    free_memory(file.dblock);
    free_memory(inode);
    hash_insert(&iName_cache, path, inode_num);
    return inode_num;
}

int create_new_file(const char* const path, struct iNode** buff, mode_t mode){
    // getting parent path
    int path_len = strlen(path);
    char parent_path[path_len+1];
    if(get_parent_path(parent_path, path, path_len)){
        // no parent path
        return -ENOENT;
    }
    int parent_inode_num = get_inode_num_from_path(parent_path);
    if(parent_inode_num==-1){
        // parent path doesnt exist
        return -ENOENT;
    }
    int child_inode_num = get_inode_num_from_path(path);
    if(child_inode_num!=-1){
        // file already exists
        return -EEXIST;
    }
    struct iNode* parent_inode = read_inode(parent_inode);
    if(!S_ISDIR(parent_inode->mode)){
        // parent is not a directory
        free_memory(parent_inode);
        return -ENOENT;
    }
    // TODO : Check Permissions
    char child_name[path_len+1];
    if(!get_child_name(child_name, path, path_len)){
        return -1;
    }
    // create new inode
    child_inode_num = create_new_inode();
    if(child_inode_num==-1){
        free_memory(parent_inode);
        return -EDQUOT;
    }
    if(!add_new_entry(parent_inode, child_inode_num, child_name)){
        free_inode(child_inode_num);
        free_memory(parent_inode);
        return -EDQUOT;
    }
    time_t curr_time = time(NULL);
    parent_inode->modification_time = curr_time;
    parent_inode->status_change_time = curr_time;
    // TODO: Check this
    parent_inode->access_time = curr_time;
    if(!write_inode(parent_inode_num, parent_inode)){
        free_memory(parent_inode);
        return -1;
    }
    *buff = read_inode(child_inode_num);
    (*buff)->link_count++;
    (*buff)->mode = mode;
    (*buff)->num_blocks = 0;
    (*buff)->file_size = 0;
    (*buff)->access_time = curr_time;
    (*buff)->modification_time = curr_time;
    (*buff)->creation_time = curr_time;
    (*buff)->status_change_time = curr_time;
    free_memory(parent_inode);
    return child_inode_num;
}

bool is_empty_dir(struct iNode* inode){
    if(inode->num_blocks>1){
        return false;
    }
    // should be the last entry if empty
    struct in_core_dir parent_ref = find_file("..", inode);
    if(parent_ref.start_pos==-1){
        return false;
    }
    int next_entry = (int*)(parent_ref.dblock+parent_ref.start_pos+INODE_SZ)[0];
    return parent_ref.start_pos+next_entry == BLOCK_SIZE;
}

bool custom_mkdir(const char* path, mode_t mode){
    struct inode* child_inode = NULL;
    int child_inode_num = create_new_file(path, &child_inode, S_IFDIR|mode);
    if(child_inode_num<=-1){
        free_memory(child_inode);
        return false;
    }
    char* name = ".";
    if(!add_new_entry(child_inode, child_inode_num, name)){
        free_memory(child_inode);
        return false;
    }
    name = "..";
    if(!add_new_entry(child_inode, child_inode_num, name)){
        free_memory(child_inode);
        return false;
    }
    if(!write_inode(child_inode_num, child_inode)){
        free_memory(child_inode);
        return false;
    }
    free_memory(child_inode);
    return true;
}

bool custom_mknod(const char* path, mode_t mode, dev_t dev){
    dev = 0; // Silence error until used
    struct iNode* child_inode = NULL;

    DEBUG_PRINTF("MKNOD mode passed: %d\n", mode);

    int child_inode_num = create_new_file(path, &child_inode_num, mode);

    if (child_inode_num <= -1) {
        printf("CANNOT CREATE FILE\n");
        free_memory(child_inode);
        return false;
    }
    if(!write_inode(child_inode_num, child_inode)){
        free_memory(child_inode);
        return false;
    }
    free_memory(child_inode);
    return true;
}

int custom_truncate(const char* path, size_t offset){}

int custom_unlik(const char* path){}

bool custom_close(int file_descriptor){}

int custom_open(){}

ssize_t custom_read(const char* path, void* buff, size_t nbytes, size_t offset){}

ssize_t custom_write(const char* path, void* buff, size_t nbytes, size_t offset){}

bool add_new_entry(struct iNode* inode, int inode_num, char* inode_name){
    if(!S_ISDIR(inode->mode)){
        printf("not a directory\n");
        return false;
    }
    int name_length = strlen(inode_name);
    if(name_length > MAX_NAME_LENGTH){
        printf("Too long name for file\n");
        return false;
    }
    unsigned short short_name_length = name_length;
    size_t new_entry_size = INODE_SZ + ADDRESS_PTR_SZ + STRING_LENGTH_SZ + short_name_length;
    if(inode->num_blocks!=0){
        int dblock_num = fblock_num_to_dblock_num(inode, inode->num_blocks-1);
        if(dblock_num<=0){
            return false;
        }
        char* dblock = read_dblock(dblock_num);
        int curr_pos = 0;
        while(curr_pos < BLOCK_SIZE){
            int next_entry = ((int*)(dblock+curr_pos+INODE_SZ))[0];
            unsigned short entry_name_length = ((unsigned short* )(dblock+curr_pos+INODE_SZ+ADDRESS_PTR_SZ))[0];
            int entry_length = INODE_SZ + ADDRESS_PTR_SZ + STRING_LENGTH_SZ + entry_name_length;
            if(next_entry - entry_length >= new_entry_size){
                // updating curr entry addr with size of entry
                ((int* )(dblock+curr_pos+INODE_SZ))[0] = entry_length;
                if(entry_length<=0){
                    return false;
                }
                int addr_ptr = next_entry - entry_length;
                curr_pos += entry_length;
                // add new entry
                ((int*) (dblock+curr_pos))[0] = inode_num;
                ((int*) (dblock+curr_pos+INODE_SZ))[0] = addr_ptr;
                ((unsigned short*) (dblock+curr_pos+INODE_SZ+ADDRESS_PTR_SZ))[0] = short_name_length;
                strncpy((char*) (dblock+curr_pos+INODE_SZ+ADDRESS_PTR_SZ+STRING_LENGTH_SZ), inode_name, name_length);
                // KYA CHAL RAHA H BC
                if(!write_dblock(dblock_num, dblock)){
                    return false;
                }
                return true;
            }
            if(next_entry<=0){
                return false;
            }
            curr_pos += next_entry;
        }
        free_memory(dblock);
    }
    int dblock_num = create_new_dblock();
    if(dblock_num<=0){
        return false;
    }
    char dblock[BLOCK_SIZE];
    memset(dblock, 0, BLOCK_SIZE);
    ((int*) dblock)[0] = inode_num;
    size_t addr_ptr = BLOCK_SIZE;
    ((int*) (dblock+INODE_SZ))[0] = addr_ptr;
    ((unsigned short*) (dblock+INODE_SZ+ADDRESS_PTR_SZ))[0] = short_name_length;
    strncpy((char*)(dblock+INODE_SZ+ADDRESS_PTR_SZ+STRING_LENGTH_SZ), inode_name, name_length);
    if(!write_dblock(dblock_num, dblock)){
        return false;
    }
    add_dblock_to_inode(inode, dblock_num);
    inode->file_size += BLOCK_SIZE;
    return true;
}

bool init_file_layer(){
    if(!make_fs()){
        return false;
    }
    struct iNode* root = read_inode(ROOT_INODE);
    if(root==NULL){
        return false;
    }
    root->allocated = true;
    root->link_count++;
    root->mode = S_IFDIR | DEFAULT_PERMS;
    root->num_blocks = 0;
    root->file_size = 0;
    time_t curr_time = time(NULL);
    root->access_time = curr_time;
    root->creation_time = curr_time;
    root->modification_time = curr_time;
    root->status_change_time = curr_time;

    char* root_name = ".";
    if(!add_new_entry(root, ROOT_INODE, root_name)){
        return false;
    }
    root_name = "..";
    if(!add_new_entry(root, ROOT_INODE, root_name)){
        return false;
    }
    if(!write_inode(ROOT_INODE, root)){
        return false;
    }
    free_memory(root);
    init_hash_table(&iName_cache, CACHE_SIZE);
    DEBUG_PRINTF("File layer initialization done \n");
    return true;
}