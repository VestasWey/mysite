#pragma once

#include "lua.hpp"
#include <memory>

namespace
{
    void lua_State_deleter(lua_State* lua);
}

typedef std::shared_ptr<lua_State> RefLuaState;

__declspec(dllexport) RefLuaState make_shared_lua_State();
__declspec(dllexport) bool call_lua_func(RefLuaState lua, const char *const func, int return_count,
    const char * fmt, ...);