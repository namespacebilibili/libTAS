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

#ifndef LIBTAS_NAMEDLUAFUNCTION_H_INCLUDED
#define LIBTAS_NAMEDLUAFUNCTION_H_INCLUDED

#include <string>

extern "C" {
#include <lua.h>
}

namespace Lua {

class NamedLuaFunction {
    
public:
    
    enum CallbackType {
        CallbackStartup,
        CallbackInput,
        CallbackFrame,
        CallbackPaint,
    };
    
    NamedLuaFunction(lua_State *L, CallbackType t);
    ~NamedLuaFunction();
    
    NamedLuaFunction(const NamedLuaFunction&) = delete;

    void call();
    
    CallbackType type;
    const std::string& file;
    
private:
    lua_State *lua_state;
    int function_ref = 0;

};
}

#endif
