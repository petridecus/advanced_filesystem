// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME 48

// #include "slist.h"
#include "pages.h"
#include "inode.h"

// only changed name from starter code
// it was giving me an error saying it was
// 'already defined' in some library header somewhere
typedef struct direntry {
    char name[DIR_NAME];
    int  inum;
    char _reserved[12];
} direntry;

// initializes the root directory
// should only be called once
void directory_init();

char* get_dir(const char* path);

/**
 * Searches for the fiven file path
 * in the root directory.
 */
int directory_lookup(inode* dd, const char* name);

int tree_lookup(const char* path);

/**
 * Creates an entry for the given file path in
 * the root directory.
 */
int directory_put(inode* dd, const char* name, int inum);

/**
 * Locates the entry with the name "from" in the root directory
 * and changes the entry name from "from" to "to"
 */
int rename_entry(const char* from, const char* to);

/**
 * Locates the entry with the given name in the root
 * directory and removes it, in addition to freeing the
 * node. 
 * Currently working under the assumption that a file won't 
 * have any additional refs, since there's only 1 dir.
 */
int directory_delete(inode* dd, const char* path);

#endif

