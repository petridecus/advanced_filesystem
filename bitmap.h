// based on cs3650 starter code

#ifndef BITMAP_H
#define BITMAP_H

#include<stdlib.h>

// struct was not necessary for hw10
// done to prep for challenge
typedef struct bitmap {
    size_t num_bits;
    size_t num_words;
    long map[4];
} bitmap;

/**
 * Returns the 'ii' bit of the given bitmap.
 */
int bitmap_get(bitmap* bm, int ii);

/**
 * Sets the 'ii' bit of the given bitmap.
 */
void bitmap_set(bitmap* bm, int ii);

/**
 * Clears the 'ii' bit of the given bitmap.
 */
void bitmap_clear(bitmap* bm, int ii);

/**
 * Prints the current contents of the bitmap.
 * Useful for debugging.
 */
void bitmap_print(bitmap* bm);

#endif
