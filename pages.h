// based on cs3650 starter code

#ifndef PAGES_H
#define PAGES_H

#include <stdio.h>
#include "bitmap.h"

/**
 * Initializes the pages for disk space, inodes and bitmaps.
 */
void pages_init(const char* path);

/**
 * Frees all the pages.
 */
void pages_free();

/**
 * gets the pnum'th page.
 */
void* pages_get_page(int pnum);

/**
 * Gets the pages bitmap.
 */
void* get_pages_bitmap();

/**
 * Gets the inode bitmap.
 */
void* get_inode_bitmap();

/**
 * Allocates a free page and returns the page number.
 */
int alloc_page();

/**
 * Free's the pnum'th page.
 */
void free_page(int pnum);

#endif
