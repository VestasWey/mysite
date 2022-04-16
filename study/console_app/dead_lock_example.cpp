#include "stdafx.h"

//#include <synchapi.h>
#include <mutex>
#include <thread>
#include <memory>

//#include <minwinbase.h>
#include <windows.h>
#include <synchapi.h>

namespace
{
    std::unique_ptr<std::thread> ta;
    std::unique_ptr<std::thread> tb;

    CRITICAL_SECTION g_cs_a;
    CRITICAL_SECTION g_cs_b;

    HANDLE g_ms_mutex_a = INVALID_HANDLE_VALUE;
    HANDLE g_ms_mutex_b = INVALID_HANDLE_VALUE;

    std::mutex g_mutex_a;
    std::mutex g_mutex_b;

    void a_thread_proc()
    {
        std::cout << "a_thread_proc start ! id = 0x" << std::hex << ::GetCurrentThreadId() << std::endl;
        std::unique_lock<std::mutex> alock(g_mutex_a);

        {
            //Sleep(2000);
            std::cout << "a thread try to lock b_mutex !" << std::endl;
            std::unique_lock<std::mutex> block(g_mutex_b);
            std::cout << "a thread locked b_mutex !" << std::endl;
        }

        std::cout << "a_thread_proc end !" << std::endl;
    }

    void b_thread_proc()
    {
        std::cout << "b_thread_proc start ! id = 0x" << std::hex << ::GetCurrentThreadId() << std::endl;
        std::unique_lock<std::mutex> block(g_mutex_b);

        {
            //Sleep(2000);
            std::cout << "b thread try to lock a_mutex !" << std::endl;
            std::unique_lock<std::mutex> alock(g_mutex_a);
            std::cout << "b thread locked a_mutex !" << std::endl;
        }

        std::cout << "b_thread_proc end !" << std::endl;
    }

    void std_mutex_test()
    {
        std::unique_lock<std::mutex> alock(g_mutex_a);
        std::unique_lock<std::mutex> block(g_mutex_b);
        ta = std::make_unique<std::thread>(a_thread_proc);
        Sleep(2000);
        tb = std::make_unique<std::thread>(b_thread_proc);
    }

    void a_cs_lock_thread_proc()
    {
        std::cout << "a_cs_lock_thread_proc start ! id = 0x" << std::hex << ::GetCurrentThreadId() << std::endl;
        ::EnterCriticalSection(&g_cs_a);

        {
            //Sleep(2000);
            std::cout << "a thread try to lock b_cs !" << std::endl;
            ::EnterCriticalSection(&g_cs_b);
            std::cout << "a thread locked b_cs !" << std::endl;
            ::LeaveCriticalSection(&g_cs_b);
        }

        ::LeaveCriticalSection(&g_cs_a);
        std::cout << "a_thread_proc end !" << std::endl;
    }

    void b_cs_lock_thread_proc()
    {
        std::cout << "b_cs_lock_thread_proc start ! id = 0x" << std::hex << ::GetCurrentThreadId() << std::endl;
        ::EnterCriticalSection(&g_cs_b);

        {
            //Sleep(2000);
            std::cout << "b thread try to lock a_cs !" << std::endl;
            ::EnterCriticalSection(&g_cs_a);
            std::cout << "b thread locked a_cs !" << std::endl;
            ::LeaveCriticalSection(&g_cs_a);
        }

        ::LeaveCriticalSection(&g_cs_b);
        std::cout << "b_cs_lock_thread_proc end !" << std::endl;
    }

    void ms_cs_test()
    {
        ::InitializeCriticalSection(&g_cs_a);
        ::InitializeCriticalSection(&g_cs_b);
        ::EnterCriticalSection(&g_cs_a);        
        ::EnterCriticalSection(&g_cs_b);

        ta = std::make_unique<std::thread>(a_cs_lock_thread_proc);
        Sleep(2000);
        tb = std::make_unique<std::thread>(b_cs_lock_thread_proc);

        ::LeaveCriticalSection(&g_cs_a);
        ::LeaveCriticalSection(&g_cs_b);
    }

    void a_mutex_lock_thread_proc()
    {
        std::cout << "a_mutex_lock_thread_proc start ! id = 0x" << std::hex << ::GetCurrentThreadId() << std::endl;
        ::WaitForSingleObject(g_ms_mutex_a, INFINITE);

        {
            Sleep(2000);
            std::cout << "a thread try to lock b_mutex !" << std::endl;
            ::WaitForSingleObject(g_ms_mutex_b, INFINITE);
            std::cout << "a thread locked b_mutex !" << std::endl;
        }

        std::cout << "a_mutex_lock_thread_proc end !" << std::endl;
    }

    void b_mutex_lock_thread_proc()
    {
        std::cout << "b_mutex_lock_thread_proc start ! id = 0x" << std::hex << ::GetCurrentThreadId() << std::endl;
        ::WaitForSingleObject(g_ms_mutex_b, INFINITE);

        {
            Sleep(2000);
            std::cout << "b thread try to lock a_mutex !" << std::endl;
            ::WaitForSingleObject(g_ms_mutex_a, INFINITE);
            std::cout << "b thread locked a_mutex !" << std::endl;
        }

        std::cout << "b_mutex_lock_thread_proc end !" << std::endl;
    }

    void ms_mutex_test()
    {
        g_ms_mutex_a = ::CreateMutex(nullptr, FALSE, L"ms mutex a");
        g_ms_mutex_b = ::CreateMutex(nullptr, FALSE, L"ms mutex b");

        ta = std::make_unique<std::thread>(a_mutex_lock_thread_proc);
        tb = std::make_unique<std::thread>(b_mutex_lock_thread_proc);

        ::ReleaseMutex(g_ms_mutex_a);
        ::ReleaseMutex(g_ms_mutex_b);
    }
}

void dead_lock_example()
{
    std::cout << "current process id " << ::GetCurrentProcessId() << std::endl;
    std::cout << "main thread id 0x" << std::hex << ::GetCurrentThreadId() << std::endl;

    //std_mutex_test();
    //ms_cs_test();
    ms_mutex_test();

    ta->join();
    tb->join();
}