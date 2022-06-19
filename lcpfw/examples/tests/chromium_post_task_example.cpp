#include "stdafx.h"

#include <iostream>           // std::cout
#include <thread>             // std::thread
#include <mutex>              // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <windows.h>
#include <conio.h>
#include <vector>
#include <atomic>
#include <sstream>
#include <memory>
#include <queue>
#include <map>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include "base/threading/platform_thread.h"
#include "base\task\sequence_manager\sequence_manager.h"
#include "base/task/post_task.h"
#include "base/location.h"


// https://www.cnblogs.com/qicosmos/p/4325949.html 可变模版参数
namespace
{
    void print_func(const char *name)
    {
        /*std::ostringstream text;
        std::this_thread::get_id()._To_text(text);*/
        std::cout << "thread: " << std::this_thread::get_id()
            << " \t msg: " << name << std::endl;
    }

    // string utils
    std::string StringPrintf(const char* format, ...)
    {
        std::string str;

        va_list  args;
        int      len;
        char     *buffer = NULL;

        va_start(args, format);

        // _vscprintf doesn't count the 
        // null terminating string so we add 1.
        len = _vscprintf_p(format, args) + 1;

        // Allocate memory for our buffer
        buffer = (char*)malloc(len * sizeof(char));
        if (buffer)
        {
            _vsprintf_p(buffer, len, format, args);
            str = buffer;
            free(buffer);
        }
        va_end(args);

        return str;
    }

    // semphore
    class semphore
    {
    public:
        semphore(long inti_value = 0)
            : count_(inti_value)
        {
            work_ = true;
        }

        virtual ~semphore()
        {
            stop();
        }

        bool wait()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (count_ <= 0)
            {
                condition_var_.wait(lock);
            }

            if (work_)
            {
                --count_;
                if (count_ < 0)
                {
                    assert(false);
                }
            }

            return work_;
        }

        void signal()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            count_++;
            condition_var_.notify_one();
        }

        void stop()
        {
            work_ = false;
            std::unique_lock<std::mutex> lock(mutex_);
            condition_var_.notify_all();
        }

    private:
        std::atomic_bool work_;
        std::atomic<long> count_;
        std::mutex mutex_;
        std::condition_variable condition_var_;
    };

    void async_call_void()
    {
        print_func("global_async_call_void");
    }

    void on_async_call_void()
    {
        print_func("global_on_async_call_void");
    }

    std::string async_call_string(float ff)
    {
        print_func(StringPrintf("global_async_call_string: %f", ff).c_str());
        return "std::string async_call_string(float ff)";
    }

    void on_async_call_string(const std::string &str)
    {
        print_func(StringPrintf("global_on_async_call_string: %s", str.c_str()).c_str());
    }

    std::string async_call_value(float ff)
    {
        print_func(StringPrintf("global_async_call_value: %f", ff).c_str());
        return "std::string async_call_value(float ff)";
    }

    void on_async_call_value(const std::string &ch)
    {
        print_func(StringPrintf("global_on_async_call_value: %s", ch.c_str()).c_str());
    }

    class WeakptrTest : public std::enable_shared_from_this<WeakptrTest>
    {
    public:
        WeakptrTest() = default;
        virtual ~WeakptrTest()
        {
        }

        void print_void()
        {
            print_func("WeakptrTest::print_void");
        }

        void on_print_void()
        {
            print_func("WeakptrTest::on_print_void");
        }

        int print_param(int i)
        {
            print_func(StringPrintf("WeakptrTest::print_param: %d", i).c_str());
            return i;
        }

        void on_print_param(int ret)
        {
            print_func(StringPrintf("WeakptrTest::on_print_param: %d", ret).c_str());
        }

        std::string print_string()
        {
            print_func("WeakptrTest::print_string");
            return "std::string WeakptrTest::print_string()";
        }

        void on_print_string(const std::string &str)
        {
            print_func(StringPrintf("WeakptrTest::on_print_string: %s", str.c_str()).c_str());
        }

        void post_task_and_reply(int index)
        {
            print_func("WeakptrTest::post_task_and_reply");
        }

    private:
        int id_ = 0;
    };
}


void chromium_post_task_study()
{
    base::PlatformThread::SetName("tests_console_chromium_example");
    std::cout << "------------------------------------" << std::endl;

    /*g_thread_map[0] = std::shared_ptr<CThread>(new CThread());
    g_thread_map[1] = std::shared_ptr<CThread>(new CThread());
    g_thread_map[2] = std::shared_ptr<CThread>(new CThread());
    for (auto &thd : g_thread_map)
    {
        thd.second->Run();
    }*/

    bool work = true;
    int index = 0;
    while (work)
    {
        int cc = _getch();
        switch (cc)
        {
        case VK_ESCAPE:
        {
            work = false;
        }
        break;
        default:
        {
            /*int i = index++;
            int thd = i % 3;
            switch (thd)
            {
            case 0:
            {
                std::shared_ptr<WeakptrTest> tobj(new WeakptrTest());
                g_thread_map[thd]->PostTask(std::bind(&WeakptrTest::print_void, tobj), std::weak_ptr<WeakptrTest>(tobj));
            }
            break;
            case 1:
                g_thread_map[thd]->PostTask(std::bind(on_async_call_string, "thread_post_task_study"));
                break;
            case 2:
                g_thread_map[thd]->PostTask(std::bind(&WeakptrTest::post_task_and_reply, obj, thd));
                break;
            default:
                break;
            }*/
        }
        break;
        }
    }

    /*for (auto &thd : g_thread_map)
    {
        thd.second->StopSoon();
    }
    g_thread_map.clear();*/
}