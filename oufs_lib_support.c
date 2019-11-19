/**
 *  Project 3
 *  oufs_lib_support.c
 *
 *  Author: CS3113
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "virtual_disk.h"
#include "oufs_lib_support.h"

extern int debug;

/**
 * Deallocate a single block.
 * - Modify the in-memory copy of the master block
 * - Add the specified block to THE END of the free block linked list
 * - Modify the disk copy of the deallocated block: next_block points to
 *     UNALLOCATED_BLOCK
 *
 * @param master_block Pointer to a loaded master block.  Changes to the MB will
 *           be made here, but not written to disk
 *
 * @param block_reference Reference to the block that is being deallocated
 *
 */
int oufs_deallocate_block(BLOCK *master_block, BLOCK_REFERENCE block_reference)
{
  BLOCK b;

  if(master_block->content.master.unallocated_front == UNALLOCATED_BLOCK) {
    // No blocks on the free list.  Both pointers point to this block now
    master_block->content.master.unallocated_front = master_block->content.master.unallocated_end =
      block_reference;

  }else{
    BLOCK c;
    BLOCK_REFERENCE br = master_block->content.master.unallocated_end;
    if (virtual_disk_read_block(br, &c) != 0)
      return (-1);
    c.next_block = block_reference;
    master_block->content.master.unallocated_end = block_reference;
    virtual_disk_write_block(br, &c);
  }

  // Update the new end block
  if(virtual_disk_read_block(block_reference, &b) != 0) {
    fprintf(stderr, "deallocate_block: error reading new end block\n");
    return(-1);
  }

  // Change the new end block to point to nowhere
  b.next_block = UNALLOCATED_BLOCK;

  // Write the block back
  if(virtual_disk_write_block(block_reference, &b) != 0) {
    fprintf(stderr, "deallocate_block: error writing new end block\n");
    return(-1);
  }

  return(0);
};


/**
 *  Initialize an inode and a directory block structure as a new directory.
 *  - Inode points to directory block (self_block_reference)
 *  - Inode size = 2 (for . and ..)
 *  - Direcory block: add entries . (self_inode_reference and .. (parent_inode_reference)
 *  -- Set all other entries to UNALLOCATED_BLOCK
 *
 * @param inode Pointer to inode structure to initialize
 * @param block Pointer to block structure to initialize as a directory
 * @param self_block_reference The block reference to the new directory block
 * @param self_inode_reference The inode reference to the new inode
 * @param parent_inode_reference The inode reference to the parent inode
 */
void oufs_init_directory_structures(INODE *inode, BLOCK *block,
				    BLOCK_REFERENCE self_block_reference,
				    INODE_REFERENCE self_inode_reference,
				    INODE_REFERENCE parent_inode_reference)
{
  memset(block, 0, BLOCK_SIZE);

  inode->type = DIRECTORY_TYPE;
  inode->size = 2;
  inode->n_references = 1;
  inode->content = self_block_reference;

  block->content.directory.entry[0].inode_reference = self_inode_reference;
  strcpy(block->content.directory.entry[0].name, ".");

  strcpy(block->content.directory.entry[1].name, "..");
  block->content.directory.entry[1].inode_reference = parent_inode_reference;
  
  for (int i = 2; i < N_INODES_PER_BLOCK; i++) {
    block->content.directory.entry[i].inode_reference = UNALLOCATED_INODE;
  }
  block->next_block = UNALLOCATED_BLOCK;
}


/**
 *  Given an inode reference, read the inode from the virtual disk.
 *
 *  @param i Inode reference (index into the inode list)
 *  @param inode Pointer to an inode memory structure.  This structure will be
 *                filled in before return)
 *  @return 0 = successfully loaded the inode
 *         -1 = an error has occurred
 *
 */
int oufs_read_inode_by_reference(INODE_REFERENCE i, INODE *inode)
{
  if(debug)
    fprintf(stderr, "\tDEBUG: Fetching inode %d\n", i);

  // Find the address of the inode block and the inode within the block
  BLOCK_REFERENCE block = i / N_INODES_PER_BLOCK + 1;
  int element = (i % N_INODES_PER_BLOCK);

  // Load the block that contains the inode
  BLOCK b;
  if(virtual_disk_read_block(block, &b) == 0) {
    // Successfully loaded the block: copy just this inode
    *inode = b.content.inodes.inode[element];
    return(0);
  }
  // Error case
  return(-1);
}


/**
 * Write a single inode to the disk
 *
 * @param i Inode reference index
 * @param inode Pointer to an inode structure
 * @return 0 if success
 *         -x if error
 *
 */
int oufs_write_inode_by_reference(INODE_REFERENCE i, INODE *inode)
{
  if(debug)
    fprintf(stderr, "\tDEBUG: Writing inode %d\n", i);

  // Find the address of the inode block and the inode within the block
  BLOCK_REFERENCE block = i / N_INODES_PER_BLOCK + 1;
  int element = (i % N_INODES_PER_BLOCK);

  // Load the block that contains the inode
  BLOCK b;
  virtual_disk_read_block(block, &b);
  b.content.inodes.inode[element] = *inode;

  if(virtual_disk_write_block(block, &b) != 0) {
    // Failed to write block
    return(-1);
  }

  // Success
  return(0);
}

/**
 * Set all of the properties of an inode
 *
 * @param inode Pointer to the inode structure to be initialized
 * @param type Type of inode
 * @param n_references Number of references to this inode
 *          (when first created, will always be 1)
 * @param content Block reference to the block that contains the information within this inode
 * @param size Size of the inode (# of directory entries or size of file in bytes)
 *
 */

void oufs_set_inode(INODE *inode, INODE_TYPE type, int n_references,
		    BLOCK_REFERENCE content, int size)
{
  inode->type = type;
  inode->n_references = n_references;
  inode->content = content;
  inode->size = size;
}


/*
 * Given a valid directory inode, return the inode reference for the sub-item
 * that matches <element_name>
 *
 * @param inode Pointer to a loaded inode structure.  Must be a directory inode
 * @param element_name Name of the directory element to look up
 *
 * @return = INODE_REFERENCE for the sub-item if found; UNALLOCATED_INODE if not found
 */

int oufs_find_directory_element(INODE *inode, char *element_name)
{
  if(debug)
    fprintf(stderr,"\tDEBUG: oufs_find_directory_element: %s\n", element_name);

  BLOCK b;
  virtual_disk_read_block(inode->content, &b);
  for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++) {
    if (b.content.directory.entry[i].inode_reference != UNALLOCATED_INODE) { 
      if (strcmp(b.content.directory.entry[i].name, element_name) == 0)
        return b.content.directory.entry[i].inode_reference;
    }
  }
  return UNALLOCATED_INODE;
}

/**
 *  Given a current working directory and either an absolute or relative path, find both the inode of the
 * file or directory and the inode of the parent directory.  If one or both are not found, then they are
 * set to UNALLOCATED_INODE.
 *
 *  This implementation handles a variety of strange cases, such as consecutive /'s and /'s at the end of
 * of the path (we have to maintain some extra state to make this work properly).
 *
 * @param cwd Absolute path for the current working directory
 * @param path Absolute or relative path of the file/directory to be found
 * @param parent Pointer to the found inode reference for the parent directory
 * @param child Pointer to the found inode reference for the file or directory specified by path
 * @param local_name String name of the file or directory without any path information
 *             (i.e., name relative to the parent)
 * @return 0 if no errors
 *         -1 if child not found
 *         -x if an error
 *
 */
int oufs_find_file(char *cwd, char * path, INODE_REFERENCE *parent, INODE_REFERENCE *child,
		   char *local_name)
{
  INODE_REFERENCE grandparent;
  char full_path[MAX_PATH_LENGTH];

  // Construct an absolute path the file/directory in question
  if(path[0] == '/') {
    strncpy(full_path, path, MAX_PATH_LENGTH-1);
  }else{
    if(strlen(cwd) > 1) {
      strncpy(full_path, cwd, MAX_PATH_LENGTH-1);
      strncat(full_path, "/", 2);
      strncat(full_path, path, MAX_PATH_LENGTH-1-strnlen(full_path, MAX_PATH_LENGTH));
    }else{
      strncpy(full_path, "/", 2);
      strncat(full_path, path, MAX_PATH_LENGTH-2);
    }
  }

  if(debug) {
    fprintf(stderr, "\tDEBUG: Full path: %s\n", full_path);
  };

  // Start scanning from the root directory
  // Root directory inode
  grandparent = *parent = *child = 0;
  if(debug)
    fprintf(stderr, "\tDEBUG: Start search: %d\n", *parent);

  // Parse the full path
  char *directory_name;
  directory_name = strtok(full_path, "/");
  while(directory_name != NULL) {
    if(strlen(directory_name) >= FILE_NAME_SIZE-1) 
      // Truncate the name
      directory_name[FILE_NAME_SIZE - 1] = 0;
    if(debug){
      fprintf(stderr, "\tDEBUG: Directory: %s\n", directory_name);
    }

    INODE inode;
    oufs_read_inode_by_reference(*child, &inode);
    grandparent = *parent;
    *parent = *child;
    *child = oufs_find_directory_element(&inode, directory_name);

    if (local_name != NULL)
      strcpy(local_name, directory_name);
    if (*child == UNALLOCATED_INODE) {
      if (strtok(NULL, "/") == NULL) {
        return (0);
      }
      return (-1);
    }

    directory_name = strtok(NULL, "/");
  };

  // Item found.
  if(*child == UNALLOCATED_INODE) {
    // We went too far - roll back one step ***
    *child = *parent;
    *parent = grandparent;
  }
  if(debug) {
    fprintf(stderr, "\tDEBUG: Found: %d, %d\n", *parent, *child);
  }

  // Success!
  return(0);
} 


/**
 * Return the bit index for the first 0 bit in a byte (starting from 7th bit
 *   and scanning to the right)
 *
 * @param value: a byte
 * @return The bit number of the first 0 in value (starting from the 7th bit
 *         -1 if no zero bit is found
 */

int oufs_find_open_bit(unsigned char value)
{
  for (int i = 7; i >= 0; --i) {
    if (( (1 << i) & value ) == 0) {
      return i;
    }
  }
  // Not found
  return(-1);
}

/**
 *  Allocate a new directory (an inode and block to contain the directory).  This
 *  includes initialization of the new directory.
 *
 * @param parent_reference The inode of the parent directory
 * @return The inode reference of the new directory
 *         UNALLOCATED_INODE if we cannot allocate the directory
 */
int oufs_allocate_new_directory(INODE_REFERENCE parent_reference)
{
  BLOCK block;
  BLOCK block2;
  // Read the master block
  if(virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &block) != 0) {
    // Read error
    return(UNALLOCATED_INODE);
  }

  INODE child;
  INODE parent;
  INODE_REFERENCE openInode;
  BLOCK_REFERENCE newBlockRef = block.content.master.unallocated_front;

  virtual_disk_read_block(newBlockRef, &block2);
  block.content.master.unallocated_front = block2.next_block;

  int index = 0;
  int bit = -1;
  for (int i = 0; i < N_INODES >> 3; i++) {
    bit = oufs_find_open_bit(block.content.master.inode_allocated_flag[i]);
    if (bit != -1) {
      index = i;
      openInode = index*8 + (7 - bit % 8);
      block.content.master.inode_allocated_flag[i] += (1 << bit);
      break;
    }
  }
  //It didn't find an open bit, return UNALLOCATED_INODE
  if (bit == -1) {
    return (UNALLOCATED_INODE);
  }

  //Read parent and child inodes
  oufs_read_inode_by_reference(parent_reference, &parent);
  oufs_read_inode_by_reference(openInode, &child);
  child.content = newBlockRef;
  parent.size = parent.size + 1;
  
  //Initialize blocks and inodes
  oufs_init_directory_structures(&child, &block2, newBlockRef, openInode, parent_reference);

  //Write all the data into the inodes and blocks
  virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &block);
  virtual_disk_write_block(newBlockRef, &block2);
  
  oufs_write_inode_by_reference(openInode, &child);
  oufs_write_inode_by_reference(parent_reference, &parent);

  //Return new inode reference
  return openInode;
};

/**
 *  Create a zero-length file within a specified diretory
 *
 *  @param parent Inode reference for the parent directory
 *  @param local_name Name of the file within the parent directory
 *  @return Inode reference index for the newly created file
 *          UNALLOCATED_INODE if an error
 *
 *  Errors include: virtual disk read/write errors, no available inodes,
 *    no available directory entrie
 */
INODE_REFERENCE oufs_create_file(INODE_REFERENCE parent, char *local_name)
{
  // Does the parent have a slot?
  INODE inode;

  // Read the parent inode
  if(oufs_read_inode_by_reference(parent, &inode) != 0) {
    return UNALLOCATED_INODE;
  }

  // Is the parent full?
  if(inode.size == N_DIRECTORY_ENTRIES_PER_BLOCK) {
    // Directory is full
    fprintf(stderr, "Parent directory is full.\n");
    return UNALLOCATED_INODE;
  }

  // TODO

  //----------------------------------
  BLOCK block;
  BLOCK pBlock;
  INODE child;
  INODE_REFERENCE inode_reference;

  //Read master block for inode table lookup
  virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &block);
  virtual_disk_read_block(inode.content, &pBlock);

  int index = 0;
  int bit = -1;
  for (int i = 0; i < N_INODES >> 3; i++) {
    bit = oufs_find_open_bit(block.content.master.inode_allocated_flag[i]);
    if (bit != -1) {
      index = i;
      inode_reference = index*8 + (7 - bit % 8);
      block.content.master.inode_allocated_flag[i] += (1 << bit);
      break;
    }
  }
  //It didn't find an open bit, return UNALLOCATED_INODE
  if (bit == -1) {
    return UNALLOCATED_INODE;
  }

  //Set parent size + 1
  inode.size = inode.size + 1;
  
  //Initialize blocks and inodes
  child.type = FILE_TYPE;
  child.n_references = 1;
  child.size = 0;
  child.content = UNALLOCATED_BLOCK;

  //Place inode into parent block and call it (local_name)
  for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++) {
    if (pBlock.content.directory.entry[i].inode_reference == UNALLOCATED_INODE) {
      DIRECTORY_ENTRY dirEn;
      dirEn.inode_reference = inode_reference;
      strcpy(dirEn.name, local_name);
      pBlock.content.directory.entry[i] = dirEn;
      break;
    }
  }

  //Write all the data into the inodes and blocks
  virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &block);
  oufs_write_inode_by_reference(inode_reference, &child);
  virtual_disk_write_block(inode.content, &pBlock);
  oufs_write_inode_by_reference(parent, &inode);
  
  //----------------------------------

  // Success
  return(inode_reference);
}

/**
 * Deallocate all of the blocks that are being used by an inode
 *
 * - Modifies the inode to set content to UNALLOCATED_BLOCK
 * - Adds any content blocks to the end of the free block list
 *    (these are added in the same order as they are in the file)
 * - If the file is using no blocks, then return success without
 *    modifications.
 * - Note: the inode is not written back to the disk (we will let
 *    the calling function handle this)
 *
 * @param inode A pointer to an inode structure that is already in memory
 * @return 0 if success
 *         -x if error
 */

int oufs_deallocate_blocks(INODE *inode)
{
  BLOCK master_block;
  BLOCK block;

  // Nothing to do if the inode has no content
  if(inode->content == UNALLOCATED_BLOCK)
    return(0);

  // TODO
  if (virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &master_block) != 0)
    return(-1);
  if (inode->type == FILE_TYPE) {
    for (int i = 0; i < (inode->size + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE; i++) {
      //if (oufs_deallocate_block(&master_block, i + inode->content) != 0)
        //return (-1);
    }
  }
  else if (inode->type == DIRECTORY_TYPE) {

  }

  // Success
  return(0);
}

/**
 * Allocate a new data block
 * - If one is found, then the free block linked list is updated
 *
 * @param master_block A link to a buffer ALREADY containing the data from the master block.
 *    This buffer may be modified (but will not be written to the disk; we will let
 *    the calling function handle this).
 * @param new_block A link to a buffer into which the new block will be read.
 *
 * @return The index of the allocated data block.  If no blocks are available,
 *        then UNALLOCATED_BLOCK is returned
 *
 */
BLOCK_REFERENCE oufs_allocate_new_block(BLOCK *master_block, BLOCK *new_block)
{
  // Is there an available block?
  if(master_block->content.master.unallocated_front == UNALLOCATED_BLOCK) {
    // Did not find an available block
    if(debug)
      fprintf(stderr, "No blocks\n");
    return(UNALLOCATED_BLOCK);
  }

  // TODO
  BLOCK_REFERENCE block_reference;
  BLOCK b;
  block_reference = master_block->content.master.unallocated_front;
  virtual_disk_read_block(block_reference, &b);
  master_block->content.master.unallocated_front = b.next_block;
  b.next_block = UNALLOCATED_BLOCK;

  return(block_reference);
}

