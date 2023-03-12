#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#define TOTAL_FILES 1000

// function to write to logfile
int logfile_write(char *msg, char *test_description, int log_file, int nsuccess, int nfail){
    // Logfile contains the results of each tests (create, open, write operations)
    int res = write(log_file, test_description, strlen(test_description));
    if(res == -1){
        sprintf(msg, "Logfile write failed\n");
        res = write(log_file, msg, strlen(msg));
        memset(msg, 0, 1000);
    }
    sprintf(msg, "Success: %d\n", nsuccess);
    res = write(log_file, msg, strlen(msg));
    memset(msg, 0, 1000);
    if(res == -1){
        sprintf(msg, "Logfile write failed\n");
        res = write(log_file, msg, strlen(msg));
        memset(msg, 0, 1000);
    }
    sprintf(msg, "Failures: %d\n", nfail);
    res = write(log_file, msg, strlen(msg));
    memset(msg, 0, 1000);
    if(res == -1){
        sprintf(msg, "Logfile write failed\n");
        res = write(log_file, msg, strlen(msg));
        memset(msg, 0, 1000);
    }
    return res;
}

int main(int argc, char *argv[]) {
    // create and open log_file with read, write and execute permissions for user, group and others
    int log_file = open("basicAPITest.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
    int file_descriptor, nsuccess = 0, nfail = 0;
    int num_bytes = -1;
    char message[1000];
    int read_bytes = -1;
    char buffer[100];
    char num[5] = {'\0'};
    char test_dir[100] = {'\0'};
    char fname[100] = {'\0'};
    char *fname_prefix = "test";
    char *fname_suffix = ".txt";
    // initialize buffer with zeros
    memset(buffer, 0, 100);
    for (int i = 1; i < TOTAL_FILES; i++){
        // formatting the filenames
        sprintf(num, "%d", i);
        strcat(strcat(strcat(fname, fname_prefix), num), fname_suffix);
        // TODO : Can make it generic by creating mount dir var and use it and create dir
        // strcat(strcat(test_dir, "/home/ubuntu/File-System-Fuse/mpoint/basic_api_test_files/"), fname);
        strcat(strcat(test_dir, "basic_api_test_files/"), fname);

        // opening the directory with create mode
        file_descriptor = open(test_dir, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

        if(file_descriptor == -1){
            printf("error opening the file\n");
            nfail++;
        }
        else{
            printf("file_descriptor:%d\n",file_descriptor);
        }

        // read the first 100 bytes of the file
        read_bytes = read(file_descriptor, buffer, 100);
        printf("No of bytes read is %d\n", read_bytes);

        // clear buffer
        memset(buffer, 0, 100);

        // write "start" to the file
        strcpy(buffer, "start");

        // set file pointer to the beginning of the file
        lseek(file_descriptor, 0, SEEK_SET);

        // write "start" to the file
        num_bytes = write(file_descriptor, buffer, strlen(buffer));

        // clear buffer
        memset(buffer, 0, 100);

        // set file pointer to the beginning of the file
        lseek(file_descriptor, 0, SEEK_SET);

        // read the first 100 bytes of the file
        num_bytes = read(file_descriptor, buffer, 100);

        // check if the content of the file matches the expected value
        //test1.txt-> start.txt
        //test20.txt-> start0.txt
        //test88.txt -> start8.txt
        //test787.txt -> start87.txt
        strncpy(fname, "start", 5);

        assert(strcmp(fname, buffer) == 0);

        // clear buffer
        memset(buffer, 0, 100);

        // write "end" to the end of the file
        strcpy(buffer, "end");

        // set file pointer to the end of the file
        lseek(file_descriptor, 0, SEEK_END);

        // append "end" to the file
        num_bytes = write(file_descriptor, buffer, strlen(buffer));

        // clear buffer
        memset(buffer, 0, 100);

        // set file pointer to the beginning of the file
        lseek(file_descriptor, 0, SEEK_SET);

        // read the first 100 bytes of the file from the beginning
        num_bytes = read(file_descriptor, buffer, 100);
        
        printf("Reading the file after writing end\n No of bytes read is %d\n Content is %s\n", num_bytes, buffer);

        strcat(fname, "end");

        // check if the content of the file matches the expected value
        assert(strcmp(fname, buffer) == 0);

        // clear buffer
        memset(buffer, 0, 100);
        memset(fname, 0, 100);
        memset(test_dir, 0, 100);
        nsuccess++;
    }
    // call to logfile_write function
    memset(message, 0, 1000);
    logfile_write(message, "SUCCESSFUL TESTING OF LSEEK, READ, WRITE OPERATION\n",
                  log_file, nsuccess, nfail);
    close(log_file);
    return 0;
}
