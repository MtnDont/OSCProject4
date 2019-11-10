#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct 
{
  int fd;
} STORAGE;


STORAGE * init_storage(char * name, char *pipe_name_base);
int close_storage(STORAGE *storage);
int get_bytes(STORAGE *storage, unsigned char *buf, int location, int len);
int put_bytes(STORAGE *storage, unsigned char *buf, int location, int len);

