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
    // NOTE changed this function to just search thru root for hw10
    int inum = directory_lookup(path);

    inode* nn = get_inode(inum);
    st->st_mode = nn->mode;
    st->st_size = nn->size;
    st->st_uid = getuid();
    st->st_atim = nn->time;
    st->st_mtim = nn->time;

    printf("getattr(%s) [%d] -> {mode: %04o, size: %ld}\n", 
		    path, inum, st->st_mode, st->st_size);

    if (inum == -1) return -ENOENT;
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
    rv = nufs_getattr("/", &st);
    if (rv < 0) return rv;
    filler(buf, ".", &st, rv);

    inode* nn = get_inode(0);

    void* page = pages_get_page(nn->ptr);
    int num_entries = *(int*)page;
    direntry* dd = (direntry*)(page + sizeof(int));

    for (int ii = 0; ii < num_entries; ++ii) {
	printf("readdir reaching %s\n", dd->name);
	rv = nufs_getattr(dd->name, &st);
	if (rv < 0) return rv;
	
	// need to strip dir path from files in buffer
	// as of now it's always just the "/" character
	char relative_path[48];
	for (size_t ii = 0; ii <= strlen(dd->name); ++ii)
	    relative_path[ii] = dd->name[ii + strlen(path)];
        
	filler(buf, relative_path, &st, rv);
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

    inode* nn = get_inode(inum);
    nn->refs = 1;
    nn->size = 0;
    nn->mode = 0100644;

    // NOTE putting all new files in root directory for hw10
    inode* rnode = get_inode(0);
    int rv = directory_put(rnode, path, inum);

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
    int rv = remove_file(path);
    printf("unlink(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_link(const char *from, const char *to)
{
    int rv = -1;
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
    int rv = -1;
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
    int inum = directory_lookup(path);
    if (inum != -1) {
        inode* nn = get_inode(inum);
        char* data = (char*)(pages_get_page(nn->ptr) + offset);
        memcpy(buf, data, size);
        printf("reading: \"%s\"\n", buf);
    }

    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, (int)size);
    if (inum == -1) return -ENOENT;
    return (int)size;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int inum = directory_lookup(path);
    if (inum != -1) {
        inode* nn = get_inode(inum);
        nn->size = max(size + offset, nn->size);
        char* data = (char*)(pages_get_page(nn->ptr) + offset);
        memcpy(data, buf, size);
        printf("wrote: \"%s\"\n", data);
    }

    printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, (int)size);
    if (inum == -1) return -ENOENT;
    return (int)size;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    int inum = directory_lookup(path);

    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, 
	   ts[1].tv_nsec, inum == -1 ? -ENOENT : 0);

    if (inum == -1) return -ENOENT;

    // NOTE time field didn't wind up being necessary but i'm keeping it
    inode_set_time(get_inode(inum), ts[0]);

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
