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
#include <functional>


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

    // callback
    class CallbackBase
    {
    public:
        CallbackBase()
        {
            post_thread_id_ = std::this_thread::get_id();
        }
        virtual ~CallbackBase() = default;
        virtual void Run() = 0;

        std::thread::id get_post_thread_id() { return post_thread_id_; }

    protected:
        std::thread::id post_thread_id_;
    };

    template<bool IsMemberFunc, typename T, bool FillRealArgument, typename BindFunc>
    class Callback;

    // global function
    template<typename BindFunc>
    class Callback<false, void, true, BindFunc> : public CallbackBase
    {
        typedef typename std::decay<BindFunc>::type _FuncType;
        typedef typename std::result_of<_FuncType()>::type _ResultType;
    public:
        Callback(BindFunc&& func)
            : func_(func)
        {
        }

        virtual void Run() override
        {
            func_();
        }

        _ResultType RunWithResult()
        {
            return func_();
        }

    private:
        _FuncType func_;
    };

    template<typename BindFunc>
    class Callback<false, void, false, BindFunc> : public CallbackBase
    {
        typedef typename std::decay<BindFunc>::type _FuncType;
        typedef typename std::result_of<_FuncType()>::type _ResultType;
    public:
        Callback(BindFunc&& func)
            : func_(func)
        {
        }

        virtual void Run() override
        {
            assert(false);
        }

        template <typename... Args>
        _ResultType RunWithParam(Args&&... args)
        {
            return func_(std::forward<Args>(args)...);
        }

    private:
        _FuncType func_;
    };

    // member function
    template<typename T, typename BindFunc>
    class Callback<true, T, true, BindFunc> : public CallbackBase
    {
        typedef typename std::decay<BindFunc>::type _FuncType;
        typedef typename std::result_of<_FuncType()>::type _ResultType;
    public:
        Callback(BindFunc&& func, std::weak_ptr<T> &weakptr)
            : func_(func)
            , weakptr_(weakptr)
        {
        }

        virtual void Run() override
        {
            std::shared_ptr<T> refptr = weakptr_.lock();
            if (refptr)
            {
                func_();
            }
            else
            {
                print_func("Run obj deleted!");
            }
        }

        _ResultType RunWithResult()
        {
            std::shared_ptr<T> refptr = weakptr_.lock();
            if (refptr)
            {
                return func_();
            }
            else
            {
                print_func("RunWithResult obj deleted!");
            }

            return _ResultType();
        }

    private:
        _FuncType func_;
        std::weak_ptr<T> weakptr_;
    };

    template<typename T, typename BindFunc>
    class Callback<true, T, false, BindFunc> : public CallbackBase
    {
        typedef typename std::decay<BindFunc>::type _FuncType;
        typedef typename std::result_of<_FuncType()>::type _ResultType;
    public:
        Callback(BindFunc&& func, std::weak_ptr<T> &weakptr)
            : func_(func)
            , weakptr_(weakptr)
        {
        }

        virtual void Run() override
        {
            assert(false);
        }

        template <typename... Args>
        _ResultType RunWithParam(Args&&... args)
        {
            std::shared_ptr<T> refptr = weakptr_.lock();
            if (refptr)
            {
                return func_(std::forward<Args>(args)...);
            }
            else
            {
                print_func("RunWithParam obj deleted!");
            }

            return _ResultType();
        }

    private:
        _FuncType func_;
        std::weak_ptr<T> weakptr_;
    };

    // create direct runable task
    template<typename BindFunc>
    std::shared_ptr<Callback<false, void, true, BindFunc>>
        CreateTask(BindFunc&& _Fx)
    {
        using task_type = Callback<false, void, true, BindFunc>;
        return std::shared_ptr<task_type>(new task_type(std::forward<BindFunc>(_Fx)));
    }

    template<typename T, typename BindFunc>
    std::shared_ptr<Callback<true, T, true, BindFunc>>
        CreateTask(BindFunc&& _Fx, std::weak_ptr<T> &weakptr)
    {
        using task_type = Callback<true, T, true, BindFunc>;
        return std::shared_ptr<task_type>(new task_type(std::forward<BindFunc>(_Fx), weakptr));
    }

    // create reply task
    template<typename BindFunc>
    std::shared_ptr<Callback<false, void, false, BindFunc>>
        CreateReplyTask(BindFunc&& _Fx)
    {
        using task_type = Callback<false, void, false, BindFunc>;
        return std::shared_ptr<task_type>(new task_type(std::forward<BindFunc>(_Fx)));
    }

    template<typename T, typename BindFunc>
    std::shared_ptr<Callback<true, T, false, BindFunc>>
        CreateReplyTask(BindFunc&& _Fx, std::weak_ptr<T> &weakptr)
    {
        using task_type = Callback<true, T, false, BindFunc>;
        return std::shared_ptr<task_type>(new task_type(std::forward<BindFunc>(_Fx), weakptr));
    }

    // post and reply
    template <typename ReplyCallback, typename... RArgs>
    void ReplyAdapter(ReplyCallback reply, RArgs... args)
    {
        reply->RunWithParam(std::move(args...));
    }

    template <typename TaskCallback, typename ReplyCallback>
    void ReturnAsParamAdapter(TaskCallback task, ReplyCallback reply)
    {
        std::shared_ptr<CThread> thd = CThread::GetThread(reply->get_post_thread_id());
        if (thd)
        {
            using result_type = std::decay<decltype(task->RunWithResult())>::type;
            result_type ret = task->RunWithResult();

            auto task = std::bind(ReplyAdapter<ReplyCallback, result_type>, reply, ret);
            thd->PostTask(task);
        }
    }

    template<typename TaskStdBindFunc, typename ReplyStdBindFunc>
    bool PostTaskAndReplyWithResult(
        int tid,
        TaskStdBindFunc &&task,
        ReplyStdBindFunc &&reply
        )
    {
        bool operate = false;

        if (g_thread_map.find(tid) != g_thread_map.end())
        {
            using ttype = std::decay<std::shared_ptr<Callback<false, void, true, TaskStdBindFunc>>>::type;
            using rtype = std::decay<std::shared_ptr<Callback<false, void, false, ReplyStdBindFunc>>>::type;
            ttype tc = CreateTask(std::forward<TaskStdBindFunc>(task));
            rtype rc = CreateReplyTask(std::forward<ReplyStdBindFunc>(reply));

            auto task = std::bind(ReturnAsParamAdapter<ttype, rtype>, tc, rc);
            g_thread_map[tid]->PostTask(task);

            operate = true;
        }

        return operate;
    }

    template<typename TaskStdBindFunc, typename TaskClassType, typename ReplyStdBindFunc>
    bool PostTaskAndReplyWithResult(
        int tid,
        TaskStdBindFunc &&task,
        std::weak_ptr<TaskClassType> &task_weakptr,
        ReplyStdBindFunc &&reply
        )
    {
        bool operate = false;

        if (g_thread_map.find(tid) != g_thread_map.end())
        {
            using ttype = std::decay<std::shared_ptr<Callback<true, TaskClassType, true, TaskStdBindFunc>>>::type;
            using rtype = std::decay<std::shared_ptr<Callback<false, void, false, ReplyStdBindFunc>>>::type;
            ttype tc = CreateTask(std::forward<TaskStdBindFunc>(task), task_weakptr);
            rtype rc = CreateReplyTask(std::forward<ReplyStdBindFunc>(reply));

            auto task = std::bind(ReturnAsParamAdapter<ttype, rtype>, tc, rc);
            g_thread_map[tid]->PostTask(task);

            operate = true;
        }

        return operate;
    }

    template<typename TaskStdBindFunc, typename ReplyStdBindFunc, typename ReplyClassType>
    bool PostTaskAndReplyWithResult(
        int tid,
        TaskStdBindFunc &&task,
        ReplyStdBindFunc &&reply,
        std::weak_ptr<ReplyClassType> &reply_weakptr
        )
    {
        bool operate = false;

        if (g_thread_map.find(tid) != g_thread_map.end())
        {
            using ttype = std::decay<std::shared_ptr<Callback<false, void, true, TaskStdBindFunc>>>::type;
            using rtype = std::decay<std::shared_ptr<Callback<true, ReplyClassType, false, ReplyStdBindFunc>>>::type;
            ttype tc = CreateTask(std::forward<TaskStdBindFunc>(task));
            rtype rc = CreateReplyTask(std::forward<ReplyStdBindFunc>(reply), reply_weakptr);

            auto task = std::bind(ReturnAsParamAdapter<ttype, rtype>, tc, rc);
            g_thread_map[tid]->PostTask(task);

            operate = true;
        }

        return operate;
    }

    template<typename TaskStdBindFunc, typename TaskClassType, typename ReplyStdBindFunc, typename ReplyClassType>
    bool PostTaskAndReplyWithResult(
        int tid,
        TaskStdBindFunc &&task,
        std::weak_ptr<TaskClassType> &task_weakptr,
        ReplyStdBindFunc &&reply,
        std::weak_ptr<ReplyClassType> &reply_weakptr
        )
    {
        bool operate = false;

        if (g_thread_map.find(tid) != g_thread_map.end())
        {
            using ttype = std::decay<std::shared_ptr<Callback<true, TaskClassType, true, TaskStdBindFunc>>>::type;
            using rtype = std::decay<std::shared_ptr<Callback<true, ReplyClassType, false, ReplyStdBindFunc>>>::type;
            ttype tc = CreateTask(std::forward<TaskStdBindFunc>(task), task_weakptr);
            rtype rc = CreateReplyTask(std::forward<ReplyStdBindFunc>(reply), reply_weakptr);

            auto task = std::bind(ReturnAsParamAdapter<ttype, rtype>, tc, rc);
            g_thread_map[tid]->PostTask(task);

            operate = true;
        }

        return operate;
    }

    // weak_ptr
    template <typename T>
    std::weak_ptr<T> GetWeakPtr(T* ptr)
    {
        return GetWeakPtr(std::shared_ptr<T>(ptr));
    }

    template <typename T>
    std::weak_ptr<T> GetWeakPtr(std::shared_ptr<T>& ptr)
    {
        return std::weak_ptr<T>(ptr);
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

    // thread
    class CThread : public std::enable_shared_from_this<CThread>
    {
    public:
        static std::shared_ptr<CThread> GetThread(const std::thread::id &id)
        {
            std::lock_guard<std::mutex> lock(thread_map_mutex_);
            if (thread_map_.find(id) != thread_map_.end())
            {
                return thread_map_[id];
            }
            return std::shared_ptr<CThread>();
        }

        CThread()
        {
        }

        virtual ~CThread()
        {
            StopSoon();
        }

        CThread(CThread &right) = delete;

        CThread& operator=(CThread &right) = delete;

        template <typename StdBindFunc>
        void PostTask(StdBindFunc &&closure)
        {
            if (!thread_)
            {
                assert(false);
                return;
            }

            PostBaseTask(std::shared_ptr<CallbackBase>(
                std::move(CreateTask(std::forward<StdBindFunc>(closure)))));
        }

        template <class StdBindFunc, typename T>
        void PostTask(StdBindFunc &&closure, std::weak_ptr<T> &weakptr)
        {
            if (!thread_)
            {
                assert(false);
                return;
            }

            PostBaseTask(std::shared_ptr<CallbackBase>(
                std::move(CreateTask(std::forward<StdBindFunc>(closure), weakptr))));
        }

        void Run()
        {
            assert(!thread_);

            keep_working_ = true;
            thread_.reset(new std::thread(&CThread::ThreadFunc, std::weak_ptr<CThread>(shared_from_this())));
            id_ = thread_->get_id();

            std::lock_guard<std::mutex> lock(thread_map_mutex_);
            thread_map_[id_] = shared_from_this();
        }

        void Stop()
        {
            keep_working_ = false;
            semphore_.stop();
        }

        void Join()
        {
            if (thread_ && thread_->joinable())
            {
                thread_->join();
            }
        }

        void StopSoon()
        {
            Stop();
            Join();
            thread_.reset();
        }

    protected:
        void PostBaseTask(std::shared_ptr<CallbackBase> &task)
        {
            {
                std::unique_lock<std::mutex> lock(task_mutex_);
                task_list.push(task);
            }

            semphore_.signal();
        }

    private:
        static void ThreadFunc(std::weak_ptr<CThread> weakptr)
        {
            std::shared_ptr<CThread> pThis = weakptr.lock();
            if (pThis)
            {
                pThis->_ThreadFunc();
            }
        }

        void _ThreadFunc()
        {
            while (keep_working_ && semphore_.wait())
            {
                std::shared_ptr<CallbackBase> task;
                {
                    std::unique_lock<std::mutex> lock(task_mutex_);
                    if (!task_list.empty())
                    {
                        task = task_list.front();
                        task_list.pop();
                    }
                }
                if (task)
                {
                    task->Run();
                }
            }

            std::lock_guard<std::mutex> lock(thread_map_mutex_);
            thread_map_.erase(id_);
        }

    private:
        std::shared_ptr<std::thread> thread_;
        std::thread::id id_;
        semphore semphore_;
        std::mutex task_mutex_;
        std::queue<std::shared_ptr<CallbackBase>> task_list;
        bool keep_working_ = true;

        static std::mutex thread_map_mutex_;
        static std::map<std::thread::id, std::shared_ptr<CThread>> thread_map_;
    };
    std::mutex CThread::thread_map_mutex_;
    std::map<std::thread::id, std::shared_ptr<CThread>> CThread::thread_map_;

    // test
    std::map<int, std::shared_ptr<CThread>> g_thread_map;

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

            int i = 2 - index;
            i = (i == index) ? 0 : i;
            std::string ss;
            std::shared_ptr<WeakptrTest> obj(new WeakptrTest);
            // member -> global
            PostTaskAndReplyWithResult(i, 
                std::bind(&WeakptrTest::print_string, obj.get()),
                GetWeakPtr(obj),
                std::bind(on_async_call_string, std::placeholders::_1));
            // global -> member
            PostTaskAndReplyWithResult(i,
                std::bind(async_call_string, 4.55f),
                std::bind(&WeakptrTest::on_print_string, this, std::placeholders::_1), 
                GetWeakPtr(shared_from_this()));
            // member -> member
            PostTaskAndReplyWithResult(i,
                std::bind(&WeakptrTest::print_string, this), 
                GetWeakPtr(shared_from_this()),
                std::bind(&WeakptrTest::on_print_string, this, std::placeholders::_1), 
                GetWeakPtr(shared_from_this()));
            // global -> global
            PostTaskAndReplyWithResult(i, 
                std::bind(async_call_string, 3.15f), 
                std::bind(on_async_call_string, std::placeholders::_1));
        }

    private:
        int id_ = 0;
    };
}


void thread_post_task_study()
{
    using namespace std::placeholders;
    std::shared_ptr<WeakptrTest> obj(new WeakptrTest());

    //auto fc = std::bind(on_async_call_value, _1);
    //auto aa = fc;
    //aa();

    //auto mf = std::move(std::bind(&WeakptrTest::print_param, obj, _1));
    //mf(1);

    auto gt = CreateTask(std::bind(async_call_value, 3.18f));
    gt->Run();
    auto gret = gt->RunWithResult();

    auto mt = CreateTask(std::bind(&WeakptrTest::print_param, obj.get(), 2345), std::weak_ptr<WeakptrTest>(obj));
    //obj.reset();
    mt->Run();
    auto mret = mt->RunWithResult();

    std::cout << "------------------------------------" << std::endl;

    auto gtr = CreateReplyTask(std::bind(async_call_value, _1));
    //gtr->Run();
    auto gretr = gtr->RunWithParam(4.5f);

    auto mtr = CreateReplyTask(std::bind(&WeakptrTest::print_param, obj.get(), _1), std::weak_ptr<WeakptrTest>(obj));
    //obj.reset();
    //mtr->Run();
    auto mretr = mtr->RunWithParam(-343);

    //std::function<void(const std::string&)> ff = std::bind(on_async_call_value, "asd");

    std::cout << "------------------------------------" << std::endl;

    g_thread_map[0] = std::shared_ptr<CThread>(new CThread());
    g_thread_map[1] = std::shared_ptr<CThread>(new CThread());
    g_thread_map[2] = std::shared_ptr<CThread>(new CThread());
    for (auto &thd : g_thread_map)
    {
        thd.second->Run();
    }

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
            int i = index++;
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
            }
        }
        break;
        }
    }

    for (auto &thd : g_thread_map)
    {
        thd.second->StopSoon();
    }
    g_thread_map.clear();

    std::lock();
}