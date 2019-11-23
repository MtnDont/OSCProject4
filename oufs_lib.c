/**
 *  Project 3
 *  oufs_lib.c
 *
 *  Author: CS3113
 *
 */

#include "oufs_lib.h"
#include "oufs_lib_support.h"
#include "virtual_disk.h"

// Yes ... a global variable
int debug = 1;

// Translate inode types to descriptive strings
const char *INODE_TYPE_NAME[] = {"UNUSED", "DIRECTORY", "FILE"};

/**
 Read the OUFS_PWD, OUFS_DISK, OUFS_PIPE_NAME_BASE environment
 variables copy their values into cwd, disk_name an pipe_name_base.  If these
 environment variables are not set, then reasonable defaults are
 given.

 @param cwd String buffer in which to place the OUFS current working directory.
 @param disk_name String buffer in which to place file name of the virtual disk.
 @param pipe_name_base String buffer in which to place the base name of the
            named pipes for communication to the server.

 PROVIDED
 */
void oufs_get_environment(char *cwd, char *disk_name,
			  char *pipe_name_base)
{
  // Current working directory for the OUFS
  char *str = getenv("OUFS_PWD");
  if(str == NULL) {
    // Provide default
    strcpy(cwd, "/");
  }else{
    // Exists
    strncpy(cwd, str, MAX_PATH_LENGTH-1);
  }

  // Virtual disk location
  str = getenv("OUFS_DISK");
  if(str == NULL) {
    // Default
    strcpy(disk_name, "vdisk1");
  }else{
    // Exists: copy
    strncpy(disk_name, str, MAX_PATH_LENGTH-1);
  }

  // Pipe name base
  str = getenv("OUFS_PIPE_NAME_BASE");
  if(str == NULL) {
    // Default
    strcpy(pipe_name_base, "pipe");
  }else{
    // Exists: copy
    strncpy(pipe_name_base, str, MAX_PATH_LENGTH-1);
  }

}

/**
 * Completely format the virtual disk (including creation of the space).
 *
 * NOTE: this function attaches to the virtual disk at the beginning and
 *  detaches after the format is complete.
 *
 * - Zero out all blocks on the disk.
 * - Initialize the master block: mark inode 0 as allocated and initialize
 *    the linked list of free blocks
 * - Initialize root directory inode 
 * - Initialize the root directory in block ROOT_DIRECTORY_BLOCK
 *
 * @return 0 if no errors
 *         -x if an error has occurred.
 *
 */

int oufs_format_disk(char  *virtual_disk_name, char *pipe_name_base)
{
  // Attach to the virtual disk
  if(virtual_disk_attach(virtual_disk_name, pipe_name_base) != 0) {
    return(-1);
  }

  BLOCK block;

  // Zero out the block
  memset(&block, 0, BLOCK_SIZE);
  for(int i = 0; i < N_BLOCKS; ++i) {
    if(virtual_disk_write_block(i, &block) < 0) {
      return(-2);
    }
  }

  //////////////////////////////
  // Master block
  block.next_block = UNALLOCATED_BLOCK;
  block.content.master.inode_allocated_flag[0] = 0x80;

  block.content.master.unallocated_front = 6;
  block.content.master.unallocated_end = N_BLOCKS - 1;

  virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &block);
  memset(&block, 0, BLOCK_SIZE);

  /*fprintf(stderr, "INSPECT 1 START: ---------------");
  system("./oufs_inspect -data 1");
  system("./oufs_inspect -inode 0");
  system("./oufs_inspect -inode 1");*/

  //////////////////////////////
  // Root directory inode / block
  INODE inode;
  oufs_init_directory_structures(&inode, &block, ROOT_DIRECTORY_BLOCK,
				 ROOT_DIRECTORY_INODE, ROOT_DIRECTORY_INODE);

  // Write the results to the disk
  if(oufs_write_inode_by_reference(0, &inode) != 0) {
    return(-3);
  }

  virtual_disk_write_block(ROOT_DIRECTORY_BLOCK, &block);
  //////////////////////////////
  // All other blocks are free blocks

  memset(&block, 0, BLOCK_SIZE);

  for (int i = 6; i < N_BLOCKS; i++) {
    memset(&block, 0, BLOCK_SIZE);
    if (i == N_BLOCKS - 1) {
      block.next_block = UNALLOCATED_BLOCK;
    }
    else {
      block.next_block = i + 1;
    }
    virtual_disk_write_block(i, &block);
  }

  for (int i = 1; i < N_INODES; i++) {
    memset(&inode, 0, sizeof(INODE));
    inode.content = UNALLOCATED_INODE;
    oufs_write_inode_by_reference(i, &inode);
  }
  
  // Done
  virtual_disk_detach();
 
  return(0);
}

/*
 * Compare two inodes for sorting, handling the
 *  cases where the inodes are not valid
 *
 * @param e1 Pointer to a directory entry
 * @param e2 Pointer to a directory entry
 * @return -1 if e1 comes before e2 (or if e1 is the only valid one)
 * @return  0 if equal (or if both are invalid)
 * @return  1 if e1 comes after e2 (or if e2 is the only valid one)
 *
 * Note: this function is useful for qsort()
 */
static int inode_compare_to(const void *d1, const void *d2)
{
  // Type casting from generic to DIRECTORY_ENTRY*
  DIRECTORY_ENTRY* e1 = (DIRECTORY_ENTRY*) d1;
  DIRECTORY_ENTRY* e2 = (DIRECTORY_ENTRY*) d2;
  
  if (e1->inode_reference == UNALLOCATED_INODE && e2->inode_reference != UNALLOCATED_INODE) {
    return (1);
  }
  else if (e1->inode_reference != UNALLOCATED_INODE && e2->inode_reference == UNALLOCATED_INODE) {
    return (-1);
  }
  else if (e1->inode_reference == UNALLOCATED_INODE && e2->inode_reference == UNALLOCATED_INODE) {
    return (0);
  }
  else {
    return strcmp(e1->name, e2->name);
  }
}


/**
 * Print out the specified file (if it exists) or the contents of the 
 *   specified directory (if it exists)
 *
 * If a directory is listed, then the valid contents are printed in sorted order
 *   (as defined by strcmp()), one per line.  We know that a directory entry is
 *   valid if the inode_reference is not UNALLOCATED_INODE.
 *   Hint: qsort() will do to sort for you.  You just have to provide a compareTo()
 *   function (just like in Java!)
 *   Note: if an entry is a directory itself, then its name must be followed by "/"
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 */

int oufs_list(char *cwd, char *path)
{
  INODE_REFERENCE parent;
  INODE_REFERENCE child;

  // Look up the inodes for the parent and child
  int ret = oufs_find_file(cwd, path, &parent, &child, NULL);

  // Did we find the specified file?
  if(ret == 0 && child != UNALLOCATED_INODE) {
    // Element found: read the inode
    INODE inode;
    if(oufs_read_inode_by_reference(child, &inode) != 0) {
      return(-1);
    }
    if(debug) {
      fprintf(stderr, "\tDEBUG: Child found (type=%s).\n",  INODE_TYPE_NAME[inode.type]);
    }


    BLOCK b;
    int count = 0;
    //char* items[N_DIRECTORY_ENTRIES_PER_BLOCK] = { 0 };
    DIRECTORY_ENTRY items[N_DIRECTORY_ENTRIES_PER_BLOCK];
    virtual_disk_read_block(inode.content, &b);
    for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++) {
      if (b.content.directory.entry[i].inode_reference != UNALLOCATED_INODE) {
        //oufs_read_inode_by_reference(b.content.directory.entry[i].inode_reference, &inode);
        //strcpy(items[count], b.content.directory.entry[i].name);
        items[count] = b.content.directory.entry[i];
        /*if (inode.type == DIRECTORY_TYPE) {
          strcat(items[count], "/");
          //printf("%s/\n", b.content.directory.entry[i].name);
        }*/
        /*else if (inode.type == FILE_TYPE) {
          printf("%s\n", b.content.directory.entry[i].name);
        }*/

        count++;
      }
    }

    //fprintf(stderr, "AFTER: \n");
    qsort(items, count, sizeof(DIRECTORY_ENTRY), inode_compare_to);
    for (int i = 0; i < count; i++) {
      oufs_read_inode_by_reference(items[i].inode_reference, &inode);
      if (inode.type == DIRECTORY_TYPE) {
        printf("%s/\n", items[i].name);
      }
      else {
        printf("%s\n", items[i].name);
      }
    }
  } else {
    // Did not find the specified file/directory
    fprintf(stderr, "Not found\n");
    if(debug)
      fprintf(stderr, "\tDEBUG: (%d)\n", ret);
  }
  // Done: return the status from the search
  return(ret);
}




///////////////////////////////////
/**
 * Make a new directory
 *
 * To be successful:
 *  - the parent must exist and be a directory
 *  - the parent must have space for the new directory
 *  - the child must not exist
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Absolute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 */
int oufs_mkdir(char *cwd, char *path)
{
  INODE_REFERENCE parent;
  INODE_REFERENCE child;

  // Name of a directory within another directory
  char local_name[MAX_PATH_LENGTH];
  int ret;

  // Attempt to find the specified directory
  if((ret = oufs_find_file(cwd, path, &parent, &child, local_name)) < -1) {
    if(debug)
      fprintf(stderr, "oufs_mkdir(): ret = %d\n", ret);
    return(-1);
  };


  BLOCK b;
  INODE parentInode;
  INODE inode;
  child = oufs_allocate_new_directory(parent);

  oufs_read_inode_by_reference(child, &inode);
  oufs_read_inode_by_reference(parent, &parentInode);

  virtual_disk_read_block(parentInode.content, &b);

  for (int i = 2; i < N_DIRECTORY_ENTRIES_PER_BLOCK - 1; i++) {
    if (b.content.directory.entry[i].inode_reference == UNALLOCATED_INODE) {
      b.content.directory.entry[i].inode_reference = child;
      strcpy(b.content.directory.entry[i].name, local_name);
      break;
    }
  }
  virtual_disk_write_block(parentInode.content, &b);

  return (0);
}

/**
 * Remove a directory
 *
 * To be successul:
 *  - The directory must exist and must be empty
 *  - The directory must not be . or ..
 *  - The directory must not be /
 *
 * @param cwd Absolute path representing the current working directory
 * @param path Abslute or relative path to the file/directory
 * @return 0 if success
 *         -x if error
 *
 */
int oufs_rmdir(char *cwd, char *path)
{
  INODE_REFERENCE parent;
  INODE_REFERENCE child;
  char local_name[MAX_PATH_LENGTH];

  // Try to find the inode of the child
  if(oufs_find_file(cwd, path, &parent, &child, local_name) < -1) {
    return(-4);
  }

  if (local_name == NULL) {
    return (-1);
  }

  //Block and inode setup
  BLOCK master;
  BLOCK pb;
  BLOCK block;
  INODE c;
  INODE p;

  //Read in appropriate values
  virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &master);
  oufs_read_inode_by_reference(child, &c);
  oufs_read_inode_by_reference(parent, &p);
  virtual_disk_read_block(p.content, &pb);
  virtual_disk_read_block(c.content, &block);

  //Error checking
  if (c.type != DIRECTORY_TYPE) {
    fprintf(stderr, "Cannot remove: TYPE ERROR\n");
    return (-1);
  }

  if (p.size <= 2 || c.size > 2) {
    fprintf(stderr, "Cannot remove: SIZE ERROR\n");
    return (-1);
  }

  if (child == 0 || child == UNALLOCATED_INODE) {
    fprintf(stderr, "Cannot remove: INODE ERROR\n");
    return (-1);
  }

  //Modify parent inode
  p.size = p.size - 1;

  //Modify parent directory block
  for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++) {
    if (pb.content.directory.entry[i].inode_reference == child) {
      DIRECTORY_ENTRY dirEn;
      memset(&dirEn, 0, sizeof(DIRECTORY_ENTRY));
      dirEn.inode_reference = UNALLOCATED_INODE;
      pb.content.directory.entry[i] = dirEn;
      break;
    }
  }
  
  //Modify master inode flag table
  master.content.master.inode_allocated_flag[child/8] -= (1 << (7 - child%8));

  //Make c a blank inode
  oufs_deallocate_block(&master, c.content);
  memset(&c, 0, sizeof(INODE));
  c.content = UNALLOCATED_BLOCK;

  //Write the content back
  oufs_write_inode_by_reference(child, &c);
  virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &master);
  virtual_disk_write_block(p.content, &pb);
  oufs_write_inode_by_reference(parent, &p);
  // Success
  return(0);
}

/**
 * Open a file
 * - mode = "r": the file must exist; offset is set to 0
 * - mode = "w": the file may or may not exist;
 *                 - if it does not exist, it is created 
 *                 - if it does exist, then the file is truncated
 *                       (size=0 and data blocks deallocated);
 *                 offset = 0 and size = 0
 * - mode = "a": the file may or may not exist
 *                 - if it does not exist, it is created 
 *                 offset = size
 *
 * @param cwd Absolute path for the current working directory
 * @param path Relative or absolute path for the file in question
 * @param mode String: one of "r", "w" or "a"
 *                 (note: only the first character matters here)
 * @return Pointer to a new OUFILE structure if success
 *         NULL if error
 */
OUFILE* oufs_fopen(char *cwd, char *path, char *mode)
{
  INODE_REFERENCE parent;
  INODE_REFERENCE child;
  char local_name[MAX_PATH_LENGTH];
  INODE inode;
  int ret;

  // Check for valid mode
  if(mode[0] != 'r' && mode[0] != 'w' && mode[0] != 'a') {
    fprintf(stderr, "fopen(): bad mode.\n");
    return(NULL);
  };

  // Try to find the inode of the child
  if((ret = oufs_find_file(cwd, path, &parent, &child, local_name)) < -1) {
    if(debug)
      fprintf(stderr, "oufs_fopen(%d)\n", ret);
    return(NULL);
  }
  
  if(parent == UNALLOCATED_INODE) {
    fprintf(stderr, "Parent directory not found.\n");
    return(NULL);
  }

  // TODO
  OUFILE* fp = (OUFILE*)malloc(sizeof(OUFILE));
  if (mode[0] == 'a') {
    if (child == UNALLOCATED_INODE) {
      child = oufs_create_file(parent, local_name);
      if (child == UNALLOCATED_INODE)
        return NULL;
      /*inode.type = FILE_TYPE;
      inode.n_references = 1;
      inode.size = 0;
      inode.content = UNALLOCATED_BLOCK;*/
      fp->inode_reference = child;
      fp->mode = "a";
      fp->offset = 0;
      fp->n_data_blocks = 0;
    }
    else {
      oufs_read_inode_by_reference(child, &inode);
      fp->inode_reference = child;
      fp->mode = 'a';
      fp->offset = inode.size;
      fp->n_data_blocks = (fp->offset + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;
      
      BLOCK b;
      virtual_disk_read_block(inode.content, &b);
      fp->block_reference_cache[0] = inode.content;
      for (int i = 1; i < fp->n_data_blocks; i++) {
        fp->block_reference_cache[i] = b.next_block;
        if (b.next_block != UNALLOCATED_BLOCK)
          virtual_disk_read_block(b.next_block, &b);
      }

    }
  }
  if (mode[0] == 'r') {
    //Child must exist
    if (child == UNALLOCATED_INODE) {
      return NULL;
    }
    else {
      oufs_read_inode_by_reference(child, &inode);
      if (inode.type != FILE_TYPE)
        return NULL;
      fp->inode_reference = child;
      fp->mode = 'r';
      fp->offset = 0;
      fp->n_data_blocks = (inode.size + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;

      BLOCK b;
      virtual_disk_read_block(inode.content, &b);
      fp->block_reference_cache[0] = inode.content;
      for (int i = 1; i < fp->n_data_blocks; i++) {
        fp->block_reference_cache[i] = b.next_block;
        if (b.next_block != UNALLOCATED_BLOCK)
          virtual_disk_read_block(b.next_block, &b);
      }
    }
  }
  if (mode[0] == 'w') {
    if (child == UNALLOCATED_INODE) {
      child = oufs_create_file(parent, local_name);
      if (child == UNALLOCATED_INODE)
        return NULL;
      /*inode.type = FILE_TYPE;
      inode.n_references = 1;
      inode.size = 0;
      inode.content = UNALLOCATED_BLOCK;*/
      fp->inode_reference = child;
      fp->mode = 'w';
      fp->offset = 0;
      fp->n_data_blocks = 0;
    }
    else {
      oufs_read_inode_by_reference(child, &inode);
      BLOCK b;
      memset(&b, 0, BLOCK_SIZE);
      int current_blocks = (inode.size + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;
      for (int i = 0; i < current_blocks; i++) {
        virtual_disk_write_block(inode.content + i, &b);
      }

      fp->inode_reference = child;
      fp->mode = 'w';
      fp->offset = 0;
      fp->n_data_blocks = 0;
    }
  }
  /*fp->inode_reference = child;
  fp->mode = mode[0];
  fp->offset;*/

  /*
  current_blocks = (fp->offset + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE
  */
  return(fp);
};

/**
 *  Close a file
 *   Deallocates the OUFILE structure
 *
 * @param fp Pointer to the OUFILE structure
 */
     
void oufs_fclose(OUFILE *fp) {
  fp->inode_reference = UNALLOCATED_INODE;
  free(fp);
}



/*
 * Write bytes to an open file.
 * - Allocate new data blocks, as necessary
 * - Can allocate up to MAX_BLOCKS_IN_FILE, at which point, no more bytes may be written
 * - file offset will always match file size; both will be updated as bytes are written
 *
 * @param fp OUFILE pointer (must be opened for w or a)
 * @param buf Character buffer of bytes to write
 * @param len Number of bytes to write
 * @return The number of written bytes
 *          0 if file is full and no more bytes can be written
 *         -x if an error
 * 
 */
int oufs_fwrite(OUFILE *fp, unsigned char * buf, int len)
{
  if(fp->mode == 'r') {
    fprintf(stderr, "Can't write to read-only file");
    return(0);
  }
  if(debug)
    fprintf(stderr, "-------\noufs_fwrite(%d)\n", len);
    
  INODE inode;
  BLOCK block;
  if(oufs_read_inode_by_reference(fp->inode_reference, &inode) != 0) {
    return(-1);
  }

  // Compute the index for the last block in the file +
  // the first free byte within the block
  
  //int current_block = (fp->offset + DATA_BLOCK_SIZE - 1) / DATA_BLOCK_SIZE;
  int current_block = fp->offset / DATA_BLOCK_SIZE;
  int used_bytes_in_last_block = fp->offset % DATA_BLOCK_SIZE;
  int free_bytes_in_last_block = DATA_BLOCK_SIZE - used_bytes_in_last_block;
  int len_written = 0;
  int len_left = len;

  // TODO
  //memset(buf, 0, len);
  if (inode.type != FILE_TYPE) {
    fprintf(stderr, "Cannot write to directories\n");
    return(-1);
  }

  if (inode.size == DATA_BLOCK_SIZE*MAX_BLOCKS_IN_FILE)
    return 0;

  BLOCK master;
  BLOCK bufB;
  BLOCK_REFERENCE br;
  int count = 0;
  virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &master);
  
  if (inode.content == UNALLOCATED_BLOCK) {
    br = oufs_allocate_new_block(&master, &block);
    inode.content = br;
    fp->block_reference_cache[0] = br;
    fp->n_data_blocks++;
    block.next_block = UNALLOCATED_BLOCK;
    virtual_disk_write_block(br, &block);
  }
  else if (used_bytes_in_last_block == 0) {
    br = oufs_allocate_new_block(&master, &block);
    virtual_disk_read_block(fp->block_reference_cache[current_block-1], &bufB);
    bufB.next_block = br;
    fp->block_reference_cache[current_block] = br;
    fp->n_data_blocks++;
    block.next_block = UNALLOCATED_BLOCK;
    virtual_disk_write_block(fp->block_reference_cache[current_block-1], &bufB);
    virtual_disk_write_block(br, &block);
  }
  else
    br = fp->block_reference_cache[current_block];

  for (int i = current_block; i < MAX_BLOCKS_IN_FILE; i++) {
    virtual_disk_read_block(br, &block);
    fprintf(stderr, "Block: %d\n", br);
    used_bytes_in_last_block = fp->offset % DATA_BLOCK_SIZE;
    free_bytes_in_last_block = DATA_BLOCK_SIZE - used_bytes_in_last_block;

    fprintf(stderr, "ERR CHECK: --------\ncurrent_block: %d\nlen_left: %d\noffset: %d\nsize: %d\n br: %d\n", current_block, len_left, fp->offset, inode.size, br);

    for (int i = 0; i < MAX_BLOCKS_IN_FILE; i++) {
      fprintf(stderr, "i: %d\n", fp->block_reference_cache[i]);
      if (fp->block_reference_cache[i] == 0)
        break;
    }

    if (len_left > free_bytes_in_last_block) {
      memcpy(block.content.data.data + used_bytes_in_last_block, buf + len_written, free_bytes_in_last_block);
      len_left -= free_bytes_in_last_block;
      len_written += free_bytes_in_last_block;
      fp->offset += free_bytes_in_last_block;
      inode.size += free_bytes_in_last_block;
      virtual_disk_write_block(br, &block);
    }
    else {
      memcpy(block.content.data.data + used_bytes_in_last_block, buf + len_written, len_left);
      len_written += len_left;
      fp->offset += len_left;
      inode.size += len_left;
      len_left = 0;
      virtual_disk_write_block(br, &block);
      fprintf(stderr, "ERR 2 CHECK: --------\nlen_left %d\noffset: %d\nsize: %d\n br: %d\n", len_left, fp->offset, inode.size, br);
      break;
    }

    if (i != MAX_BLOCKS_IN_FILE - 1) {
      br = oufs_allocate_new_block(&master, &bufB);
      fp->block_reference_cache[i+1] = br;
      fp->n_data_blocks++;
      block.next_block = br;
      virtual_disk_write_block(fp->block_reference_cache[i], &block);
      virtual_disk_write_block(br, &bufB);
    }
  }

  virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &master);
  oufs_write_inode_by_reference(fp->inode_reference, &inode);

  // Done
  return(len_written);
}


/*
 * Read a sequence of bytes from an open file.
 * - offset is the current position within the file, and will never be larger than size
 * - offset will be updated with each read operation
 *
 * @param fp OUFILE pointer (must be opened for r)
 * @param buf Character buffer to place the bytes into
 * @param len Number of bytes to read at max
 * @return The number of bytes read
 *         0 if offset is at size
 *         -x if an error
 * 
 */

int oufs_fread(OUFILE *fp, unsigned char * buf, int len)
{
  // Check open mode
  if(fp->mode != 'r') {
    fprintf(stderr, "Can't read from a write-only file");
    return(0);
  }
  if(debug)
    fprintf(stderr, "\n-------\noufs_fread(%d)\n", len);
    
  INODE inode;
  BLOCK block;
  if(oufs_read_inode_by_reference(fp->inode_reference, &inode) != 0) {
    return(-1);
  }
      
  // Compute the current block and offset within the block
  int current_block = fp->offset / DATA_BLOCK_SIZE;
  int byte_offset_in_block = fp->offset % DATA_BLOCK_SIZE;
  int len_read = 0;
  int end_of_file = inode.size;
  len = MIN(len, end_of_file - fp->offset);
  int len_left = len;

  fprintf(stderr, "Data: %d\n", current_block);

  // TODO
  memset(buf, 0, strlen(buf));
  //If there is no more data
  if (inode.type != FILE_TYPE)
    return -1;
  if (fp->offset == inode.size)
    return 0;
  
  BLOCK b;
  int count = 0;

  for (int i = current_block; i < fp->n_data_blocks; i++) {
    virtual_disk_read_block(fp->block_reference_cache[i], &b);
    if (len_left / (DATA_BLOCK_SIZE - byte_offset_in_block) >= 1) {
      memcpy(buf + len_read, b.content.data.data + byte_offset_in_block, DATA_BLOCK_SIZE - byte_offset_in_block);
      len_read += DATA_BLOCK_SIZE - byte_offset_in_block;
      fp->offset += (DATA_BLOCK_SIZE - byte_offset_in_block);
      len_left -= DATA_BLOCK_SIZE - byte_offset_in_block;
    }
    else {
      memcpy(buf + len_read, b.content.data.data + byte_offset_in_block, len_left);
      len_read += len_left;
      fp->offset += len_left;
      break;
    }

    byte_offset_in_block = (fp->offset % DATA_BLOCK_SIZE);

  }
  // Done
  return(len_read);
}


/**
 * Remove a file
 *
 * Full implementation:
 * - Remove the directory entry
 * - Decrement inode.n_references
 * - If n_references == 0, then deallocate the contents and the inode
 *
 * @param cwd Absolute path for the current working directory
 * @param path Absolute or relative path of the file to be removed
 * @return 0 if success
 *         -x if error
 *
 */

int oufs_remove(char *cwd, char *path)
{
  INODE_REFERENCE parent;
  INODE_REFERENCE child;
  char local_name[MAX_PATH_LENGTH];
  INODE inode;
  INODE inode_parent;
  BLOCK block;

  // Try to find the inode of the child
  if(oufs_find_file(cwd, path, &parent, &child, local_name) < -1) {
    return(-3);
  };
  
  if(child == UNALLOCATED_INODE) {
    fprintf(stderr, "File not found\n");
    return(-1);
  }
  // Get the inode
  if(oufs_read_inode_by_reference(child, &inode) != 0) {
    return(-4);
  }

  // Is it a file?
  if(inode.type != FILE_TYPE) {
    // Not a file
    fprintf(stderr, "Not a file\n");
    return(-2);
  }

  // TODO
  oufs_read_inode_by_reference(parent, &inode_parent);
  virtual_disk_read_block(inode_parent.content, &block);

  for (int i = 0; i < N_DIRECTORY_ENTRIES_PER_BLOCK; i++) {
    if (block.content.directory.entry[i].inode_reference != UNALLOCATED_INODE) {
      if (block.content.directory.entry[i].inode_reference == child) {
        DIRECTORY_ENTRY dirEn;
        memset(&dirEn, 0, sizeof(DIRECTORY_ENTRY));
        dirEn.inode_reference = UNALLOCATED_INODE;
        block.content.directory.entry[i] = dirEn;
        break;
      }
    }
  }
  inode_parent.size--;
  virtual_disk_write_block(inode_parent.content, &block);
  oufs_write_inode_by_reference(parent, &inode_parent);

  inode.n_references--;

  if (inode.n_references == 0) {
    //Modify master inode flag table
    BLOCK master;
    virtual_disk_read_block(MASTER_BLOCK_REFERENCE, &master);
    master.content.master.inode_allocated_flag[child/8] -= (1 << (7 - child%8));
    oufs_deallocate_blocks(&inode);
    memset(&inode, 0, sizeof(INODE));
    inode.content = UNALLOCATED_BLOCK;
    oufs_write_inode_by_reference(child, &inode);
    virtual_disk_write_block(MASTER_BLOCK_REFERENCE, &master);
  }
  
  // Success
  return(0);
};


/**
 * Create a hard link to a specified file
 *
 * Full implemenation:
 * - Add the new directory entry
 * - Increment inode.n_references
 *
 * @param cwd Absolute path for the current working directory
 * @param path_src Absolute or relative path of the existing file to be linked
 * @param path_dst Absolute or relative path of the new file inode to be linked
 * @return 0 if success
 *         -x if error
 * 
 */
int oufs_link(char *cwd, char *path_src, char *path_dst)
{
  INODE_REFERENCE parent_src;
  INODE_REFERENCE child_src;
  INODE_REFERENCE parent_dst;
  INODE_REFERENCE child_dst;
  char local_name[MAX_PATH_LENGTH];
  char local_name_bogus[MAX_PATH_LENGTH];
  INODE inode_src;
  INODE inode_dst;
  BLOCK block;

  // Try to find the inodes
  if(oufs_find_file(cwd, path_src, &parent_src, &child_src, local_name_bogus) < -1) {
    return(-5);
  }
  if(oufs_find_file(cwd, path_dst, &parent_dst, &child_dst, local_name) < -1) {
    return(-6);
  }

  // SRC must exist
  if(child_src == UNALLOCATED_INODE) {
    fprintf(stderr, "Source not found\n");
    return(-1);
  }

  // DST must not exist, but its parent must exist
  if(parent_dst == UNALLOCATED_INODE) {
    fprintf(stderr, "Destination parent does not exist.\n");
    return(-2);
  }
  if(child_dst != UNALLOCATED_INODE) {
    fprintf(stderr, "Destination already exists.\n");
    return(-3);
  }

  // Get the inode of the dst parent
  if(oufs_read_inode_by_reference(parent_dst, &inode_dst) != 0) {
    return(-7);
  }

  if(inode_dst.type != DIRECTORY_TYPE) {
    fprintf(stderr, "Destination parent must be a directory.");
  }
  // There must be space in the directory
  if(inode_dst.size == N_DIRECTORY_ENTRIES_PER_BLOCK) {
    fprintf(stderr, "No space in destination parent.\n");
    return(-4);
  }


  // TODO

  return(0);
}
