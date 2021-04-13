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
    printf("inode of size %d\n", node->size);
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
    bitmap* ibm = get_inode_bitmap();
    for (int ii = 0; ii < MAX_INODES; ++ii) {
        if (!bitmap_get(ibm, ii)) {
            bitmap_set(ibm, ii);
	        inode* nn = get_inode(ii);
	        nn->ptrs[0] = alloc_page();
            return ii;
        }
    }
    printf("uh oh, there are no free inodes left!");
    return -1;
}

void
free_inode(int inum) { 
    bitmap* ibm = get_inode_bitmap();
    assert(bitmap_get(ibm, inum));

    inode* nn = get_inode(inum);

    for (int ii = 0; ii < nn->pages; ++ii)
        if (ii < DIRECT_PAGES) free_page(nn->ptrs[ii]);
    
    // need to eventually clear indirect pointer as well...
    
    bitmap_clear(ibm, inum);
}

// NOTE - only one ptr per node for hw10
int
inode_get_pnum(inode* node, int fpn) {
    assert(fpn < DIRECT_PAGES);
        return node->ptrs[fpn];

    // need case for indirect pointers
    return -1;
}
