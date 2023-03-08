#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "../../include/file_layer.h"

void mkdir_test()
{
    // Test to make directories under the root directory
    printf("Creating new directories under root directory...\n");
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    {                                         // a, b, c, ...
        char dir_name = i + 'a';              // set directory name
        char path[3] = {'/', dir_name, '\0'}; // set directory path
        if (!custom_mkdir(path, S_IFDIR))
        { // create directory and check for errors
            printf("FAILED to create directory: %s\n", path);
            exit(-1);
        }
        printf("Directory created successfully: %s\n\n", path);
    }
    printf("Directories created successfully under root.\n\n");
    // check the number of blocks in the root directory
    struct iNode *root_inode = read_inode(ROOT_INODE);
    if (root_inode->num_blocks != 1)
    {
        printf("Number of blocks check failed.\nExpected: %i, Actual: %i\n", 1, root_inode->num_blocks);
        exit(-1);
    }
    printf("Number of blocks check successful.\n");
    // find the directories under the root directory
    printf("Finding the created directories under root directory...\n");
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    {                                                              // a, b, c, ...
        char dir_name = i + 'a';                                   // set directory name
        char name[2] = {dir_name, '\0'};                           // set directory path
        struct file_pos_in_dir file = find_file(name, root_inode); // find directory
        if (file.start_pos == -1)
        { // check for errors
            printf("Failed to find directory: %s\n", name);
            exit(-1);
        }
        printf("Directory found successfully: %s\n", name);
    }
    printf("Successfully found all directories under root.\n");
    free_memory(root_inode); // free memory
}

void mk_numerous_files_test()
{
    printf("Creating multiple files in a directory...\n");
    // Directory name
    char *dir_name = "/test";
    // Create directory
    custom_mkdir(dir_name, S_IFDIR);
    // Path to file
    char path[15];
    // Create 2000 files
    for (int i = 0; i < 2000; i++)
    {
        // Generate file path
        snprintf(path, 15, "/test/%d.txt", i);
        // Create file
        if (!custom_mknod(path, S_IFREG, 0))
        {
            // Print error message if file creation failed
            printf("FAILED ON FILE: %s\n", path);
            exit(-1);
        }
        // Get inode number of the file
        int inode_num = get_inode_num_from_path(path);
        // Check if inode number is valid
        if (inode_num == -1)
        {
            // Print error message if inode number is invalid
            printf("FAILED ON FILE: %s\n", path);
            exit(-1);
        }
    }
    // Print message if multiple files were created successfully
    printf("Creating multiple files in a directory: PASSED\n");
}

void unlink_multiple_files_test()
{
    printf("Testing unlinking multiple files in directory...\n");
    char filePath[15]; // buffer to hold file path
    for (int i = 0; i < 2000; i++)
    {
        // format the file path with the current index
        snprintf(filePath, 15, "/test/%d.txt", i);
        // attempt to unlink the file at the path
        if (custom_unlink(filePath) < 0)
        {
            // if unlink fails, print an error message and exit
            printf("FAILED ON FILE: %s\n", filePath);
            exit(-1);
        }
    }
    // verify that the directory inode now has only one block
    int dirInodeNum = get_inode_num_from_path("/test");
    struct iNode *dirInode = read_inode(dirInodeNum);
    if (dirInode->num_blocks != 1)
    {
        // if the number of blocks is not 1, print an error message and exit
        printf("FAILED: EXPECTED 1 BLOCK, ACTUAL: %i\n", dirInode->num_blocks);
        exit(-1);
    }
    // if all tests passed, print a success message
    printf("Unlinking multiple files in directory test: PASSED\n");
}

void mk_file_test()
{
    time_t previous_time, after_time;
    previous_time = time(NULL);
    printf("Creating a new regular file in each new directory...\n");
    // Loop through each directory (a, b, c, ...) and create a new file in it
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    {
        char dir_name = i + 'a';
        char file_path[5] = {'/', dir_name, '/', dir_name, '\0'};
        // Try to create a new file in the directory
        if (!custom_mknod(file_path, S_IFREG, 0))
        {
            printf("Failed to create file: %s\n", file_path);
            exit(-1);
        }
    }
    after_time = time(NULL);
    printf("Finding those files...\n");
    // Loop through each directory again and check if the previously created file exists and has accurate timestamps
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    {
        char dir_name = i + 'a';
        char file_path[5] = {'/', dir_name, '/', dir_name, '\0'};
        // Get the inode number of the file from its path
        int inode_num = get_inode_num_from_path(file_path);
        if (inode_num == -1)
        {
            printf("Failed to find file: %s\n", file_path);
            exit(-1);
        }
        printf("Found file: %s, inode number: %i\n", file_path, inode_num);
        // Read the inode of the file and check if its timestamps are accurate
        struct iNode *inode = read_inode(inode_num);
        if (!(inode->creation_time >= previous_time && inode->creation_time <= after_time))
        {
            printf("Failed on file %s: birth time should be between %ld and %ld, actual: %ld\n", file_path, previous_time, after_time, inode->creation_time);
            exit(-1);
        }
        else if (!(inode->access_time >= previous_time && inode->access_time <= after_time))
        {
            printf("Failed on file %s: access time should be between %ld and %ld, actual: %ld\n", file_path, previous_time, after_time, inode->access_time);
            exit(-1);
        }
        else if (!(inode->status_change_time >= previous_time && inode->status_change_time <= after_time))
        {
            printf("Failed on file %s: change time should be between %ld and %ld, actual: %ld\n", file_path, previous_time, after_time, inode->status_change_time);
            exit(-1);
        }
        else if (!(inode->modification_time >= previous_time && inode->modification_time <= after_time))
        {
            printf("Failed on file %s: modify time should be between %ld and %ld, actual: %ld\n", file_path, previous_time, after_time, inode->modification_time);
            exit(-1);
        }
        else
        {
            printf("Timestamps are accurate\n");
        }
        free_memory(inode);
    }
    printf("Creating a new regular file in each new directory: Passed\n");
}

void write_test()
{
    printf("STARTING WRITE TEST...\n");
    // Get current time to check timestamp accuracy later
    time_t previous_time, after_time;
    previous_time = time(NULL);
    // Create buffer and get its length
    char *buffer = "This is a test buffer to test the writing functionality of custom_write";
    int nbytes = strlen(buffer);
    // Write buffer to file /a/a at offset 0
    if (custom_write("/a/a", buffer, nbytes, 0) == -1)
    {
        printf("FAILED TO WRITE TO FILE /a/a at offset 0\n");
        exit(-1);
    }
    printf("WRITE TO FILE /a/a SUCCESSFUL\n");
    // Read back the written buffer to check if it was written successfully
    char buffer_cmp[nbytes + 1];
    buffer_cmp[nbytes] = '\0';
    if (custom_read("/a/a", buffer_cmp, nbytes, 0) == -1)
    {
        printf("FAILED TO READ FILE /a/a\n");
        exit(-1);
    }
    printf("CHECKING IF BUFFER WAS WRITTEN SUCCESSFULLY...\n");
    if (strcmp(buffer, buffer_cmp))
    {
        printf("FAILED\nEXPECTED: %s, ACTUAL: %s\n", buffer, buffer_cmp);
        exit(-1);
    }
    printf("BUFFER WAS WRITTEN SUCCESSFULLY\n");
    // Check if the access and modification timestamps are accurate
    printf("CHECKING TIMESTAMPS...\n");
    after_time = time(NULL);
    char *file_path = "/a/a";
    int inode_num = get_inode_num_from_path(file_path);
    if (inode_num == -1)
    {
        printf("FAILED TO FIND FILE: %s\n", file_path);
        exit(-1);
    }
    struct iNode *inode = read_inode(inode_num);
    if (!(inode->access_time >= previous_time && inode->access_time <= after_time))
    {
        printf("Failed on file %s: access time should be between %ld and %ld, actual: %ld\n", file_path, previous_time, after_time, inode->access_time);
        exit(-1);
    }
    else if (!(inode->modification_time >= previous_time && inode->modification_time <= after_time))
    {
        printf("Failed on file %s: modify time should be between %ld and %ld, actual: %ld\n", file_path, previous_time, after_time, inode->modification_time);
        exit(-1);
    }
    else
    {
        printf("Timestamps are accurate\n");
    }
    printf("WRITING TO FILE: PASSED\n");
    free_memory(inode);
}

void two_block_write_test()
{
    printf("Performing two block write to files...\n");
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    { // iterating through blocks a, b, c, ...
        char block = i + 'a';
        char path[5] = {'/', block, '/', block, '\0'}; // create path
        char *buffer = "This is a test buffer to test the writing functionality of custom_write";
        unsigned int nbytes = strlen(buffer);     // get buffer size
        int num_writes = BLOCK_SIZE / nbytes + 1; // calculate number of writes to write two blocks
        for (int j = 0; j < num_writes; j++)
        { // iterate for each write
            if (custom_write(path, buffer, nbytes, j * nbytes) == -1)
            { // custom_write returns -1 if write fails
                printf("Failed to write to file %s at offset %i\n", path, j * nbytes);
                exit(-1); // exit with error status if write fails
            }
            char bufcmp[nbytes + 1];
            bufcmp[nbytes] = '\0';
            if (custom_read(path, bufcmp, nbytes, nbytes * j) == -1)
            { // read back from file
                printf("Failed to read file %s at offset %i\n", path, j * nbytes);
                exit(-1); // exit with error status if read fails
            }
            if (strcmp(buffer, bufcmp))
            { // compare buffer with read data
                printf("Checking if buffer was written successfully...");
                printf("Failed!\nExpected: %s, Actual: %s\n", buffer, bufcmp);
                exit(-1); // exit with error status if comparison fails
            }
        }
        // check if file size and number of blocks were updated correctly
        printf("Checking if file size and number of blocks were updated...");
        int inode_num = get_inode_num_from_path(path); // get inode number from path
        assert(inode_num != -1);
        struct iNode *inode = read_inode(inode_num); // read inode data
        if (inode->file_size != num_writes * nbytes)
        { // check if file size is updated
            printf("Failed!\nExpected file size: %d, Actual: %d\n", num_writes * nbytes, inode->file_size);
            exit(-1); // exit with error status if size is not as expected
        }
        if (inode->num_blocks != (num_writes * nbytes) / BLOCK_SIZE + 1)
        { // check if number of blocks is updated
            printf("Failed!\nExpected number of blocks: %d, Actual: %d\n", (num_writes * nbytes) / BLOCK_SIZE + 1, inode->num_blocks);
            exit(-1); // exit with error status if number of blocks is not as expected
        }
        printf("Success!\n"); // print success message
        free_memory(inode);   // free memory allocated to inode
    }
    printf("Two block write to files: Passed\n"); // print overall test result
}

void single_indirect_block_write_test()
{
    printf("Testing single indirect block write to files...\n");
    // Loop through direct block count
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    {
        // Generate path for file
        char c = i + 'a';
        char path[5] = {'/', c, '/', c, '\0'};
        // Set up buffer and write to file
        char *buf = "This is a test buffer to test the writing functionality of fs_write";
        unsigned int nbytes = strlen(buf);
        int num_writes = BLOCK_SIZE * DIRECT_B_COUNT / nbytes + 1;
        for (int j = 0; j < num_writes; j++)
        {
            // Write buffer to file
            if (custom_write(path, buf, nbytes, j * nbytes) == -1)
            {
                printf("Failed to write to file %s at offset %i\n", path, j * nbytes);
                exit(-1);
            }
            // Read buffer from file and compare
            char bufcmp[nbytes + 1];
            bufcmp[nbytes] = '\0';
            if (custom_read(path, bufcmp, nbytes, nbytes * j) == -1)
            {
                printf("Failed to read file %s at offset %i\n", path, j * nbytes);
                exit(-1);
            }
            if (strcmp(buf, bufcmp))
            {
                printf("Checking if buffer was written successfully... Failed\nExpected: %s, Actual: %s\n", buf, bufcmp);
                exit(-1);
            }
        }
        // Check if file size and number of blocks were updated
        printf("Checking if file size and number of blocks were updated... ");
        int inode_num = get_inode_num_from_path(path);
        assert(inode_num != -1);
        struct iNode *inode = read_inode(inode_num);
        if (inode->file_size != num_writes * nbytes)
        {
            printf("Failed\nExpected file size: %d, Actual: %d\n", (i + 1) * nbytes, inode->file_size);
            exit(-1);
        }
        if (inode->num_blocks != num_writes * nbytes / BLOCK_SIZE + 1)
        {
            printf("Failed\nExpected number of blocks: %d, Actual: %d\n", ((i + 1) * nbytes) / BLOCK_SIZE + 1, inode->num_blocks);
            exit(-1);
        }
        printf("Success\n");
        free_memory(inode);
    }
    printf("Single indirect block write to files: Passed\n");
}

void double_indirect_block_write_test()
{
    printf("Starting Double Indirect Block Write Test...\n");
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    {                                          // iterate over direct blocks (a, b, c, ...)
        char c = i + 'a';                      // get current letter for path
        char path[5] = {'/', c, '/', c, '\0'}; // create path using current letter
        char *data = "This is a test buffer to test the writing functionality of fs_write";
        unsigned int data_size = strlen(data);                                                        // get size of data
        int num_writes = BLOCK_SIZE * (DIRECT_B_COUNT + SINGLE_INDIRECT_BLOCK_COUNT) / data_size + 1; // calculate number of writes
        for (int j = 0; j < num_writes; j++)                                                          // write to file multiple times
        {
            if (custom_write(path, data, data_size, j * data_size) == -1) // write to file at offset j * data_size
            {
                printf("Failed to write to file %s at offset %i\n", path, j * data_size);
                exit(-1);
            }
            char data_read[data_size + 1];                                    // buffer to read data from file
            data_read[data_size] = '\0';                                      // add null terminator to buffer
            if (custom_read(path, data_read, data_size, data_size * j) == -1) // read from file at offset j * data_size
            {
                printf("Failed to read file %s at offset %i\n", path, j * data_size);
                exit(-1);
            }
            if (strcmp(data, data_read)) // check if data written and read are the same
            {
                printf("Checking if data was written to file successfully...");
                printf("FAILED\nEXPECTED: %s, ACTUAL: %s\n", data, data_read);
                exit(-1);
            }
        }
        printf("Checking if file size and number of blocks were updated...");
        int inode_num = get_inode_num_from_path(path); // get inode number of file
        assert(inode_num != -1);
        struct iNode *inode = read_inode(inode_num);    // read inode from disk
        if (inode->file_size != num_writes * data_size) // check if file size was updated correctly
        {
            printf("Failed\nExpected file size: %d, Actual: %d\n", (i + 1) * data_size, inode->file_size);
            exit(-1);
        }
        if (inode->num_blocks != num_writes * data_size / BLOCK_SIZE + 1) // check if number of blocks was updated correctly
        {
            printf("Failed\nExpected number of blocks: %d, Actual: %d\n", ((i + 1) * data_size) / BLOCK_SIZE + 1, inode->num_blocks);
            exit(-1);
        }
        printf("Success\n");
        free_memory(inode); // free memory allocated for inode
    }
    printf("Double indirect block write to files: Passed\n");
}

void truncate_test()
{
    printf("Testing Truncate Functionality...\n");
    // For each file in the direct blocks
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    {
        // Construct the file path
        char file_char = i + 'a';
        char file_path[5] = {'/', file_char, '/', file_char, '\0'};
        // Truncate the file to 500 bytes
        custom_truncate(file_path, 500);
        // Check if the file size and number of blocks were updated
        printf("Checking if file size and number of blocks were updated for %s...", file_path);
        int inode_num = get_inode_num_from_path(file_path);
        assert(inode_num != -1);
        struct iNode *file_inode = read_inode(inode_num);
        if (file_inode->file_size != 500)
        {
            printf("FAILED\nExpected file size: %d, Actual: %d\n", 500, file_inode->file_size);
            exit(-1);
        }
        if (file_inode->num_blocks != 1)
        {
            printf("FAILED\nExpected number of blocks: %d, Actual: %d\n", 1, file_inode->num_blocks);
            exit(-1);
        }
        printf("PASSED\n");
        // Check if the direct blocks were zeroed out
        printf("Checking if direct blocks were zeroed out for %s...", file_path);
        for (int j = 1; j < DIRECT_B_COUNT; j++)
        {
            if (file_inode->direct_blocks[j] != 0)
            {
                printf("FAILED\nExpected direct_block[%d] to be 0, Actual: %d\n", j, file_inode->direct_blocks[j]);
                exit(-1);
            }
        }
        char *dblock = read_dblock(file_inode->direct_blocks[0]);
        for (int i = 500; i < BLOCK_SIZE; i++)
        {
            if (dblock[i] != 0)
            {
                printf("FAILED\nExpected first dblocl[%d] to be 0, Actual: %d\n", i, dblock[i]);
                exit(-1);
            }
        }
        printf("Success\n");
        free_memory(dblock);
        free_memory(file_inode);
    }
    printf("Truncate files: Passed\n");
}

void unlink_files_test()
{
    printf("Starting Unlink Test...\n");
    // Iterate over files a, b, c, ... in the root directory
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    {
        char file_char = i + 'a';
        char file_path[5] = {'/', file_char, '/', file_char, '\0'};
        // Remove file
        custom_unlink(file_path);
        // Check if file still exists
        int inum = get_inode_num_from_path(file_path);
        if (inum != -1)
        {
            printf("Unlink Test Failed on file: %s\n", file_path);
            exit(-1);
        }
        printf("File Unlinked: %s\n", file_path);
    }
    printf("Unlink Test Passed\n");
}

void unlink_full_dir_test()
{
    printf("Testing unlinking full directories...\n");
    // Loop through directories a, b, c, ...
    for (int dir_num = 0; dir_num < DIRECT_B_COUNT; dir_num++)
    {
        // Get the alphabetical character based on directory number
        char dir_char = dir_num + 'a';
        // Create path with / + character + null character
        char path[3] = {'/', dir_char, '\0'};
        // Try to unlink the directory
        int ret = custom_unlink(path);
        // Check if the error code matches the expected value
        if (ret != -ENOTEMPTY)
        {
            printf("Unlinking full directories failed on directory: %s\n", path);
            printf("Expected error code: -ENOTEMPTY, Actual error code: %i\n", ret);
            // Exit with an error if the error code is not as expected
            exit(-1);
        }
        printf("Unlinking full directories correctly failed for directory: %s\n", path);
    }
    printf("Testing unlinking full directories passed.\n");
}

void unlink_empty_dir_test()
{
    // Displaying a message to indicate the start of the test
    printf("Testing unlinking empty directories...\n");
    // Iterating through a range of directories
    for (int i = 0; i < DIRECT_B_COUNT; i++)
    {
        // Determining the directory name based on the index
        char dir_name = i + 'a';
        char path[3] = {'/', dir_name, '\0'};
        // Attempting to unlink the directory and checking for errors
        int unlink_result = custom_unlink(path);
        if (unlink_result < 0)
        {
            // Displaying an error message and exiting if unlinking fails
            printf("Error: Failed to unlink directory %s\n", path);
            exit(-1);
        }
        // Displaying a success message if unlinking is successful
        printf("Directory %s has been unlinked\n", path);
    }
    // Displaying a message to indicate the end of the test
    printf("Unlinking empty directories test has passed\n");
}

void unlink_root_test()
{
    printf("Running UNLINK ROOT test...\n");
    // Call custom_unlink() function to attempt to unlink the root directory ("/").
    int result = custom_unlink("/");
    // Check the return value of custom_unlink() and print appropriate message.
    if (result != -EINVAL)
    {
        printf("UNLINK ROOT test: FAILED with error code %i\n", result);
        exit(-1);
    }
    else
    {
        printf("UNLINK ROOT test: PASSED\n");
    }
}

int main()
{
    // Initialize file system
    printf("Initializing file layer test...\n");
    if (!init_file_layer())
    {
        printf("Failed: init_file_layer() returned non zero\n");
        exit(-1);
    }
    printf("File layer initialization test passed.\n");
    // // Test to unlink root dir
    printf("------------------------------------------------------------------------\n");
    unlink_root_test();
    printf("------------------------------------------------------------------------\n");
    // Test to make directories
    printf("------------------------------------------------------------------------\n");
    mkdir_test();
    printf("------------------------------------------------------------------------\n");
    // Test to make file
    mk_file_test();
    printf("------------------------------------------------------------------------\n");
    mk_numerous_files_test();
    printf("------------------------------------------------------------------------\n");
    unlink_multiple_files_test();
    printf("------------------------------------------------------------------------\n");
    // Test to write to files
    printf("Sleeping to pass the time...\n");
    sleep(1);
    write_test();
    printf("------------------------------------------------------------------------\n");
    two_block_write_test();
    printf("------------------------------------------------------------------------\n");
    truncate_test();
    printf("------------------------------------------------------------------------\n");
    single_indirect_block_write_test();
    printf("------------------------------------------------------------------------\n");
    double_indirect_block_write_test();
    printf("----------------------------------------------------------------------\n");
    truncate_test();
    // Test to unlink full dir
    printf("------------------------------------------------------------------------\n");
    unlink_full_dir_test();
    // Test to unlink files
    printf("------------------------------------------------------------------------\n");
    unlink_files_test();
    // Test to unlink empty dir
    printf("------------------------------------------------------------------------\n");
    unlink_empty_dir_test();
    printf("All tests passed.\n");
    return 0;
}
