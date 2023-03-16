#ifndef __FILE_LAYER_H__
#define __FILE_LAYER_H__

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "disk_layer.h"
#include "block_layer.h"

#define SINGLE_INDIRECT_BLOCK_COUNT (BLOCK_SIZE / ADDRESS_SIZE) //1024
#define DOUBLE_INDIRECT_BLOCK_COUNT (SINGLE_INDIRECT_BLOCK_COUNT * (BLOCK_SIZE / ADDRESS_SIZE)) //1048576
#define TRIPLE_INDIRECT_BLOCK_COUNT (DOUBLE_INDIRECT_BLOCK_COUNT * (BLOCK_SIZE / ADDRESS_SIZE)) //1073742000
#define ADDRESS_PTR_SZ 8
#define INODE_SZ 8
#define MAX_NAME_LENGTH 255 // max len of a name
#define ROOT_INODE 2
#define STRING_LENGTH_SZ 2
#define CACHE_SIZE 50000
#define DEFAULT_PERMS (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

// used when he details of a specific dir entry has to be retrieved.
struct file_pos_in_dir{
    char* dblock;
    ssize_t dblock_num;
    ssize_t fblock_num;
    ssize_t start_pos;//contains the pointer to the inum in the dir entry
    ssize_t prev_entry;// usually contains the record len of the previous entry. If the entry is the 1st entry, it stores -1
};

// fblock is essentially position of the datablock relative to that file

// Returns char array index of the last slash of parent's path
ssize_t get_parent_id(const char* const path, ssize_t path_len);
// copy parent path into a given buffer
bool get_parent_path(char* const buff, const char* const path, ssize_t path_len);
// copy child file into a given buffer
bool get_child_name(char* const buff, const char* const path, ssize_t path_len);
// get dblock num corr to file block number
ssize_t fblock_num_to_dblock_num(const struct iNode* const inode, ssize_t fblock_num);

bool write_dblock_to_inode(struct iNode* inode, ssize_t fblock_num, ssize_t dblock_num);
/*
Adds a dblock number to an inode
Inputs:
    inode: File to add the datablock
    dblock_num: the datablock number to add
Returns:
    true/false
*/
bool add_dblock_to_inode(struct iNode* inode, const ssize_t dblock_num);
// free the blocks inside single indirect
bool remove_single_indirect(ssize_t dblock_num);
// free the blocks inside double indirect
bool remove_double_indirect(ssize_t dblock_num);

bool remove_dblocks_from_inode(struct iNode* inode, ssize_t fblock_num);
/*
Returns the inode for the file at the end of the path
Inputs:
    path: path to file
Returns:
    i-node number on success and -1 on failure
*/
ssize_t get_inode_num_from_path(const char* const path);

ssize_t create_new_file(const char* const path, struct iNode** buff, mode_t mode);

bool is_empty_dir(struct iNode* inode);
/*
Creates a new directory
Inputs:
    path: A C-string specifying the path
    mode: An unsigned short specifying the permission bits
Returns:
    true/false
*/
bool custom_mkdir(const char* path, mode_t mode);
/*
Make a special file node
Inputs:
    path: A C-string specifying the path
    mode: An unsigned short specifying major/minor device numbers
    dev: If mode indicates a block or special device, dev is
    a specification of a character or block I/O device and the
    superblock of the device
Returns:
    true/mode
*/
bool custom_mknod(const char* path, mode_t mode, dev_t dev);

struct file_pos_in_dir find_file(const char *const name, const struct iNode* const parent_inode);

ssize_t custom_truncate(const char* path, size_t offset);

ssize_t custom_unlink(const char* path);

ssize_t custom_close(ssize_t file_descriptor);

ssize_t custom_open(const char* path, ssize_t oflag);

ssize_t custom_read(const char* path, void* buff, size_t nbytes, size_t offset);

ssize_t custom_write(const char* path, void* buff, size_t nbytes, size_t offset);

/*
Adds a new entry to a directory represented by the inode
Inputs:
    inode: Directory to add the new entry
    inode_num: The new entry to add
    inode_name: The name of the new entry
Returns:
    true / false
*/
bool add_new_entry(struct iNode* inode, ssize_t inode_num, char* inode_name);

bool init_file_layer();

#endif
