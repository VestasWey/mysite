
// DateTimePickerTestDlg.h : 头文件
//

#pragma once
#include "afxdtctl.h"
#include "afxwin.h"
#include "explorer1.h"


// CDateTimePickerTestDlg 对话框
class CDateTimePickerTestDlg : public CDialogEx
{
// 构造
public:
	CDateTimePickerTestDlg(CWnd* pParent = NULL);	// 标准构造函数
    virtual ~CDateTimePickerTestDlg();

// 对话框数据
	enum { IDD = IDD_DATETIMEPICKERTEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
    virtual BOOL OnInitDialog();
    virtual BOOL DestroyWindow() override;
    virtual void OnOK() override;

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    CDateTimeCtrl m_DateTimeCtrl;
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    CButton btn_ok;
    CButton btn_cancel;
    CExplorer1 m_ieCtrl;
};
