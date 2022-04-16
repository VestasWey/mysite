
// DlgProxy.h: 头文件
//

#pragma once

class Clua_gui_demoDlg;


// Clua_gui_demoDlgAutoProxy 命令目标

class Clua_gui_demoDlgAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(Clua_gui_demoDlgAutoProxy)

	Clua_gui_demoDlgAutoProxy();           // 动态创建所使用的受保护的构造函数

// 特性
public:
	Clua_gui_demoDlg* m_pDialog;

// 操作
public:

// 重写
	public:
	virtual void OnFinalRelease();

// 实现
protected:
	virtual ~Clua_gui_demoDlgAutoProxy();

	// 生成的消息映射函数

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(Clua_gui_demoDlgAutoProxy)

	// 生成的 OLE 调度映射函数

	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

