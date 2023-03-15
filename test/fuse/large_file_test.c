#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#define ITER_COUNT 1500

// we need to write more than 5 blocks of 4K capacity
//total size of content: more than 20480 bytes 
// in each iteration we shall writ 15 bytes to the content.

int main(int agrc, char **argv){
    //opening the log file
    int log_file = open("LargeFileLog.txt", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    int fd, res=-1;
    char msg[1001];
    
    memset(msg, 0, 1000);

    //open the large file
    fd = open("large_file.txt", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd==-1){
        sprintf(msg, "Failure to open/create large file \'large_file.txt\'");
        res=write(log_file, msg, strlen(msg));
        return 0;
    }

    // Large file is successfully opened here. Start writing to it.
    for(int i=0;i< ITER_COUNT;i++){
        //in each iteration write about 15 bytes
        char buff[11]="large_File";
        for(int j=0;j<100;j++){
            strcpy(msg,buff);
        }
        res = write(fd,msg,strlen(msg));
        if(res==-1){
            memset(msg,0,1001);
            sprintf(msg, "Error writing on large File in iter %d\n",i);
            res=write(log_file, msg, strlen(msg));
            return 0;
        }
    }
    // writing to log file is finished. Update the log file.
    memset(msg, 0, 1001);
    sprintf(msg,"SUCCESS! FILE SIZE: %d bytes\n", ITER_COUNT *1000);
    res=write(log_file, msg, strlen(msg));      

    close(fd);
    close(log_file);
    return 0;
}