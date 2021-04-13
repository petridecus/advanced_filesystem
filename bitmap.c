#include<stdio.h>
#include<stdlib.h>
#include <assert.h>

#include "bitmap.h"
#include "assert.h"

#define WORD_SIZE 64

// NOTE read bitmap.h file for documentation on functions/struct

void
bitmap_print(bitmap* bm) {
    for (size_t ii = 0; ii < bm->num_words; ++ii) {
        printf("word %ld: ", bm->map[ii]);
        for (size_t jj = 0; jj < sizeof(long); ++jj) {
            printf("%d", bitmap_get(bm, sizeof(long) * ii + jj));
        }
        printf("\n");
    }
}

int
bitmap_get(bitmap *bm, int ii) {
    size_t word_num = ii / WORD_SIZE;
    assert(word_num <= bm->num_words);
    
    long word = bm->map[word_num];
    int ind = ii % WORD_SIZE;

    return word & (1 << ind);
}

void
bitmap_set(bitmap *bm, int ii) {
    size_t word_num = ii / WORD_SIZE;
    assert(word_num <= bm->num_words);

    long word = bm->map[word_num];
    int ind = ii % WORD_SIZE;

    word |= (1 << ind);
    bm->map[word_num] = word;
}

void
bitmap_clear(bitmap *bm, int ii) {
    size_t word_num = ii / WORD_SIZE;
    assert(word_num <= bm->num_words);

    long word = bm->map[word_num];
    int ind = ii % WORD_SIZE;

    word &= ~(1 << ind);
    bm->map[word_num] = word;
}
