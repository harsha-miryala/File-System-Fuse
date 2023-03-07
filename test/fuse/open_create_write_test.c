#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#define TOTAL_FILES 1000

int logfile_write( char *msg, char *test_description, int log_file, int nsuccess, int nfail){
    //Logfile contains the results of each tests (create, open, write operations)
    int res = write(log_file, test_description, strlen(test_description));
    if(res == -1){
        sprintf(msg, "Logfile write failed\n",strlen(msg));
        write(log_file, msg, strlen(msg));
        memset(msg,0,1000);
    }

    sprintf(msg, "Success: %d\n", nsuccess);
    res= write(logfile, msg, strlen(msg));
    memset(msg,0,1000);

    if(res==-1){
        sprintf(msg, "Logfile write failed\n",strlen(msg));
        write(log_file, msg, strlen(msg));
        memset(msg,0,1000);
    }

    sprintf(msg, "Failures: %d\n", nfail);
    res= write(logfile, msg, strlen(msg));
    memset(msg,0,1000);

    if(res==-1){
        sprintf(msg, "Logfile write failed\n",strlen(msg));
        write(log_file, msg, strlen(msg));
        memset(msg,0,1000);
    }
}


int main(int argc, char *argv[]){
    int log_file= open("basicAPITest.txt", O_CREAT | O_RDWR ,  S_IRWXU | S_IRWXG | S_IRWXO);
    int res=-1, nsuccess=0, nfail=0;
    int fd;

    char test_dir[100]={'\n'};
    char fname[100]={'\0'};
    char *fname_prefix = "test";
    char *fname_suffix = ".txt";

    char msg[1000];
    char num[5]={'\0'};

    for (int i=1; i<TOTAL_FILES; i++){
        //formatting the filemames
        sprintf(num,"%d",i);
        strcat(strcat(strcat(fname, fname_prefix),num),fname_suffix);
        strcat(strcat(test_dir, "basic_api_test_files/"),fname);   

        //opening the directory
        fd=open(test_dir, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO );

        if(fd==-1){
            sprintf(msg,"File %s creation failed\n", fname);
            res= write(log_file, msg, strlen(msg));
            memset(msg,0,1000);
        }
        else{
            // file open is successful
            res = write(fd, fname, strlen(fname));
            if(res= -1){
                    nfail++;
                    sprintf(msg,"File %s write failed\n", fname);
                    write(fd, msg, strlen(msg));
                    memset(msg,0,1000);
            }
            else{
                nsuccess++;
            }
            close(fd);
        }
        memset(fname,0,100);
        memset(test_dir,0,100);
    }
    logfile_write(msg, "Results of OPEN, CREATE, WRITE:\n", log_file, nsuccess, nfail);

    close(log_file);
    return 0;
}