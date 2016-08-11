/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "Config.h"

Config config = {
    hud_framecount : true,
    hud_inputs : true,
    prevent_savefiles : true
};

void Config::default_hotkeys()
{
    hotkeys[HOTKEY_PLAYPAUSE] = XK_Pause;
    hotkeys[HOTKEY_FRAMEADVANCE] = XK_v;
    hotkeys[HOTKEY_FASTFORWARD] = XK_Tab;
    hotkeys[HOTKEY_READWRITE] = XK_p;
    hotkeys[HOTKEY_SAVESTATE] = XK_s;
    hotkeys[HOTKEY_LOADSTATE] = XK_m;

    input_mapping[XK_w].type = IT_CONTROLLER1_BUTTON_A;
    input_mapping[XK_w].value = 1;
    input_mapping[XK_x].type = IT_CONTROLLER1_BUTTON_B;
    input_mapping[XK_x].value = 1;
    input_mapping[XK_c].type = IT_CONTROLLER1_BUTTON_X;
    input_mapping[XK_c].value = 1;
    input_mapping[XK_d].type = IT_CONTROLLER1_BUTTON_LEFTSHOULDER;
    input_mapping[XK_d].value = 1;
    input_mapping[XK_f].type = IT_CONTROLLER1_BUTTON_RIGHTSHOULDER;
    input_mapping[XK_f].value = 1;
    input_mapping[XK_i].type = IT_CONTROLLER1_BUTTON_DPAD_UP;
    input_mapping[XK_i].value = 1;
    input_mapping[XK_k].type = IT_CONTROLLER1_BUTTON_DPAD_DOWN;
    input_mapping[XK_k].value = 1;
    input_mapping[XK_j].type = IT_CONTROLLER1_BUTTON_DPAD_LEFT;
    input_mapping[XK_j].value = 1;
    input_mapping[XK_l].type = IT_CONTROLLER1_BUTTON_DPAD_RIGHT;
    input_mapping[XK_l].value = 1;
}
