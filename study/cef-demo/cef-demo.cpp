// cef-demo.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "cef-demo.h"

#include <list>

#include "cef/include/cef_app.h"
#include "cef/include/cef_browser.h"
#include "cef/include/cef_client.h"
#include "cef/include/wrapper/cef_closure_task.h"
#include "cef/include/wrapper/cef_helpers.h"
//-------------------- -
//作者：Cynhard85
//来源：CSDN
//原文：https ://blog.csdn.net/u011304970/article/details/77601198 
//版权声明：本文为博主原创文章，转载请附上博文链接！

void ShowCefDemo(HWND parent);


class MyClient : 
    public CefClient, 
    public CefLifeSpanHandler,
    public CefDisplayHandler,
    public CefLoadHandler
{
    // Constructor & Destructor
public:
    virtual ~MyClient() {}


    // CefClient methods:
    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE
    {
        return this;
    }
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
    {
        return this;
    }
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE { return this; }


    // CefDisplayHandler methods:
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                               const CefString& title) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        // Set the title of the window using platform APIs.
        PlatformTitleChange(browser, title);
    }

    // CefLifeSpanHandler methods:
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        // Add to the list of existing browsers.
        browser_list_.push_back(browser);
    }
    virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        // Closing the main window requires special handling. See the DoClose()
        // documentation in the CEF header for a detailed destription of this
        // process.
        if (browser_list_.size() == 1)
        {
            // Set a flag to indicate that the window close should be allowed.
            is_closing_ = true;
        }

        // Allow the close. For windowed browsers this will result in the OS close
        // event being sent.
        return false;
    }
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override
    {
        CEF_REQUIRE_UI_THREAD();

        // Remove from the list of existing browsers.
        BrowserList::iterator bit = browser_list_.begin();
        for (; bit != browser_list_.end(); ++bit)
        {
            if ((*bit)->IsSame(browser))
            {
                browser_list_.erase(bit);
                break;
            }
        }

        if (browser_list_.empty())
        {
            // All browser windows have closed. Quit the application message loop.
            //CefQuitMessageLoop();
        }
    }

    // CefLoadHandler methods:
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             ErrorCode errorCode,
                             const CefString& errorText,
                             const CefString& failedUrl) OVERRIDE
    {
        CEF_REQUIRE_UI_THREAD();

        // Don't display an error for downloaded files.
        if (errorCode == ERR_ABORTED)
            return;

        // Display a load error message.
        std::stringstream ss;
        ss << "<html><body bgcolor=\"white\">"
            "<h2>Failed to load URL "
            << std::string(failedUrl) << " with error " << std::string(errorText)
            << " (" << errorCode << ").</h2></body></html>";
        frame->LoadString(ss.str(), failedUrl);
    }

    bool IsClosing() const { return is_closing_; }

 private:
     // Platform-specific implementation.
     void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
                              const CefString& title)
     {
         CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
         SetWindowText(hwnd, std::wstring(title).c_str());
     }

private:
    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;

    bool is_closing_ = false;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(MyClient);
};

// Implement application-level callbacks for the browser process.
class MyApp : public CefApp, public CefBrowserProcessHandler
{
public:
    virtual ~MyApp() {}

    // CefApp methods:
    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }

    // CefBrowserProcessHandler methods:
    virtual void OnContextInitialized() override
    {
        //ShowCefDemo(nullptr);
    }

private:
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(MyApp);
};

void ShowCefDemo(HWND parent)
{
    //CEF_REQUIRE_UI_THREAD();

    std::string url = "https://www.bilibili.com/";

    // SimpleHandler implements browser-level callbacks.
    CefRefPtr<MyClient> client(new MyClient());

    // Information used when creating the native window.
    CefWindowInfo window_info;

    // Specify CEF browser settings here.
    CefBrowserSettings browser_settings;

    // On Windows we need to specify certain flags that will be passed to
    // CreateWindowEx().
    if (!parent)
    {
        window_info.SetAsPopup(parent, "cef-demo-app");
    }
    else
    {
        //window_info.SetAsWindowless(parent);
        RECT rect{ 100, 100, 900, 700 };
        window_info.SetAsChild(parent, rect);
    }

    // Create the first browser window.
    bool ret = CefBrowserHost::CreateBrowser(window_info, client, url, browser_settings, NULL);
    return;
}
#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // cef[
    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();

    void* sandbox_info = NULL;

    // Provide CEF with command-line arguments.
    CefMainArgs main_args(hInstance);

    // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
    // that share the same executable. This function checks the command-line and,
    // if this is a sub-process, executes the appropriate logic.
    int exit_code = CefExecuteProcess(main_args, NULL, sandbox_info);
    if (exit_code >= 0)
    {
        // The sub-process has completed so return here.
        return exit_code;
    }

    // Specify CEF global settings here.
    CefSettings settings;
    settings.no_sandbox = true;
    settings.multi_threaded_message_loop = true;
    //settings.windowless_rendering_enabled = true;

    // SimpleApp implements application-level callbacks for the browser process.
    // It will create the first browser instance in OnContextInitialized() after
    // CEF has initialized.
    CefRefPtr<MyApp> myApp(new MyApp());

    // Initialize CEF.
    CefInitialize(main_args, settings, myApp.get(), sandbox_info);

    // ]cef

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CEFDEMO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CEFDEMO));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Shut down CEF.
    CefShutdown();

    return (int) msg.wParam;

    // -------------------------------------------------------------------------------------------------------------------

    //// Enable High-DPI support on Windows 7 or newer.
    //CefEnableHighDPISupport();

    //void* sandbox_info = NULL;

    //// Provide CEF with command-line arguments.
    //CefMainArgs main_args(hInstance);

    //// CEF applications have multiple sub-processes (render, plugin, GPU, etc)
    //// that share the same executable. This function checks the command-line and,
    //// if this is a sub-process, executes the appropriate logic.
    //int exit_code = CefExecuteProcess(main_args, NULL, sandbox_info);
    //if (exit_code >= 0)
    //{
    //    // The sub-process has completed so return here.
    //    return exit_code;
    //}

    //// Specify CEF global settings here.
    //CefSettings settings;
    //settings.no_sandbox = true;
    //settings.external_message_pump = true;

    //// SimpleApp implements application-level callbacks for the browser process.
    //// It will create the first browser instance in OnContextInitialized() after
    //// CEF has initialized.
    //CefRefPtr<MyApp> myApp(new MyApp());

    //// Initialize CEF.
    //CefInitialize(main_args, settings, myApp.get(), sandbox_info);

    //// Run the CEF message loop. This will block until CefQuitMessageLoop() is
    //// called.
    ////CefRunMessageLoop();
    //CefDoMessageLoopWork();

    //// Shut down CEF.
    //CefShutdown();

    //return 0;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CEFDEMO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CEFDEMO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        ShowCefDemo(nullptr);
    }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

