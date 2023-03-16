#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#define ITER_COUNT 1500000

int main(int agrc, char **argv){
    //opening the log file
    int log_file = open("ExtraLargeFileLog.txt", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    int fd, res=-1;
    char msg[4096];
    memset(msg, 'h', 4096);

    //open the large file
    fd = open("extra_large_file.txt", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd==-1){
        sprintf(msg, "Failure to open/create large file \'large_file.txt\'");
        res=write(log_file, msg, strlen(msg));
        return 0;
    }

    // Large file is successfully opened here. Start writing to it.
    for(int i=0;i< ITER_COUNT;i++){
        res = write(fd,msg,strlen(msg));
        if(res==-1){
            memset(msg,0,4096);
            sprintf(msg, "Error writing on large File in iter %d\n",i);
            res=write(log_file, msg, strlen(msg));
            return 0;
        }
    }
    // writing to log file is finished. Update the log file.
    memset(msg, 0, 4096);
    sprintf(msg,"SUCCESS! FILE SIZE: %d GB\n", (ITER_COUNT/1000000)*4);
    res=write(log_file, msg, strlen(msg));

    close(fd);
    close(log_file);
    return 0;
}