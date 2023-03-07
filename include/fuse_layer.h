#ifndef __FUSE_LAYER_H__
#define __FUSE_LAYER_H__

#define FUSE_USE_VERSION 31 // TODO: Not sure 

#include <fuse.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stddef.h>
#include "debug.h"
#include "file_layer.h"

static const struct fuse_operations fuse_ops;

// Errors:
    // ENOENT (No such file or directory)

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

// TODO 
// opendir
// init - not reqd as init_file_layer launched

static int inode_to_stdbuff(struct iNode* inode, struct stat* stdbuff);

static int charm_access(const char* path, int mode);

static int charm_chown(const char* path, uid_t uid, gid_t gid);

static int charm_chmod(const char* path, mode_t mode);

static int charm_create(const char* path, mode_t mode, struct fuse_file_info* file_info);

static int charm_getattr(const char* path, struct stat* stdbuff);

static int charm_open(const char* path, struct fuse_file_info* file_info);

static int charm_read(const char* path, char* buff, size_t size, off_t offset, struct fuse_file_info* file_info);

static int charm_readdir(const char* path, void* buff, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* file_info);

static int charm_rmdir(const char* path);

static int charm_mkdir(const char* path, mode_t mode);

static int charm_mknod(const char* path, mode_t mode, dev_t dev);

static int charm_utimens(const char* path, const struct timespec time_spec[2]);

static int charm_truncate(const char* path, off_t offset);

static int charm_unlink(const char* path);

static int charm_write(const char* path, const char* buff, size_t size, off_t offset, struct fuse_file_info* file_info);

#endif