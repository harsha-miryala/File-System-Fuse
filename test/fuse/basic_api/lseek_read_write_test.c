#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char *argv[]) {
    // create and open log_file with read, write and execute permissions for user, group and others
    int log_file = open("basicAPITest.txt", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

    int file_descriptor, res = -1, nsuccess = 0, nfail = 0;
    int num_bytes = -1;
    char message[1000];
    int read_bytes = -1;
    char buffer[100];

    // initialize buffer with zeros
    memset(buffer, 0, 100);

    // create and open file with read, write and execute permissions for user, group and others
    file_descriptor = open("basic_api_test_files/test90.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);

    // read the first 100 bytes of the file
    read_bytes = read(file_descriptor, buffer, 100);

    // check if the file name matches the expected value
    assert(strcmp("test90.txt", buffer) == 0);

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
    assert(strcmp("start0.txt", buffer) == 0);

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

    // set file pointer to the end of the file
    lseek(file_descriptor, 0, SEEK_END);

    // read the first 100 bytes of the file from the end
    num_bytes = read(file_descriptor, buffer, 100);

    // check if the content of the file matches the expected value
    assert(strcmp("start0.txtend", buffer) == 0);

    // clear buffer
    memset(buffer, 0, 100);

    // log successful testing of lseek, read, and write operations to the log_file
    memset(message, 0, 1000);
    sprintf(message, "SUCCESSFUL TESTING OF LSEEK, READ, WRITE OPERATION");
    write(log_file, message, strlen(message));

    // close the log_file and file_descriptor
    close(log_file);

    return 0;
}
