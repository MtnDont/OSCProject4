#include <stdio.h>
#include "oufs.h"

int main(int argc, char **argv)
{
  printf("BLOCK_SIZE: %d\n", BLOCK_SIZE);
  printf("N_BLOCKS: %d\n", N_BLOCKS);
  printf("UNALLOCATED_BLOCK reference: %d\n", UNALLOCATED_BLOCK);
  printf("UNALLOCATED_INODE reference: %d\n", UNALLOCATED_INODE);
  printf("DATA_BLOCK_SIZE: %d\n", DATA_BLOCK_SIZE);
  printf("INODES_PER_BLOCK: %d\n", N_INODES_PER_BLOCK);
  printf("N_INODES: %d\n", N_INODES);
  printf("DIRECTORY_ENTRIES_PER_BLOCK: %d\n", N_DIRECTORY_ENTRIES_PER_BLOCK);
  
}
