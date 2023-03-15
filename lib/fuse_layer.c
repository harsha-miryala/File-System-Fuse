#include <errno.h>
#include <stdbool.h>
#include "../include/fuse_layer.h"

static const struct fuse_operations fuse_ops = {
    .access   = charm_access,
    .chown    = charm_chown,
    .chmod    = charm_chmod,
    .create   = charm_create,
    .getattr  = charm_getattr,
    .mkdir    = charm_mkdir,
    .rmdir    = charm_rmdir,
    .unlink   = charm_unlink,
    .truncate = charm_truncate,
    .mknod    = charm_mknod,
    .readdir  = charm_readdir,
    .open     = charm_open,
    .read     = charm_read,
    .write    = charm_write,
    .utimens  = charm_utimens,
    .rename   = charm_rename,
};

// struct stat {
    // dev_t     st_dev;         /* ID of device containing file */
    // ino_t     st_ino;         /* Inode number */
    // mode_t    st_mode;        /* File type and mode */
    // nlink_t   st_nlink;       /* Number of hard links */
    // uid_t     st_uid;         /* User ID of owner */
    // gid_t     st_gid;         /* Group ID of owner */
    // dev_t     st_rdev;        /* Device ID (if special file) */
    // off_t     st_size;        /* Total size, in bytes */
    // blksize_t st_blksize;     /* Block size for filesystem I/O */
    // blkcnt_t  st_blocks;      /* Number of 512B blocks allocated */
// }

static int inode_to_stdbuff(struct iNode* inode, struct stat* stdbuff){
    stdbuff->st_mode = inode->mode;
    stdbuff->st_nlink = inode->link_count;
    stdbuff->st_uid = 0; // only one user for now
    stdbuff->st_gid = 0; // only one group
    stdbuff->st_rdev = 0;
    stdbuff->st_size = inode->file_size;
    stdbuff->st_blksize = BLOCK_SIZE;
    stdbuff->st_blocks = inode->num_blocks;
    // setting times
    stdbuff->st_atime = inode->access_time;
    stdbuff->st_ctime = inode->status_change_time;
    stdbuff->st_mtime = inode->modification_time;
    return 0;
}

static int charm_access(const char* path, int mode){
    int inode_num = get_inode_num_from_path(path);
    if(inode_num == -1){
        return -ENOENT;
    }
    // TODO: Check Permissions
    return 0;
}

static int charm_chown(const char* path, uid_t uid, gid_t gid){
    // TODO: Check for improvement
    return 0; // out of scope as of now as only one user
}

static int charm_chmod(const char* path, mode_t mode){
    int inode_num = get_inode_num_from_path(path);
    if(inode_num == -1){
        return -ENOENT;
    }
    struct iNode* inode = read_inode(inode_num);
    if(inode==NULL){
        printf("FUSE LAYER : Inode read returned null\n");
        return -1;
    }
    inode->mode = mode;
    if(!write_inode(inode_num, inode)){
        printf("FUSE LAYER : Inode write unsuccessful\n");
        return -1;
    }
    return 0;
} 

static int charm_create(const char* path, mode_t mode, struct fuse_file_info* file_info){
    bool status = false;
    if(file_info->flags & O_CREAT){
        status = custom_mknod(path, S_IFREG|mode, -1);
    }
    else{
        status = custom_mknod(path, S_IFREG|0775, -1);
    }
    if(!status){
        printf("FUSE LAYER : File creation unsuccessful\n");
        return -1;
    }
    return 0;
}

static int charm_getattr(const char* path, struct stat* stdbuff){
    memset(stdbuff, 0, sizeof(struct stat));
    int inode_num = get_inode_num_from_path(path);
    if(inode_num==-1){
        return -ENOENT;
    }
    struct iNode* inode = read_inode(inode_num);
    inode_to_stdbuff(inode, stdbuff);
    stdbuff->st_ino = inode_num;
    return 0;
}

static int charm_open(const char* path, struct fuse_file_info* file_info){
    int inode_num = custom_open(path, file_info->flags);
    if(inode_num<=-1){
        printf("FUSE LAYER : Open function unsuccessful\n");
        return -1;
    }
    return 0;
}

static int charm_read(const char* path, char* buff, size_t size, off_t offset, struct fuse_file_info* file_info){
    int nbytes_read = custom_read(path, buff, size, offset);
    return nbytes_read;
}

static int charm_readdir(const char* path, void* buff, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info* file_info){
    (void) offset;
    (void) file_info;
    int inode_num = get_inode_num_from_path(path);
    if(inode_num==-1){
        return -ENOENT;
    }
    struct iNode* inode = read_inode(inode_num);
    if(!S_ISDIR(inode->mode)){
        return -ENOTDIR;
    }
    int num_blocks = inode->num_blocks;
    for(int fblock_num=0; fblock_num<num_blocks; fblock_num++){
        int dblock_num = fblock_num_to_dblock_num(inode, fblock_num);
        char* dblock = read_dblock(dblock_num);
        int offset = 0;
        int next_entry_loc = 0;
        while(offset<BLOCK_SIZE){
            // reading inode
            unsigned int file_inode_num;
            memcpy(&file_inode_num, dblock+offset, INODE_SZ);
            offset += INODE_SZ;
            // space left or length to move
            unsigned int entry_size; // could be either size left or how much to move
            memcpy(&entry_size, dblock+offset, ADDRESS_PTR_SZ);
            next_entry_loc += entry_size;
            offset += ADDRESS_PTR_SZ;
            // file size
            unsigned short file_name_size;
            memcpy(&file_name_size, dblock+offset, STRING_LENGTH_SZ);
            offset += STRING_LENGTH_SZ;
            // file name
            char file_name[file_name_size+1];
            file_name[file_name_size] = '\0';
            memcpy(file_name, dblock+offset, file_name_size);
            offset += file_name_size;
            // pull inode and send through filler
            struct iNode* file_inode = read_inode(file_inode_num);
            struct stat stdbuff_data;
            memset(&stdbuff_data, 0, sizeof(struct stat));
            struct stat *stdbuff = &stdbuff_data;
            inode_to_stdbuff(file_inode, stdbuff);
            stdbuff->st_ino = file_inode_num;
            filler(buff, file_name, stdbuff, 0);
            offset = next_entry_loc;
            free_memory(file_inode);
        }
        free_memory(dblock);
    }
    free_memory(inode);
    return 0;
}

static int charm_rmdir(const char* path){
    int status = custom_unlink(path);
    if(status==-1){
        printf("FUSE LAYER : rm dir unsuccessful\n");
        return -1;
    }
    return 0;
}

static int charm_mkdir(const char* path, mode_t mode){
    bool status = custom_mkdir(path, mode);
    if(!status){
        printf("FUSE LAYER : mkdir unsuccessful\n");
        return -1;
    }
    return 0;
}

static int charm_mknod(const char* path, mode_t mode, dev_t dev){
    bool status = custom_mknod(path, mode, dev);
    if(!status){
        printf("FUSE LAYER : mknod unsuccessful\n");
        return -1;
    }
    return 0;
}

static int charm_utimens(const char* path, const struct timespec time_spec[2]){
    // TODO: check
    return 0;
}

static int charm_truncate(const char* path, off_t offset){
    int status = custom_truncate(path, offset);
    if(status==-1){
        printf("FUSE LAYER : truncate unsuccessful\n");
        return -1;
    }
    return 0;
}

static int charm_unlink(const char* path){
    int status = custom_unlink(path);
    if(status==-1){
        printf("FUSE LAYER : unlink unsuccessful\n");
        return -1;
    }
    return 0;
}

static int charm_write(const char* path, const char* buff, size_t size, off_t offset, struct fuse_file_info* file_info){
    int nbytes_wrote = custom_write(path, (void*)buff, size, offset);
    return nbytes_wrote;
}

static int charm_rename(const char *from, const char *to)
{
    // Hacky approach - copy file to the new location and remove the old one
    // check if from exists
    struct fuse_file_info to_fi;
    memset(&to_fi, 0, sizeof(to_fi));
    if(charm_access(from, O_RDONLY)!=0){
        printf("FUSE LAYER : from file exists\n");
        return -errno;
    }
    // check if to doesn't exist
    if(charm_access(to, O_WRONLY | O_CREAT | O_EXCL)==0){
        printf("FUSE LAYER : to file doesn't exist\n");
        return -errno;
    }
    // create to
    to_fi.flags = O_WRONLY | O_CREAT | O_TRUNC;
    if(charm_open(to, &to_fi)!=0){
        printf("FUSE LAYER : created to file\n");
        return -errno;
    }
    // copy data from -> to
    size_t offset = 0;
    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    size_t bytes_read, bytes_written;
    while((bytes_read=custom_read(from, buffer, BLOCK_SIZE, offset))>0){
        bytes_written = custom_write(to, (void*)buffer, bytes_read, offset);
        if(bytes_read!=bytes_written){
            return -errno;
        }
        offset += bytes_read;
    }
    // delete from
    if(custom_unlink(from)==-1){
        printf("FUSE LAYER : removing old path\n");
        return -errno;
    }
    printf("FUSE LAYER : rename successful\n");
    return 0;
}

int main(int argc, char* argv[]){
    init_file_layer();
    umask(0000);
    return fuse_main(argc, argv, &fuse_ops, NULL);
}