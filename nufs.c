// based on cs3650 starter code

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <bsd/string.h>
#include <assert.h>
#include <libgen.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "util.h"

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{
    printf("access(%s, %04o)\n", path, mask);
    return 0;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
    // first check that the directory is real
    char* dir = get_dir(path);
    int dir_inum = directory_lookup(get_inode(0), dir);
    
    if (dir_inum < 0) {
        printf("%s isn't a valid directory\n", dir);
        return -ENOENT;
    }

    printf("%s was a valid directory!\n", dir);

    free(dir);
    inode* dd = get_inode(dir_inum);
    int inum = directory_lookup(dd, path);
    
    if (inum < 0) {
        printf("file %s doesn't exist\n", path);
        return -ENOENT;
    } 

    inode* nn = get_inode(inum);
    st->st_mode = nn->mode;
    st->st_size = nn->size;
    st->st_uid = getuid();
    st->st_nlink = nn->refs; // TODO make sure to keep track of refs throughout.

    printf("getattr(%s) [%d] -> {mode: %04o, size: %ld}\n", 
		    path, inum, st->st_mode, st->st_size);

    if (inum < 0) return -ENOENT;
    
    return 0;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    struct stat st;
    int rv;

    // NOTE hard coding readdir to root for hw10
    rv = nufs_getattr(path, &st);
    if (rv < 0) return rv;

    printf("nufs_readdir(), successfully ran getattr for %s\n", path);

    filler(buf, ".", &st, rv);
    if (strcmp(path, "/")) {
        rv = nufs_getattr("/", &st);
        //filler(buf, "..", &st, rv);
    }

    inode* nn = get_inode(directory_lookup(get_inode(0), path)); // basically depth of 2 now

    void* page = pages_get_page(nn->ptrs[0]);
    int num_entries = nn->size / sizeof(direntry);
    direntry* dd = (direntry*)(page);

    printf("%d entries in %s\n", num_entries, path);

    for (int ii = 0; ii < num_entries; ++ii) {
	    printf("readdir reaching %s\n", dd->name);
	    rv = nufs_getattr(dd->name, &st);
	    if (rv < 0) return rv;
	
	    // need to strip dir path from files in buffer
	    // as of now it's always just the "/" character
	    char* name_tmp = strdup(dd->name);
        char* base = basename(name_tmp);

        printf("filling info for file %s\n", base);
	    filler(buf, base, &st, rv);
        free(name_tmp);
        ++dd;
    }

    printf("readdir(%s): %d files -> %d\n", path, num_entries, 0);
    return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int inum = alloc_inode(); // NOTE - ptr page also allocated in alloc_inode

    char* dir = get_dir(path);
    int dir_inum = directory_lookup(get_inode(0), dir);
    if (dir_inum < 0) {
        puts("trying to add something to an invalid dirctory");
        return -ENOENT;
    }

    inode* nn = get_inode(inum);
    nn->refs = 1;
    nn->size = 0;
    nn->mode = mode;

    // NOTE putting all new files in root directory for hw10
    inode* dd = get_inode(dir_inum);
    int rv = directory_put(dd, path, inum);

    printf("mknod(%s, %04o) -> %d (inum %d)\n", path, mode, rv, inum);
    return 0;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    int rv = nufs_mknod(path, mode | 040000, 0);
    printf("mkdir(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_unlink(const char *path)
{
    int rv = 0;
    char* dir = get_dir(path);
    int inum = directory_lookup(get_inode(0), dir);
    inode* node = get_inode(inum);
    directory_delete(node, path);
    free(dir);
    printf("unlink(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_link(const char *from, const char *to)
{
    printf("from: %s, to: %s\n", from, to);
    int rv = 0;
    // directory lookup, get inode from.
    // directory lookup to, add direntry and set inum to inum of from.
    inode* root = get_inode(0);
    int fnum = directory_lookup(root, from);
    inode* fnode = get_inode(fnum);
    fnode->refs++;
    assert(fnum != -1);
    directory_put(root, to, fnum);
    printf("link(%s => %s) -> %d\n", from, to, rv);
    return rv;
}

int
nufs_rmdir(const char *path)
{
    int rv = -1;
    printf("rmdir(%s) -> %d\n", path, rv);
    return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    int rv = rename_entry(from, to);
    printf("rename(%s => %s) -> %d\n", from, to, rv);
    return rv;
}

int
nufs_chmod(const char *path, mode_t mode)
{
    int rv = -1;
    printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

int
nufs_truncate(const char *path, off_t size)
{
    int rv = 0;
    printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
    return rv;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    printf("open(%s)\n", path);
    return 0;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char* dir = get_dir(path);
    int dir_inum = directory_lookup(get_inode(0), dir);
    if (dir_inum == -1) return -ENOENT;
    free(dir);

    int inum = directory_lookup(get_inode(dir_inum), path);

    if (inum != -1) {
        inode* nn = get_inode(inum);
        int page_offset = offset % 4096;
        int page_num = offset / 4096;

        char* data = (char*)((uintptr_t)pages_get_page(nn->ptrs[page_num]) + page_offset);
        memcpy(buf, data, size);
    }

    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, (int)size);
    if (inum == -1) return -ENOENT;
    return (int)size;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    char* dir = get_dir(path);
    int dir_inum = directory_lookup(get_inode(0), dir);
    if (dir_inum == -1) return -ENOENT;
    free(dir);

    int inum = directory_lookup(get_inode(dir_inum), path);

    if (inum != -1) {
        inode* nn = get_inode(inum);
        int page_offset = offset % 4096;
        int page_num = offset / 4096;
        
        if (size + offset > (size_t)nn->size) inode_grow(nn, size + offset);
        else if (size + offset < (size_t)nn->size) inode_shrink(nn, size + offset);

        char* data = (char*)(pages_get_page(nn->ptrs[page_num]) + page_offset);
        memcpy(data, buf, size);
    }

    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, (int)size);
    if (inum == -1) return -ENOENT;
    return (int)size;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    printf("utimens(%s) -> %d\n", path, -1);
    return 0;
}

// Extended operations
int
nufs_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi,
           unsigned int flags, void* data)
{
    int rv = -1;
    printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
    return rv;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->link     = nufs_link;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
    ops->ioctl    = nufs_ioctl;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    pages_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}
