/**
 *  Project 3
 *  virtual_disk.c
 *
 *  Author: CS3113
 *
 *  Implementation of block-level I/O with a disk
 */


#include "oufs.h"
#include "storage.h"
#include "virtual_disk.h"

// Yes, another global variable: this is how we achieve persistence in 
//  this case.
STORAGE *storage = NULL;

/**
 *  Atttach to the specified virtual disk
 *
 *  @param virtual_disk_name Name of the virtual disk to open
 *  @param pipe_name_base  Base name of the input and outputs
 *    NOTE: NOT USED IN THIS IMPLEMENTATION
 *  @return 0 if success; -1 with an error
 */
int virtual_disk_attach(char *virtual_disk_name, char *pipe_name_base)
{
  // Initialize the general storage system
  storage = init_storage(virtual_disk_name, pipe_name_base);

  // Parse result
  if(storage == NULL) 
    return(-1);
  else {
    // Success
    return(0);
  }
}

/**
 *  Detach from the specified vitual disk.
 *
 * @return Status after closing the connection to the server
 * @return 0 if closed succesfully; -1  if an error
 */
int virtual_disk_detach()
{
  if(storage == NULL)
    return(-1);
  int ret = close_storage(storage);

  storage = NULL;
  return(ret);
}

/**
 *  Read the specified block from the storage file
 *
 * @param block_ref Integer index of the block to read
 * @param block Buffer in which to store the read block
 * @return -1 if an error has occurred; 0 if successful
 */

int virtual_disk_read_block(BLOCK_REFERENCE block_ref, void *block)
{
  if(block_ref >= N_BLOCKS) {
    // Improper ref
    return(-1);
  };

  // Read the bytes
  int ret = get_bytes(storage, block, block_ref * BLOCK_SIZE, BLOCK_SIZE);
  if(ret > 0)
    // Success
    return(0);
  else
    // Error
    return(-1);
}
/**
 * Write the specified block to the storage file
 *
 * @param block_ref Integer index of the block to write
 * @param block Buffer containing the block to write
 * @return -1 if an error has occurred; 0 if successful
 */

int virtual_disk_write_block(BLOCK_REFERENCE block_ref, void *block)
{
  if(block_ref >= N_BLOCKS) {
    return(-1);
  };

  // Write the bytes
  int ret = put_bytes(storage, block, block_ref * BLOCK_SIZE, BLOCK_SIZE);
  
  if(ret > 0)
    // SUccess
    return(0);
  else
    // Error
    return(-1);

}
