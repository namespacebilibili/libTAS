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

#ifndef LIBTAS_GLOBAL_H_INCL
#define LIBTAS_GLOBAL_H_INCL

#include "../shared/SharedConfig.h"
#include "../shared/GameInfo.h"

namespace libtas {

namespace Global {

    /* Configuration parameters that are shared between the program and the game */
    extern SharedConfig shared_config;

    /* Some informations about the game that are send to the program */
    extern GameInfo game_info;

    /* Indicate if the library constructor has started */
    extern volatile bool is_inited;

    /* Indicate if the game is exiting. It helps avoiding some invalid or blocking calls */
    extern volatile bool is_exiting;

    /* Indicate if this is a forked process. We should not use socket on forked processes */
    extern volatile bool is_fork;

    /* Do we skip all rendering functions for the current frame */
    extern bool skipping_draw;
}
}

#endif
