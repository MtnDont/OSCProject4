//Camron Bartlow Project 2

#include "storage.h"

/**
 * Initialize the storage file
 *
 * @param name Name of the storage file
 * @return NULL if there is an error;
 *         otherwise, a poiner to the initialized STORAGE object
 */

STORAGE * init_storage(char * name, char *pipe_name_base)
{
  // Open the file
  int fd = open(name, O_RDWR | O_CREAT,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

  // Is there an error?
  if(fd <= 0) {
    fprintf(stderr, "Unable to open %s\n", name);
    return NULL;
  }

  // Allocate the STORAGE object and populate it
  STORAGE *s = malloc(sizeof(STORAGE));
  s->fd = fd;

  // Success
  return s;
};


/**
 *  Close an open storage object
 *
 * @param storage Pointer to an initialized storage object
 * @return -1 on error; 0 on success
 *
 */
int close_storage(STORAGE *storage)
{
  // Close the storage file
  int ret = close(storage->fd);

  // Was there an error?
  if(ret < 0) {
    fprintf(stderr, "Unable to close storage.\n");
    return(-1);
  };

  // Closed: now free the allocated space
  free(storage);

  // Success
  return(0);
}

/**
 *  Read a set of bytes from the storage file.
 *
 * @param storage A pointer to an initialized storage object
 * @param buf The buffer to place the read bytes into
 * @param location The point in the file to start reading from
 * @param len The number of bytes to read
 * @return -1 if an error; 
 *         otherwise, the number of bytes read from the storage file
 */
int get_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{
  // Seek to the starting location
  int ret = lseek(storage->fd, location, SEEK_SET);

  // Was there an error?
  if(ret < 0) {
    fprintf(stderr, "Unable to seek\n");
    return(-1);
  };

  // Read the bytes
  if((ret = read(storage->fd, buf, len)) < 0){
    // There was a reading error
    fprintf(stderr, "Error reading fd\n");
    return(-1);
  };

  // Success: return the number of bytes read
  return(ret);
};

/**
 *  Write a set of bytes to the storage file
 *
 * @param storage A pointer to an initialized storage object
 * @param buf The buffer containing the bytes to be written
 * @param location The point in the file to start writing to
 * @param len The number of bytes to write
 * @return -1 if an error; 
 *         otherwise, the number of bytes written to the storage file
 */
int put_bytes(STORAGE *storage, unsigned char *buf, int location, int len)
{
  // Seek to the point in the file
  int ret = lseek(storage->fd, location, SEEK_SET);

  // Was there an error?
  if(ret < 0) {
    fprintf(stderr, "Unable to seek\n");
    return(-1);
  };

  // Write the bytes to the file
  if((ret = write(storage->fd, buf, len)) < 0){
    // There was an error
    fprintf(stderr, "Error reading fd\n");
    return(-1);
  };

  // Success: return the number of bytes written
  return(ret);
};

