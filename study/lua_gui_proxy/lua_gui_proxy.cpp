// lua_gui_proxy.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "lua_gui_proxy.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO:  如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如: 
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。  这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// Clua_gui_proxyApp

BEGIN_MESSAGE_MAP(Clua_gui_proxyApp, CWinApp)
END_MESSAGE_MAP()


// Clua_gui_proxyApp 构造

Clua_gui_proxyApp::Clua_gui_proxyApp()
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 Clua_gui_proxyApp 对象

Clua_gui_proxyApp theApp;


// Clua_gui_proxyApp 初始化

BOOL Clua_gui_proxyApp::InitInstance()
{
    // 如果一个运行在 Windows XP 上的应用程序清单指定要
    // 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
    //则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
    //INITCOMMONCONTROLSEX InitCtrls;
    //InitCtrls.dwSize = sizeof(InitCtrls);
    //// 将它设置为包括所有要在应用程序中使用的
    //// 公共控件类。
    //InitCtrls.dwICC = ICC_WIN95_CLASSES;
    //InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	return TRUE;
}
