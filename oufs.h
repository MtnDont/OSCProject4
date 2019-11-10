/*******
 * Low-level file system definitions
 *
 * DO NOT CHANGE THIS DATA STRUCTURE
 *
 * CS 3113
 *
 * 
 */

// Only evaluate these definitions once, even if included multiple times
#ifndef FILE_STRUCTS_H
#define FILE_STRUCTS_H

#include <string.h>
#include <limits.h>


// Implementation of min operator
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

/**********************************************************************/
// Default virtual disk parameters (used if they are not yet defined)
#ifndef BLOCK_SIZE

// Number of bytes in a disk block
#define BLOCK_SIZE 256

// Total number of blocks
#define N_BLOCKS 128

// Number of inode blocks on the virtual disk
#define N_INODE_BLOCKS 4

#endif


/**********************************************************************/
/*
File system layout onto disk blocks:

Block 0: Master block
Blocks 1 ... N_INODE_BLOCKS: inodes
Blocks N_INODE_BLOCKS+1 ... N_BLOCKS_ON_DISK-1: data for files and directories
   (Block N_BLOCKS+1 is allocated for the root directory)
*/


/**********************************************************************/
// Basic types and sizes
// Chosen carefully so that all block types pack nicely into a full block


// An index for a block (0, 1, 2, ...)
typedef unsigned short BLOCK_REFERENCE;

// Value used as an index when it does not refer to a block
#define UNALLOCATED_BLOCK (USHRT_MAX-1)

// An index that refers to an inode
typedef unsigned short INODE_REFERENCE;

// Value used as an index when it does not refer to an inode
#define UNALLOCATED_INODE (USHRT_MAX)

// Number of bytes available for block data
#define DATA_BLOCK_SIZE ((int)(BLOCK_SIZE-sizeof(int)))

// The block on the virtual disk containing the root directory
#define ROOT_DIRECTORY_BLOCK (N_INODE_BLOCKS + 1)

// The Inode for the root directory
#define ROOT_DIRECTORY_INODE 0

// Size of file/directory name
#define FILE_NAME_SIZE ((int)(16 - sizeof(INODE_REFERENCE)))

/**********************************************************************/
// Data block: storage for file contents (project 4!)
typedef struct data_block_s
{
  unsigned char data[DATA_BLOCK_SIZE];
} DATA_BLOCK;


/**********************************************************************/
// Inode Types
typedef enum {UNUSED_TYPE=0, DIRECTORY_TYPE, FILE_TYPE} INODE_TYPE;

// Single inode
typedef struct inode_s
{
  // Type of INODE
  INODE_TYPE type;

  // Number of directory references to this inode
  unsigned char n_references;

  // Contents.  UNALLOCATED_BLOCK means that this entry is not used
  BLOCK_REFERENCE content;

  // File: size in bytes; Directory: number of directory entries
  //  (including . and ..)
  unsigned int size;
} INODE;

// Number of inodes stored in each block
#define N_INODES_PER_BLOCK ((int)(DATA_BLOCK_SIZE/sizeof(INODE)))

// Total number of inodes in the file system
// #define N_INODES (N_INODES_PER_BLOCK * N_INODE_BLOCKS)
// Hack to deal with the fact that our inode allocation table is not 
//   declared properly
#define N_INODES (((N_INODES_PER_BLOCK * N_INODE_BLOCKS)>>3)<<3)

// Block of inodes
typedef struct inode_block_s
{
  INODE inode[N_INODES_PER_BLOCK];
} INODE_BLOCK;


/**********************************************************************/
// Block 0
#define MASTER_BLOCK_REFERENCE 0

typedef struct master_block_s
{
  // 8 inodes per byte: One inode per bit: 1 = allocated, 0 = free
  // Inode 0 (zero) is byte 0, bit 7 
  //       1        is byte 0, bit 6
  //       8        is byte 1, bit 7
  unsigned char inode_allocated_flag[N_INODES >> 3];

  // Double-ended linked list representation for unallocated blocks
  BLOCK_REFERENCE unallocated_front;
  BLOCK_REFERENCE unallocated_end;

} MASTER_BLOCK;

/**********************************************************************/
// Single directory element
typedef struct directory_entry_s
{
  // Name of file/directory
  char name[FILE_NAME_SIZE];

  // UNALLOCATED_INODE if this directory entry is non-existent
  INODE_REFERENCE inode_reference;

} DIRECTORY_ENTRY;

// Number of directory entries stored in one data block
#define N_DIRECTORY_ENTRIES_PER_BLOCK ((int)(DATA_BLOCK_SIZE / sizeof(DIRECTORY_ENTRY)))

// Directory block
typedef struct directory_block_s
{
  DIRECTORY_ENTRY entry[N_DIRECTORY_ENTRIES_PER_BLOCK];
} DIRECTORY_BLOCK;

/**********************************************************************/
// All-encompassing structure for a disk block
// The union says that all 4 of these elements occupy overlapping bytes in 
//  memory (hence, a block will only be one of these 4 at any given time)

typedef struct
{
  BLOCK_REFERENCE next_block;
  union {
    DATA_BLOCK data;
    MASTER_BLOCK master;
    INODE_BLOCK inodes;
    DIRECTORY_BLOCK directory;
  } content;
} BLOCK;


/**********************************************************************/
// Representing files (project 4!)

#define MAX_BLOCKS_IN_FILE 100

typedef struct oufile_s
{
  INODE_REFERENCE inode_reference;
  char mode;
  int offset;

  // Cache for file content details.  Use of these is optional
  int n_data_blocks;
  BLOCK_REFERENCE block_reference_cache[MAX_BLOCKS_IN_FILE];
} OUFILE;


#endif
