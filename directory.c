#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#include "directory.h"

#define MAX_ENTRIES 254 // (4096 - sizeof(int)) / sizeof(dirent)

// NOTE view directory.h for documentation on direntry functions/struct

void
directory_init() {
    int existed = bitmap_get(get_pages_bitmap(), 1);
    if (existed) return;

    assert(alloc_page() == 1);
    void* root = pages_get_page(1);

    int* num_entries = (int*)root;
    *num_entries = 0;

    assert(alloc_inode() == 0);
    inode* rnode = get_inode(0);

    rnode->refs = 1;
    rnode->mode = 040755;
    rnode->size = 4096;
    rnode->ptr = 1;
}

int 
directory_lookup(const char *name) {
    if (strcmp(name, "/") == 0) return 0;

    int* num_entries = (int*)(pages_get_page(1)); // hard coded to root
    direntry* entry = (direntry*)(pages_get_page(1) + sizeof(int));

    // iterate thru all entries in the directory
    for (int ii = 0; ii < *num_entries; ++ii) {
        if (!strcmp(entry->name, name)) return entry->inum;
        ++entry;
    }

    return -1;
}

int
directory_put(inode* dd, const char* name, int inum) {
    void* dir_page = pages_get_page(dd->ptr);

    int* num_entries = (int*)dir_page;
    
    if (*num_entries >= MAX_ENTRIES) {
        printf("num entries is garbage: %d\n", *num_entries);
        return -1;
    }

    puts("getting to point where entry is being created");
    direntry* new_entry = (direntry*)(dir_page + sizeof(int) 
                                      + (*num_entries) * sizeof(direntry));
    strcpy(new_entry->name, name);
    new_entry->inum = inum;
    *num_entries = *num_entries + 1;
    dd->refs = dd->refs + 1;

    return 0;
}

int
rename_entry(const char* from, const char* to) {
    void* dir_page = pages_get_page(1); // hard coded to root
    int* num_entries = (int*)dir_page;

    for (int ii = 0; ii < *num_entries; ++ii) {
        direntry* curr_entry = (direntry*)(dir_page + sizeof(int) 
				+ ii * sizeof(direntry));
    	if (!strcmp(curr_entry->name, from)) {
	        strcpy(curr_entry->name, to);
            return 0;
        }
    }

    return -ENOENT;
}

int
remove_file(const char* path) {
    void* dir_page = pages_get_page(1);
    int *num_entries = (int*)dir_page;

    for (int ii = 0; ii < *num_entries; ++ii) {
        direntry* curr_entry = (direntry*)(dir_page + sizeof(int) 
						+ ii * sizeof(direntry));
    	if (!strcmp(curr_entry->name, path)) {
            free_inode(curr_entry->inum);
            
	    for (int jj = ii; jj < *num_entries; ++jj) {
		direntry* curr_entry = (direntry*)(dir_page + sizeof(int)
						+ jj * sizeof(direntry));
		direntry* next_entry = curr_entry + 1;

		memset(curr_entry->name, 0, strlen(curr_entry->name) + 1);
                strcpy(curr_entry->name, next_entry->name);
		curr_entry->inum = next_entry->inum;
            }

            *num_entries = *num_entries - 1;
            return 0;
        }
    }

    return -ENOENT;
}
