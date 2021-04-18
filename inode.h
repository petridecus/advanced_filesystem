// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include <time.h>

#include "pages.h"

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    size_t size; // bytes
    int ptrs[10]; // direct pointer, only using 1 for hw10
    int iptrs[2]; // only one is necessary but 2 makes an even 64 bytes
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

int inode_grow(inode* nn, int size);

int inode_shrink(inode* nn, int size);

/**
 * Frees the inode with the given inum.
 * Does so by clearing the field in the bitmap,
 * and freeing the block that ptr is pointing to.
 * NOTE - added the inum field, it wasn't there in the starter code.
 */
void free_inode(int inum); // NOTE i added the inum

/**
 * Currently ignores fpn and returns the singular
 * ptr of this inode. Will have to change for ch03.
 */
int inode_get_pnum(inode* node, int fpn);

#endif
