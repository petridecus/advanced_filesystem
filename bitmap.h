// based on cs3650 starter code

#ifndef BITMAP_H
#define BITMAP_H

#include <stdlib.h>

/**
 * Returns the 'ii' bit of the given bitmap.
 */
int bitmap_get(void* bm, int ii);

/**
 * Sets the 'ii' bit of the given bitmap to int vv.
 */
void bitmap_set(void* bm, int ii, int vv);

/**
 * Prints the current contents of the bitmap.
 * Useful for debugging.
 */
void bitmap_print(void* bm, int size);

#endif
