#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <limits.h>

int main() {
    const char *dir_name = "tada";
    const int num_dirs = 30; // number of directories to create
    char path[255] = { 0 }; // path buffer
    // Create directories
    for (int i = 0; i < num_dirs; i++) {
        if(i!=0){
            sprintf(path, "%s/%s", path, dir_name);
        }
        else{
            sprintf(path, "%s%s", path, dir_name);
        }
        if (mkdir(path, 0777) != 0) {
            printf("Error: Unable to create directory %s\n", path);
            exit(1);
        }
    }

    printf("Directory created successfully: %s\n", path);
    return 0;
}