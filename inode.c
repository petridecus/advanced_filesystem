#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "inode.h"
#include "pages.h"
#include "bitmap.h"
#include "util.h"

#define MAX_INODES 64 // want it to fit in one page

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
    
    return (inode*)(pages_get_page(0) + 8*sizeof(long) + inum*sizeof(inode));
}

int
alloc_inode() {
    bitmap* ibm = get_inode_bitmap();
    for (int ii = 0; ii < MAX_INODES; ++ii) {
        if (!bitmap_get(ibm, ii)) {
            bitmap_set(ibm, ii);
	    inode* nn = get_inode(ii);
	    nn->ptr = alloc_page();
	    timespec_get(&nn->time, TIME_UTC);
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
    if (nn->ptr) free_page(nn->ptr);

    bitmap_clear(ibm, inum);
}

void
inode_set_time(inode*nn, struct timespec tt) {
    nn->time = tt;
}

// NOTE - only one ptr per node for hw10
int
inode_get_pnum(inode* node, int fpn) {
    assert(fpn == 0);
    return node->ptr;
}
