#include <stdio.h>

int main() {
    long long int size = 6LL * 1024 * 1024 * 1024;  // 6 GB in bytes
    FILE *fp = fopen("file.bin", "wb");
    if (fp == NULL) {
        printf("Error: unable to create file.\n");
        return 1;
    }
    fseek(fp, size - 1, SEEK_SET);
    fputc(0, fp);
    fclose(fp);
    return 0;
}