#include "stdafx.h"
#include "GuiObjectProxy.h"
#include "BaseDialog.h"
#include "lua_gui_proxy.h"
#include "lua_object.h"
#include <windowsx.h>

namespace
{
    const char *kWidgetClassName = "widget";
    CBaseDialog *g_dialog = nullptr;

    void RefBaseDialogDeleter(CBaseDialog *dialog)
    {
        if (dialog)
        {
            if (dialog->GetSafeHwnd())
            {
                dialog->EndDialog(IDCANCEL);
            }
            delete dialog;
        }
    }
}

extern "C"
{
    int widget_gc(lua_State *lua)
    {
        //得到第一个传入的对象参数（在stack最底部）
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        if (wrapper)
        {
            delete (*wrapper);
        }
        return 0;
    }

    int widget_DoModal(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        int ret = (*wrapper)->get()->DoModal();

        lua_pushnumber(lua, ret);

        return 1;
    }

    int widget_EndDialog(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        if (lua_isinteger(lua, -1))
        {
            int ret = lua_tointeger(lua, -1);
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            (*wrapper)->get()->EndDialog(ret);
        }

        return 0;
    }

    int widget_Create(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        BOOL ret = (*wrapper)->get()->Create(CBaseDialog::IDD);
        (*wrapper)->get()->ShowWindow(SW_SHOW);

        lua_pushboolean(lua, ret);

        return 1;
    }

    int widget_DestroyWindow(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        BOOL ret = (*wrapper)->get()->DestroyWindow();

        lua_pushboolean(lua, ret);

        return 1;
    }

    int widget_IsDlgButtonChecked(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        bool checked = false;
        if (lua_isinteger(lua, -1))
        {
            int id = lua_tointeger(lua, -1);
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            checked = ((*wrapper)->get()->IsDlgButtonChecked(id) != 0);
        }

        lua_pushboolean(lua, checked);

        return 1;
    }

    int widget_SetDlgItemEnable(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        if (lua_isinteger(lua, -2) && lua_isboolean(lua, -1))
        {
            int id = lua_tointeger(lua, -2);
            int enable = lua_toboolean(lua, -1);
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            CWnd *pWnd = (*wrapper)->get()->GetDlgItem(id);
            if (pWnd)
            {
                pWnd->EnableWindow(enable);
            }
        }

        return 0;
    }

    int widget_Edit_GetText(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        CStringA astr;
        if (lua_isinteger(lua, -1))
        {
            int id = lua_tointeger(lua, -1);
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            CWnd *pWnd = (*wrapper)->get()->GetDlgItem(id);
            if (pWnd && strcmp(pWnd->GetRuntimeClass()->m_lpszClassName, "CEdit") == 0)
            {
                int len = Edit_GetTextLength(pWnd->m_hWnd);
                len++;
                wchar_t *buff = new wchar_t[len];
                Edit_GetText(pWnd->m_hWnd, buff, len);
                astr = CStringA(buff);
                delete buff;
            }
        }

        lua_pushstring(lua, astr.GetBuffer());
        astr.ReleaseBuffer();

        return 1;
    }

    int widget_Edit_AppendText(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        CStringA astr;
        if (lua_isinteger(lua, -2) && lua_isstring(lua, -1))
        {
            const char *text = lua_tostring(lua, -1);
            int id = lua_tointeger(lua, -2);
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            CWnd *pWnd = (*wrapper)->get()->GetDlgItem(id);
            if (pWnd && strcmp(pWnd->GetRuntimeClass()->m_lpszClassName, "CEdit") == 0)
            {
                CEdit *pEdit = (CEdit*)pWnd;
                Edit_SetSel(pWnd->m_hWnd, -1, -1);
                Edit_ReplaceSel(pWnd->m_hWnd, CString(text));
            }
        }

        return 0;
    }

    int widget_GetDlgItemText(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        CStringA astr;
        if (lua_isinteger(lua, -1))
        {
            int id = lua_tointeger(lua, -1);
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            CWnd *pWnd = (*wrapper)->get()->GetDlgItem(id);
            if (pWnd)
            {
                CString cstr;
                pWnd->GetWindowText(cstr);
                astr = CStringA(cstr);
            }
        }

        lua_pushstring(lua, astr.GetBuffer());
        astr.ReleaseBuffer();

        return 1;
    }

    int widget_SetDlgItemText(lua_State *lua)
    {
        RefBaseDialog **wrapper = (RefBaseDialog**)luaL_checkudata(lua, 1, kWidgetClassName);
        luaL_argcheck(lua, wrapper != NULL, 1, "invalid data");

        CStringA astr;
        if (lua_isinteger(lua, -2) && lua_isstring(lua, -1))
        {
            const char *text = lua_tostring(lua, -1);
            int id = lua_tointeger(lua, -2);
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            CWnd *pWnd = (*wrapper)->get()->GetDlgItem(id);
            if (pWnd)
            {
                pWnd->SetWindowText(CString(text));
            }
        }

        return 0;
    }

    luaL_Reg kWidgetMemberFuncs[] =
    {
        { "DoModal", widget_DoModal },
        { "EndDialog", widget_EndDialog },
        { "Create", widget_Create },
        { "DestroyWindow", widget_DestroyWindow },
        { "IsDlgButtonChecked", widget_IsDlgButtonChecked },
        { "SetDlgItemEnable", widget_SetDlgItemEnable },

        { "Edit_GetText", widget_Edit_GetText },
        { "Edit_AppendText", widget_Edit_AppendText },

        { "GetDlgItemText", widget_GetDlgItemText },
        { "SetDlgItemText", widget_SetDlgItemText },
        { "__gc", widget_gc },
        //{ "__tostring", widget_tostring },
        { nullptr, nullptr }
    };

    int WidgetDoModal(lua_State *lua)
    {
        int ret = -1;
        if (lua_isuserdata(lua, -1))
        {
            RefLuaState *L = (RefLuaState*)lua_touserdata(lua, -1);
            // 要在其他模块调用另一dll中的mfc资源有两种方式，实质是要使能从对应的模块加载那个模块自己的资源而不是用主exe的资源
            // 一、进行模块状态切换
            AFX_MANAGE_STATE(AfxGetStaticModuleState());
            CBaseDialog dlg(L, AfxGetApp()->m_pMainWnd);
            ret = dlg.DoModal();

            // 方式二
            //HINSTANCE save_hInstance = AfxGetResourceHandle();
            //AfxSetResourceHandle(theApp.m_hInstance);
            //CBaseDialog dlg(AfxGetApp()->m_pMainWnd);
            //dlg.DoModal();
            ////方法2的状态还原
            //AfxSetResourceHandle(save_hInstance);
        }
        lua_pushnumber(lua, ret);
        return 1;
    }

    int CreateWidget(lua_State *lua)
    {
        RefLuaState *L = (RefLuaState*)lua_touserdata(lua, -1);
        //创建一个对象指针放到stack里，返回给Lua中使用，userdata的位置-1
        RefBaseDialog **shareptr = (RefBaseDialog**)lua_newuserdata(lua, sizeof(RefBaseDialog*));
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        *shareptr = new RefBaseDialog(new CBaseDialog(L, AfxGetApp()->m_pMainWnd), RefBaseDialogDeleter);

        //Lua->stack，得到全局元表位置-1,userdata(proxy)位置-2
        luaL_getmetatable(lua, kWidgetClassName);
        //将元表赋值给位置-2的userdata(proxy)，并弹出-1的元表
        lua_setmetatable(lua, -2);

        return 1;
    }

    int ShowMessageBox(lua_State *lua)
    {
        if (lua_isstring(lua, -1))
        {
            AfxMessageBox(CString(lua_tostring(lua, -1)));
        }
        return 0;
    }

    luaL_Reg cFuntions[] =
    {
        { "DoModal", WidgetDoModal },
        { "CreateWidget", CreateWidget },
        { "MessageBox", ShowMessageBox },
        { nullptr, nullptr }
    };

    __declspec(dllexport) int luaopen_lua_gui_proxy(lua_State *lua)
    {
        //创建全局元表（里面包含了对LUA_REGISTRYINDEX的操作），元表的位置为-1
        luaL_newmetatable(lua, kWidgetClassName);
        //将元表作为一个副本压栈到位置-1，原元表位置-2
        lua_pushvalue(lua, -1);
        //设置-2位置元表的__index索引的值为位置-1的元表，并弹出位置-1的元表，原元表的位置为-1
        lua_setfield(lua, -2, "__index");
        //将成员函数注册到元表中（到这里，全局元表的设置就全部完成了）
        luaL_setfuncs(lua, kWidgetMemberFuncs, 0);

        lua_pushinteger(lua, IDCANCEL);
        lua_setglobal(lua, "IDCANCEL");

        // 注册该库的函数
        luaL_newlib(lua, cFuntions);

        return 1;
    }
}
