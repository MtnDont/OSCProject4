#include <stdio.h>
#include <string.h>

#include "oufs_lib.h" 
#include "virtual_disk.h"

#define BUF_SIZE 100

int main(int argc, char** argv) {
  // Fetch the key environment vars
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];
  char pipe_name_base[MAX_PATH_LENGTH];

  oufs_get_environment(cwd, disk_name, pipe_name_base);

  // Open the virtual disk
  virtual_disk_attach(disk_name, pipe_name_base);
  if(argc == 1) {
    fprintf(stderr, "Usage: oufs_create <file name>\n");
  }else{
    OUFILE *fp = oufs_fopen(cwd, argv[1], "w");
    unsigned char buf[BUF_SIZE];
    if(fp != NULL) {
      int n;
      while((n = read(0, buf, BUF_SIZE)) != 0) {
	oufs_fwrite(fp, buf, n);
      }
    
      oufs_fclose(fp);
    }
  }

  // Clean up
  virtual_disk_detach();
  
  return(0);
}
