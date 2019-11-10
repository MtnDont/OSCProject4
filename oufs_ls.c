#include <stdio.h>
#include <string.h>

#include "oufs_lib.h"
#include "virtual_disk.h"

int main(int argc, char** argv) {
  // Fetch the key environment vars
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];
  char pipe_name_base[MAX_PATH_LENGTH];

  oufs_get_environment(cwd, disk_name, pipe_name_base);

  // Open the virtual disk
  virtual_disk_attach(disk_name, pipe_name_base);

  if(argc == 1) {
    oufs_list(cwd, "");
  }else if(argc == 2){
    oufs_list(cwd, argv[1]);
  }else{
    fprintf(stderr, "Usage: oufs_ls [<name>]\n");
  }

  // Clean up
  virtual_disk_detach();
}
