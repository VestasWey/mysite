
// DateTimePickerTestDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DateTimePickerTest.h"
#include "DateTimePickerTestDlg.h"
#include "afxdialogex.h"
#include <Gdiplus.h>
#include "ModalDialogTest.h"

#pragma comment(lib, "gdiplus.lib")

class CGdiplusObject
{
public:
    CGdiplusObject()
    {
        m_Status = Gdiplus::GdiplusStartup(&m_pGdiToken, &m_gdiplusStartupInput, NULL);
    }

    ~CGdiplusObject()
    {
        if (Gdiplus::Ok == m_Status)
        {
            Gdiplus::GdiplusShutdown(m_pGdiToken);
        }
    }

    Gdiplus::Status StartupStatus(){ return m_Status; }

private:
    Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;
    ULONG_PTR m_pGdiToken;
    Gdiplus::Status m_Status;
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const COLORREF clr_trans = RGB(0, 0, 255);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDateTimePickerTestDlg 对话框



CDateTimePickerTestDlg::CDateTimePickerTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDateTimePickerTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CDateTimePickerTestDlg::~CDateTimePickerTestDlg()
{
}

void CDateTimePickerTestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DATETIMEPICKER1, m_DateTimeCtrl);
    DDX_Control(pDX, IDOK, btn_ok);
    DDX_Control(pDX, IDCANCEL, btn_cancel);
    DDX_Control(pDX, IDC_EXPLORER1, m_ieCtrl);
}

BEGIN_MESSAGE_MAP(CDateTimePickerTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_CTLCOLOR()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CDateTimePickerTestDlg 消息处理程序

BOOL CDateTimePickerTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

    static CGdiplusObject obj;
	// TODO:  在此添加额外的初始化代码
    LONG drag_frame_saved_window_ex_style_ = ::GetWindowLong(GetSafeHwnd(), GWL_EXSTYLE);
    ::SetWindowLong(GetSafeHwnd(), GWL_EXSTYLE, drag_frame_saved_window_ex_style_ | WS_EX_LAYERED);
    
    ::SetLayeredWindowAttributes(GetSafeHwnd(), clr_trans, 255, LWA_COLORKEY/*LWA_ALPHA*/);

    CImage image;
    HRESULT ret = image.Load(L"pic-upload.png");
    //HRESULT ret = image.Load(L"ng_complete.png");
    HDC hdc = ::GetDC(GetSafeHwnd());
    BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

    RECT wr;
    ::GetWindowRect(GetSafeHwnd(), &wr);
    SIZE size = { wr.right - wr.left, wr.bottom - wr.top };

    HDC dib_dc = ::CreateCompatibleDC(hdc);
    HBITMAP bmp = ::CreateCompatibleBitmap(hdc, size.cx, size.cy);
    ::SelectObject(dib_dc, bmp);
    //image.AlphaBlend(dib_dc, CPoint(), 255, 0);
    //image.TransparentBlt(dib_dc, CRect(0, 0, image.GetWidth(), image.GetHeight()), RGB(255, 0, 0));
    //image.BitBlt(dib_dc, 0, 0);

    CRect rtOk;
    btn_ok.GetWindowRect(rtOk);

    // GDI+绘制
    {
        Gdiplus::Bitmap bitmap(size.cx, size.cy);
        Gdiplus::Graphics graphics(dib_dc);
    //    
    //    // 1、直接改图片buffer
    //    /*Gdiplus::BitmapData bd;
    //    bitmap.LockBits(&Gdiplus::Rect(0, 0, 150, 100), Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &bd);
    //    BYTE* pDstPointer = (BYTE*)bd.Scan0;
    //    BYTE *pixel, *a, *r, *g, *b;
    //    for (int i = 0; i < bd.Height; i++)
    //    {
    //        pixel = pDstPointer;
    //        for (int j = 0; j < bd.Stride; j += 4)
    //        {
    //            a = pixel + 3;
    //            r = pixel + 2;
    //            g = pixel + 1;
    //            b = pixel + 0;
    //            *a = 255;
    //            *r = 255;
    //            *g = 0;
    //            *b = 0;

    //            pixel += 4;
    //        }
    //        pDstPointer += bd.Stride;
    //    }
    //    bitmap.UnlockBits(&bd);*/

    //    // 2、逐像素改图片色值
    //    /*for (int i = 0; i < 150; i++)
    //    {
    //        for (int j = 0; j < 100; j++)
    //        {
    //            bitmap.SetPixel(i, j, Gdiplus::Color(255, 0, 0));
    //        }
    //    }*/

    //    // 3、先画好图再把图画到目标DC
        Gdiplus::Graphics bmp_graphics(&bitmap);
        /*bmp_graphics.FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(1, 255, 0, 0)), 
            0, 0, size.cx, size.cy);
        bmp_graphics.DrawRectangle(&Gdiplus::Pen(Gdiplus::Color(255, 0, 0)), 
            0, 0, size.cx - 1, size.cy - 1);
        bmp_graphics.FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0)), 165, 125, 150, 100);
        bmp_graphics.FillRectangle(&Gdiplus::SolidBrush(Gdiplus::Color(1, 255, 0, 255)), 
            rtOk.left, rtOk.top, rtOk.Width(), rtOk.Height());
        bmp_graphics.DrawRectangle(&Gdiplus::Pen(Gdiplus::Color(255, 0, 0)), 
            rtOk.left, rtOk.top, rtOk.Width(), rtOk.Height());*/

        Gdiplus::Region region(Gdiplus::Rect(0, 0, size.cx, size.cy));
        region.Xor(Gdiplus::Rect(100, 100, size.cx - 200, size.cy - 200));
        //Gdiplus::Image img(L"pic-upload.png");
        //bmp_graphics.DrawImage(&img, Gdiplus::Rect(0, 0, size.cx, size.cy), 0, 0, img.GetWidth(), img.GetHeight(), Gdiplus::UnitPixel);
        //bmp_graphics.SetCompositingMode(Gdiplus::CompositingMode::CompositingModeSourceCopy);
        bmp_graphics.FillRegion(&Gdiplus::SolidBrush(Gdiplus::Color(100, 255, 0, 0)), &region);
        
        /*Gdiplus::CachedBitmap cb(&bitmap, &graphics);
        graphics.DrawCachedBitmap(&cb, 0, 0);*/
        graphics.DrawImage(&bitmap, 0, 0);
    }

    // GDI绘制不能实现不透明
    {
        //HDC fill_dc = ::CreateCompatibleDC(hdc);
        //HBITMAP fill_bmp = ::CreateCompatibleBitmap(hdc, 150, 100);
        //::SelectObject(fill_dc, fill_bmp);

        //::FillRect(fill_dc, CRect(0, 0, 150, 100), (HBRUSH)::CreateSolidBrush(RGB(255, 0, 0)));
        //::FrameRect(fill_dc, CRect(0, 0, 150, 100), (HBRUSH)::CreateSolidBrush(RGB(0, 0, 255)));
        ////::AlphaBlend(dib_dc, 0, 0, 150, 100, fill_dc, 0, 0, 150, 100, blend);
        //::BitBlt(dib_dc, 0, 0, 150, 100, fill_dc, 0, 0, SRCCOPY);
        //::DeleteDC(fill_dc);
        //::DeleteObject(fill_bmp);

        /*::FillRect(dib_dc, CRect(CPoint(165, 125), CSize(150, 100)), (HBRUSH)::CreateSolidBrush(RGB(255, 0, 0)));
        ::FrameRect(dib_dc, CRect(CPoint(165, 125), CSize(150, 100)), (HBRUSH)::CreateSolidBrush(RGB(0, 0, 255)));*/
    }

    POINT position = { wr.left, wr.top };
    POINT zero = { 0, 0 };
    //::UpdateLayeredWindow(GetSafeHwnd(), hdc, &position, &size, dib_dc, &zero,
    //    RGB(0, 0, 0), &blend, /*ULW_COLORKEY*//*ULW_OPAQUE*/ULW_ALPHA);
    ::DeleteDC(dib_dc);
    ::DeleteObject(bmp);
    ::ReleaseDC(GetSafeHwnd(), hdc);

    CRuntimeClass* cls = GetRuntimeClass();

    /*CFileFind ff;
    if (ff.FindFile(L"D:\\2.0\\2x\\*.png"))
    {
        CString cstr, rname;
        while (ff.FindNextFile())
        {
            if (!ff.IsDots())
            {
                rname = cstr = ff.GetFilePath();
                rname.Replace(L"@2x", L"");
                rname.Replace(L" ", L"_");
                CFile::Rename(cstr, rname);
            }
        };
    }*/

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BOOL CDateTimePickerTestDlg::DestroyWindow()
{
    return __super::DestroyWindow();
}

void CDateTimePickerTestDlg::OnOK()
{
    CModalDialogTest modal(this);
    modal.DoModal();

    ::OutputDebugString(L"CDateTimePickerTestDlg\r\n");
}

void CDateTimePickerTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDateTimePickerTestDlg::OnPaint()
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
		//CDialogEx::OnPaint();

        RECT clt = { 0 };
        GetClientRect(&clt);
        CPaintDC dc(this);

        static CImage image;
        if (image.IsNull())
        {
            //HRESULT ret = image.Load(L"trans.png");
            HRESULT ret = image.Load(L"ng_complete.png");
            //HRESULT ret = image.Load(L"pic-upload.png");
        }
        /*HDC dib_dc = ::CreateCompatibleDC(dc);
        HBITMAP bmp = ::CreateCompatibleBitmap(dc, image.GetWidth(), image.GetHeight());
        ::SelectObject(dib_dc, bmp);*/
        //image.AlphaBlend(dc, CPoint(), 255, 0);
        //image.TransparentBlt(dc, CRect(0, 0, image.GetWidth(), image.GetHeight()), RGB(255, 255, 255));
        dc.FillSolidRect(165, 125, 150, 100, RGB(255, 255, 0));

        /*::TransparentBlt(dc, clt.left, clt.top, clt.right - clt.left, clt.bottom - clt.top, 
            dib_dc, 0, 0, image.GetWidth(), image.GetHeight(), RGB(240, 240, 240));*/
        /*::DeleteObject(bmp);
        ::DeleteDC(dib_dc);*/
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDateTimePickerTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



HBRUSH CDateTimePickerTestDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  在此更改 DC 的任何特性

    // TODO:  如果默认的不是所需画笔，则返回另一个画笔
    if (nCtlColor == CTLCOLOR_DLG)
    {
        static HBRUSH hbr_white = ::CreateSolidBrush(clr_trans);
        hbr = hbr_white;
        //hbr = (HBRUSH)::GetStockObject(NULL_BRUSH);
    }
    return hbr;
}


void CDateTimePickerTestDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO:  在此添加消息处理程序代码和/或调用默认值
    SendMessage(WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(point.x, point.y));
    CDialogEx::OnLButtonDown(nFlags, point);
}
