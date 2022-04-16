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


/*
 * 
 */

using namespace std::placeholders;

// https://www.cnblogs.com/qicosmos/p/4325949.html 可变模版参数
namespace
{
    class TestBindCls
    {
    public:
        int Run(const std::string& str)
        {
            std::cout << "TestBindCls::Run -> '" << str << "'" << std::endl;
            return -2233;
        }

        int RunConst(const std::string& str) const
        {
            std::cout << "TestBindCls::Run -> '" << str << "'" << std::endl;
            return 2233;
        }

        static TestBindCls CreateObject(const std::string& str)
        {
            std::cout << "TestBindCls::CreateObject -> '" << str << "'" << std::endl;
            return TestBindCls();
        }
    };

    int GlobalRun(const std::string& str)
    {
        std::cout << "GlobalRun -> '" << str << "'" << std::endl;
        return -3454;
    }
}

namespace
{
    class Closure;
    class CallbackBase
    {
    public:
        virtual ~CallbackBase() = default;

    private:
        friend class Closure;
        virtual void BaseRun() = 0;
    };

    template <class Sig, class Functor>
    class Callback;

    template <class R, class T, class... FnArgs, class Method>
    class Callback<R(T::*)(FnArgs...), Method> : public CallbackBase
    {
    public:
        Callback(Method& method, std::weak_ptr<T> weak_ptr)
            : method_(method)
            , weak_ptr_(weak_ptr)
        {
        }

        template <class... Args>
        R Run(Args&& ...args)
        {
            std::shared_ptr<T> sp = weak_ptr_.lock();
            if (!sp)
            {
                return R();
            }

            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        Method method_;
        std::weak_ptr<T> weak_ptr_;
    };

    template <class R, class T, class... FnArgs, class Method>
    class Callback<R(T::*)(FnArgs...)const, Method> : public CallbackBase
    {
    public:
        Callback(Method& method, std::weak_ptr<T> weak_ptr)
            : method_(method)
            , weak_ptr_(weak_ptr)
        {
        }

        template <class... Args>
        R Run(Args&& ...args) const
        {
            std::shared_ptr<T> sp = weak_ptr_.lock();
            if (!sp)
            {
                return R();
            }

            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        Method method_;
        std::weak_ptr<T> weak_ptr_;
    };

    template <class R, class... FnArgs, class Functor>
    class Callback<R(*)(FnArgs...), Functor> : public CallbackBase
    {
    public:
        explicit Callback(Functor& functor)
            : functor_(functor)
        {
        }

        template <class... Args>
        R Run(Args&& ...args)
        {
            return functor_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        Functor functor_;
    };

    class Closure
    {
    public:
        template <class R, class T, class... FnArgs, class Method>
        Closure(const Callback<R(T::*)(FnArgs...), Method>& callback)
        {
            callback_ = std::make_shared<Callback<R(T::*)(FnArgs...), Method>>(callback);
        }

        template <class R, class T, class... FnArgs, class Method>
        Closure(const Callback<R(T::*)(FnArgs...)const, Method>& callback)
        {
            callback_ = std::make_shared<Callback<R(T::*)(FnArgs...)const, Method>>(callback);
        }

        template <class R, class... FnArgs, class Functor>
        Closure(const Callback<R(*)(FnArgs...), Functor>& callback)
        {
            callback_ = std::make_shared<Callback<R(*)(FnArgs...), Functor>>(callback);
        }

        void Run()
        {
            if (callback_)
            {
                callback_->BaseRun();
            }
        }

    private:
        std::shared_ptr<CallbackBase> callback_;
    };

    template <class R, class T, class... FnArgs, class... Args>
    auto Bind(R(T::*method)(FnArgs...), std::weak_ptr<T> wp, Args&& ...args)
    {
        auto std_binder = std::bind(method, wp.lock().get(), std::forward<Args>(args)...);
        using CallbackType = Callback<R(T::*)(FnArgs...), decltype(std_binder)>;
        return CallbackType(std_binder, wp);
    }

    template <class R, class T, class... FnArgs, class... Args>
    auto Bind(R(T::*method)(FnArgs...)const, std::weak_ptr<T> wp, Args&& ...args)
    {
        auto std_binder = std::bind(method, wp.lock().get(), std::forward<Args>(args)...);
        using CallbackType = Callback<R(T::*)(FnArgs...)const, decltype(std_binder)>;
        return CallbackType(std_binder, wp);
    }

    template <class R, class... FnArgs, class... Args>
    auto Bind(R(*method)(FnArgs...), Args&& ...args)
    {
        auto std_binder = std::bind(method, std::forward<Args>(args)...);
        using CallbackType = Callback<R(*)(FnArgs...), decltype(std_binder)>;
        return CallbackType(std_binder);
    }
}


void thread_std_bind_task_study()
{
    std::shared_ptr<TestBindCls> st = std::make_shared<TestBindCls>();
    std::weak_ptr<TestBindCls> wp(st);
    auto sfn(std::bind(&TestBindCls::Run, st, _1));
    sfn("asd");

    /*auto fn = Bind(&TestBindCls::RunConst, wp, "12312");
    auto ret = fn.Run();
    std::cout << "RunResult -> '" << ret << "'" << std::endl;

    Closure cls(Bind(&TestBindCls::RunConst, wp, "fsdfds"));
    Closure cc = cls;
    Closure cl(cls);
    cl.Run();*/

    /*auto gfn = Bind(GlobalRun, "12312");
    auto gret = gfn.Run();
    std::cout << "GlobalRun -> '" << gret << "'" << std::endl;

    Closure cls(Bind(GlobalRun, "fsdfds"));
    Closure cc = cls;
    Closure cl(cls);
    cl.Run();*/

    auto gfn = Bind(TestBindCls::CreateObject, "12312");
    auto gret = gfn.Run();
    //std::cout << "CreateObject -> '" << gret << "'" << std::endl;

    Closure cls(Bind(TestBindCls::CreateObject, "fsdfds"));
    Closure cc = cls;
    Closure cl(cls);
    cl.Run();

    return;
}