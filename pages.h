// based on cs3650 starter code

#ifndef PAGES_H
#define PAGES_H

#include <stdio.h>
#include "bitmap.h"

void pages_init(const char* path);
void pages_free();
void* pages_get_page(int pnum);
bitmap* get_pages_bitmap();
bitmap* get_inode_bitmap();
int alloc_page();
void free_page(int pnum);

#endif
