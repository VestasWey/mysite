
// DlgProxy.cpp : 实现文件
//

#include "stdafx.h"
#include "lua_gui_demo.h"
#include "DlgProxy.h"
#include "lua_gui_demoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Clua_gui_demoDlgAutoProxy

IMPLEMENT_DYNCREATE(Clua_gui_demoDlgAutoProxy, CCmdTarget)

Clua_gui_demoDlgAutoProxy::Clua_gui_demoDlgAutoProxy()
{
	EnableAutomation();
	
	// 为使应用程序在自动化对象处于活动状态时一直保持 
	//	运行，构造函数调用 AfxOleLockApp。
	AfxOleLockApp();

	// 通过应用程序的主窗口指针
	//  来访问对话框。  设置代理的内部指针
	//  指向对话框，并设置对话框的后向指针指向
	//  该代理。
	ASSERT_VALID(AfxGetApp()->m_pMainWnd);
	if (AfxGetApp()->m_pMainWnd)
	{
		ASSERT_KINDOF(Clua_gui_demoDlg, AfxGetApp()->m_pMainWnd);
		if (AfxGetApp()->m_pMainWnd->IsKindOf(RUNTIME_CLASS(Clua_gui_demoDlg)))
		{
			m_pDialog = reinterpret_cast<Clua_gui_demoDlg*>(AfxGetApp()->m_pMainWnd);
			m_pDialog->m_pAutoProxy = this;
		}
	}
}

Clua_gui_demoDlgAutoProxy::~Clua_gui_demoDlgAutoProxy()
{
	// 为了在用 OLE 自动化创建所有对象后终止应用程序，
	//	析构函数调用 AfxOleUnlockApp。
	//  除了做其他事情外，这还将销毁主对话框
	if (m_pDialog != NULL)
		m_pDialog->m_pAutoProxy = NULL;
	AfxOleUnlockApp();
}

void Clua_gui_demoDlgAutoProxy::OnFinalRelease()
{
	// 释放了对自动化对象的最后一个引用后，将调用
	// OnFinalRelease。  基类将自动
	// 删除该对象。  在调用该基类之前，请添加您的
	// 对象所需的附加清理代码。

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(Clua_gui_demoDlgAutoProxy, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(Clua_gui_demoDlgAutoProxy, CCmdTarget)
END_DISPATCH_MAP()

// 注意: 我们添加了对 IID_Ilua_gui_demo 的支持
//  以支持来自 VBA 的类型安全绑定。  此 IID 必须同附加到 .IDL 文件中的
//  调度接口的 GUID 匹配。

// {C720E8CB-F20E-427B-8D0F-50FBB9279154}
static const IID IID_Ilua_gui_demo =
{ 0xC720E8CB, 0xF20E, 0x427B, { 0x8D, 0xF, 0x50, 0xFB, 0xB9, 0x27, 0x91, 0x54 } };

BEGIN_INTERFACE_MAP(Clua_gui_demoDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(Clua_gui_demoDlgAutoProxy, IID_Ilua_gui_demo, Dispatch)
END_INTERFACE_MAP()

// IMPLEMENT_OLECREATE2 宏在此项目的 StdAfx.h 中定义
// {74EFA0ED-FA22-45B1-B6E5-F724269C1ED3}
IMPLEMENT_OLECREATE2(Clua_gui_demoDlgAutoProxy, "lua_gui_demo.Application", 0x74efa0ed, 0xfa22, 0x45b1, 0xb6, 0xe5, 0xf7, 0x24, 0x26, 0x9c, 0x1e, 0xd3)


// Clua_gui_demoDlgAutoProxy 消息处理程序
