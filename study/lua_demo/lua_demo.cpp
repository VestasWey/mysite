// lua_demo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <process.h>

#include "lua.hpp"

#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <winuser.rh>

#include "lua_proxy/LuaProxy.h"

void stackDump(lua_State* L)
{
    std::cout << "begin dump lua stack" << std::endl;
    int i = 0;
    int top = lua_gettop(L);
    for (i = 1; i <= top; ++i)
    {
        int t = lua_type(L, i);
        switch (t)
        {
        case LUA_TSTRING: {
            printf("'%s' \n", lua_tostring(L, i));
        }
                          break;
        case LUA_TBOOLEAN: {
            printf(lua_toboolean(L, i) ? "true \n" : "false \n");
        }
                           break;
        case LUA_TNUMBER: {
            printf("%g \n", lua_tonumber(L, i));
        }
                          break;
        default: {
            printf("%s \n", lua_typename(L, t));
        }
                 break;
        }
    }
    std::cout << "end dump lua stack" << std::endl;
}

int lua_call_cpp_fn(lua_State *lua)
{
    printf("c++ lua_call_cpp_fn param order\n");
    //stackDump(lua);

    lua_pushnumber(lua, 22);
    lua_pushnumber(lua, 33);
    return 2;
}

bool test_lua()
{
    bool ret = false;
    static lua_State *lua = luaL_newstate();
    if (lua)
    {
        // 载入Lua基本库
        luaL_openlibs(lua);

        // lua中有调用到宿主函数的要先注册函数
        lua_register(lua, "lua_call_cpp_fn", lua_call_cpp_fn);

        // 加载并执行脚本
        int code = luaL_dofile(lua, "lua_script/test.lua");
        if (LUA_OK == code)
        {
            std::cout << "--------------cpp begin----------------" << std::endl;

            // 拿全局元表方法一
            //lua_getfield(lua, LUA_REGISTRYINDEX, "CLuaProxy");
            // 拿全局元表方法二
            lua_pushstring(lua, "CLuaProxy");            
            lua_gettable(lua, LUA_REGISTRYINDEX);
            if (lua_istable(lua, -1))
            {
                lua_getfield(lua, -1, "NotMenberFn");
                if (lua_isfunction(lua, -1))
                {
                    lua_pcall(lua, 0, 0, 0); // 无效
                }
            }

            //读取变量-------------------------------------------------------------
            lua_getglobal(lua, "g_member");   //string to be indexed
            std::cout << "g_member = " << lua_tostring(lua, -1) << std::endl;

            lua_getglobal(lua, "proxy_raw_obj");
            if (lua_isuserdata(lua, -1))
            {
                // 一,lua full userdata
                CLuaProxy **proxy = (CLuaProxy**)lua_touserdata(lua, -1);

                // 二，lua light userdata
                //CLuaProxy *proxy = (CLuaProxy*)lua_touserdata(lua, -1);

                std::cout << "proxy_raw_obj = " << (*proxy)->ct() << std::endl;

                (*proxy)->Release();
            }

            //读取表，key-value---------------------------------------------------
            lua_getglobal(lua, "g_table");  //table to be indexed
            if (lua_istable(lua, -1))
            {
                lua_Integer num = luaL_len(lua, -1);

                //根据已知key取表中元素
                lua_getfield(lua, -1, "name");
                std::cout << "g_table->name = " << lua_tostring(lua, -1) << std::endl;
                lua_pop(lua, 1);

                // 遍历table的key为混合类型的哈希表
                lua_pushnil(lua); // 将第一个遍历的key压栈
                //lua_pushstring(lua, "name");  // 从key='name'的下一个k-v键值对开始
                //lua_pushinteger(lua, 10101);// 从key=10101的下一个k-v键值对开始
                while (lua_next(lua, -2))   // 弹出位置为-1的key，将该key的下一个key-value对压栈，key=-2，value=-1，并返回非0
                {
                    int k_type = lua_type(lua, -2);
                    switch (k_type)
                    {
                    case LUA_TNUMBER:
                        if (lua_isinteger(lua, -2))
                        {
                            printf("g_table key = %lld(integer), ", lua_tointeger(lua, -2));
                        }
                        else if (lua_isnumber(lua, -2))
                        {
                            printf("g_table key = %g(number), ", lua_tonumber(lua, -2));
                        }
                        break;
                    case LUA_TSTRING:
                        printf("g_table key = '%s'(string), ", lua_tostring(lua, -2));
                        break;
                    default:
                        printf("g_table key type %s, ", lua_typename(lua, k_type));
                        break;
                    }

                    int v_type = lua_type(lua, -1);
                    switch (v_type)
                    {
                    case LUA_TNUMBER:
                        if (lua_isinteger(lua, -1))
                        {
                            printf("value = %lld(integer)\n", lua_tointeger(lua, -1));
                        }
                        else if (lua_isnumber(lua, -1))
                        {
                            printf("value = %g(number)\n", lua_tonumber(lua, -1));
                        }
                        break;
                    case LUA_TSTRING:
                        printf("value = '%s'(string)\n", lua_tostring(lua, -1));
                        break;
                    default:
                        printf("value type %s \n", lua_typename(lua, v_type));
                        break;
                    }

                    lua_pop(lua, 1);// 将value出栈，此时当前key位置为-1，下一次lua_next以该key为基准位置向下遍历
                }

                // 对于混合类型的哈希table，若key类型是整形，则c++层可以像取数组值一样将key当做数组索引来取（似乎没什么意义）
                // 若人为指定key=1，则仅对数组生效的luaL_len将得到从key=1开始的连续键值的数量（不管table中到底有多少键值）
                int type = lua_rawgeti(lua, -1, 10101);
                if (lua_istable(lua, -1))
                {
                    std::cout << "g_table->10101 is table " << std::endl;
                }
                lua_pop(lua, 1);
            }

            //修改表中元素
            lua_pushstring(lua, "bean");
            lua_setfield(lua, -2, "name");
            lua_getfield(lua, -1, "name");
            std::cout << "new table->name = " << lua_tostring(lua, -1) << std::endl;
            lua_pop(lua, 1);

            // 创建key-value哈希table
            lua_newtable(lua);
            lua_pushinteger(lua, 22);
            lua_setfield(lua, -2, "p1");

            lua_pushnumber(lua, 33.0f);
            lua_setfield(lua, -2, "p2");

            lua_pushstring(lua, "c++ string");
            lua_setfield(lua, -2, "p3");

            lua_setglobal(lua, "cpp_map");

            // 读取数组---------------------------------------------------------------
            lua_getglobal(lua, "g_vector");  //table to be indexed
            if (lua_istable(lua, -1))
            {
                // 遍历纯粹数组
                lua_Integer num = luaL_len(lua, -1);
                for (lua_Integer i = 1; i <= num; ++i)
                {
                    int type = lua_rawgeti(lua, -1, i);
                    switch (type)
                    {
                    case LUA_TNUMBER:
                        if (lua_isinteger(lua, -1))
                        {
                            printf("g_vector[%lld] lua_isinteger %lld\n", i, lua_tointeger(lua, -1));
                        }
                        else if (lua_isnumber(lua, -1))
                        {
                            printf("g_vector[%lld] lua_isnumber %g\n", i, lua_tonumber(lua, -1));
                        }
                        break;
                    case LUA_TSTRING:
                        printf("g_vector[%lld] lua_isstring %s\n", i, lua_tostring(lua, -1));
                        break;
                    default:
                        printf("g_vector[%lld] type is %s \n", i, lua_typename(lua, type));
                        break;
                    }
                    lua_pop(lua, 1);
                }

                // 对于纯数组，其实lua默认其key type为integer，并且从1递增
                lua_pushnil(lua); // 将第一个遍历的key压栈
                while (lua_next(lua, -2))   // 弹出位置为-1的key，将该key的下一个key-value对压栈，key=-2，value=-1，并返回非0
                {
                    int k_type = lua_type(lua, -2);
                    switch (k_type)
                    {
                    case LUA_TNUMBER:
                        if (lua_isinteger(lua, -2))
                        {
                            printf("g_vector key = %lld(integer), ", lua_tointeger(lua, -2));
                        }
                        else if (lua_isnumber(lua, -2))
                        {
                            printf("g_vector key = %g(number), ", lua_tonumber(lua, -2));
                        }
                        break;
                    case LUA_TSTRING:
                        printf("g_vector key = '%s'(string), ", lua_tostring(lua, -2));
                        break;
                    default:
                        printf("g_vector key type %s, ", lua_typename(lua, k_type));
                        break;
                    }

                    int v_type = lua_type(lua, -1);
                    switch (v_type)
                    {
                    case LUA_TNUMBER:
                        if (lua_isinteger(lua, -1))
                        {
                            printf("value = %lld(integer)\n", lua_tointeger(lua, -1));
                        }
                        else if (lua_isnumber(lua, -1))
                        {
                            printf("value = %g(number)\n", lua_tonumber(lua, -1));
                        }
                        break;
                    case LUA_TSTRING:
                        printf("value = '%s'(string)\n", lua_tostring(lua, -1));
                        break;
                    default:
                        printf("value type %s \n", lua_typename(lua, v_type));
                        break;
                    }

                    lua_pop(lua, 1);// 将value出栈，此时当前key位置为-1，下一次lua_next以该key为基准位置向下遍历
                }
            }

            // 创建数组
            lua_newtable(lua);
            lua_pushinteger(lua, 22);
            lua_rawseti(lua, -2, 1);

            lua_pushnumber(lua, 33.0f);
            lua_rawseti(lua, -2, 2);

            lua_pushstring(lua, "c++ string");
            lua_rawseti(lua, -2, 3);

            lua_setglobal(lua, "cpp_vector");

            // 取函数-----------------------------------------------------------
            std::cout << "c++ call lua check_cpp_global" << std::endl;
            lua_getglobal(lua, "check_cpp_global");
            lua_pcall(lua, 0, 0, 0);

            std::cout << "c++ call lua add" << std::endl;
            lua_getglobal(lua, "add");
            lua_pushnumber(lua, 22);
            lua_pushnumber(lua, 33);
            lua_pcall(lua, 2, 1, 0);//2-参数个数，1-返回值个数，调用函数，函数执行完，会将返回值压入栈中
            std::cout << "add fn result = " << lua_tonumber(lua, -1) << std::endl;

            std::cout << "c++ call lua attch_proxy" << std::endl;
            CLuaProxy *proxy = new CLuaProxy();
            lua_getglobal(lua, "attch_proxy");
            lua_pushlightuserdata(lua, proxy);
            lua_pcall(lua, 1, 0, 0);//2-参数个数，1-返回值个数，调用函数，函数执行完，会将返回值压入栈中

            //// 从宿主注册的函数通过getglobal是拿不到的
            //if(LUA_OK == lua_getglobal(lua, "lua_call_cpp_fn"))
            //{
            //    std::cout << "c++ call lua_call_cpp_fn" << std::endl;
            //    lua_pushnumber(lua, 11);
            //    lua_pushnumber(lua, 22);
            //    lua_pcall(lua, 0, 1, 0);//2-参数格式，1-返回值个数，调用函数，函数执行完，会将返回值压入栈中
            //}

            proxy->Release();

            std::cout << "--------------cpp end----------------" << std::endl;

            ////查看栈
            //stackDump(lua);

            ////查看栈
            //stackDump(lua);

            //lua_pcall(lua, 0, LUA_MULTRET, 0);

            ret = true;
        }
        else
        {
            std::cout << lua_tostring(lua, -1) << std::endl;
        }

        //关闭state
        //lua_close(lua);
    }
    return ret;
}

int _tmain(int argc, _TCHAR* argv[])
{
    /*MyClass<int> mc;
    int a = mc.add(1, 2);
    int b = sub<int>(2, 1);
    int c = nor(2 , 2);*/

    do
    {
        test_lua();
    } while (/*getchar()*//*getc(stdin)*/_getche() != VK_ESCAPE);
	return 0;
}

