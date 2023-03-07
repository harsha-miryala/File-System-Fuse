#include "fuse_layer.h"

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
    struct timespec time;
    time.tv_sec = inode->access_time;
    time.tv_nsec = time.tv_sec * 1000000000;
    stdbuff->st_atimespec = time;
    time.tv_sec = inode->status_change_time;
    time.tv_nsec = time.tv_sec * 1000000000;
    stdbuff->st_ctimespec = time;
    time.tv_sec = inode->modification_time;
    time.tv_nsec = time.tv_sec * 1000000000;
    stdbuff->st_mtimespec = time;
    time.tv_sec = inode->creation_time;
    time.tv_nsec = time.tv_sec * 1000000000;
    stdbuff->st_birthtimespec = time;
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
    int status = 0;
    if(file_info->flags & O_CREAT){
        status = custom_mknod(path, S_IFREG|mode, -1);
    }
    else{
        status = custom_mknod(path, S_IFREG|0775, -1);
    }
    if(status <= -1){
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
        return res;
    }
    return 0;
}

static int charm_read(const char* path, char* buff, size_t size, off_t offset, struct fuse_file_info* file_info){
    int nbytes_read = custom_read(path, buff, size, offset);
    return nbytes_read;
}

static int charm_readdir(const char* path, void* buff, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* file_info){

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
    int status = custom_mkdir(path, mode);
    if(status==-1){
        printf("FUSE LAYER : mkdir unsuccessful\n");
        return -1;
    }
    return 0;
}

static int charm_mknod(const char* path, mode_t mode, dev_t dev){
    int status = custom_mknod(path, mode, dev);
    if(status==-1){
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

int main(int argc, char* argv[]){
    init_file_layer();
    umask(0000);
    return fuse_main(argc, argv, &fuse_ops, NULL);
}