/**
 *  Project 3
 *  oufs_inspect
 *
 *  Allows the user to view raw block and inode-level information. 
 *  This is not intended as a standard user executable.  Instead, it
 *  is intended to aid in debugging and testing.
 *
 *  Usage: oufs_inspect -help
 *
 *  Author: CS3113
 *
 */

#include <stdio.h>
#include <string.h>

#include "oufs_lib_support.h"

// NOTE: this is the only oufs exeutable that should include this file
#include "virtual_disk.h"

int main(int argc, char** argv) {
  // Get the key environment variables
  char cwd[MAX_PATH_LENGTH];
  char disk_name[MAX_PATH_LENGTH];
  char pipe_name_base[MAX_PATH_LENGTH];

  // Get the environment variable information
  oufs_get_environment(cwd, disk_name, pipe_name_base);

  // Connect to the virtual disk
  if(virtual_disk_attach(disk_name, pipe_name_base) != 0) {
    return(-1);
  }

  // Respond to the different options
  if(argc == 1){
    // Zero arguments
    printf("Usage: oufs_inspect -help\n");
  }else if(argc == 2){
    if(strncmp(argv[1], "-master", 8) == 0) {
      // Master record
      BLOCK block;
      if(virtual_disk_read_block(0, &block) != 0) {
	fprintf(stderr, "Error reading master block\n");
      }else{
	// Block read: report state
	printf("Inode table:\n");
	for(int i = 0; i < N_INODES >> 3; ++i) {
	  printf("%02x\n", block.content.master.inode_allocated_flag[i]);
	}
	printf("Unallocated front: %d\n", block.content.master.unallocated_front);
	printf("Unallocated end: %d\n", block.content.master.unallocated_end);
      }

    }else if(strncmp(argv[1], "-help", 6) == 0) {
      // User is asking for help
      printf("Usage:\n");
      printf("oufs_inspect -master\t\t Show the master block\n");
      printf("oufs_inspect -help\t\t Print this help\n");
      printf("oufs_inspect -inode <#>\t\t Print contents of INODE #\n");
      printf("oufs_inspect -dblock <#>\t Print the contents of directory block #\n");
      printf("oufs_inspect -block <#>\t\t Print the top-level block data for block #\n");
      printf("oufs_inspect -data <#>\t\t Print the raw data contents for the block (including printable characters)\n");
    }else{
      fprintf(stderr, "Unknown argument (%s)\n", argv[1]);
    }

  }else if(argc == 3) {
    if(strncmp(argv[1], "-inode", 7) == 0) {
      // Inode query
      int index;
      if(sscanf(argv[2], "%d", &index) == 1){
	if(index < 0 || index >= N_INODES) {
	  fprintf(stderr, "Inode index out of range (%s)\n", argv[2]);
	}else{
	  INODE inode;
	  oufs_read_inode_by_reference(index, &inode);

	  printf("Inode: %d\n", index);
	  printf("Type: ");
	  switch(inode.type)
	    {
	    case UNUSED_TYPE:
	      printf("UNUSED\n");
	      break;
	    case DIRECTORY_TYPE:
	      printf("DIRECTORY\n");
	      break;
	    case FILE_TYPE:
	      printf("FILE\n");
	      break;
	    }
	  printf("Nreferences: %d\n", inode.n_references);
	  printf("Content block: %d\n", inode.content);
	  printf("Size: %d\n", inode.size);
	}
      }else{
	fprintf(stderr, "Unknown argument (-inode %s)\n", argv[2]);
      }

    }else if(strncmp(argv[1], "-dblock", 8) == 0) {
      // Inspect directory block
      int index;

      // Parse parameter
      if(sscanf(argv[2], "%d", &index) == 1){
	if(index < 0 || index >= N_BLOCKS) {
	  fprintf(stderr, "Block index out of range (%s)\n", argv[2]);
	}else{
	  // success
	  BLOCK block;
	  // Read the block
	  virtual_disk_read_block(index, &block);

	  // display block data
	  printf("Directory at block %d:\n", index);
	  for(int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; ++i) {
	    if(block.content.directory.entry[i].inode_reference != UNALLOCATED_INODE) {
	      printf("Entry %d: name=\"%s\", inode=%d\n", i,
		     block.content.directory.entry[i].name,
		     block.content.directory.entry[i].inode_reference);
	    }
	  }
	}
      }

    }else if(strncmp(argv[1], "-block", 7) == 0) {
      // Inspect high-level block
      int index;

      // Parse the one argument
      if(sscanf(argv[2], "%d", &index) == 1){
	if(index < 0 || index >= N_BLOCKS) {
	  fprintf(stderr, "Block index out of range (%s)\n", argv[2]);
	}else{
	  // Success
	  BLOCK block;
	  virtual_disk_read_block(index, &block);
	  printf("Block %d:\n", index);
	  printf("Next block: %d\n", block.next_block);
	}
      }

    }else if(strncmp(argv[1], "-data", 5) == 0) {
      // Inspect raw block
      int index;

      // Parse the argument
      if(sscanf(argv[2], "%d", &index) == 1){
	if(index < 0 || index >= N_BLOCKS) {
	  fprintf(stderr, "Block index out of range (%s)\n", argv[2]);
	}else{
	  // Success
	  BLOCK block;

	  // Get the spcified block
	  virtual_disk_read_block(index, &block);
	  printf("Raw data at block %d:\n", index);
	  for(int i = 0; i < DATA_BLOCK_SIZE; ++i) {
	    if(block.content.data.data[i] >= ' ' && block.content.data.data[i] <= '~')
	      printf("%3d: %02x %c\n", i, block.content.data.data[i],
		     block.content.data.data[i]);
	    else
	      printf("%3d: %02x\n", i, block.content.data.data[i]);
	  }
	  printf("Next block: %d\n", block.next_block);
	}
      }
    }

  }

  // All done: detach from the disk
  virtual_disk_detach();
}

