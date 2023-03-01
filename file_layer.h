#ifndef __FILE_LAYER_H__
#define __FILE_LAYER_H__

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "disk_layer.h"
#include "block_layer.h"

#define SINGLE_INDIRECT_BLOCK_COUNT (BLOCK_SIZE / ADDRESS_SIZE)
#define DOUBLE_INDIRECT_BLOCK_COUNT (SINGLE_INDIRECT_NUM * BLOCK_SIZE / ADDRESS_SIZE)
#define ADDRESS_PTR_SZ 8
#define INODE_SZ 8 // donno
#define MAX_NAME_LENGTH 255 // max len of a name
#define ROOT_INODE 2
#define STRING_LENGTH_SZ 2
#define CACHE_SIZE 5000
#define DEFAULT_PERMS (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

// TODO: name it more relevant
struct in_core_dir{
    char* dblock;
    size_t dblock_num;
    size_t fblock_num;
    size_t start_pos;
    size_t prev_entry;
};

// fblock is essentially position of the datablock relative to that file

// Returns char array index of the last slash of parent's path
int get_parent_id(const char* const path, int path_len);
// copy parent path into a given buffer
bool get_parent_path(char* const buff, const char* const path, int path_len);
// copy child file into a given buffer
bool get_child_name(char* const buff, const char* const path, int path_len);
// get dblock num corr to file block number
int fblock_num_to_dblock_num(const struct iNode* const inode, int fblock_num);

bool write_dblock_to_inode(struct iNode* inode, int fblock_num, int dblock_num);
/*
Adds a dblock number to an inode
Inputs:
    inode: File to add the datablock
    dblock_num: the datablock number to add
Returns:
    true/false
*/
bool add_dblock_to_inode(struct iNode* inode, const int dblock_num);
// free the blocks inside single indirect
bool remove_single_indirect(int dblock_num);
// free the blocks inside double indirect
bool remove_double_indirect(int dblock_num);

bool remove_dblocks_from_inode(struct iNode* inode, int fblock_num);
/*
Returns the inode for the file at the end of the path
Inputs:
    path: path to file
Returns:
    i-node number on success and -1 on failure
*/
int get_inode_num_from_path(const char* const path);

int create_new_file(const char* const path, struct iNode** buff, mode_t mode);

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

// TODO
void fs_readdir();

struct in_core_dir find_file(const char *const name, const struct iNode* const parent_inode);

int custom_truncate(const char* path, size_t offset);

int custom_unlink(const char* path);

int custom_close(int file_descriptor);

int custom_open(const char* path, int oflag);

ssize_t custom_read(const char* path, void* buff, size_t nbytes, size_t offset);

ssize_t custom_write(const char* path, void* buff, size_t nbytes, size_t offset);

/////////////////////

/*
Adds a new entry to a directory represented by the inode
Inputs:
    inode: Directory to add the new entry
    inode_num: The new entry to add
    inode_name: The name of the new entry
Returns:
    true / false
*/
bool add_new_entry(struct iNode* inode, int inode_num, char* inode_name);

bool init_file_layer();

#endif