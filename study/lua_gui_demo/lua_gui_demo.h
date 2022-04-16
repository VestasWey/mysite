
// lua_gui_demo.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// Clua_gui_demoApp: 
// 有关此类的实现，请参阅 lua_gui_demo.cpp
//

class Clua_gui_demoApp : public CWinApp
{
public:
	Clua_gui_demoApp();

// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern Clua_gui_demoApp theApp;