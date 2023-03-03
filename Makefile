CC = gcc
CFLAGS = -Wall -g
EXECUTABLES = disk block
# fuseflag = `pkg-config fuse3 --cflags --libs`

disk_layer.o: disk_layer.c disk_layer.h
	$(CC) $(CFLAGS) -c disk_layer.c

disk: disk_layer.o
	$(CC) $(CFLAGS) -o disk disk_layer.o

block_layer.o: block_layer.c block_layer.h disk_layer.o disk_layer.h
	$(CC) $(CFLAGS) -c block_layer.c

block: block_layer.o disk_layer.o
	$(CC) $(CFLAGS) -o block block_layer.o disk_layer.o

clean:
	rm -f *.o $(EXECUTABLES)
