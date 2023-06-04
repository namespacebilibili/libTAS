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

#include "posixiowrappers.h"

#include "../logging.h"
#include "../hook.h"
#include "../global.h"
#include "SaveFileList.h"
#include "FileHandleList.h"
#include "../GlobalState.h"
#ifdef __linux__
#include "URandom.h"
#include "../inputs/jsdev.h"
#include "../inputs/evdev.h"
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <cstdarg>
#include <cstring>
#include <unistd.h>

namespace libtas {

DEFINE_ORIG_POINTER(open)
DEFINE_ORIG_POINTER(open64)
DEFINE_ORIG_POINTER(openat)
DEFINE_ORIG_POINTER(openat64)
DEFINE_ORIG_POINTER(creat)
DEFINE_ORIG_POINTER(creat64)
DEFINE_ORIG_POINTER(close)
DEFINE_ORIG_POINTER(access)
DEFINE_ORIG_POINTER(__xstat)
DEFINE_ORIG_POINTER(__lxstat)
DEFINE_ORIG_POINTER(__fxstat)
DEFINE_ORIG_POINTER(__xstat64)
DEFINE_ORIG_POINTER(__lxstat64)
DEFINE_ORIG_POINTER(__fxstat64)
DEFINE_ORIG_POINTER(dup)
DEFINE_ORIG_POINTER(dup2)

int open (const char *file, int oflag, ...)
{
    LINK_NAMESPACE_GLOBAL(open);

    mode_t mode = 0;
#ifdef __linux__
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
#else
    if (oflag & O_CREAT)
#endif
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (GlobalState::isNative())
        return orig::open(file, oflag, mode);

    /* Special case for file opened by je_malloc.
     * We should not allocate any memory here otherwise deadlock. */
    if (file && strcmp(file, "/proc/sys/vm/overcommit_memory") == 0) {
        return orig::open(file, oflag, mode);
    }

    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);
    else {
        debuglogstdio(LCF_FILEIO, "%s call with filename <NULL> and flag %o", __func__, oflag);
        return orig::open(file, oflag, mode);
    }

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::open(file, oflag, mode);

    int fd = 0;

#ifdef __linux__
    if ((strcmp(file, "/dev/urandom") == 0) || (strcmp(file, "/dev/random") == 0)) {
        return urandom_create_fd();
    } else
#endif
    if (strcmp(file, "/proc/uptime") == 0) {
        if (SaveFileList::getSaveFileFd(file) == 0) {
            /* Create a file with memory storage (reusing the savefile code),
             * and fill it with values from the initial time, so that, for
             * games that use it as PRNG seed, tweaking the initial time will
             * change the seed value.
             */
            fd = SaveFileList::openSaveFile(file, O_RDWR | O_TRUNC);

            std::ostringstream datestr;
            datestr << Global::shared_config.initial_time_sec << ".";
            datestr << std::setfill ('0') << std::setw (2);
            datestr << Global::shared_config.initial_time_nsec / 10000000;

            std::string s = datestr.str();

            debuglogstdio(LCF_FILEIO, "Creating fake %s with %s", file, s.c_str());
            write(fd, s.c_str(), s.size());
            write(fd, " ", 1);
            write(fd, s.c_str(), s.size());
            lseek(fd, 0, SEEK_SET);
        }
        else {
            fd = SaveFileList::openSaveFile(file, oflag);
        }
    }

#ifdef __linux__
    /* Check if joystick device */
    else if (is_jsdev(file) >= 0) {
        fd = open_jsdev(file, oflag);
        /* We already stored the file descriptor */
        return fd;
    }

    else if (is_evdev(file) >= 0) {
        fd = open_evdev(file, oflag);
        /* We already stored the file descriptor */
        return fd;
    }
#endif

    else if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        fd = SaveFileList::openSaveFile(file, oflag);
    }

    else {
        fd = orig::open(file, oflag, mode);
    }

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int open64 (const char *file, int oflag, ...)
{
    LINK_NAMESPACE_GLOBAL(open64);

    mode_t mode = 0;
#ifdef __linux__
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
#else
    if (oflag & O_CREAT)
#endif
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (GlobalState::isNative())
        return orig::open64(file, oflag, mode);

    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);
    else {
        debuglogstdio(LCF_FILEIO, "%s call with filename <NULL> and flag %o", __func__, oflag);
        return orig::open64(file, oflag, mode);
    }

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::open64(file, oflag, mode);

    int fd = 0;

#ifdef __linux__
    if ((strcmp(file, "/dev/urandom") == 0) || (strcmp(file, "/dev/random") == 0)) {
        return urandom_create_fd();
    } else
#endif
    if (strcmp(file, "/proc/uptime") == 0) {
        if (SaveFileList::getSaveFileFd(file) == 0) {
            /* Create a file with memory storage (reusing the savefile code),
             * and fill it with values from the initial time, so that, for
             * games that use it as PRNG seed, tweaking the initial time will
             * change the seed value.
             */
            fd = SaveFileList::openSaveFile(file, O_RDWR | O_TRUNC);

            std::ostringstream datestr;
            datestr << Global::shared_config.initial_time_sec << ".";
            datestr << std::setfill ('0') << std::setw (2);
            datestr << Global::shared_config.initial_time_nsec / 10000000;

            std::string s = datestr.str();

            debuglogstdio(LCF_FILEIO, "Creating fake %s with %s", file, s.c_str());
            write(fd, s.c_str(), s.size());
            write(fd, " ", 1);
            write(fd, s.c_str(), s.size());
            lseek(fd, 0, SEEK_SET);
        }
        else {
            fd = SaveFileList::openSaveFile(file, oflag);
        }
    }

#ifdef __linux__
    /* Check if joystick device */
    else if (is_jsdev(file) >= 0) {
        fd = open_jsdev(file, oflag);
        /* We already stored the file descriptor */
        return fd;
    }

    else if (is_evdev(file) >= 0) {
        fd = open_evdev(file, oflag);
        /* We already stored the file descriptor */
        return fd;
    }
#endif

    else if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        fd = SaveFileList::openSaveFile(file, oflag);
    }

    else {
        fd = orig::open64(file, oflag, mode);
    }

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int openat (int dirfd, const char *file, int oflag, ...)
{
    LINK_NAMESPACE_GLOBAL(openat);

    mode_t mode = 0;
#ifdef __linux__
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
#else
    if (oflag & O_CREAT)
#endif
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (GlobalState::isNative())
        return orig::openat(dirfd, file, oflag, mode);

    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);
    else {
        debuglogstdio(LCF_FILEIO, "%s call with filename <NULL> and flag %o", __func__, oflag);
        return orig::openat(dirfd, file, oflag, mode);
    }

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::openat(dirfd, file, oflag, mode);

    int fd = 0;

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        fd = SaveFileList::openSaveFile(file, oflag);
    }

    else {
        fd = orig::openat(dirfd, file, oflag, mode);
    }

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int openat64 (int dirfd, const char *file, int oflag, ...)
{
    LINK_NAMESPACE_GLOBAL(openat64);

    mode_t mode = 0;
#ifdef __linux__
    if ((oflag & O_CREAT) || (oflag & O_TMPFILE))
#else
    if (oflag & O_CREAT)
#endif
    {
        va_list arg_list;

        va_start(arg_list, oflag);
        mode = va_arg(arg_list, mode_t);
        va_end(arg_list);
    }

    if (GlobalState::isNative())
        return orig::openat64(dirfd, file, oflag, mode);

    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with filename %s and flag %o", __func__, file, oflag);
    else {
        debuglogstdio(LCF_FILEIO, "%s call with filename <NULL> and flag %o", __func__, oflag);
        return orig::openat64(dirfd, file, oflag, mode);
    }

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::openat64(dirfd, file, oflag, mode);

    int fd = 0;

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        fd = SaveFileList::openSaveFile(file, oflag);
    }

    else {
        fd = orig::openat64(dirfd, file, oflag, mode);
    }

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int creat (const char *file, mode_t mode)
{
    LINK_NAMESPACE_GLOBAL(creat);

    if (GlobalState::isNative())
        return orig::creat(file, mode);

    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with file %s", __func__, file);
    else {
        debuglogstdio(LCF_FILEIO, "%s call with file <NULL>", __func__);
        return orig::creat(file, mode);
    }

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::creat(file, mode);

    /* From creat() man page, creat() is just open() with flags
     * O_CREAT, O_WRONLY and O_TRUNC
     */
    int oflag = O_CREAT|O_WRONLY|O_TRUNC;

    int fd = 0;

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        fd = SaveFileList::openSaveFile(file, oflag);
    }

    else {
        fd = orig::creat(file, mode);
    }

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int creat64 (const char *file, mode_t mode)
{
    LINK_NAMESPACE_GLOBAL(creat64);

    if (GlobalState::isNative())
        return orig::creat64(file, mode);

    if (file)
        debuglogstdio(LCF_FILEIO, "%s call with file %s", __func__, file);
    else {
        debuglogstdio(LCF_FILEIO, "%s call with file <NULL>", __func__);
        return orig::creat64(file, mode);
    }

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::creat64(file, mode);

    int oflag = O_CREAT|O_WRONLY|O_TRUNC;

    int fd = 0;

    if (!GlobalState::isOwnCode() && SaveFileList::isSaveFile(file, oflag)) {
        debuglogstdio(LCF_FILEIO, "  savefile detected");
        fd = SaveFileList::openSaveFile(file, oflag);
    }
    else {
        fd = orig::creat64(file, mode);
    }

    /* Store the file descriptor */
    FileHandleList::openFile(file, fd);

    return fd;
}

int close (int fd)
{
    LINK_NAMESPACE_GLOBAL(close);

    if (GlobalState::isNative())
        return orig::close(fd);

    debuglogstdio(LCF_FILEIO, "%s call", __func__);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::close(fd);

#ifdef __linux__
    /* Check for urandom */
    if (urandom_get_fd() == fd) {
        return 0;
    }
#endif

    /* Check if we must actually close the file */
    bool doClose = FileHandleList::closeFile(fd);

    if (doClose) {
        int ret = SaveFileList::closeSaveFile(fd);

        if (ret != 1)
            return ret;

        return orig::close(fd);
    }

    return 0;
}

int access(const char *name, int type) __THROW
{
    LINK_NAMESPACE_GLOBAL(access);

    if (GlobalState::isNative())
        return orig::access(name, type);

    if (name)
        debuglogstdio(LCF_FILEIO, "%s call with name %s", __func__, name);
    else {
        debuglogstdio(LCF_FILEIO, "%s call with name <NULL>", __func__);
        return orig::access(name, type);
    }

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::access(name, type);

#ifdef __linux__
    /* Check if joystick device */
    int joy = is_jsdev(name);
    if (joy == -1) joy = is_evdev(name);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }
#endif

    /* Check for savefile. */
    if (SaveFileList::getSaveFileFd(name) != 0) {
        if (SaveFileList::isSaveFileRemoved(name)) {
            errno = ENOENT;
            return -1;
        }
        else {
            return 0;
        }
    }

    return orig::access(name, type);
}

int __xstat(int ver, const char *path, struct stat *buf) __THROW
{
    LINK_NAMESPACE_GLOBAL(__xstat);

    if (GlobalState::isNative())
        return orig::__xstat(ver, path, buf);

    debuglogstdio(LCF_FILEIO, "%s call with path %s", __func__, path);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::__xstat(ver, path, buf);

#ifdef __linux__
    /* Check if joystick device */
    int joy = is_jsdev(path);
    if (joy == -1) joy = is_evdev(path);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }
#endif

    /* Check if savefile. */
    int fd = SaveFileList::getSaveFileFd(path);
    if (fd != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            NATIVECALL(__fxstat(ver, fd, buf));
            return 0;
        }
    }

    return orig::__xstat(ver, path, buf);
}

int __xstat64(int ver, const char *path, struct stat64 *buf) __THROW
{
    LINK_NAMESPACE_GLOBAL(__xstat64);

    if (GlobalState::isNative())
        return orig::__xstat64(ver, path, buf);

    debuglogstdio(LCF_FILEIO, "%s call with path %s", __func__, path);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::__xstat64(ver, path, buf);

#ifdef __linux__
    /* Check if joystick device */
    int joy = is_jsdev(path);
    if (joy == -1) joy = is_evdev(path);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }
#endif

    /* Check if savefile. */
    int fd = SaveFileList::getSaveFileFd(path);
    if (fd != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            NATIVECALL(__fxstat64(ver, fd, buf));
            return 0;
        }
    }

    return orig::__xstat64(ver, path, buf);
}

int __lxstat(int ver, const char *path, struct stat *buf) __THROW
{
    LINK_NAMESPACE_GLOBAL(__lxstat);

    if (GlobalState::isNative())
        return orig::__lxstat(ver, path, buf);

    debuglogstdio(LCF_FILEIO, "%s call with path %s", __func__, path);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::__lxstat(ver, path, buf);

#ifdef __linux__
    /* Check if joystick device */
    int joy = is_jsdev(path);
    if (joy == -1) joy = is_evdev(path);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }
#endif

    /* Check if savefile. */
    int fd = SaveFileList::getSaveFileFd(path);
    if (fd != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            NATIVECALL(__fxstat(ver, fd, buf));
            return 0;
        }
    }

    return orig::__lxstat(ver, path, buf);
}

int __lxstat64(int ver, const char *path, struct stat64 *buf) __THROW
{
    LINK_NAMESPACE_GLOBAL(__lxstat64);

    if (GlobalState::isNative())
        return orig::__lxstat64(ver, path, buf);

    debuglogstdio(LCF_FILEIO, "%s call with path %s", __func__, path);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::__lxstat64(ver, path, buf);

#ifdef __linux__
    /* Check if joystick device */
    int joy = is_jsdev(path);
    if (joy == -1) joy = is_evdev(path);

    if (joy >= 0) {
        if (joy == 1)
            return 0;
        else {
            errno = ENOENT;
            return -1;
        }
    }
#endif

    /* Check if savefile. */
    int fd = SaveFileList::getSaveFileFd(path);
    if (fd != 0) {
        if (SaveFileList::isSaveFileRemoved(path)) {
            errno = ENOENT;
            return -1;
        }
        else {
            NATIVECALL(__fxstat64(ver, fd, buf));
            return 0;
        }
    }

    return orig::__lxstat64(ver, path, buf);
}

int __fxstat(int ver, int fd, struct stat *buf) __THROW
{
    LINK_NAMESPACE_GLOBAL(__fxstat);

    if (GlobalState::isNative())
        return orig::__fxstat(ver, fd, buf);

    debuglogstdio(LCF_FILEIO, "%s call with fd %d", __func__, fd);
    return orig::__fxstat(ver, fd, buf);
}

int __fxstat64(int ver, int fd, struct stat64 *buf) __THROW
{
    LINK_NAMESPACE_GLOBAL(__fxstat64);

    if (GlobalState::isNative())
        return orig::__fxstat64(ver, fd, buf);

    debuglogstdio(LCF_FILEIO, "%s call with fd %d", __func__, fd);
    return orig::__fxstat64(ver, fd, buf);
}

int dup (int fd) __THROW
{
    debuglogstdio(LCF_FILEIO, "%s call on %d", __func__, fd);
    LINK_NAMESPACE_GLOBAL(dup);

    int newfd = orig::dup(fd);
    debuglogstdio(LCF_FILEIO, "   new fd: %d", newfd);
    return newfd;
}

int dup2 (int fd, int fd2) __THROW
{
    debuglogstdio(LCF_FILEIO, "%s call: %d -> %d", __func__, fd2, fd);
    LINK_NAMESPACE_GLOBAL(dup2);

    if (Global::shared_config.debug_state & SharedConfig::DEBUG_NATIVE_FILEIO)
        return orig::dup2(fd, fd2);

    if (fd2 == 2) {
        /* Prevent the game from redirecting stderr (2) to a file */
        return 2;
    }

    return orig::dup2(fd, fd2);
}

}
