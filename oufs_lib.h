#ifndef OUFS_LIB_H
#define OUFS_LIB_H
#include "oufs.h"

#define MAX_PATH_LENGTH 200

// PROVIDED
void oufs_get_environment(char *cwd, char *disk_name, char *pipe_name_base);

// PROJECT 3: to implement
int oufs_format_disk(char  *virtual_disk_name, char *pipe_name_base);
int oufs_mkdir(char *cwd, char *path);
int oufs_list(char *cwd, char *path);
int oufs_rmdir(char *cwd, char *path);

// PROJECT 4: to implement
OUFILE* oufs_fopen(char *cwd, char *path, char *mode);
void oufs_fclose(OUFILE *fp);
int oufs_fwrite(OUFILE *fp, unsigned char * buf, int len);
int oufs_fread(OUFILE *fp, unsigned char * buf, int len);
int oufs_remove(char *cwd, char *path);
int oufs_link(char *cwd, char *path_src, char *path_dst);

#endif

