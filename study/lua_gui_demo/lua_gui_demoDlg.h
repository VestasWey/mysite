
// lua_gui_demoDlg.h : 头文件
//

#pragma once
#include "lua.hpp"
#include <memory>

class Clua_gui_demoDlgAutoProxy;

namespace
{
    class ConsoleObject;
}

// Clua_gui_demoDlg 对话框
class Clua_gui_demoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(Clua_gui_demoDlg);
	friend class Clua_gui_demoDlgAutoProxy;

// 构造
public:
	Clua_gui_demoDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~Clua_gui_demoDlg();

// 对话框数据
	enum { IDD = IDD_LUA_GUI_DEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	Clua_gui_demoDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;

	BOOL CanExit();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()

private:
    BOOL m_bTracking;
    lua_State *m_lua;
    std::unique_ptr<ConsoleObject> console_;

    void ReloadPlugins();
public:
    virtual BOOL DestroyWindow();
    afx_msg void OnCommand(UINT uId);
    afx_msg void OnUpdateCommandUIRange(CCmdUI* uId);
    afx_msg void OnUpdateUIState(UINT /*nAction*/, UINT /*nUIElement*/);
};
