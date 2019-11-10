/**
Remove a file

CS3113

*/

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

  // Check arguments
  if(argc == 2) {
    // Open the virtual disk
    virtual_disk_attach(disk_name, pipe_name_base);

    oufs_remove(cwd, argv[1]);
    // Clean up
    virtual_disk_detach();
    
  }else{
    fprintf(stderr, "Usage: oufs_remove <file name>\n");
  }

}
