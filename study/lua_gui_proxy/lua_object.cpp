#include "stdafx.h"
#include "lua_object.h"
#include <stdarg.h>
#include <string>

namespace
{
    void lua_State_deleter(lua_State* lua)
    {
        lua_close(lua);
    }
}

__declspec(dllexport) RefLuaState make_shared_lua_State()
{
    RefLuaState lua(luaL_newstate(), lua_State_deleter);
    return lua;
}

bool call_lua_func(RefLuaState lua, const char *const func, int return_count, const char * fmt, ...)
{
    if (!lua)
    {
        return false;
    }

    if (LUA_TFUNCTION == lua_getglobal(lua.get(), func))
    {
        int n = 0;
        if (fmt)
        {
            va_list argp;       /// 定义可变参数指针
            va_start(argp, fmt);
            for (;;)
            {
                const char *e = strchr(fmt, '%');
                if (e == NULL)
                    break;

                switch (*(e + 1))
                {
                case 's': {  /* zero-terminated string */
                    const char *s = va_arg(argp, char *);
                    if (s == NULL) s = "";
                    lua_pushstring(lua.get(), s);
                    n++;
                    break;
                }
                case 'b': {  /* an 'int' as a character */
                    bool b = va_arg(argp, bool);
                    lua_pushboolean(lua.get(), b);
                    n++;
                    break;
                }
                case 'd': {  /* an 'int' */
                    int d = va_arg(argp, int);
                    lua_pushinteger(lua.get(), d);
                    n++;
                    break;
                }
                case 'f': {  /* a 'lua_Number' */
                    double db = va_arg(argp, double);
                    lua_pushnumber(lua.get(), db);
                    n++;
                    break;
                }
                case 'p': {  /* a pointer */
                    void *p = va_arg(argp, void *);
                    lua_pushlightuserdata(lua.get(), p);
                    n++;
                    break;
                }
                          //case 'I': {  /* a 'lua_Integer' */
                          //    setivalue(L->top, cast(lua_Integer, va_arg(argp, l_uacInt)));
                          //    goto top2str;
                          //}
                          //case 'U': {  /* an 'int' as a UTF-8 sequence */
                          //    char buff[UTF8BUFFSZ];
                          //    int l = luaO_utf8esc(buff, cast(long, va_arg(argp, long)));
                          //    pushstr(L, buff + UTF8BUFFSZ - l, l);
                          //    break;
                          //}
                          //case '%': {
                          //    pushstr(L, "%", 1);
                          //    break;
                          //}
                default: {
                    assert(0);
                    break;
                }
                }
                fmt = e + 2;
            }
            va_end(argp);
        }

        if (0 != lua_pcall(lua.get(), n, return_count, 0))
        {
            TRACE((std::string(lua_tostring(lua.get(), -1)) + "\r\n").c_str());
        }

        return true;
    }
    return false;
}
