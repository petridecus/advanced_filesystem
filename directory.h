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

/**
 * Searches for the fiven file path
 * in the directory pointed to by dd.
 */
int directory_lookup(inode* dd, const char* name);

/**
 * Returns the inode number of the directory that contains the given path.
 * Note: does nothing to check if the last part of the path is real (done by
 * directory_lookup).
 */
int tree_lookup(const char* path);

/**
 * Creates an entry for the given file path in
 * the directory pointed to by dd.
 */
int directory_put(inode* dd, const char* name, int inum);

/**
 * Locates the entry with the name "from"
 * and changes the entry name from "from" to "to".
 */
int rename_entry(const char* from, const char* to);

/**
 * Locates the entry with the given name in the directory pointed
 * to by dd and removes it, in addition to freeing the node if it
 * has no more refs. 
 */
int directory_delete(inode* dd, const char* path);

#endif

