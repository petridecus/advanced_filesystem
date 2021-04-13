// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include <time.h>

#include "pages.h"

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes
    int ptrs[10]; // direct pointer, only using 1 for hw10
    int pages;
} inode;

/**
 * Prints out details about the given inode
 */
void print_inode(inode* node);

/**
 * Returns the inode curresponding to the given inum.
 */
inode* get_inode(int inum);

/**
 * Returns the next free inode.
 */
int alloc_inode();

/**
 * Frees the inode with the given inum.
 * Does so by clearing the field in the bitmap,
 * and freeing the block that ptr is pointing to.
 * NOTE - added the inum field, it wasn't there in the starter code.
 */
void free_inode(int inum); // NOTE i added the inum

/**
 * Sets the time field of the given inode
 * based on the given timespec.
 */
void inode_set_time(inode* nn, struct timespec tt);

/**
 * Currently ignores fpn and returns the singular
 * ptr of this inode. Will have to change for ch03.
 */
int inode_get_pnum(inode* node, int fpn);

#endif
