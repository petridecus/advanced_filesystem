#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <libgen.h>
#include <errno.h>
#include <sys/stat.h>
#include "directory.h"
#include "slist.h"
#include "util.h"

#define MAX_ENTRIES 254 // (4096 - sizeof(int)) / sizeof(direntry)

// NOTE view directory.h for documentation on direntry functions/struct

void
directory_init() {
    int existed = bitmap_get(get_pages_bitmap(), 3);
    if (existed) return;

    assert(alloc_page() == 3 && alloc_inode() == 0);

    inode* rnode = get_inode(0);
    rnode->mode = 040755;
    rnode->ptrs[0] = 3;
}

int
tree_lookup(const char* path) {
    if (!strcmp(path, "/")) return 0;

    int rv = 0;
    int ret = 0;

    slist* path_dirs = s_split(path, '/');
    slist* orig_dirs = path_dirs;

    path_dirs = path_dirs->next;
    if (!path_dirs) {
        s_free(orig_dirs);
        return 0;
    }

    char* dir = malloc(48 * sizeof(char));
    memcpy(dir, "/", 2);
    inode* curr_inode = get_inode(0);

    printf("in tree_lookup, starting search in dir %s\n", dir);

    while (path_dirs->next) {
        char* subdir = malloc(48 * sizeof(char));
        memcpy(subdir, dir, strlen(dir) + 1);
        printf("setting subdir to %s + %s\n", dir, path_dirs->data);

        if (strlen(subdir) > 1) strcat(subdir, "/");
        strcat(subdir, path_dirs->data);
	
        rv = directory_lookup(curr_inode, subdir);
	
        if (rv == -1) {
            free(dir);
            free(subdir);
            s_free(orig_dirs);
            return rv;
        }

	    curr_inode = get_inode(rv);

	    if (!S_ISDIR(curr_inode->mode)) {
	        free(dir);
	        free(subdir);
	        s_free(orig_dirs);
	        break;
	    }
	
	    ret = rv;

	    path_dirs = path_dirs->next;
	    free(dir);
	    dir = subdir;
    }

    s_free(orig_dirs);
    return ret; 
}

int 
directory_lookup(inode* dd, const char *name) {
    if (strcmp(name, "/") == 0) return 0;

    int num_entries = dd->size / sizeof(direntry);
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
    int dnum = tree_lookup(from);
    inode* dd = get_inode(dnum);

    void* dir_page = pages_get_page(dd->ptrs[0]);
    int num_entries = dd->size / sizeof(direntry);

    for (int ii = 0; ii < num_entries; ++ii) {
        direntry* curr_entry = (direntry*)((uintptr_t)dir_page + ii * sizeof(direntry));
    	if (!strcmp(curr_entry->name, from)) {
	    // memset(curr_entry->name, 0, 48);
	    strcpy(curr_entry->name, to);
            return 0;
        }
    }

    return -ENOENT;
}

int
directory_delete(inode* dd, const char* path) {
    int pnum = dd->ptrs[0];
    void* dir_page = pages_get_page(pnum);
    int num_entries = dd->size / sizeof(direntry);

    for (int ii = 0; ii < num_entries; ++ii) {
        direntry* curr_entry = (direntry*)((uintptr_t)dir_page + ii * sizeof(direntry));
    	if (strcmp(curr_entry->name, path) == 0) {
	        inode* deleted = get_inode(curr_entry->inum);
	        deleted->refs -= 1;

	        if (deleted->refs == 0) {
                free_inode(curr_entry->inum);
	        }
            
	        for (int jj = ii; jj < num_entries; ++jj) {
		        direntry* curr_entry = (direntry*)(dir_page + jj * sizeof(direntry));
		        direntry* next_entry = (direntry*)((uintptr_t)curr_entry + sizeof(direntry));

		        // memset((void*)curr_entry, 0, sizeof(direntry));
                memcpy((void*)curr_entry, (void*)next_entry, sizeof(direntry));
            }

            dd->size -= sizeof(direntry);
            return 0;
        }
    }

    return -ENOENT;
}
