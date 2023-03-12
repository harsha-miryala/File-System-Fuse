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
    // Open the logfile
    int log_file = open("basicAPITest.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
    int nsuccess = 0, nfail = 0;
    int res = -1;
    char msg[1000];
    char filename[100] = {'\0'};
    char dir_name[100] = {'\0'};
    char num[5] = {'\0'};
    char *fname_prefix = "test";
    char *fname_suffix = ".txt";

    // Unlink every 1 in 5 files
    for (int i = 1; i < TOTAL_FILES; i++) {
        if (i % 5 == 0) {
            // Construct the filename and directory path
            sprintf(num, "%d", i);
            strcat(strcat(strcat(filename, fname_prefix), num), fname_suffix);
            strcat(strcat(dir_name, "basic_api_test_files/"), filename);
            memset(num, 0, 5);

            // Attempt to unlink the file and update the test results accordingly
            res = unlink(dir_name);
            if (res == -1) {
                sprintf(msg, "UNLINK FAILURE for %s\n", filename);
                res = write(log_file, msg, strlen(msg));
                memset(msg, 0, 1000);
                nfail++;
            } else {
                nsuccess++;
            }
            memset(filename, 0, 100);
            memset(dir_name, 0, 100);
        } else {
            continue;
        }
    }
    // Write the unlink test results to the logfile
    logfile_write(msg, "UNLINK TEST RESULTS:\n", log_file, nsuccess, nfail);
}
