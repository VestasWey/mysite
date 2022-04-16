#include "stdafx.h"
#include <windows.h>
#include <thread>
#include <memory>
#include <conio.h>


namespace
{
    void peekmessage_thread_proc()
    {
        DWORD dw = 0;
        DWORD result = WAIT_OBJECT_0;
        MSG msg = { 0 };
        bool work = true;
        while (work)
        {
            result = MsgWaitForMultipleObjectsEx(0, NULL, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);

            if (WAIT_OBJECT_0 == result)
            {
                dw = GetQueueStatus(QS_ALLINPUT | QS_ALLPOSTMESSAGE);
                printf("status: h = 0x%04x, l = 0x%04x\n", HIWORD(dw), LOWORD(dw));

                work = !!::GetMessage(&msg, nullptr, 0, 0);
                //work = ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
                if (work)
                {
                    ::TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                    printf("msg = 0x%04x\n", msg.message);
                }
            }
        }
        printf("WM_QUIT\n");
    }
}


void thread_message_example()
{
    std::thread g_thread(peekmessage_thread_proc);
    DWORD dwID = ::GetThreadId(g_thread.native_handle());

    bool work = true;
    while (work)
    {
        int cc = _getch();
        switch (cc)
        {
        case VK_ESCAPE:
            ::PostThreadMessage(dwID, WM_QUIT, 0, 0);
            work = false;
        break;
        case 'p':
            ::PostThreadMessage(dwID, WM_MOUSEMOVE, 0, 0);
            break;
        case 'q':
            ::PostThreadMessage(dwID, WM_QUIT, 0, 0);
            break;
        default:
            ::PostThreadMessage(dwID, WM_NCDESTROY, 0, 0);
            break;
        }
    }

    printf("join\n");
    g_thread.join();
}