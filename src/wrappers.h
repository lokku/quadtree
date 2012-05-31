#ifndef WRAPPERS_H
#define WRAPPERS_H

/*
 * Wrapper macros. These rewrite syscalls/libary-functions to versions that
 * provide the same interface, but handle more errors, or do things more
 * intelligently.
 */

ssize_t write_wrapper(int fd, const void *buf, size_t count);
ssize_t read_wrapper(int fd, const void *buf, size_t count);

#define write(a,b,c) write_wrapper(a,b,c)
#define read(a,b,c)  read_wrapper(a,b,c)


#endif /* WRAPPERS_H */

