
// lua_gui_demoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "lua_gui_demo.h"
#include "lua_gui_demoDlg.h"
#include "DlgProxy.h"
#include "afxdialogex.h"
#include <io.h>
#include <fcntl.h>
#include <string>
#include <map>
#include <algorithm>
#include <vector>
#include <set>
#include "..\lua_gui_proxy\lua_object.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace
{
    class ConsoleObject
    {
    public:
        ConsoleObject()
            : m_sout(nullptr)
            , m_serr(nullptr)
        {
            int nCrt = 0;
            FILE* fp;
            AllocConsole();
            nCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
            fp = _fdopen(nCrt, "w"); //ON_COMMAND; WM_COMMAND
            *stdout = *fp;
            setvbuf(stdout, NULL, _IONBF, 0);

            /*AllocConsole();
            errno_t err;
            err = freopen_s(&m_sout, "CONOUT$", "w", stdout);
            err = freopen_s(&m_serr, "CONOUT$", "w", stderr);*/
        }

        ~ConsoleObject()
        {
            if (m_sout)
            {
                fclose(m_sout);
            }
            if (m_serr)
            {
                fclose(m_serr);
            }
        }

    protected:
    private:
        FILE* m_sout;
        FILE* m_serr;
    };

    enum
    {
        DYNAMIC_CTRL_ID_BEGIN = 1000,

        MENU_ID_BEGIN,
        MENU_ID_END = MENU_ID_BEGIN + 10000,

        DYNAMIC_CTRL_ID_END
    };
    
    const char* kInfosLua = "infos.lua";
    const char* kLogicLua = "logic.lua";
    const char* kLogicMainFunc = "main";

    struct plugin_info
    {
        plugin_info()
        {
            id = 0;
        }

        plugin_info(const plugin_info& right)
        {
            id = 1;

            plugin_folder = right.plugin_folder;
            menu_item_id_map = right.menu_item_id_map;
        }

        int id;
        std::string plugin_folder;
        std::map<UINT, int> menu_item_id_map;// 插件中菜单项在宿主菜单中的ID -> 插件中自己定义的菜单ID
    };

    UINT g_menu_id = MENU_ID_BEGIN;
    std::vector<plugin_info> g_plugins_vector;
}

// Clua_gui_demoDlg 对话框


IMPLEMENT_DYNAMIC(Clua_gui_demoDlg, CDialogEx);

Clua_gui_demoDlg::Clua_gui_demoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Clua_gui_demoDlg::IDD, pParent)
    , m_bTracking(FALSE)
    , m_lua(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pAutoProxy = NULL;
}

Clua_gui_demoDlg::~Clua_gui_demoDlg()
{
	// 如果该对话框有自动化代理，则
	//  将此代理指向该对话框的后向指针设置为 NULL，以便
	//  此代理知道该对话框已被删除。
	if (m_pAutoProxy != NULL)
		m_pAutoProxy->m_pDialog = NULL;

    if (m_lua)
    {
        lua_close(m_lua);
        m_lua = nullptr;
    }
}

void Clua_gui_demoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Clua_gui_demoDlg, CDialogEx)
    ON_WM_CLOSE()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_COMMAND_RANGE(DYNAMIC_CTRL_ID_BEGIN, DYNAMIC_CTRL_ID_END, OnCommand)
    ON_WM_UPDATEUISTATE()
    ON_UPDATE_COMMAND_UI_RANGE(MENU_ID_BEGIN, MENU_ID_END, OnUpdateCommandUIRange)
END_MESSAGE_MAP()


// Clua_gui_demoDlg 消息处理程序

BOOL Clua_gui_demoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

    //console_.reset(new ConsoleObject());

    // 第一种方式，将UI的布局配置和消息处理传给lua去处理
    // 宿主负责窗体的创建、绘制，此种方式应当只适用于将逻辑与布局分离而已
    // 不适用于程序插件需求
    m_lua = luaL_newstate();
    if (m_lua)
    {
        // 载入Lua基本库
        luaL_openlibs(m_lua);
    }
    //ListBox_AddItemData;
    ReloadPlugins();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BOOL Clua_gui_demoDlg::DestroyWindow()
{
    // TODO:  在此添加专用代码和/或调用基类

    return CDialogEx::DestroyWindow();
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Clua_gui_demoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Clua_gui_demoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 当用户关闭 UI 时，如果控制器仍保持着它的某个
//  对象，则自动化服务器不应退出。  这些
//  消息处理程序确保如下情形: 如果代理仍在使用，
//  则将隐藏 UI；但是在关闭对话框时，
//  对话框仍然会保留在那里。

void Clua_gui_demoDlg::OnClose()
{
	if (CanExit())
		CDialogEx::OnClose();
}

void Clua_gui_demoDlg::OnOK()
{
	/*if (CanExit())
		CDialogEx::OnOK();*/
    //RestoreGuiLua();
    //int ret = 0;
    //std::string str;
    //if (m_lua)
    //{
    //    if (LUA_TFUNCTION == lua_getglobal(m_lua, "domodal"))
    //    {
    //        lua_pushlightuserdata(m_lua, m_hWnd);
    //        ret = lua_pcall(m_lua, 1, 1, 0);
    //        ret = lua_tointeger(m_lua, -1);//IDOK
    //    }
    //}
    ReloadPlugins();
}

void Clua_gui_demoDlg::OnCancel()
{
	if (CanExit())
		CDialogEx::OnCancel();
}

BOOL Clua_gui_demoDlg::CanExit()
{
	// 如果代理对象仍保留在那里，则自动化
	//  控制器仍会保持此应用程序。
	//  使对话框保留在那里，但将其 UI 隐藏起来。
	if (m_pAutoProxy != NULL)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}

void Clua_gui_demoDlg::ReloadPlugins()
{
    if (!m_lua)
        return;

    lua_settop(m_lua, 0);
    g_menu_id = MENU_ID_BEGIN;
    g_plugins_vector.clear();

    HMENU hMenu = CreateMenu();

    TCHAR szFilePath[MAX_PATH] = { 0 }; // MAX_PATH
    //GetModuleFileName(NULL, szFilePath, MAX_PATH);
    //(_tcsrchr(szFilePath, _T('\\')))[1] = 0;//删除文件名，只获得路径
    CString plugins_folder(szFilePath);

    CFileFind finder;
    BOOL bWorking = finder.FindFile(plugins_folder + L"plugins\\*.*");
    {
        while (bWorking)
        {
            bWorking = finder.FindNextFile();
            CString plugin_path = finder.GetFilePath();
            if (!finder.IsDots() && finder.IsDirectory())
            {
                if (m_lua)
                {
                    CStringA lua_path = CStringA(plugin_path + L"\\") + kInfosLua;
                    if (LUA_OK == luaL_dofile(m_lua, lua_path))
                    {
                        int type = lua_getglobal(m_lua, "name");
                        if (LUA_TSTRING == type)
                        {
                            plugin_info plugin;
                            plugin.plugin_folder = CStringA(plugin_path);

                            std::string name = lua_tostring(m_lua, -1);
                            HMENU hPluginMenu = CreateMenu();
                            ::AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hPluginMenu, CString(name.c_str()));

                            type = lua_getglobal(m_lua, "menus");
                            if (LUA_TTABLE == type)
                            {
                                // 遍历纯粹数组
                                lua_Integer num = luaL_len(m_lua, -1);
                                for (lua_Integer item = 1; item <= num; ++item)
                                {
                                    type = lua_rawgeti(m_lua, -1, item);
                                    if (type == LUA_TTABLE)
                                    {
                                        bool separator = true;
                                        int id;
                                        std::string text;
                                        bool enable = true;
                                        type = lua_getfield(m_lua, -1, "id");
                                        if (LUA_TNUMBER == type)
                                        {
                                            id = lua_tointeger(m_lua, -1);
                                            separator = false;
                                        }
                                        lua_pop(m_lua, 1);

                                        type = lua_getfield(m_lua, -1, "text");
                                        if (LUA_TSTRING == type)
                                        {
                                            text = lua_tostring(m_lua, -1);
                                        }
                                        lua_pop(m_lua, 1);

                                        type = lua_getfield(m_lua, -1, "enable");
                                        if (LUA_TBOOLEAN == type)
                                        {
                                            enable = lua_toboolean(m_lua, -1);
                                        }
                                        lua_pop(m_lua, 1);

                                        if (!separator)
                                        {
                                            int mid = g_menu_id++;
                                            plugin.menu_item_id_map[mid] = id;

                                            ::AppendMenu(hPluginMenu, MF_STRING | enable ? MF_ENABLED : MF_DISABLED,
                                                mid, CString(text.c_str()));
                                        }
                                        else
                                        {
                                            ::AppendMenu(hPluginMenu, MF_SEPARATOR, 0, nullptr);
                                        }
                                    }
                                    lua_pop(m_lua, 1);
                                }
                            }
                            lua_pop(m_lua, 1);

                            g_plugins_vector.push_back(plugin);
                        }
                        lua_pop(m_lua, 1);
                    }
                    else
                    {
                        std::string str = lua_tostring(m_lua, -1);
                        TRACE(lua_tostring(m_lua, -1));
                    }
                }
            }
        }
    }

    if (hMenu)
    {
        ::SetMenu(m_hWnd, hMenu);
    }
}

void Clua_gui_demoDlg::OnCommand(UINT uId)
{
    if (MENU_ID_BEGIN <= uId && MENU_ID_END >= uId)
    {
        for (plugin_info &var : g_plugins_vector)
        {
            if (var.menu_item_id_map.find(uId) != var.menu_item_id_map.end())
            {
                if (m_lua)
                {
                    lua_settop(m_lua, 1);
                    if (LUA_OK == luaL_dofile(m_lua, (var.plugin_folder + "\\" + kInfosLua).c_str()))
                    {
                        if (LUA_TFUNCTION == lua_getglobal(m_lua, "OnMenuSelected"))
                        {
                            lua_pushnumber(m_lua, (LUA_NUMBER)var.menu_item_id_map[uId]);
                            lua_pcall(m_lua, 1, 1, 0);

                            if (lua_isstring(m_lua, -1))
                            {
                                std::string dst_view = lua_tostring(m_lua, -1);
                                if (!dst_view.empty())
                                {
                                    RefLuaState lua = make_shared_lua_State();
                                    if (lua)
                                    {
                                        luaL_openlibs(lua.get());

                                        std::string path = var.plugin_folder + "\\" + dst_view + "\\" + kLogicLua;

                                        if (LUA_OK == luaL_dofile(lua.get(), path.c_str()))
                                        {
                                            if (LUA_TFUNCTION == lua_getglobal(lua.get(), kLogicMainFunc))
                                            {
                                                lua_pushlightuserdata(lua.get(), &lua);
                                                lua_pcall(lua.get(), 1, 1, 0);
                                                if (lua_isnumber(lua.get(), -1))
                                                {
                                                    if (-1 == lua_tointeger(lua.get(), -1))
                                                    {
                                                        AfxMessageBox(L"open plugin view faild!");
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            TRACE(lua_tostring(lua.get(), -1));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}

void Clua_gui_demoDlg::OnUpdateCommandUIRange(CCmdUI* uId)
{
    int id = uId->m_nID;
}

void Clua_gui_demoDlg::OnUpdateUIState(UINT /*nAction*/, UINT /*nUIElement*/)
{
    // 该功能要求使用 Windows 2000 或更高版本。
    // 符号 _WIN32_WINNT 和 WINVER 必须 >= 0x0500。
    // TODO:  在此处添加消息处理程序代码
}
