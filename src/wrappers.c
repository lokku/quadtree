#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#define NO_WRAPPERS 1
#include "wrappers.h"


/*
 * A wrapper for write(2) that retries and supports writing in multiple chunks
 */
ssize_t write_wrapper(int fd, const void *buf, size_t count)
{
    size_t written = 0; /* Bytes actually written */

    while (written < count) {
        ssize_t chunk_size = write(fd, (char *)buf+written, count-written);

        if (chunk_size == -1) {
            switch (errno) {
                case EINTR:
                    /* Interuppted by a signal, nothing was written, retry */
                    continue;
                default:
                    /* Let the caller handle errors */
                    return -1;
            }
        }

        written += chunk_size;
    }

    return (ssize_t)written;
}

/*
 * A wrapper for read(2) that retries and supports reading in multiple chunks
 */
ssize_t read_wrapper(int fd, const void *buf, size_t count)
{
    size_t rread = 0; /* Bytes actually read (grr name conflict) */

    while (rread < count) {
        ssize_t chunk_size = read(fd, (char *)buf+rread, count-rread);

        if (chunk_size == -1) {
            switch (errno) {
                case EINTR:
                    /* Interuppted by a signal, nothing was written, retry */
                    continue;
                default:
                    /* Let the caller handle errors */
                    return -1;
            }
        }

        rread += chunk_size;
    }

    return (ssize_t)rread;
}

