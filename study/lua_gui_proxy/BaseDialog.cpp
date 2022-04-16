// BaseDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "lua_gui_proxy.h"
#include "BaseDialog.h"
#include "afxdialogex.h"
#include <string>

namespace
{
    const char* kLayoutLua = "layout.lua";
    const char* kLogicLua = "logic.lua";
    const char* kLogicMainFunc = "main";

    const char* kTitleField = "title";
    const char* kIconField = "icon";
    const char* kWidthField = "width";
    const char* kHeightField = "height";
    const char* kLayoutField = "layout";
}
// CBaseDialog 对话框

IMPLEMENT_DYNAMIC(CBaseDialog, CDialogEx)

CBaseDialog::CBaseDialog(RefLuaState *lua, CWnd* pParent /*=NULL*/)
	: CDialogEx(CBaseDialog::IDD, pParent)
    , m_lua(*lua)
    , m_bTracking(FALSE)
{
}

CBaseDialog::~CBaseDialog()
{
}

void CBaseDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBaseDialog, CDialogEx)
    ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()


// CBaseDialog 消息处理程序


BOOL CBaseDialog::DestroyWindow()
{
    // TODO:  在此添加专用代码和/或调用基类
    bool bret = CDialogEx::DestroyWindow();

    RemoveAllChild();

    return bret;
}

BOOL CBaseDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    Layout();

    //ListBox_AddItemData

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常:  OCX 属性页应返回 FALSE
}

void CBaseDialog::RemoveAllChild()
{
    for (UINT i = 0; i < m_ctrls.GetSize(); i++)
    {
        delete m_ctrls[i];
    }
    m_ctrls.RemoveAll();
}

CWnd* CBaseDialog::CreateCrtl(const ctrl_info &info)
{
    CWnd *pCtrl = nullptr;
    if (info.type == "static")
    {
        pCtrl = new CStatic();
        ((CStatic*)pCtrl)->Create(CString(info.text.c_str()), WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE,
            CRect(info.x, info.y, info.x + info.width, info.y + info.height), this, info.id);
    }
    else if (info.type == "check")
    {
        pCtrl = new CButton();
        ((CButton*)pCtrl)->Create(CString(info.text.c_str()), WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX,
            CRect(info.x, info.y, info.x + info.width, info.y + info.height), this, info.id);
    }
    else if (info.type == "radio")
    {
        pCtrl = new CButton();
        ((CButton*)pCtrl)->Create(CString(info.text.c_str()), WS_CHILD | WS_VISIBLE | /*BS_RADIOBUTTON |*/ BS_AUTORADIOBUTTON,
            CRect(info.x, info.y, info.x + info.width, info.y + info.height), this, info.id);
    }
    else if (info.type == "edit")
    {
        pCtrl = new CEdit();
        ((CEdit*)pCtrl)->Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_WANTRETURN,
            CRect(info.x, info.y, info.x + info.width, info.y + info.height), this, info.id);
    }
    else if (info.type == "button")
    {
        pCtrl = new CButton();
        ((CButton*)pCtrl)->Create(CString(info.text.c_str()), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            CRect(info.x, info.y, info.x + info.width, info.y + info.height), this, info.id);
    }
    else
    {
    }
    return pCtrl;
}

void CBaseDialog::Layout()
{
    RemoveAllChild();

    if (m_lua.get())
    {
        std::string title;
        std::string icon;
        int width = 500;
        int height = 500;
        if (LUA_TSTRING == lua_getglobal(m_lua.get(), kTitleField))
        {
            title = lua_tostring(m_lua.get(), -1);
        }
        lua_pop(m_lua.get(), 1);
        if (LUA_TSTRING == lua_getglobal(m_lua.get(), kIconField))
        {
            icon = lua_tostring(m_lua.get(), -1);
        }
        lua_pop(m_lua.get(), 1);
        if (LUA_TNUMBER == lua_getglobal(m_lua.get(), kWidthField))
        {
            width = lua_tointeger(m_lua.get(), -1);
        }
        lua_pop(m_lua.get(), 1);
        if (LUA_TNUMBER == lua_getglobal(m_lua.get(), kHeightField))
        {
            height = lua_tointeger(m_lua.get(), -1);
        }
        lua_pop(m_lua.get(), 1);

        if (!title.empty())
        {
            SetWindowText(CString(title.c_str()));
        }
        if (!icon.empty())
        {
        }
        SetWindowPos(nullptr, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);

        if (LUA_TTABLE == lua_getglobal(m_lua.get(), kLayoutField))
        {
            if (lua_istable(m_lua.get(), -1))
            {
                lua_Integer num = luaL_len(m_lua.get(), -1);
                for (lua_Integer i = 1; i <= num; ++i)
                {
                    int tp = lua_rawgeti(m_lua.get(), -1, i);
                    if (tp == LUA_TTABLE)
                    {
                        ctrl_info info;
                        if (LUA_TSTRING == lua_getfield(m_lua.get(), -1, "type"))
                        {
                            info.type = lua_tostring(m_lua.get(), -1);
                        }
                        if (LUA_TNUMBER == lua_getfield(m_lua.get(), -2, "id"))
                        {
                            info.id = lua_tointeger(m_lua.get(), -1);
                        }
                        if (LUA_TSTRING == lua_getfield(m_lua.get(), -3, "text"))
                        {
                            info.text = lua_tostring(m_lua.get(), -1);
                        }
                        if (LUA_TNUMBER == lua_getfield(m_lua.get(), -4, "x"))
                        {
                            info.x = lua_tointeger(m_lua.get(), -1);
                        }
                        if (LUA_TNUMBER == lua_getfield(m_lua.get(), -5, "y"))
                        {
                            info.y = lua_tointeger(m_lua.get(), -1);
                        }
                        if (LUA_TNUMBER == lua_getfield(m_lua.get(), -6, "width"))
                        {
                            info.width = lua_tointeger(m_lua.get(), -1);
                        }
                        if (LUA_TNUMBER == lua_getfield(m_lua.get(), -7, "height"))
                        {
                            info.height = lua_tointeger(m_lua.get(), -1);
                        }
                        lua_pop(m_lua.get(), 7);

                        CWnd *pCtrl = CreateCrtl(info);
                        if (pCtrl)
                        {
                            m_ctrls.Add(pCtrl);
                        }
                    }
                    lua_pop(m_lua.get(), 1);
                }
            }
        }
        lua_pop(m_lua.get(), 1);
    }
}

BOOL CBaseDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    // TODO:  在此添加专用代码和/或调用基类
    if (m_lua)
    {
        if (!call_lua_func(m_lua, "OnCommand", 0, 
            "%d %d %b %p", LOWORD(wParam), HIWORD(wParam)))
        {
            TRACE("no OnCommand lua fun \r\n");
        }
    }
    if (GetSafeHwnd())
    {
        return CDialogEx::OnCommand(wParam, lParam);
    }
    return TRUE;
}

LRESULT CBaseDialog::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
    if (m_lua)
    {
        if (!call_lua_func(m_lua, "OnKickIdle", 0, nullptr))
        {
            TRACE("no OnKickIdle lua fun \r\n");
        }
    }
    return 0;
}

void CBaseDialog::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO:  在此添加消息处理程序代码和/或调用默认值
    if (!m_bTracking)
    {
        //   鼠标移入窗时，请求WM_MOUSEHOVER和WM_MOUSELEAVE 消息  
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(tme);
        tme.hwndTrack = m_hWnd;
        tme.dwFlags = TME_LEAVE | TME_HOVER;
        tme.dwHoverTime = 1;
        m_bTracking = _TrackMouseEvent(&tme);
    }

    call_lua_func(m_lua, "OnMouseMove", 0, "%d,%d", point.x, point.y);

    CDialogEx::OnMouseMove(nFlags, point);
}

void CBaseDialog::OnMouseLeave()
{
    // TODO:  在此添加消息处理程序代码和/或调用默认值
    m_bTracking = FALSE;

    call_lua_func(m_lua, "OnMouseExit", 0, nullptr);

    CDialogEx::OnMouseLeave();
}
