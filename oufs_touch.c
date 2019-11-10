/**
   If the specified file does not exist, then create it.

   Author: CS 3113

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

  if(argc != 2) {
    fprintf(stderr, "Usage: oufs_touch <file name>\n");
  }else{
    // Open the virtual disk
    virtual_disk_attach(disk_name, pipe_name_base);

    // Open the file
    OUFILE *fp = oufs_fopen(cwd, argv[1], "a");

    if(fp != NULL) {
      oufs_fclose(fp);
    }else{
      fprintf(stderr, "Error opening file.\n");
    }
    // Clean up
    virtual_disk_detach();
  }
  return(0);
}
