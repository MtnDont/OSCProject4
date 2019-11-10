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
  if(argc != 3) {
    fprintf(stderr, "Usage: oufs_copy <source file name> <destination file name>\n");
    return(-1);
  }else{
    OUFILE *fp_in = oufs_fopen(cwd, argv[1], "r");
    OUFILE *fp_out = oufs_fopen(cwd, argv[2], "w");
    unsigned char buf[1000];
    if(fp_in != NULL && fp_out != NULL) {
      int n;
      while((n = oufs_fread(fp_in, buf, 1000)) != 0) {
	oufs_fwrite(fp_out, buf, n);
      }
    
      oufs_fclose(fp_in);
      oufs_fclose(fp_out);
    }
  }

  // Clean up
  virtual_disk_detach();
  
  return(0);
}
