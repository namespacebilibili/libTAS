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

#include "URandom.h"

#include "FileHandleList.h"
#include "../logging.h"
#include <fcntl.h>
#include <unistd.h> // getpid()
#include <signal.h>
#include "../global.h"
#include "../GlobalState.h"

namespace libtas {

static int readfd = -1;
static int writefd = -1;
static FILE* stream = nullptr;
static uint64_t prng_state = 0;

static void urandom_handler(int signum)
{
    debuglogstdio(LCF_FILEIO | LCF_RANDOM, "Filling urandom fd");
    if (!prng_state)
        prng_state = Global::shared_config.initial_time_sec;
    
    int err = 0;
    do {
        /* Use xorshift64* algorithm to generate pseudo-random values */
        uint64_t r = prng_state;	/* The state must be seeded with a nonzero value. */
    	r ^= r >> 12;
    	r ^= r << 25;
    	r ^= r >> 27;
    	prng_state = r;
        r *= 0x2545F4914F6CDD1DULL;
        err = write(writefd, &r, sizeof(uint64_t));
    } while (err != -1);
}

int urandom_create_fd()
{
    debuglogstdio(LCF_FILEIO | LCF_RANDOM, "Open /dev/urandom");

    if (readfd == -1) {
        std::pair<int, int> fds = FileHandleList::createPipe(O_NONBLOCK);
        readfd = fds.first;
        writefd = fds.second;

        /* Set the pipe size to some small value, because we won't need much.
         * It should be at least 2*page_size, because on Linux a pipe is
         * considered writeable if at least page_size can be written.
         */
        MYASSERT(fcntl(writefd, F_SETPIPE_SZ, 2*4096) != -1);

        /* Fill the pipe */
        urandom_handler(0);

        GlobalNative gn;

        /* Add async signal for when the pipe is writeable */
        MYASSERT(fcntl(writefd, F_SETOWN, getpid()) != -1);
        MYASSERT(fcntl(writefd, F_SETSIG, 0) != -1);
        MYASSERT(fcntl(writefd, F_SETFL, O_ASYNC | O_NONBLOCK) != -1);

        /* Unblock SIGIO signal */
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGIO);
        MYASSERT(pthread_sigmask(SIG_UNBLOCK, &mask, nullptr) == 0);

        /* Add signal handler for SIGIO signal */
        struct sigaction sigio;
        sigfillset(&sigio.sa_mask);
        sigio.sa_flags = SA_RESTART;
        sigio.sa_handler = urandom_handler;
        MYASSERT(sigaction(SIGIO, &sigio, nullptr) == 0)
    }

    debuglogstdio(LCF_FILEIO | LCF_RANDOM, "Return fd %d", readfd);
    return readfd;
}

int urandom_get_fd() {
    return readfd;
}

FILE* urandom_create_file() {
    if (!stream) {
        int readfd = urandom_create_fd();
        stream = fdopen(readfd, "r");
        setvbuf(stream, nullptr, _IONBF, 0);
    }
    return stream;
}

FILE* urandom_get_file() {
    return stream;
}

void urandom_disable_handler() {
    GlobalNative gn;

    if (writefd != -1) {
        MYASSERT(fcntl(writefd, F_SETFL, O_NONBLOCK) != -1);
    }
}

void urandom_enable_handler() {
    GlobalNative gn;

    if (writefd != -1) {
        MYASSERT(fcntl(writefd, F_SETFL, O_ASYNC | O_NONBLOCK) != -1);
    }
}

}
