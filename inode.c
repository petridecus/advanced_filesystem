#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "inode.h"
#include "pages.h"
#include "bitmap.h"
#include "util.h"

#define MAX_INODES 128 // 2 pages of 64 byte structs
#define DIRECT_PAGES 10

// NOTE view inode.h for documentation on inode functions/struct

void
print_inode(inode* node) {
    printf("inode of size %ld\n", node->size);
}

inode*
get_inode(int inum) {
    printf("trying to get inode %d\n", inum);
    if (inum > MAX_INODES) 
	    puts("there's not even supposed to be that many dude");
    
    return (inode*)(pages_get_page(1) + inum*sizeof(inode));
}

int
alloc_inode() {
    void* ibm = get_inode_bitmap();
    
    puts("inode bitmap:");
    bitmap_print(ibm, 128);
    puts("pages bitmap:");
    bitmap_print(get_pages_bitmap(), 128);

    for (int ii = 0; ii < MAX_INODES; ++ii) {
        if (bitmap_get(ibm, ii) == 0) {
            bitmap_set(ibm, ii, 1);
	        inode* nn = get_inode(ii);
            nn->size = 0;
    	    nn->refs = 1;
	        printf("+ alloc_inode -> %d\n", ii);
            return ii;
        }
    }

    printf("uh oh, there are no free inodes left!");
    return -1;
}

void
free_inode(int inum) { 
    void* ibm = get_inode_bitmap();
    assert(bitmap_get(ibm, inum));

    inode* nn = get_inode(inum);

    memset(nn, 0, sizeof(inode));

    int num_pages = bytes_to_pages(nn->size);

    for (int ii = 0; ii < num_pages; ++ii)
        if (ii < DIRECT_PAGES) free_page(nn->ptrs[ii]);
    
    bitmap_set(ibm, inum, 0);
    printf("- free_inode(%d)\n", inum);
}

int
inode_grow(inode* nn, int size) {
    assert(size > nn->size);

    int num_pages = bytes_to_pages(nn->size);
    int new_num_pages = bytes_to_pages(size);
    nn->size = size;

    if (num_pages == new_num_pages) return 0;
    
    for (int ii = num_pages; ii < new_num_pages; ++ii) {
        nn->ptrs[ii] = alloc_page();
        printf("allocated pate %d to inode's ptr %d\n", nn->ptrs[ii], ii);
    }

    printf("grew from %d to %d pages\n", num_pages, new_num_pages);

    return 0;
}

int
inode_shrink(inode* nn, int size) {
    assert(size < nn->size);

    int num_pages = bytes_to_pages(nn->size);
    int new_num_pages = bytes_to_pages(size);
    nn->size = size;

    if (num_pages == new_num_pages) return 0;
    
    for (int ii = num_pages; ii < new_num_pages; ++ii) {
        free_page(nn->ptrs[ii]);
    }

    return 0;
}

// NOTE - only one ptr per node for hw10
int
inode_get_pnum(inode* node, int fpn) {
    assert(fpn < DIRECT_PAGES);
        return node->ptrs[fpn];

    // need case for indirect pointers
    return -1;
}
