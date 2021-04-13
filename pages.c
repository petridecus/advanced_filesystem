// based on cs3650 starter code

#define _GNU_SOURCE
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>

#include "pages.h"
#include "util.h"
#include "bitmap.h"
#include "inode.h"
#include "directory.h"

const int PAGE_COUNT = 256;
const int NUFS_SIZE  = 4096 * 256; // 1MB

static int   pages_fd   = -1;
static void* pages_base =  0;

void
pages_init(const char* path)
{
    pages_fd = open(path, O_CREAT | O_RDWR, 0644);
    assert(pages_fd != -1);

    int rv = ftruncate(pages_fd, NUFS_SIZE);
    assert(rv == 0);

    pages_base = mmap(0, NUFS_SIZE, 
                      PROT_READ | PROT_WRITE, MAP_SHARED,
                      pages_fd, 0);
    
    assert(pages_base != MAP_FAILED);

    bitmap* pbm = get_pages_bitmap();

    // if the bitmaps are already initialized, don't reset them
    if (!bitmap_get(pbm, 0)) {
        pbm->num_words = 1;
        bitmap_set(pbm, 0); // bitmap page
        bitmap_set(pbm, 1); // inode page 1
        bitmap_set(pbm, 2); // inode page 2

        bitmap* ibm = get_inode_bitmap();
        ibm->num_words = 1;
    }

    // will also check bitmaps before mutating base/root pages
    directory_init(); 
}

void
pages_free()
{
    int rv = munmap(pages_base, NUFS_SIZE);
    assert(rv == 0);
}

void*
pages_get_page(int pnum)
{
    return pages_base + 4096 * pnum;
}

bitmap*
get_pages_bitmap()
{
    return (bitmap*)(pages_get_page(0) + 4 * sizeof(long)); // inode bitmap will never be > 128 bits
}

bitmap*
get_inode_bitmap()
{
    return (bitmap*)pages_get_page(0); // NOTE switched to make inode bitmap first, since it's smaller
}

int
alloc_page()
{
    bitmap* pbm = get_pages_bitmap();

    // page 0 is for bitmaps, pages 1 & 2 are for inodes
    for (int ii = 3; ii < PAGE_COUNT; ++ii) {
        if (!bitmap_get(pbm, ii)) {
            bitmap_set(pbm, ii);
            printf("+ alloc_page() -> %d\n", ii);
            return ii;
        }
    }

    return -1;
}

void
free_page(int pnum)
{
    printf("+ free_page(%d)\n", pnum);
    void* pbm = get_pages_bitmap();
    bitmap_clear(pbm, pnum);
}

