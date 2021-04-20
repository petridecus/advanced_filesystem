#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "bitmap.h"
#include "assert.h"

#define WORD_SIZE 64

// NOTE read bitmap.h file for documentation on functions/struct

void
bitmap_print(void* bm, int size) {
    for (int ii = 0; ii < size; ++ii) {
        printf("bitmap %d: %d\n ", ii, bitmap_get(bm, ii));
    }
}

int
bitmap_get(void* bm, int ii) {
    char* slot = (char*)bm + ii;
    char cc = *(slot);
    int val = (int) cc;
    return val;
}

void
bitmap_set(void* bm, int ii, int vv) {
    char* slot = (char*)(bm+ii);
    char cc = (char) vv;
    memcpy(slot, &cc, 1);
    printf("bitmap_set() slot %d, val %d\n", ii, vv);

}
