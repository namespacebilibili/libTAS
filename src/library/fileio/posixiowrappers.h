/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_POSIXIO_H_INCLUDED
#define LIBTAS_POSIXIO_H_INCLUDED

#include "../hook.h"
#include <sys/types.h> // mode_t
#include <sys/stat.h>
// #include <sys/sendfile.h>

namespace libtas {

/* Open FILE and return a new file descriptor for it, or -1 on error.
   OFLAG determines the type of access used.  If O_CREAT or O_TMPFILE is set
   in OFLAG, the third argument is taken as a `mode_t', the mode of the
   created file. */
OVERRIDE int open (const char *file, int oflag, ...);
OVERRIDE int open64 (const char *__file, int __oflag, ...);

/* Similar to `open' but a relative path name is interpreted relative to
   the directory for which FD is a descriptor.

   NOTE: some other `openat' implementation support additional functionality
   through this interface, especially using the O_XATTR flag.  This is not
   yet supported here. */
OVERRIDE int openat (int fd, const char *file, int oflag, ...);
OVERRIDE int openat64 (int fd, const char *file, int oflag, ...);

/* Create and open FILE, with mode MODE.  This takes an `int' MODE
   argument because that is what `mode_t' will be widened to. */
OVERRIDE int creat (const char *file, mode_t mode);
OVERRIDE int creat64 (const char *file, mode_t mode);

/* Close the file descriptor FD. */
OVERRIDE int close (int fd);

/* Test for access to NAME using the real UID and real GID. */
OVERRIDE int access (const char *name, int type) __THROW;

/* Wrappers for stat functions */
OVERRIDE int __xstat(int ver, const char *path, struct stat *buf) __THROW;
OVERRIDE int __xstat64(int ver, const char *path, struct stat64 *buf) __THROW;
OVERRIDE int __lxstat(int ver, const char *path, struct stat *buf) __THROW;
OVERRIDE int __lxstat64(int ver, const char *path, struct stat64 *buf) __THROW;
OVERRIDE int __fxstat(int ver, int fd, struct stat *buf) __THROW;
OVERRIDE int __fxstat64(int ver, int fd, struct stat64 *buf) __THROW;

// OVERRIDE ssize_t sendfile (int out_fd, int in_fd, off_t *offset, size_t count) __THROW;

/* Duplicate FD to FD2, closing FD2 and making it open on the same file.  */
OVERRIDE int dup (int fd) __THROW;
OVERRIDE int dup2 (int fd, int fd2) __THROW;

}

#endif
