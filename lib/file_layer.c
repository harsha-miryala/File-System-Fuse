#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include "../include/debug.h"
#include "../include/disk_layer.h"
#include "../include/file_layer.h"
#include "../include/lru_cache.h"

static struct lru_cache iname_cache;

// Returns char array index of the last slash of parent's path
ssize_t get_parent_id(const char* const path, ssize_t path_len){
    // TODO: Ignore multiple consecutive or figure what should happen here "/" i.e. /dev/dvb/data//file.c
    for(ssize_t id=path_len-1; id>=0; id--){
        if(path[id]=='/' && id!=(path_len-1)){
            return id;
        }
    }
    return -1;
}

// copy parent path into a given buffer
bool get_parent_path(char* const buff, const char* const path, ssize_t path_len){
    ssize_t pos = get_parent_id(path, path_len);
    if(pos==-1){
        return false;
    }
    memcpy(buff, path, pos+1);
    buff[pos+1] = '\0';
    return true;
}

// copy child file into a given buffer
bool get_child_name(char* const buff, const char* const path, ssize_t path_len){
    ssize_t start = get_parent_id(path, path_len);
    if(start==-1){
        return false;
    }
    start++;
    ssize_t end = path_len;
    if(path[end-1]=='/'){
        // removing trailing slash
        end--;
    }
    memcpy(buff, path+start, end-start);
    buff[end-start]='\0';
    return true;
}

// get dblock num corr to file block number
ssize_t fblock_num_to_dblock_num(const struct iNode* const inode, ssize_t fblock_num){
    ssize_t dblock_num = -1;
    if(fblock_num > inode->num_blocks){
        printf("invalid file block num - %ld with block count as %ld\n", fblock_num, inode->num_blocks);
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
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(inode->single_indirect);
        dblock_num = single_indirect_buff[fblock_num-DIRECT_B_COUNT];
        free_memory(single_indirect_buff);
    }
    // double indirect
    else if(fblock_num < DIRECT_B_COUNT + SINGLE_INDIRECT_BLOCK_COUNT + DOUBLE_INDIRECT_BLOCK_COUNT){
        if(inode->double_indirect==0){
            return dblock_num;
        }
        ssize_t* double_indirect_buff = (ssize_t*) read_dblock(inode->double_indirect);
        ssize_t offset = fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT;
        dblock_num = double_indirect_buff[offset/SINGLE_INDIRECT_BLOCK_COUNT];
        free_memory(double_indirect_buff);
        if(dblock_num<=0){
            return -1;
        }
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(dblock_num);
        dblock_num = single_indirect_buff[offset%SINGLE_INDIRECT_BLOCK_COUNT];
        free_memory(single_indirect_buff);
    }
    // triple indirect
    else{
        if(inode->triple_indirect==0){
            return dblock_num;
        }
        ssize_t* triple_indirect_buff = (ssize_t*) read_dblock(inode->triple_indirect);
        ssize_t offset = fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT - DOUBLE_INDIRECT_BLOCK_COUNT;
        dblock_num = triple_indirect_buff[offset/DOUBLE_INDIRECT_BLOCK_COUNT];
        free_memory(triple_indirect_buff);
        if(dblock_num<=0){
            return -1;
        }
        ssize_t* double_indirect_buff = (ssize_t*) read_dblock(dblock_num);
        dblock_num = double_indirect_buff[(offset/SINGLE_INDIRECT_BLOCK_COUNT)%SINGLE_INDIRECT_BLOCK_COUNT];
        free_memory(double_indirect_buff);
        if(dblock_num<=0){
            return -1;
        }
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(dblock_num);
        dblock_num = single_indirect_buff[offset%SINGLE_INDIRECT_BLOCK_COUNT];
        free_memory(single_indirect_buff);
    }
    return dblock_num;
}

bool write_dblock_to_inode(struct iNode* inode, ssize_t fblock_num, ssize_t dblock_num){
    // over-writing an existing block with a new block
    if(fblock_num > inode->num_blocks){
        printf("invalid file block num");
        return false;
    }
    // if direct block
    if(fblock_num < DIRECT_B_COUNT){
        inode->direct_blocks[fblock_num] = dblock_num;
    }
    // single indirect
    else if(fblock_num < DIRECT_B_COUNT + SINGLE_INDIRECT_BLOCK_COUNT){
        if(inode->single_indirect==0){
            return false;
        }
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(inode->single_indirect);
        single_indirect_buff[fblock_num-DIRECT_B_COUNT] = dblock_num;
        write_dblock(inode->single_indirect, (char*)single_indirect_buff);
        free_memory(single_indirect_buff);
    }
    // double indirect
    else if(fblock_num < DIRECT_B_COUNT + SINGLE_INDIRECT_BLOCK_COUNT + DOUBLE_INDIRECT_BLOCK_COUNT){
        if(inode->double_indirect==0){
            return false;
        }
        ssize_t* double_indirect_buff = (ssize_t*) read_dblock(inode->double_indirect);
        ssize_t offset = fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT;
        ssize_t single_dblock_num = double_indirect_buff[offset/SINGLE_INDIRECT_BLOCK_COUNT];
        free_memory(double_indirect_buff);
        if(single_dblock_num<=0){
            return false;
        }
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(single_dblock_num);
        single_indirect_buff[offset%SINGLE_INDIRECT_BLOCK_COUNT] = dblock_num;
        write_dblock(single_dblock_num, (char*)single_indirect_buff);
        free_memory(single_indirect_buff);
    }
    // triple indirect
    else{
        if(inode->triple_indirect==0){
            return dblock_num;
        }
        ssize_t* triple_indirect_buff = (ssize_t*) read_dblock(inode->triple_indirect);
        ssize_t offset = fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT - DOUBLE_INDIRECT_BLOCK_COUNT;
        ssize_t double_dblock_num = triple_indirect_buff[offset/DOUBLE_INDIRECT_BLOCK_COUNT];
        free_memory(triple_indirect_buff);
        if(double_dblock_num<=0){
            return -1;
        }
        ssize_t* double_indirect_buff = (ssize_t*) read_dblock(double_dblock_num);
        ssize_t single_dblock_num = double_indirect_buff[(offset/SINGLE_INDIRECT_BLOCK_COUNT)%SINGLE_INDIRECT_BLOCK_COUNT];
        free_memory(double_indirect_buff);
        if(single_dblock_num<=0){
            return -1;
        }
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(single_dblock_num);
        single_indirect_buff[offset%SINGLE_INDIRECT_BLOCK_COUNT] = dblock_num;
        write_dblock(single_dblock_num, (char*)single_indirect_buff);
        free_memory(single_indirect_buff);
    }
    return true;
}

// TODO: VERIFY// Done
struct file_pos_in_dir find_file(const char* const name, const struct iNode* const parent_inode){
    /*
    Each file entry in the directory is stored in the following format.
    INUM, RECORD_LEN/ADDR_PTR, FILE_STR_LEN, FILE_NAME
    INUM, RECORD_LEN/ADDR_PTR, FILE_STR_LEN sizes are predefined in file_layer.h
    as INODE_SZ, ADDRESS_PTR_SZ, STRING_LENGTH_SZ. Only the FILE_NAME size is variable based on len.
    */
    struct file_pos_in_dir file;
    /*
    This data structure is what is returned.
    It will hold the datablock containing the file_record, corresponding dBlock_num, fblock_num,
    pointer to the inode_num for the file (1st field of record) and a pointer to the record
    previous to the file to be found. This is sent for the sake of manipulation wrt to records during deletion.
    */
    // if the file is found, the pointer to the inode is present in file.start_pos
    // file.prev_entry holds the starting point of the preceding entry the file entry in question.
    file.prev_entry = -1;
    //if not a directory, return
    // printf("Inside find_file(), checking is this a dir\n");
    // printf("%ld\n",S_ISDIR(parent_inode->mode));
    if(!S_ISDIR(parent_inode->mode)){
        file.start_pos = -1;
        return file;
    }
    // TODO : Check if name is in bounds, else return garbage
    ssize_t name_length = strlen(name);
    // traverse through the datablocks in parent directory to find the entry for the file.
    // printf("parent_inode->num_blocks:%ld\n",parent_inode->num_blocks);
    for(ssize_t i=0; i<parent_inode->num_blocks; i++){
        file.dblock_num = fblock_num_to_dblock_num(parent_inode, i);
        file.fblock_num = i;
        file.prev_entry = -1; // prev_entry will ideally contain the pointer record which preceeds the file record.
        // However, it will be initialied to -1 in the beginning for each block.
        if(file.dblock_num<=0){
            file.start_pos = -1;
            printf("file.dblock_num<=0\n");
            return file;
        }
        file.dblock = read_dblock(file.dblock_num);
        ssize_t curr_pos = 0;
        // curr_pos points to the beginning of a record that we are currently inspecting.
        // It will be updated based on the record_len field of the curr_record.
        while(curr_pos<BLOCK_SIZE){
            if(((ssize_t*) (file.dblock+curr_pos))[0]!=0){ //refers to inum for an entry
                ssize_t str_start_pos = curr_pos + INODE_SZ + ADDRESS_PTR_SZ + STRING_LENGTH_SZ;
                //points to the start of filename string for the record.
                unsigned short str_length = ((unsigned short*) (file.dblock+curr_pos+INODE_SZ+ADDRESS_PTR_SZ))[0];
                if(str_length==name_length && strncmp(file.dblock+str_start_pos, name, name_length)==0){
                    //checks if the file_name and len matches that with the curr_record
                    file.start_pos = curr_pos;
                    return file;
                }
            }
            // if there is no match
            ssize_t next_entry_offset = ((ssize_t*) (file.dblock+curr_pos+INODE_SZ))[0]; //rec_len corresponding to curr_pos
            if(next_entry_offset<=0){
                file.start_pos = -1;
                printf("next_entry_offset: %ld\n",next_entry_offset);
                return file;
            }
            file.prev_entry = curr_pos; //prev will now point to curr and curr will point to the next record in the block.
            curr_pos += next_entry_offset;
        }
    }
    //record not found
    // printf("record not found\n");
    file.start_pos = -1; // -1 is set to start_pos when the record is not found.
    file.prev_entry = -1;
    return file;
}

bool add_dblock_to_inode(struct iNode* inode, const ssize_t dblock_num){
    // adding a new data block in inode, num_blocks would already be incremented
    ssize_t fblock_num = inode->num_blocks;
    // if direct block
    if(fblock_num < DIRECT_B_COUNT){
        inode->direct_blocks[fblock_num] = dblock_num;
    }
    // single indirect
    else if(fblock_num < DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT){
        if(fblock_num == DIRECT_B_COUNT){
            ssize_t single_dblock_num = create_new_dblock();
            if(single_dblock_num==-1){
                return false;
            }
            inode->single_indirect = single_dblock_num;
        }
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(inode->single_indirect);
        single_indirect_buff[fblock_num-DIRECT_B_COUNT] = dblock_num;
        if(!write_dblock(inode->single_indirect, (char*)single_indirect_buff)){
            return false;
        }
        free_memory(single_indirect_buff);
    }
    // double indirect
    else if(fblock_num < DIRECT_B_COUNT + SINGLE_INDIRECT_BLOCK_COUNT + DOUBLE_INDIRECT_BLOCK_COUNT){
        if(fblock_num==DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT){
            ssize_t double_dblock_num = create_new_dblock();
            if(double_dblock_num==-1){
                return false;
            }
            inode->double_indirect = double_dblock_num;
        }
        ssize_t* double_indirect_buff = (ssize_t*) read_dblock(inode->double_indirect);
        ssize_t offset = fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT;
        if(offset%SINGLE_INDIRECT_BLOCK_COUNT == 0){
            ssize_t single_dblock_num = create_new_dblock();
            if(single_dblock_num==-1){
                return false;
            }
            double_indirect_buff[offset/SINGLE_INDIRECT_BLOCK_COUNT] = single_dblock_num;
            if(!write_dblock(inode->double_indirect, (char*)double_indirect_buff)){
                return false;
            }
        }
        ssize_t single_dblock_num = double_indirect_buff[offset/SINGLE_INDIRECT_BLOCK_COUNT];
        if(single_dblock_num<=0){
            return false;
        }
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(single_dblock_num);
        single_indirect_buff[offset%SINGLE_INDIRECT_BLOCK_COUNT] = dblock_num;
        if(!write_dblock(single_dblock_num, (char*)single_indirect_buff)){
            return false;
        }
        free_memory(double_indirect_buff);
        free_memory(single_indirect_buff);
    }
    // triple indirect
    else{
        if(fblock_num==DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT+DOUBLE_INDIRECT_BLOCK_COUNT){
            ssize_t triple_dblock_num = create_new_dblock();
            if(triple_dblock_num==-1){
                return false;
            }
            inode->triple_indirect = triple_dblock_num;
        }
        ssize_t* triple_indirect_buff = (ssize_t*) read_dblock(inode->triple_indirect);
        ssize_t triple_fblock_num = fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT - DOUBLE_INDIRECT_BLOCK_COUNT;
        ssize_t triple_fblock_offset = triple_fblock_num/DOUBLE_INDIRECT_BLOCK_COUNT;
        if(triple_fblock_num % DOUBLE_INDIRECT_BLOCK_COUNT == 0){
            ssize_t double_dblock_num = create_new_dblock();
            if(double_dblock_num==-1){
                return false;
            }
            triple_indirect_buff[triple_fblock_offset] = double_dblock_num;
            if(!write_dblock(inode->triple_indirect, (char *)triple_indirect_buff)){
                return false;
            }
        }
        ssize_t* double_indirect_buff = (ssize_t*) read_dblock(triple_indirect_buff[triple_fblock_offset]);
        if(triple_fblock_num % SINGLE_INDIRECT_BLOCK_COUNT == 0){
            ssize_t single_dblock_num = create_new_dblock();
            if(single_dblock_num==-1){
                return false;
            }
            double_indirect_buff[(triple_fblock_num/SINGLE_INDIRECT_BLOCK_COUNT)%SINGLE_INDIRECT_BLOCK_COUNT] = single_dblock_num;
            if(!write_dblock(triple_indirect_buff[triple_fblock_offset], (char*)double_indirect_buff)){
                return false;
            }
        }
        ssize_t double_fblock_offset = (triple_fblock_num/SINGLE_INDIRECT_BLOCK_COUNT)%SINGLE_INDIRECT_BLOCK_COUNT;
        ssize_t single_dblock_num = double_indirect_buff[double_fblock_offset];
        if(single_dblock_num<=0){
            return false;
        }
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(single_dblock_num);
        single_indirect_buff[triple_fblock_num % SINGLE_INDIRECT_BLOCK_COUNT] = dblock_num;
        if(!write_dblock(single_dblock_num, (char*)single_indirect_buff)){
            return false;
        }
        free_memory(triple_indirect_buff);
        free_memory(double_indirect_buff);
        free_memory(single_indirect_buff);
    }
    inode->num_blocks++;
    return true;
}

// free the blocks inside single indirect
bool remove_single_indirect(ssize_t dblock_num){
    if(dblock_num<=0){
        printf("Invalid dblock num for single indirection removal");
        return true;
    }
    ssize_t* single_indirect_buff = (ssize_t*) read_dblock(dblock_num);
    for(ssize_t i=0; i<BLOCK_SIZE/ADDRESS_SIZE; i++){
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
bool remove_double_indirect(ssize_t dblock_num){
    if(dblock_num<=0){
        printf("Invalid dblock num for double indirection removal");
        return true;
    }
    ssize_t* double_indirect_buff = (ssize_t*) read_dblock(dblock_num);
    for(ssize_t i=0; i<BLOCK_SIZE/ADDRESS_SIZE; i++){
        if(double_indirect_buff[i]==0){
            break;
        }
        remove_single_indirect(double_indirect_buff[i]);
    }
    free_dblock(dblock_num);
    free_memory(double_indirect_buff);
    return true;
}

// free the blocks inside triple indirect
bool remove_triple_indirect(ssize_t dblock_num){
    if(dblock_num<=0){
        printf("Invalid dblock num for triple indirection removal");
        return true;
    }
    ssize_t* triple_indirect_buff = (ssize_t*) read_dblock(dblock_num);
    for(ssize_t i=0; i<BLOCK_SIZE/ADDRESS_SIZE; i++){
        if(triple_indirect_buff[i]==0){
            break;
        }
        remove_double_indirect(triple_indirect_buff[i]);
    }
    free_dblock(dblock_num);
    free_memory(triple_indirect_buff);
    return true;
}

bool remove_dblocks_from_inode(struct iNode* inode, ssize_t fblock_num){
    // remove blocks from fblock_num to num_blocks from inode
    if(fblock_num >= inode->num_blocks){
        printf("Can't remove block that doesn't exist");
        return false;
    }
    ssize_t curr_blocks = inode->num_blocks;
    inode->num_blocks = fblock_num;
    // remove direct blocks
    if(fblock_num<=DIRECT_B_COUNT){
        for(ssize_t i=fblock_num; i<curr_blocks && i<DIRECT_B_COUNT; i++){
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
        if(curr_blocks <= DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT+DOUBLE_INDIRECT_BLOCK_COUNT){
            return true;
        }
        remove_double_indirect(inode->triple_indirect);
        inode->triple_indirect = 0;
    }
    // remove single indirect blocks
    else if(fblock_num <= DIRECT_B_COUNT+SINGLE_INDIRECT_BLOCK_COUNT){
        ssize_t* single_indirect_buff = (ssize_t*) read_dblock(inode->single_indirect);
        ssize_t single_indirect_offset = fblock_num - DIRECT_B_COUNT;
        ssize_t single_indirect_total = curr_blocks - DIRECT_B_COUNT;
        ssize_t blocks_to_free = SINGLE_INDIRECT_BLOCK_COUNT - single_indirect_offset;
        if(single_indirect_total < blocks_to_free){
            blocks_to_free = single_indirect_total - single_indirect_offset;
        }
        for(ssize_t i=single_indirect_offset; i<single_indirect_offset+blocks_to_free; i++){
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
    else if (fblock_num <= DIRECT_B_COUNT + SINGLE_INDIRECT_BLOCK_COUNT + DOUBLE_INDIRECT_BLOCK_COUNT){
        ssize_t* double_indirect_buff = (ssize_t*) read_dblock(inode->double_indirect);
        ssize_t double_indirect_offset = (fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT) / SINGLE_INDIRECT_BLOCK_COUNT;
        ssize_t single_indirect_offset = (fblock_num - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT) % SINGLE_INDIRECT_BLOCK_COUNT;
        // either single or partial single indirect to be removed
        if(single_indirect_offset==0){
            remove_single_indirect(double_indirect_buff[double_indirect_offset]);
            double_indirect_buff[double_indirect_offset] = 0;
        } else{
            ssize_t* single_indirect_buff = (ssize_t*) read_dblock(double_indirect_buff[double_indirect_offset]);
            for(ssize_t i=single_indirect_offset; i<SINGLE_INDIRECT_BLOCK_COUNT; i++){
                free_dblock(single_indirect_buff[i]);
                single_indirect_buff[i] = 0;
            }
            free_memory(single_indirect_buff);
        }
        // removing remaining single indirect
        // TODO : Can simplify this
        ssize_t double_indirect_total = curr_blocks - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT;
        ssize_t single_blocks_to_free = SINGLE_INDIRECT_BLOCK_COUNT - double_indirect_offset;
        if(double_indirect_total/SINGLE_INDIRECT_BLOCK_COUNT < single_blocks_to_free){
            single_blocks_to_free = double_indirect_total - double_indirect_offset;
        }
        for(ssize_t i=double_indirect_offset+1; i<double_indirect_offset+single_blocks_to_free; i++){
            remove_single_indirect(double_indirect_buff[i]);
            double_indirect_buff[i] = 0;
        }
        free_memory(double_indirect_buff);
    }
    // remove triple indirect blocks
    else{
        ssize_t* triple_indirect_buff = (ssize_t*) read_dblock(inode->triple_indirect);
        ssize_t triple_fblock_num = fblock_num - (DIRECT_B_COUNT + SINGLE_INDIRECT_BLOCK_COUNT + DOUBLE_INDIRECT_BLOCK_COUNT);
        ssize_t triple_indirect_offset = triple_fblock_num / DOUBLE_INDIRECT_BLOCK_COUNT;
        ssize_t double_indirect_offset = (triple_fblock_num / SINGLE_INDIRECT_BLOCK_COUNT) % SINGLE_INDIRECT_BLOCK_COUNT;
        ssize_t single_indirect_offset = triple_fblock_num % SINGLE_INDIRECT_BLOCK_COUNT;
        if(single_indirect_offset==0 && double_indirect_offset==0){
            remove_double_indirect(triple_indirect_buff[triple_indirect_offset]);
            triple_indirect_buff[triple_indirect_offset] = 0;
        } else if(single_indirect_offset==0){
            ssize_t* double_indirect_buff = (ssize_t*) read_dblock(triple_indirect_buff[triple_indirect_offset]);
            for(ssize_t i=double_indirect_offset; i<SINGLE_INDIRECT_BLOCK_COUNT; i++){
                remove_single_indirect(double_indirect_buff[i]);
                double_indirect_buff[i] = 0;
            }
            free_memory(double_indirect_buff);
        } else{
            ssize_t* double_indirect_buff = (ssize_t*) read_dblock(triple_indirect_buff[triple_indirect_offset]);
            ssize_t* single_indirect_buff = (ssize_t*) read_dblock(double_indirect_buff[double_indirect_offset]);
            for(ssize_t i=single_indirect_offset; i<SINGLE_INDIRECT_BLOCK_COUNT; i++){
                free_dblock(single_indirect_buff[i]);
                single_indirect_buff[i] = 0;
            }
            for(ssize_t i=double_indirect_offset+1; i<SINGLE_INDIRECT_BLOCK_COUNT; i++){
                remove_single_indirect(double_indirect_buff[i]);
                double_indirect_buff[i] = 0;
            }
            free_memory(double_indirect_buff);
            free_memory(single_indirect_buff);
        }
        ssize_t triple_indirect_total = curr_blocks - DIRECT_B_COUNT - SINGLE_INDIRECT_BLOCK_COUNT - DOUBLE_INDIRECT_BLOCK_COUNT;
        ssize_t double_blocks_to_free = SINGLE_INDIRECT_BLOCK_COUNT - triple_indirect_offset;
        if(triple_indirect_total < double_blocks_to_free){
            double_blocks_to_free = triple_indirect_total - triple_indirect_offset;
        }
        for(ssize_t i=triple_indirect_offset+1; i<triple_indirect_offset+double_blocks_to_free; i++){
            remove_double_indirect(triple_indirect_buff[i]);
            triple_indirect_buff[i] = 0;
        }
        free_memory(triple_indirect_buff);
    }
    return true;
}

ssize_t get_inode_num_from_path(const char* const path){
    ssize_t path_len = strlen(path);
    if(path_len==1 && path[0]=='/'){
        return ROOT_INODE;
    }
    // check if present in cache
    ssize_t cached_inode_num = get_cache(&iname_cache, path);
    if(cached_inode_num>0){
        return cached_inode_num;
    }
    // otherwise compute using recursive validation
    // get parent path
    char parent_path[path_len+1];
    if(!get_parent_path(parent_path, path, path_len)){
        return -1;
    }
    ssize_t parent_inode_num = get_inode_num_from_path(parent_path);
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
    struct file_pos_in_dir file = find_file(child_name, inode);
    if(file.start_pos==-1){
        free_memory(inode);
        return -1;
    }
    ssize_t inode_num = ((ssize_t*) (file.dblock + file.start_pos))[0];
    free_memory(file.dblock);
    free_memory(inode);
    set_cache(&iname_cache, path, inode_num);
    return inode_num;
}

ssize_t create_new_file(const char* const path, struct iNode** buff, mode_t mode){
    // getting parent path
    ssize_t path_len = strlen(path);
    char parent_path[path_len+1];
    if(!get_parent_path(parent_path, path, path_len)){
        // no parent path
        printf("No parent path exists\n");
        return -ENOENT;
    }
    ssize_t parent_inode_num = get_inode_num_from_path(parent_path);
    if(parent_inode_num==-1){
        printf("Invalid parent path : %s\n", parent_path);
        return -ENOENT;
    }
    ssize_t child_inode_num = get_inode_num_from_path(path);
    if(child_inode_num!=-1){
        // file already exists
        printf("File already exists\n");
        return -EEXIST;
    }
    struct iNode* parent_inode = read_inode(parent_inode_num);
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
    // TODO : write inode 
    return child_inode_num;
}

bool is_empty_dir(struct iNode* inode){
    printf("checking if dir is empty\n");
    if(inode->num_blocks>1){
        printf("No of dblocks %ld\n", inode->num_blocks);
        return false;
    }
    // should be the last entry if empty
    struct file_pos_in_dir parent_ref = find_file("..", inode);
    if(parent_ref.start_pos==-1){
        printf("Did not find the location of .. for the dir to check if its empty\n");
        return false;
    }
    ssize_t next_entry = ((ssize_t*)(parent_ref.dblock+parent_ref.start_pos+INODE_SZ))[0];
    return parent_ref.start_pos+next_entry == BLOCK_SIZE;
}

bool custom_mkdir(const char* path, mode_t mode){
    struct iNode* child_inode = NULL;
    ssize_t child_inode_num = create_new_file(path, &child_inode, S_IFDIR|mode);
    printf("Inode Num : %ld assigned for the new dir : %s\n", child_inode_num, path);
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
    // TODO: Buggy code
    dev = 0; // Silence error until used
    struct iNode* child_inode = NULL;

    DEBUG_PRINTF("MKNOD mode passed: %ld\n", mode);

    ssize_t child_inode_num = create_new_file(path, &child_inode, mode);

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

ssize_t custom_truncate(const char* path, size_t offset){
    ssize_t inode_num = get_inode_num_from_path(path);
    if(inode_num==-1){
        return -1;
    }
    struct iNode* inode = read_inode(inode_num);
    if(offset>inode->file_size){
        free_memory(inode);
        return -1;
    }
    if(offset==0 && inode->file_size==0){
        free_memory(inode);
        return 0;
    }
    ssize_t curr_block = offset/BLOCK_SIZE;
    if(inode->num_blocks > curr_block+1){
        // this removes everything from curr_block+1
        remove_dblocks_from_inode(inode, curr_block+1);
    }
    // get_curr_block
    ssize_t dblock_num = fblock_num_to_dblock_num(inode, curr_block);
    if(dblock_num<=0){
        return -1;
    }
    char* dblock_buff = read_dblock(dblock_num);
    ssize_t block_offset = offset % BLOCK_SIZE;
    memset(dblock_buff+block_offset, 0, BLOCK_SIZE-block_offset);
    write_dblock(dblock_num, dblock_buff);
    free_memory(dblock_buff);
    inode->file_size = offset;
    write_inode(inode_num, inode);
    free_memory(inode);
    return 0;
}

ssize_t custom_unlink(const char* path){
    ssize_t path_len = strlen(path);

    ssize_t inum = get_inode_num_from_path(path);
    // printf("Inside custom_unlink()! inum val is %ld \n", inum);
    if(inum==-1){
        DEBUG_PRINTF("FILE LAYER UNLINK ERROR: Inode for %s not found\n", path);
        return -EEXIST;
    }
    struct iNode *inode= read_inode(inum);
    // if the path is a dir and it is not empty.
    if(S_ISDIR(inode->mode) && !is_empty_dir(inode)){
        DEBUG_PRINTF("FILE LAYER ERROR: Unlink failed as the dir %s is not empty\n", path);
        printf("Dir %s is not empty\n",path);
        free_memory(inode);
        return -ENOTEMPTY;
    }

    char parent_path[path_len+1];

    //should copy the parent path to buffer parent_path
    if(!get_parent_path(parent_path, path, path_len)){
        DEBUG_PRINTF("FILE LAYER ERROR: Failed to unlink %s\n",path);
        printf("couldn't fetch parent path during unlink\n");
        free_memory(inode);
        return -EINVAL;
    }

    ssize_t parent_inode_num = get_inode_num_from_path(parent_path);

    if(parent_inode_num == -1){
        printf("parent inum -1\n");
        return -1;
    }

    struct iNode *parent_inode = read_inode(parent_inode_num);

    char child_name[path_len+1];
    if(!get_child_name(child_name, path, path_len)){
        return -1;
    }

    //find the location of the path file entry in the parent.
    struct file_pos_in_dir file = find_file(child_name, parent_inode);

    DEBUG_PRINTF("Path : %s, Parent Inode: %ld, Pos in Dir: %ld, DBlock: %ld, Block Count: %ld\n",
           path, parent_inode_num, file.start_pos, file.dblock_num, parent_inode->num_blocks);

    // Didnt find file in parent
    if(file.start_pos == -1){
        return -1;
    }

    ssize_t next_entry_offset_from_curr = ((ssize_t *)(file.dblock + file.start_pos + INODE_SZ))[0];
    if(file.prev_entry!=-1){
        // There is a preceeding record to the curr record entry
        // Need to increment its pointer by this pointer
        // so we dont traverse that record in the future.
        // this means F1, F2, F3 and now we delete F3 and free it up or delete F2 and then point from F1 to F3
        ((ssize_t *)(file.dblock + file.prev_entry + INODE_SZ))[0]+= next_entry_offset_from_curr;
        write_dblock(file.dblock_num, file.dblock);
    }  
    else if(file.start_pos == 0 && next_entry_offset_from_curr != BLOCK_SIZE){
        // this is the first entry in the directory and it is followed by other entries
        // Move next entry to the start of block
        // F1, F2, F3 and if you delete F1, F2 will be moved to F1 while also updating offset
        unsigned short next_entry_name_len = ((unsigned short *)(file.dblock + next_entry_offset_from_curr + INODE_SZ + ADDRESS_PTR_SZ))[0];//next entry namestr len
        ssize_t next_entry_len = INODE_SZ + ADDRESS_PTR_SZ + STRING_LENGTH_SZ + next_entry_name_len;
        memcpy(file.dblock, file.dblock+ next_entry_offset_from_curr, next_entry_len); //2nd entry is copied to 1st entry
        ((ssize_t *)(file.dblock + INODE_SZ))[0] += next_entry_offset_from_curr;//record size is updated to include both entries.
        write_dblock(file.dblock_num, file.dblock);
    }
    else if(file.start_pos==0){
        //Remove Datablock
        ssize_t end_dblock_num = fblock_num_to_dblock_num(parent_inode, parent_inode->num_blocks-1);
        ssize_t curr_dblock_num = fblock_num_to_dblock_num(parent_inode, file.fblock_num);
        if(curr_dblock_num <=0){
            return -1;
        }
        if(end_dblock_num <=0){
            return -1;
        }
        free_dblock(curr_dblock_num);
        // replace the removed block num with the end block
        write_dblock_to_inode(parent_inode, file.fblock_num, end_dblock_num);
        parent_inode->num_blocks--;
    }
    free_memory(file.dblock);

    time_t curr_time = time(NULL);
    parent_inode->access_time = curr_time;
    parent_inode->modification_time = curr_time;
    parent_inode->status_change_time = curr_time;

    //decrementing the link count
    inode->link_count--;
    if(inode->link_count==0){
        //if link_count is 0, the file has to be deleted.
        pop_cache(&iname_cache, path);
        free_inode(inum);
    }
    else{
        inode->status_change_time = curr_time;
        write_inode(inum, inode);
    }
    //updating parent inode
    write_inode(parent_inode_num, parent_inode);
    free_memory(parent_inode);
    free_memory(inode);
    return 0;
}

ssize_t custom_open(const char* path, ssize_t oflag){
    // open a file, if not there create one, else existing one by scraping everything if asked
    ssize_t inode_num = get_inode_num_from_path(path);
    // create file
    if(oflag & O_CREAT && inode_num<=-1){
        struct iNode* inode = NULL;
        inode_num = create_new_file(path, &inode, S_IFREG|DEFAULT_PERMS);
        if(inode_num<=-1){
            return -1;
        }
        write_inode(inode_num, inode);
        free_memory(inode);
    }
    // Truncate
    if(oflag & O_TRUNC){
        if(custom_truncate(path, 0)==-1){
            return -1;
        }
    }
    return inode_num;
}

ssize_t custom_close(ssize_t file_descriptor){
    // TODO
    file_descriptor = 0;
    return 0;
}

//shouldn't the return type be int as we are returning number of bytes read?
//TO VERIFY
ssize_t custom_read(const char* path, void* buff, size_t nbytes, size_t offset){
    memset(buff, 0, nbytes);
    // fetch inum from path
    ssize_t inum = get_inode_num_from_path(path);
    if (inum == -1) {
        printf("ERROR: Inode file %s not found\n", path);
        return -1;
    }
    struct iNode *inode= read_inode(inum);
    if (inode->file_size == 0) {
        return 0;
    }
    if (offset > inode->file_size) {
        printf("ERROR: Offset %zu for read is greater than size of the file %ld\n", offset, inode->file_size);
        return -1;
    }
    //update nbytes when read_len from offset exceeds file_size. 
    if (offset + nbytes > inode->file_size){
        nbytes = inode->file_size - offset;
    }

    ssize_t start_block = offset / BLOCK_SIZE;
    ssize_t start_block_top_ceil = offset % BLOCK_SIZE;

    ssize_t end_block = (offset + nbytes) / BLOCK_SIZE;
    ssize_t end_block_bottom_floor = BLOCK_SIZE - (offset + nbytes) % BLOCK_SIZE;

    ssize_t nblocks_read = end_block - start_block + 1;

    // printf("start_block %ld, start_block_start %ld, end_block %ld, end_block_end %ld, blocks_to_read %ld\n",
    //         start_block, start_block_top_ceil, end_block, BLOCK_SIZE-end_block_bottom_floor, nblocks_read);

    DEBUG_PRINTF("Total number of blocks to read: %ld\n", nblocks_read);

    size_t bytes_read = 0;
    ssize_t dblock_num;
    char *buf_read = NULL;

    if(nblocks_read==1){
        //only 1 block to be read;
        dblock_num=fblock_num_to_dblock_num(inode,start_block);
        if(dblock_num<=0){
            printf("Error fetching dblock_num %ld from fblock_num during the read. Max Blocks:%ld \n",start_block,inode->num_blocks);
            return -1;
        }

        buf_read=read_dblock(dblock_num);
        if(buf_read==NULL){
            printf("Error fetching dblock_num %ld during the read operation for %s\n",dblock_num,path);
            return -1;
        }
        memcpy(buff, buf_read+start_block_top_ceil, nbytes);
        bytes_read=nbytes;
    }
    else{
        //when there are multiple blocks to read
        for(ssize_t i=0; i<nblocks_read;i++){
            dblock_num=fblock_num_to_dblock_num(inode, start_block+i);
            if(dblock_num<=0){
                printf("Error fetching dblock_num %ld from fblock_num during the read. Max Blocks:%ld \n",start_block+i,inode->num_blocks);
                free_memory(inode);
                return -1;
            }

            buf_read=read_dblock(dblock_num);

            if(buf_read == NULL){
                printf("Error fetching dblock_num %ld during the read operation for %s\n",dblock_num,path);
                free_memory(inode);
                return -1;
            }
            // for the 1st block, read only the contents after the start ceil.
            if(i==0){
                memcpy(buff, buf_read+start_block_top_ceil, BLOCK_SIZE-start_block_top_ceil);
                bytes_read+=BLOCK_SIZE-start_block_top_ceil;
            }
            else if(i== nblocks_read-1){
                // for the last block, read contents only till the bottom floor.
                memcpy(buff+bytes_read, buf_read, BLOCK_SIZE-end_block_bottom_floor);
                bytes_read+=BLOCK_SIZE-end_block_bottom_floor;
            }
            else{
                memcpy(buff+bytes_read, buf_read, BLOCK_SIZE);
                bytes_read+=BLOCK_SIZE;
            }
        }
    }
    free_memory(buf_read);
    // DEBUG_PRINTF("FILE_LAYER: Read Successful for the file %s\n Bytes read: %zu\n",path, bytes_read);
    time_t curr_time = time(NULL);
    inode->access_time = curr_time;

    if(!write_inode(inum, inode)){
        printf("Error: INODE update during file read failed for file %s\n",path);
    }
    free_memory(inode);
    return bytes_read; 
}

//TO VERIFY 
// on failure what to return?
ssize_t custom_write(const char* path, void* buff, size_t nbytes, size_t offset){
    ssize_t inum = get_inode_num_from_path(path);
    if (inum == -1) {
        printf("ERROR in FILE_LAYER: Unable to find Inode for file %s\n", path);
        return -1;
    }

    struct iNode *inode = read_inode(inum);
    ssize_t bytes_to_add=0;

    if(offset + nbytes > inode->file_size){
        //add new blocks; 
        bytes_to_add = (offset + nbytes) - inode->file_size;
        ssize_t new_blocks_to_be_added = ((offset + nbytes) / BLOCK_SIZE) - inode->num_blocks + 1;
        // printf("Creating %ld new blocks for writing %ld bytes with %ld offset to inode %ld\n",
        //        new_blocks_to_be_added, nbytes, offset, inum);
        // DEBUG_PRINTF("Total new blocks being added to the file is %ld\n", new_blocks_to_be_added);
        ssize_t new_block_id;
        for(ssize_t i=0; i<new_blocks_to_be_added; i++){
            new_block_id = create_new_dblock();
            if(new_block_id<=0){
                printf("Failed to allocated new Data_Block for the write operation for the file %s\n", path);
                free_memory(inode);
                return -1;
            }
            if(!add_dblock_to_inode(inode, new_block_id)){
                printf("New Data Block addition to inode failed for file %s\n", path);
                free_memory(inode);
                return -1;
            }
        }
    }

    ssize_t start_block = offset / BLOCK_SIZE;
    ssize_t start_block_top_ceil = offset % BLOCK_SIZE;

    ssize_t end_block = (offset + nbytes) / BLOCK_SIZE;
    ssize_t end_block_bottom_floor = BLOCK_SIZE - (offset + nbytes)% BLOCK_SIZE;

    ssize_t nblocks_write = end_block - start_block + 1;

    // printf("start_block %ld, start_block_start %ld, end_block %ld, end_block_end %ld, blocks_to_write %ld\n",
    //         start_block, start_block_top_ceil, end_block, BLOCK_SIZE-end_block_bottom_floor, nblocks_write);

    DEBUG_PRINTF("writing %ld blocks to file\n", nblocks_write);

    size_t bytes_written=0;
    char *buf_read = NULL;

    // if there is only 1 block to write
    if(nblocks_write==1){
        ssize_t dblock_num = fblock_num_to_dblock_num(inode, start_block);
        if(dblock_num<=0){
            printf("Error getting dblocknum from Fblock_num during %ld block write for %s\n",start_block, path);
            free_memory(inode);
            return -1;
        }

        buf_read = read_dblock(dblock_num);
        if(buf_read == NULL){
            printf("Error encountered reading dblocknum %ld during %s write\n", dblock_num, path);
            free_memory(inode);
            return -1;
        }
        memcpy(buf_read+start_block_top_ceil, buff, nbytes);
        if(!write_dblock(dblock_num, buf_read)){
            printf("Error writing to dblock_num %ld during %s write", dblock_num, path);
            free_memory(buf_read);
            free_memory(inode);
            return -1;
        }
    }
    else{
        for(ssize_t i=0; i<nblocks_write; i++){
            ssize_t dblock_num = fblock_num_to_dblock_num(inode, start_block+i);
            if(dblock_num<=0){
                printf("Error in fetching the dblock_num from fblock_num %ld for %s\n", start_block+i, path);
                free_memory(inode);
                return -1;
            }
            buf_read=read_dblock(dblock_num);
            if(buf_read == NULL){
                printf("Error reading dblock_num %ld during the write of file %s\n",dblock_num, path);
                free_memory(inode);
                return -1;
            }
            // for the 1st block, start only after start_block_top_ceil
            if(i==0){
                memcpy(buf_read + start_block_top_ceil, buff, BLOCK_SIZE - (start_block_top_ceil));
                // printf("Added - %s\n", buf_read+start_block_top_ceil);
                bytes_written += BLOCK_SIZE-start_block_top_ceil;
            }
            else if(i==nblocks_write-1){
                //last block to be written
                memcpy(buf_read, buff + bytes_written, BLOCK_SIZE-end_block_bottom_floor);
                // printf("Added - %s\n", buf_read);
                bytes_written+= BLOCK_SIZE - end_block_bottom_floor;
            }
            else{
                memcpy(buf_read, buff + bytes_written, BLOCK_SIZE);
                // printf("Added - %s\n", buf_read);
                bytes_written += BLOCK_SIZE;
            }
            if(!write_dblock(dblock_num, buf_read)){
                printf("Error in writing to %ld dblock_num during the write of %s\n", dblock_num, path);
                free_memory(buf_read);
                free_memory(inode);
                return -1;
            }
            // printf("Bytes written %ld, Dblock %ld, Fblock %ld\n", bytes_written, dblock_num, start_block+i);
        }
    }
    free_memory(buf_read);
    inode->file_size += bytes_to_add;
    time_t curr_time= time(NULL);
    inode->access_time = curr_time;
    inode->modification_time = curr_time;
    inode->status_change_time = curr_time;
    if(!write_inode(inum, inode)){
        printf("Updating inode %ld for the file %s during the write operation failed\n",inum, path);
        return -1;
    }
    free_memory(inode);
    return nbytes;
}

bool add_new_entry(struct iNode* inode, ssize_t child_inode_num, char* child_name){
    if(!S_ISDIR(inode->mode)){
        printf("not a directory\n");
        return false;
    }
    ssize_t name_length = strlen(child_name);
    if(name_length > MAX_NAME_LENGTH){
        printf("Too long name for file\n");
        return false;
    }
    unsigned short short_name_length = name_length; // TODO : Play using int
    // 8 + 8 + 2 + name_len
    ssize_t new_entry_size = INODE_SZ + ADDRESS_PTR_SZ + STRING_LENGTH_SZ + short_name_length;
    //if there is already a datablock for the dir file, we will add entry to it if it has space
    if(inode->num_blocks!=0){
        ssize_t dblock_num = fblock_num_to_dblock_num(inode, inode->num_blocks-1);
        if(dblock_num<=0){
            return false;
        }
        char* dblock = read_dblock(dblock_num);
        ssize_t curr_pos = 0;
        while(curr_pos < BLOCK_SIZE){
            /*
            INUM(8) | RECORD_LEN/ADDR_PTR(8 - either record len / space left) | FILE_STR_LEN | FILE_NAME
            addr_ptr for earlier records holds the len/size of record.
            However, for the last record it holds the BLOCKSIZE-(sum of all prev records len)
            this is fetched in next_entry. 
            For example, when the 1st entry is made to the block, the add_ptr holds the block size 4096
            when a 2nd entry is made, the add_ptr of the 1st entry holds rec_len while the add_ptr of 2nd entry hols 4096-(sum of prev record len)
            Hence, if BLOCK is able to accomodate more entries, it will be indicated in the add_ptr field of last entry.
            */      
            ssize_t next_entry_offset_from_curr = ((ssize_t*)(dblock+curr_pos+INODE_SZ))[0]; // this gives record_len or space_left
            unsigned short curr_entry_name_length = ((unsigned short* )(dblock+curr_pos+INODE_SZ+ADDRESS_PTR_SZ))[0];//len of filename/iname pointed by curr_pos
            ssize_t curr_ptr_entry_length = INODE_SZ + ADDRESS_PTR_SZ + STRING_LENGTH_SZ + curr_entry_name_length;
            // the following condition holds true only for the last entry of the dir and if the block is able to accomodate new entry. Until the we move the curr_pos
            if(next_entry_offset_from_curr - curr_ptr_entry_length >= new_entry_size){
                // updating curr entry addr with size of entry
                ((ssize_t* )(dblock+curr_pos+INODE_SZ))[0] = curr_ptr_entry_length;
                if(curr_ptr_entry_length<=0){
                    // again just to be sure
                    return false;
                }
                ssize_t addr_ptr = next_entry_offset_from_curr - curr_ptr_entry_length; // this is essentially 4096-(sum of all prev record len)
                // This will even handle the padding.
                curr_pos += curr_ptr_entry_length;
                // add new entry
                ((ssize_t*) (dblock+curr_pos))[0] = child_inode_num;
                ((ssize_t*) (dblock+curr_pos+INODE_SZ))[0] = addr_ptr;
                ((unsigned short*) (dblock+curr_pos+INODE_SZ+ADDRESS_PTR_SZ))[0] = short_name_length;
                strncpy((char*) (dblock+curr_pos+INODE_SZ+ADDRESS_PTR_SZ+STRING_LENGTH_SZ), child_name, name_length);
                if(!write_dblock(dblock_num, dblock)){
                    printf("Writing new entry of dir to dblock failed\n");
                    return false;
                }
                return true;
            }
            if(next_entry_offset_from_curr<=0){ // this will never be true unless there is a bug
                printf("Could be a bug, check this\n");
                return false;
            }
            // this will come here if diff is 0 or we can't accomodate in the current block
            curr_pos += next_entry_offset_from_curr;
        }
        free_memory(dblock);
    }
    ssize_t dblock_num = create_new_dblock();
    if(dblock_num<=0){
        return false;
    }
    char dblock[BLOCK_SIZE];
    memset(dblock, 0, BLOCK_SIZE);
    ((ssize_t*) dblock)[0] = child_inode_num;
    ssize_t addr_ptr = BLOCK_SIZE;
    ((ssize_t*) (dblock+INODE_SZ))[0] = addr_ptr;
    ((unsigned short*) (dblock+INODE_SZ+ADDRESS_PTR_SZ))[0] = short_name_length;
    strncpy((char*)(dblock+INODE_SZ+ADDRESS_PTR_SZ+STRING_LENGTH_SZ), child_name, name_length);
    if(!write_dblock(dblock_num, dblock)){
        printf("Could be a bug, check this\n");
        return false;
    }
    //this part is modified. TODO: verify
    if(!add_dblock_to_inode(inode, dblock_num)){
        printf("couldn't add dblock to inode\n");
        return false;
    }
    
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
    // TODO: If already created and re-mounting, just read inode works
    // you can verify it by looking at allocated is true already
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
    printf("root dir created\n");
    printf("No of dblocks for root:%ld\n",root->num_blocks);
    free_memory(root);
    create_cache(&iname_cache, CACHE_SIZE);
    DEBUG_PRINTF("File layer initialization done \n");
    return true;
}
