#pragma once

#include <memory>
#include <functional>

namespace mctm
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

    template <class Sig, class Functor, bool IsRawPtr>
    class Callback;

    // smart pointer
    template <class R, class T, class... FnArgs, class Method>
    class Callback<R(T::*)(FnArgs...), Method, false> : public CallbackBase
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
    class Callback<R(T::*)(FnArgs...)const, Method, false> : public CallbackBase
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

    // raw pointer
    template <class R, class T, class... FnArgs, class Method>
    class Callback<R(T::*)(FnArgs...), Method, true> : public CallbackBase
    {
    public:
        Callback(Method& method, T* ptr)
            : method_(method)
            , ptr_(ptr)
        {
        }

        template <class... Args>
        R Run(Args&& ...args)
        {
            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        Method method_;
        T* ptr_ = nullptr;
    };

    template <class R, class T, class... FnArgs, class Method>
    class Callback<R(T::*)(FnArgs...)const, Method, true> : public CallbackBase
    {
    public:
        Callback(Method& method, T* ptr)
            : method_(method)
            , ptr_(ptr)
        {
        }

        template <class... Args>
        R Run(Args&& ...args)
        {
            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        Method method_;
        T* ptr_ = nullptr;
    };

    // global func
    template <class R, class... FnArgs, class Functor>
    class Callback<R(*)(FnArgs...), Functor, false> : public CallbackBase
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
        Closure() = default;

        // smart pointer
        template <class R, class T, class... FnArgs, class Method>
        Closure(const Callback<R(T::*)(FnArgs...), Method, false>& callback)
        {
            callback_ = std::make_shared<Callback<R(T::*)(FnArgs...), Method, false>>(callback);
        }

        template <class R, class T, class... FnArgs, class Method>
        Closure(const Callback<R(T::*)(FnArgs...)const, Method, false>& callback)
        {
            callback_ = std::make_shared<Callback<R(T::*)(FnArgs...)const, Method, false>>(callback);
        }

        // raw pointer
        template <class R, class T, class... FnArgs, class Method>
        Closure(const Callback<R(T::*)(FnArgs...), Method, true>& callback)
        {
            callback_ = std::make_shared<Callback<R(T::*)(FnArgs...), Method, true>>(callback);
        }

        template <class R, class T, class... FnArgs, class Method>
        Closure(const Callback<R(T::*)(FnArgs...)const, Method, true>& callback)
        {
            callback_ = std::make_shared<Callback<R(T::*)(FnArgs...)const, Method, true>>(callback);
        }

        // global func
        template <class R, class... FnArgs, class Functor>
        Closure(const Callback<R(*)(FnArgs...), Functor, false>& callback)
        {
            callback_ = std::make_shared<Callback<R(*)(FnArgs...), Functor, false>>(callback);
        }

        void Run() const
        {
            if (callback_)
            {
                callback_->BaseRun();
            }
        }

        void Reset()
        {
            callback_.reset();
        }
        
        bool Equals(const Closure& other) const
        {
            return callback_.get() == other.callback_.get();
        }

    private:
        std::shared_ptr<CallbackBase> callback_;
    };

    // smart pointer
    template <class R, class T, class... FnArgs, class... Args>
    auto Bind(R(T::*method)(FnArgs...), std::weak_ptr<T> wp, Args&& ...args)
    {
        auto std_binder = std::bind(method, wp.lock().get(), std::forward<Args>(args)...);
        using CallbackType = Callback<R(T::*)(FnArgs...), decltype(std_binder), false>;
        return CallbackType(std_binder, wp);
    }

    template <class R, class T, class... FnArgs, class... Args>
    auto Bind(R(T::*method)(FnArgs...)const, std::weak_ptr<T> wp, Args&& ...args)
    {
        auto std_binder = std::bind(method, wp.lock().get(), std::forward<Args>(args)...);
        using CallbackType = Callback<R(T::*)(FnArgs...)const, decltype(std_binder), false>;
        return CallbackType(std_binder, wp);
    }

    // raw pointer
    template <class R, class T, class... FnArgs, class... Args>
    auto Bind(R(T::*method)(FnArgs...), T* ptr, Args&& ...args)
    {
        auto std_binder = std::bind(method, ptr, std::forward<Args>(args)...);
        using CallbackType = Callback<R(T::*)(FnArgs...), decltype(std_binder), true>;
        return CallbackType(std_binder, ptr);
    }

    template <class R, class T, class... FnArgs, class... Args>
    auto Bind(R(T::*method)(FnArgs...)const, T* ptr, Args&& ...args)
    {
        auto std_binder = std::bind(method, ptr, std::forward<Args>(args)...);
        using CallbackType = Callback<R(T::*)(FnArgs...)const, decltype(std_binder), true>;
        return CallbackType(std_binder, ptr);
    }

    // global func
    template <class R, class... FnArgs, class... Args>
    auto Bind(R(*method)(FnArgs...), Args&& ...args)
    {
        auto std_binder = std::bind(method, std::forward<Args>(args)...);
        using CallbackType = Callback<R(*)(FnArgs...), decltype(std_binder), false>;
        return CallbackType(std_binder);
    }
}