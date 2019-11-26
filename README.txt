Camron Bartlow Project 4

This program implements a file system onto a file with a series of
interconnecting blocks and inodes.

oufs_append {filename}
    Appends data to {filename} and creates it if it does not exist.

oufs_cat {filename}
    Prints data held within a file.

oufs_copy {filename source} {destination}
    Copies a file from one directory to another

oufs_create {filename}
    Writes data to a file, and clears its data if the file exists.

oufs_format
    Formats the disk.

oufs_inspect
    Inspects various parts of data within the disk. Execute the program
    for more information

oufs_link {source} {destination}
    Links one file in one directory to one in another directory,
    pointing to the same data.

oufs_ls {directory}
    Lists the directories in the a listed directory or the current working
    directory if one is not listed.

oufs_mkdir {directory name}
    Adds a directory to the disk

oufs_remove {filename}
    Removes a file and its data.

oufs_rmdir {directory name}
    Removes a directory from the disk

oufs_stats
    Reveals statistical data about the global variables used in the program

oufs_touch {filename}
    Adds a file named {filename}.