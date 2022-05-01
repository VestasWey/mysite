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
#include <ratio>


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

    // invoker
    template<size_t N>
    struct InvokeHelper
    {
        template<typename Function, typename Tuple, typename... Args>
        static inline auto Invoke(Function&& func, Tuple&& tpl, Args &&... args)
            -> decltype(InvokeHelper<N - 1>::Invoke(
            std::forward<Function>(func),
            std::forward<Tuple>(tpl),
            std::get<N - 1>(std::forward<Tuple>(tpl)),
            std::forward<Args>(args)...
            ))
        {
            return InvokeHelper<N - 1>::Invoke(
                std::forward<Function>(func),
                std::forward<Tuple>(tpl),
                std::get<N - 1>(std::forward<Tuple>(tpl)),
                std::forward<Args>(args)...
                );
        }

        template<typename Function, typename T, typename Tuple, typename... Args>
        static inline auto Invoke(Function&& func, T* obj, Tuple&& tpl, Args &&... args)
            -> decltype(InvokeHelper<N - 1>::Invoke(
            std::forward<Function>(func),
            obj,
            std::forward<Tuple>(tpl),
            std::get<N - 1>(std::forward<Tuple>(tpl)),
            std::forward<Args>(args)...
            ))
        {
            return InvokeHelper<N - 1>::Invoke(
                std::forward<Function>(func),
                obj,
                std::forward<Tuple>(tpl),
                std::get<N - 1>(std::forward<Tuple>(tpl)),
                std::forward<Args>(args)...
                );
        }
    };

    template<>
    struct InvokeHelper<0>
    {
        template<typename Function, typename Tuple, typename... Args>
        static inline auto Invoke(Function &&func, Tuple&&, Args &&... args)
            -> decltype(std::forward<Function>(func)(std::forward<Args>(args)...))
        {
            return std::forward<Function>(func)(std::forward<Args>(args)...);
        }

        template<typename Function, typename T, typename Tuple, typename... Args>
        static inline auto Invoke(Function &&func, T* obj, Tuple&&, Args &&... args)
            -> decltype((obj->*(std::forward<Function>(func)))(std::forward<Args>(args)...))
        {
            return (obj->*(std::forward<Function>(func)))(std::forward<Args>(args)...);
        }
    };

    template<typename Function, typename Tuple>
    auto Invoke(Function&& func, Tuple&& tpl)
        -> decltype(InvokeHelper<std::tuple_size<typename std::decay<Tuple>::type>::value>::
        Invoke(std::forward<Function>(func), std::forward<Tuple>(tpl)))
    {
        return InvokeHelper<std::tuple_size<typename std::decay<Tuple>::type>::value>::
            Invoke(std::forward<Function>(func), std::forward<Tuple>(tpl));
    }

    template<typename Function, typename T, typename Tuple>
    auto Invoke(Function&& func, T* obj, Tuple&& tpl)
        -> decltype(InvokeHelper<std::tuple_size<typename std::decay<Tuple>::type>::value>::
        Invoke(std::forward<Function>(func), obj, std::forward<Tuple>(tpl)))
    {
        return InvokeHelper<std::tuple_size<typename std::decay<Tuple>::type>::value>::
            Invoke(std::forward<Function>(func), obj, std::forward<Tuple>(tpl));
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

    template<bool IsMemberFunc, typename ReturnType, typename T, typename... Args>
    class Callback;

    template<typename ReturnType, typename... Args>
    class Callback<false, ReturnType, void, Args...> : public CallbackBase
    {
    public:
        Callback(ReturnType(*func)(Args...), Args&&... args)
            : func_(func)
            , _Mybargs(std::forward<Args>(args)...)
        {
        }

        virtual void Run() override
        {
            RunWithResult();
        }

        template<typename ParamType>
        void RunWithParam(ParamType *result)
        {
            if (std::tuple_size<_Bargs>::value)
            {
                using fstType = std::tuple_element<0, _Bargs>::type;
                std::get<0>(_Mybargs) = std::move(*((fstType*)result));
            }

            Invoke(func_, _Mybargs);
        }

        ReturnType RunWithResult()
        {
            return Invoke(func_, _Mybargs);
        }

    private:
        //typedef typename std::decay<_Fun>::type _Funx;
        typedef std::tuple<typename std::decay<Args>::type...> _Bargs;

        ReturnType(*func_)(Args...);
        _Bargs _Mybargs;	// the bound arguments
    };

    template<typename ReturnType, typename T, typename... Args>
    class Callback<true, ReturnType, T, Args...> : public CallbackBase
    {
        //typedef typename std::decay<_Fun>::type _Funx;
        //using _ArgsType = typename std::decay<Args>::type...;
        typedef std::tuple<typename std::decay<Args>::type...> _Bargs;

    public:
        Callback(ReturnType(T::*func)(Args...), std::weak_ptr<T> &weakptr, Args&&... args)
            : func_(func)
            , weakptr_(weakptr)
            , _Mybargs(std::forward<Args>(args)...)
        {
        }

        virtual void Run() override
        {
            RunWithResult();
        }

        template<typename ParamType>
        void RunWithParam(ParamType *result)
        {
            std::shared_ptr<T> ptr = weakptr_.lock();
            if (!ptr)
            {
                print_func("obj deleted");
                return;
            }

            if (std::tuple_size<_Bargs>::value)
            {
                using fstType = std::tuple_element<0, _Bargs>::type;
                std::get<0>(_Mybargs) = std::move(*((fstType*)result));
            }
            
            Invoke(func_, ptr.get(), _Mybargs);
        }

        ReturnType RunWithResult()
        {
            std::shared_ptr<T> ptr = weakptr_.lock();
            if (!ptr)
            {
                print_func("obj deleted");
                return ReturnType();
            }

            return Invoke(func_, ptr.get(), _Mybargs);
        }

    private:
        std::weak_ptr<T> weakptr_;
        ReturnType(T::*func_)(Args...);
        _Bargs _Mybargs;	// the bound arguments
    };

    // bind
    template <typename ReturnType, typename... Args>
    Callback<false, ReturnType, void, Args...> 
        Bind(ReturnType(*func)(Args...), Args... args)
    {
        return Callback<false, ReturnType, void, Args...>(
            func, 
            std::forward<Args>(args)...);
    }

    template <typename ReturnType, typename T, typename... Args>
    Callback<true, ReturnType, T, Args...> 
        Bind(ReturnType(T::*func)(Args...), std::weak_ptr<T> &weakptr, Args... args)
    {
        return Callback<true, ReturnType, T, Args...>(
            func,
            weakptr,
            std::forward<Args>(args)...);
    }

    // posttask
    template <typename ReturnType, typename... Args>
    void PostTask(int tid, Callback<false, ReturnType, void, Args...>&& closure)
    {
        if (g_thread_map.find(tid) != g_thread_map.end())
        {
            g_thread_map[tid]->PostTask(std::move(closure));
        }
    }

    template <typename ReturnType, typename T, typename... Args>
    void PostTask(int tid, Callback<true, ReturnType, T, Args...> &&closure)
    {
        if (g_thread_map.find(tid) != g_thread_map.end())
        {
            g_thread_map[tid]->PostTask(std::move(closure));
        }
    }

    // reply
    template<bool IsReturnVoid, bool IsMemberFunc>
    struct ReplyHelper;

    // class member function
    template<>
    class ReplyHelper<false, true>
    {
    public:
        template <typename RReturnType, typename RT, typename... RArgs, typename TaskReturnType>
        static void ReplyAdapter(
            std::shared_ptr<Callback<true, RReturnType, RT, RArgs...>> reply,
            TaskReturnType *result
            )
        {
            std::unique_ptr<TaskReturnType> ptr(result);
            reply->RunWithParam(ptr.get());
        }

        template <typename TReturnType, typename TT, typename... TArgs, typename RReturnType, typename RT, typename... RArgs>
        static void ReturnAsParamAdapter(
            std::shared_ptr<Callback<true, TReturnType, TT, TArgs...>> task,
            std::shared_ptr<Callback<true, RReturnType, RT, RArgs...>> reply
            )
        {
            std::shared_ptr<CThread> thd = CThread::GetThread(reply->get_post_thread_id());
            if (thd)
            {
                using rtype = std::decay<Callback<true, RReturnType, RT, RArgs...>>::type;

                std::unique_ptr<TReturnType> result(new TReturnType(task->RunWithResult()));

                thd->PostTask(Bind<void, std::shared_ptr<rtype>, TReturnType*>(
                    ReplyAdapter,
                    reply,
                    result.release())
                    );
            }
        }
    };

    template<>
    class ReplyHelper<true, true>
    {
    public:
        template <typename RReturnType, typename RT, typename... RArgs>
        static void ReplyAdapter(
            std::shared_ptr<Callback<true, RReturnType, RT, RArgs...>> reply
            )
        {
            reply->Run();
        }

        template <typename TReturnType, typename TT, typename... TArgs, typename RReturnType, typename RT, typename... RArgs>
        static void ReturnAsParamAdapter(
            std::shared_ptr<Callback<true, TReturnType, TT, TArgs...>> task,
            std::shared_ptr<Callback<true, RReturnType, RT, RArgs...>> reply
            )
        {
            std::shared_ptr<CThread> thd = CThread::GetThread(reply->get_post_thread_id());
            if (thd)
            {
                using rtype = std::decay<Callback<true, RReturnType, RT, RArgs...>>::type;

                task->Run();

                thd->PostTask(Bind<void, std::shared_ptr<rtype>>(
                    ReplyAdapter,
                    reply)
                    );
            }
        }
    };

    // global function
    template<>
    class ReplyHelper<false, false>
    {
    public:
        template <typename RReturnType, typename... RArgs, typename TaskReturnType>
        static void ReplyAdapter(
            std::shared_ptr<Callback<false, RReturnType, void, RArgs...>> reply,
            TaskReturnType *ret
            )
        {
            std::unique_ptr<TaskReturnType> result(ret);
            reply->RunWithParam(result.get());
        }

        template <typename TReturnType, typename... TArgs, typename RReturnType, typename... RArgs>
        static void ReturnAsParamAdapter(
            std::shared_ptr<Callback<false, TReturnType, void, TArgs...>> task,
            std::shared_ptr<Callback<false, RReturnType, void, RArgs...>> reply
            )
        {
            std::shared_ptr<CThread> thd = CThread::GetThread(reply->get_post_thread_id());
            if (thd)
            {
                using rtype = std::decay<Callback<false, RReturnType, void, RArgs...>>::type;

                std::unique_ptr<TReturnType> result(new TReturnType(task->RunWithResult()));

                thd->PostTask(Bind<void, std::shared_ptr<rtype>, TReturnType*>(
                    ReplyAdapter,
                    reply,
                    result.release())
                    );
            }
        }
    };

    template<>
    class ReplyHelper<true, false>
    {
    public:
        template <typename RReturnType, typename... RArgs>
        static void ReplyAdapter(
            std::shared_ptr<Callback<false, RReturnType, void, RArgs...>> reply
            )
        {
            reply->Run();
        }

        template <typename TReturnType, typename... TArgs, typename RReturnType, typename... RArgs>
        static void ReturnAsParamAdapter(
            std::shared_ptr<Callback<false, TReturnType, void, TArgs...>> task,
            std::shared_ptr<Callback<false, RReturnType, void, RArgs...>> reply
            )
        {
            std::shared_ptr<CThread> thd = CThread::GetThread(reply->get_post_thread_id());
            if (thd)
            {
                using rtype = std::decay<Callback<false, RReturnType, void, RArgs...>>::type;

                task->Run();

                thd->PostTask(Bind<void, std::shared_ptr<rtype>>(
                    ReplyAdapter,
                    reply)
                    );
            }
        }
    };

    // postandreply
    template<bool IsMemberFunc>
    class PostHelper;
    
    template<>
    class PostHelper<true>
    {
    public:
        template <typename TReturnType, typename TT, typename... TArgs, typename RReturnType, typename RT, typename... RArgs>
        static void PostTaskAndReplyWithResult(int tid,
            Callback<true, TReturnType, TT, TArgs...> &&task,
            Callback<true, RReturnType, RT, RArgs...> &&reply)
        {
            if (g_thread_map.find(tid) != g_thread_map.end())
            {
                using ttype = std::decay<Callback<true, TReturnType, TT, TArgs...>>::type;
                using rtype = std::decay<Callback<true, RReturnType, RT, RArgs...>>::type;

                g_thread_map[tid]->PostTask(Bind<void, std::shared_ptr<ttype>, std::shared_ptr<rtype>>(
                    ReplyHelper<std::is_same<TReturnType, void>::value, true>::ReturnAsParamAdapter,
                    std::shared_ptr<ttype>(new ttype(task)),
                    std::shared_ptr<rtype>(new rtype(reply)))
                    );
            }
        }
    };

    template<>
    class PostHelper<false>
    {
    public:
        template <typename TReturnType, typename... TArgs, typename RReturnType, typename... RArgs>
        static void PostTaskAndReplyWithResult(int tid,
            Callback<false, TReturnType, void, TArgs...> &&task,
            Callback<false, RReturnType, void, RArgs...> &&reply)
        {
            if (g_thread_map.find(tid) != g_thread_map.end())
            {
                using ttype = std::decay<Callback<false, TReturnType, void, TArgs...>>::type;
                using rtype = std::decay<Callback<false, RReturnType, void, RArgs...>>::type;

                g_thread_map[tid]->PostTask(Bind<void, std::shared_ptr<ttype>, std::shared_ptr<rtype>>(
                    ReplyHelper<std::is_same<TReturnType, void>::value, false>::ReturnAsParamAdapter,
                    std::shared_ptr<ttype>(new ttype(task)),
                    std::shared_ptr<rtype>(new rtype(reply)))
                    );
            }
        }

    };

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

        template <class ReturnType, typename... Args>
        void PostTask(Callback<false, ReturnType, void, Args...> &&closure)
        {
            if (!thread_)
            {
                assert(false);
                return;
            }

            PostBaseTask(std::shared_ptr<CallbackBase>(new Callback<false, ReturnType, void, Args...>(std::move(closure))));
        }

        template <class ReturnType, typename T, typename... Args>
        void PostTask(Callback<true, ReturnType, T, Args...> &&closure)
        {
            if (!thread_)
            {
                assert(false);
                return;
            }

            PostBaseTask(std::shared_ptr<CallbackBase>(new Callback<true, ReturnType, T, Args...>(std::move(closure))));
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
        print_func("async_call_void");
    }

    void on_async_call_void()
    {
        print_func("on_async_call_void");
    }

    std::string async_call_string(float ff)
    {
        print_func(StringPrintf("async_call_string: %f", ff).c_str());
        return "async_call";
    }

    void on_async_call_string(const std::string &str)
    {
        print_func(StringPrintf("on_async_call_string: %s", str.c_str()).c_str());
    }

    std::string async_call_value(float ff)
    {
        print_func(StringPrintf("async_call_value: %f", ff).c_str());
        return "async_call";
    }

    void on_async_call_value(std::string ch)
    {
        print_func(StringPrintf("on_async_call_value: %s", ch.c_str()).c_str());
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
            print_func("print_void");
        }

        void on_print_void()
        {
            print_func("on_print_void");
        }

        int print_param(int i)
        {
            print_func(StringPrintf("print_param: %d", i).c_str());
            return i;
        }

        void on_print_param(int ret)
        {
            print_func(StringPrintf("on_print_param: %d", ret).c_str());
        }

        void post_task_and_reply(int index)
        {
            print_func("post_task_and_reply");

            int i = 2 - index;
            i = (i == index) ? 0 : i;
            /*PostHelper<true>::PostTaskAndReplyWithResult(i,
                Bind(&WeakptrTest::print_param, GetWeakPtr(shared_from_this()), index),
                Bind(&WeakptrTest::on_print_param, GetWeakPtr(shared_from_this()), 0)
                );*/
            /*PostHelper<true>::PostTaskAndReplyWithResult(i,
                Bind(&WeakptrTest::print_void, GetWeakPtr(shared_from_this())),
                Bind(&WeakptrTest::on_print_void, GetWeakPtr(shared_from_this()))
                );*/
            /*PostHelper<false>::PostTaskAndReplyWithResult(i,
                Bind(async_call_void),
                Bind(on_async_call_void)
                );*/
            std::string ss;
            PostHelper<false>::PostTaskAndReplyWithResult(i,
                Bind(async_call_value, 3.14f),
                Bind(on_async_call_value, ss)
                );
        }

    private:
        int id_ = 0;
    };

    void mutex_test()
    {
        /*std::timed_mutex;
        std::ratio;
        std::chrono::milliseconds;
        std::chrono::system_clock;*/
        //std::lock_guard<std::mutex> guard;
        //std::unique_lock<std::mutex> lck;
    }
}


void thread_atomic_study()
{
    /*int intpram = 111;
    auto cb = Bind(print_func, "thread_atomic_study");
    cb.Run();
    auto ret = cb.RunWithResult();
    Invoke(print_func, std::make_tuple("thread_atomic_study"));*/

    /*std::shared_ptr<WeakptrTest> tobj(new WeakptrTest());
    auto cbw = Bind(&WeakptrTest::print_param, std::weak_ptr<WeakptrTest>(tobj), 1212);
    Invoke(&WeakptrTest::print_param, tobj.get(), std::make_tuple(2019));
    cbw.Run();
    auto retw = cbw.RunWithResult();*/

    g_thread_map[0] = std::shared_ptr<CThread>(new CThread());
    g_thread_map[1] = std::shared_ptr<CThread>(new CThread());
    g_thread_map[2] = std::shared_ptr<CThread>(new CThread());
    for (auto &thd : g_thread_map)
    {
        thd.second->Run();
    }

    std::shared_ptr<WeakptrTest> obj(new WeakptrTest());
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
                PostTask(thd, Bind(&WeakptrTest::print_void, GetWeakPtr(tobj)));
            }
                break;
            case 1:
                PostTask(thd, Bind(&WeakptrTest::print_param, GetWeakPtr(obj), i));
                break;
            case 2:
                PostTask(thd, Bind(&WeakptrTest::post_task_and_reply, GetWeakPtr(obj), thd));
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
}