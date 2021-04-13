TODO

need to reconfigure the space before data pages to include
 - 1 dedicated page for bitmaps
 - 2 dedicated pages for inodes (8192 / 64 = 128 inodes)

read/write need to be adjusted to take multiple ptrs into account
 - calculate a start and end page based on size/offset
 - iterate read or write from start to end page
 - already passing the test for this since it's presumably just writing
   beyond the page for the file. 

hard links, and then symbolic links 
 - hard links share an inode
 - i tried to quickly add this in during hw10 and failed,
   so it's harder than it seems for some reason.
 - i have no idea what the heck a symbolic link is.

readdir and other functions need to work for nested directories
 - most important question is how do we retrieve the inode
   for a directory using just it's path.

