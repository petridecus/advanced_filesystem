// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include <time.h>

#include "pages.h"

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes
    int ptrs[10]; // direct pointers
    int iptrs; 
    int ts[2];
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
 * Grows size of node nn to given size and allocates pages accordingly.
 */
int inode_grow(inode* nn, int size);

/**
 * Shrinks size of node nn to given size and deallocates pages accordingly.
 */
int inode_shrink(inode* nn, int size);

/**
 * Frees the inode with the given inum.
 * Does so by clearing the field in the bitmap,
 * and freeing the block that ptr is pointing to.
 * NOTE - added the inum field, it wasn't there in the starter code.
 */
void free_inode(int inum); // NOTE i added the inum

#endif
