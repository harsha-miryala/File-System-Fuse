#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char *argv[]){
    int log_file = open("basicAPITest.txt", O_CREAT | O_RDWR ,  S_IRWXU | S_IRWXG | S_IRWXO);
    int fd, res=-1, nsuccess=0, nfail=0;
    int nbytes=-1;
    char msg[1000];
    int read_bytes=-1;
    char buf[100];
    memset(buf,0,100);

    fd= open("basic_api_test_files/test90.txt", O_CREAT | O_WRONLY | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);
    read_bytes= read(fd, buf, 100);
    //the files contain the corresponding file names. Check if it is correctly being read.
    assert(strcmp("test90.txt", buf) ==0);
    memset(buf, 0, 100);

    strcpy(buf, "start"); // why are we doing this?
    lseek(fd, 0, SEEK_SET); //sets the file pointer to the 0th pos.
    nbytes= write(fd, buf, strlen(buf));
    memset(buf, 0, 100);

    lseek(fd, 0, SEEK_SET);
    nbytes= read(fd, buf, 100); // read from the beginning
    assert(strcmp("start0.txt",buf)==0); // since the 1st 5 char are overwritten by previous write. The contents should now match "first0.txt"
    memset(buf, 0, 100); //reset the buff

    //testing lseek from the end. The string should be appended to the end.

    strcpy(buf,"end");
    lseek(fd, 0, SEEK_END);
    nbytes= write(fd, buf, strlen(buf));
    memset(buf,0,100);
    lseek(fd, 0, SEEK_END);
    nbytes= read(fd, buf, 100);
    assert(strcmp("start0.txtend",buf)==0);
    memset(buf, 0, 100);

    // logging to log_file
    memset(msg, 0, 1000);
    sprintf(msg, "SUCCESSFUL TESTING OF LSEEK, READ, WRITE OPERATION");
    write(log_file, msg, strlen(msg));

    close(log_file);
    return 0;

}
