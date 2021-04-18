#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <libgen.h>
#include <errno.h>
#include "directory.h"

#define MAX_ENTRIES 254 // (4096 - sizeof(int)) / sizeof(dirent)

// NOTE view directory.h for documentation on direntry functions/struct

void
directory_init() {
    // NOTE something is going wrong when making 10+ files after chaning page structure
    // was only having issues at 30+ before change...

    int existed = bitmap_get(get_pages_bitmap(), 3);
    if (existed) return;

    assert(alloc_page() == 3 && alloc_inode() == 0);

    inode* rnode = get_inode(0);

    // TODO debug this
    // rnode->refs = 1;
    // rnode->size = 4096;
    // rnode->pages = 1;
    rnode->mode = 040755;
    rnode->ptrs[0] = 3;
}

char*
get_dir(const char* path) {
    char* path_tmp = strdup(path);
    char* dir_tmp = dirname(path_tmp);
    char* dir = strdup(dir_tmp);
    free(path_tmp);
    return dir;
}

int 
directory_lookup(inode* dd, const char *name) {
    if (strcmp(name, "/") == 0) return 0;

    int num_entries = dd->size / sizeof(direntry); // hard coded to root
    direntry* entry = (direntry*)(pages_get_page(dd->ptrs[0]));

    // iterate thru all entries in the directory
    for (int ii = 0; ii < num_entries; ++ii) {
        if (!strcmp(entry->name, name)) { 
	    return entry->inum;
	}
        ++entry;
    }

    return -1;
}

int
directory_put(inode* dd, const char* name, int inum) {
    int num_entries = dd->size / sizeof(direntry);
	
    printf("currently %d entries in dir\n", num_entries);

    if (dd->ptrs[0] == 0) dd->ptrs[0] = alloc_page();
    void* dir_page = pages_get_page(dd->ptrs[0]);

    if (num_entries >= MAX_ENTRIES) {
        printf("num entries is garbage: %d\n", num_entries);
        return -1;
    }

    printf("getting to point where entry %d is being created\n", num_entries);

    direntry* new_entry = (direntry*)(dir_page + (num_entries)*sizeof(direntry));
    strcpy(new_entry->name, name);
    new_entry->inum = inum;
    dd->size += sizeof(direntry);
    dd->refs = dd->refs + 1;

    return 0;
}

int
rename_entry(const char* from, const char* to) {
    void* dir_page = pages_get_page(3); // hard coded to root
    inode* root = get_inode(0);
    int num_entries = root->size / sizeof(direntry);

    for (int ii = 0; ii < num_entries; ++ii) {
        direntry* curr_entry = (direntry*)(dir_page + ii * sizeof(direntry));
    	if (!strcmp(curr_entry->name, from)) {
	    strcpy(curr_entry->name, to);
            return 0;
        }
    }

    return -ENOENT;
}

int
directory_delete(inode* dd, const char* path) {
    int inum = dd->ptrs[0];
    void* dir_page = pages_get_page(inum);
    int num_entries = dd->size / sizeof(direntry);

    for (int ii = 0; ii < num_entries; ++ii) {
        direntry* curr_entry = (direntry*)(dir_page + ii * sizeof(direntry));
    	if (strcmp(curr_entry->name, path) == 0) {
	    inode* deleted = get_inode(curr_entry->inum);
	    deleted->refs -= 1;
	    if (deleted->refs == 0) {
                free_inode(curr_entry->inum);
	    }
            
	    for (int jj = ii; jj < num_entries; ++jj) {
		direntry* curr_entry = (direntry*)(dir_page + jj * sizeof(direntry));
		direntry* next_entry = curr_entry + 1;

		memset(curr_entry->name, 0, strlen(curr_entry->name) + 1);
                strcpy(curr_entry->name, next_entry->name);
		        curr_entry->inum = next_entry->inum;
            }

            dd->size -= sizeof(direntry);
            return 0;
        }
    }

    return -ENOENT;
}
