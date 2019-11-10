#include <stdio.h>
#include <string.h>
#include "oufs_lib.h"

int main(int argc, char **argv)
{
  // Get the environmental variables
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];
  char pipe_name_base[MAX_PATH_LENGTH];
  oufs_get_environment(cwd, disk_name,  pipe_name_base);

  // Format the disk
  oufs_format_disk(disk_name, pipe_name_base);

  return(0);

}
