#pragma once


// CModalDialogTest 对话框

class CModalDialogTest : public CDialogEx
{
	DECLARE_DYNAMIC(CModalDialogTest)

public:
	CModalDialogTest(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModalDialogTest();

// 对话框数据
	enum { IDD = IDD_DATETIMEPICKERTEST_DIALOG };

    virtual void OnOK() override;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButton1();

private:
    static int inc_id_;
    int id_;
public:
    virtual BOOL OnInitDialog();
    virtual BOOL DestroyWindow() override;
};
