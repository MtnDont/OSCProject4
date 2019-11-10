#ifndef OUFS_LIB_SUPPORT_H
#define OUFS_LIB_SUPPORT_H

#include "oufs_lib.h"

// Implement these for project 3
int oufs_read_inode_by_reference(INODE_REFERENCE i, INODE *inode);
int oufs_write_inode_by_reference(INODE_REFERENCE i, INODE *inode);
void oufs_set_inode(INODE *inode, INODE_TYPE type, int n_references,
		    BLOCK_REFERENCE content, int size);
void oufs_init_directory_structures(INODE *inode, BLOCK *block,
				    BLOCK_REFERENCE self_block_reference,
				    INODE_REFERENCE self_inode_reference,
				    INODE_REFERENCE parent_inode_reference);

int oufs_find_file(char *cwd, char * path, INODE_REFERENCE *parent,
		   INODE_REFERENCE *child, char *local_name);
 
int oufs_deallocate_block(BLOCK *master_block, BLOCK_REFERENCE block_reference);

int oufs_allocate_new_directory(INODE_REFERENCE parent_reference);
int oufs_find_open_bit(unsigned char value);


// Implement these for project 4
INODE_REFERENCE oufs_create_file(INODE_REFERENCE parent, char *local_name);
int oufs_deallocate_blocks(INODE *inode);
BLOCK_REFERENCE oufs_allocate_new_block(BLOCK *master_block, BLOCK *new_block);

#endif
