# File System Implementation using FUSE

## Packages to download

- sudo apt-get install libfuse-dev
- sudo apt-get install fuse fuse3
- sudo apt-get install make automake autoconf

## How to mount external partition for fuse
- Add the block device name in disk_layer.h
- ``` sudo su ``` to login as root user
- Open two terminals and create a directory to intercept sys calls through fuse in that area (this is to unmount if already mounted) - ``` fusermount -u mpoint ```
- In one of the terminals launch the fuse layer instance through - ``` make init ``` and ``` ./init -f mpoint ```
- In the other terminal, cd into the mpoint directory and anything you do in through that dir will be intercepted by fuse
- Note: mount directory should be empty before launching fuse 
- You can copy the test cases using cp into the mpoint e.g. ``` cp -r ../../File-System-Fuse/test/fuse . ```

## Tests

### Unit Tests

- cd File-System-Fuse
- make test

#### LRU Cache layer

- ./obj/lru_cache_test

#### Disk layer

- ./obj/disk_test

#### Block layer

- ./obj/block_test

#### File layer

- ./obj/file_test

#### Fuse Layer tests that invoke FS calls

- cd PATH/TO/test/fuse

- make test

First tests the open_create_write_test.c This will create multiple files within the directory basic_api_test_files.
- ./obj/open_create_write_test

Then run tests for lseek operation
- ./obj/lseek_read_write_test

To run tests for unlink
- ./obj/unlink_test

Testing large and extra large files.
- ./obj/large_file_test

- ./obj/extra_large_file_test



