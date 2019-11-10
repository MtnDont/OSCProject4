/**
Print the contents of a file in the OU File System.

CS3113

*/

#include <stdio.h>
#include <string.h>

#include "oufs_lib.h"
#include "virtual_disk.h"

#define BUF_SIZE 1000

int main(int argc, char** argv) {
  // Fetch the key environment vars
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];
  char pipe_name_base[MAX_PATH_LENGTH];

  oufs_get_environment(cwd, disk_name, pipe_name_base);

  // Open the virtual disk
  virtual_disk_attach(disk_name, pipe_name_base);

  // Check parameters
  if(argc != 2) {
    fprintf(stderr, "Usage: oufs_cat <file name>\n");
  }else{
    OUFILE *fp = oufs_fopen(cwd, argv[1], "r");
    unsigned char buf[BUF_SIZE];
    if(fp != NULL) {
      // Successfully opened the file for reading
      int n;
      // Loop until the contents of the file are all printed to STDOUT
      while((n = oufs_fread(fp, buf, BUF_SIZE)) != 0) {
	write(1, buf, n);
      }

      // Clean up
      oufs_fclose(fp);
    }
  }

  // Clean up
  virtual_disk_detach();
  
  return(0);
}
