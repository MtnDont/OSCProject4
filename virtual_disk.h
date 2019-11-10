#ifndef VDISK_H

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include "oufs.h"

int virtual_disk_attach(char *virtual_disk_name, char *pipe_name_base);
int virtual_disk_detach();
int virtual_disk_read_block(BLOCK_REFERENCE block_ref, void *block);
int virtual_disk_write_block(BLOCK_REFERENCE block_ref, void *block);

#endif
