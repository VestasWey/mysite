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

    enum CallbackPtrType
    {
        raw,
        weak,
        shared
    };
    template <class Sig, class Functor, CallbackPtrType PtrType>
    class Callback;

    // weak pointer
    template <class _Ret, class T, class... FnArgs, class _Fx>
    class Callback<_Ret(T::*)(FnArgs...), _Fx, weak> : public CallbackBase
    {
    public:
        Callback(_Fx& method, std::weak_ptr<T> weak_ptr)
            : method_(method)
            , weak_ptr_(weak_ptr)
        {
        }

        template <class... Args>
        _Ret Run(Args&& ...args)
        {
            std::shared_ptr<T> sp = weak_ptr_.lock();
            if (!sp)
            {
                return _Ret();
            }

            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        _Fx method_;
        std::weak_ptr<T> weak_ptr_;
    };

    template <class _Ret, class T, class... FnArgs, class _Fx>
    class Callback<_Ret(T::*)(FnArgs...)const, _Fx, weak> : public CallbackBase
    {
    public:
        Callback(_Fx& method, std::weak_ptr<T> weak_ptr)
            : method_(method)
            , weak_ptr_(weak_ptr)
        {
        }

        template <class... Args>
        _Ret Run(Args&& ...args) const
        {
            std::shared_ptr<T> sp = weak_ptr_.lock();
            if (!sp)
            {
                return _Ret();
            }

            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        _Fx method_;
        std::weak_ptr<T> weak_ptr_;
    };

    // shared pointer
    template <class _Ret, class T, class... FnArgs, class _Fx>
    class Callback<_Ret(T::*)(FnArgs...), _Fx, shared> : public CallbackBase
    {
    public:
        Callback(_Fx& method, std::shared_ptr<T> shared_ptr)
            : method_(method)
            , shared_ptr_(shared_ptr)
        {
        }

        template <class... Args>
        _Ret Run(Args&& ...args)
        {
            if (!shared_ptr_)
            {
                return _Ret();
            }

            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        _Fx method_;
        std::shared_ptr<T> shared_ptr_;
    };

    template <class _Ret, class T, class... FnArgs, class _Fx>
    class Callback<_Ret(T::*)(FnArgs...)const, _Fx, shared> : public CallbackBase
    {
    public:
        Callback(_Fx& method, std::shared_ptr<T> shared_ptr)
            : method_(method)
            , shared_ptr_(shared_ptr)
        {
        }

        template <class... Args>
        _Ret Run(Args&& ...args) const
        {
            if (!shared_ptr_)
            {
                return _Ret();
            }

            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        _Fx method_;
        std::shared_ptr<T> shared_ptr_;
    };

    // raw pointer
    template <class _Ret, class T, class... FnArgs, class _Fx>
    class Callback<_Ret(T::*)(FnArgs...), _Fx, raw> : public CallbackBase
    {
    public:
        Callback(_Fx& method, T* ptr)
            : method_(method)
            , ptr_(ptr)
        {
        }

        template <class... Args>
        _Ret Run(Args&& ...args)
        {
            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        _Fx method_;
        T* ptr_ = nullptr;
    };

    template <class _Ret, class T, class... FnArgs, class _Fx>
    class Callback<_Ret(T::*)(FnArgs...)const, _Fx, raw> : public CallbackBase
    {
    public:
        Callback(_Fx& method, T* ptr)
            : method_(method)
            , ptr_(ptr)
        {
        }

        template <class... Args>
        _Ret Run(Args&& ...args)
        {
            return method_(args...);
        }

    protected:
        void BaseRun() override
        {
            Run();
        }

    private:
        _Fx method_;
        T* ptr_ = nullptr;
    };

    // global func
    template <class _Ret, class... FnArgs, class Functor>
    class Callback<_Ret(*)(FnArgs...), Functor, raw> : public CallbackBase
    {
    public:
        explicit Callback(Functor& functor)
            : functor_(functor)
        {
        }

        template <class... Args>
        _Ret Run(Args&& ...args)
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

        // weak pointer
        template <class _Ret, class T, class... FnArgs, class _Fx>
        Closure(const Callback<_Ret(T::*)(FnArgs...), _Fx, weak>& callback)
        {
            callback_ = std::make_shared<Callback<_Ret(T::*)(FnArgs...), _Fx, weak>>(callback);
        }

        template <class _Ret, class T, class... FnArgs, class _Fx>
        Closure(const Callback<_Ret(T::*)(FnArgs...)const, _Fx, weak>& callback)
        {
            callback_ = std::make_shared<Callback<_Ret(T::*)(FnArgs...)const, _Fx, weak>>(callback);
        }

        // shared pointer
        template <class _Ret, class T, class... FnArgs, class _Fx>
        Closure(const Callback<_Ret(T::*)(FnArgs...), _Fx, shared>& callback)
        {
            callback_ = std::make_shared<Callback<_Ret(T::*)(FnArgs...), _Fx, shared>>(callback);
        }

        template <class _Ret, class T, class... FnArgs, class _Fx>
        Closure(const Callback<_Ret(T::*)(FnArgs...)const, _Fx, shared>& callback)
        {
            callback_ = std::make_shared<Callback<_Ret(T::*)(FnArgs...)const, _Fx, shared>>(callback);
        }

        // raw pointer
        template <class _Ret, class T, class... FnArgs, class _Fx>
        Closure(const Callback<_Ret(T::*)(FnArgs...), _Fx, raw>& callback)
        {
            callback_ = std::make_shared<Callback<_Ret(T::*)(FnArgs...), _Fx, raw>>(callback);
        }

        template <class _Ret, class T, class... FnArgs, class _Fx>
        Closure(const Callback<_Ret(T::*)(FnArgs...)const, _Fx, raw>& callback)
        {
            callback_ = std::make_shared<Callback<_Ret(T::*)(FnArgs...)const, _Fx, raw>>(callback);
        }

        // global func
        template <class _Ret, class... FnArgs, class Functor>
        Closure(const Callback<_Ret(*)(FnArgs...), Functor, raw>& callback)
        {
            callback_ = std::make_shared<Callback<_Ret(*)(FnArgs...), Functor, raw>>(callback);
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

    // weak pointer
    template <class _Ret, class T, class... FnArgs, class... Args>
    auto Bind(_Ret(T::*method)(FnArgs...), std::weak_ptr<T> wp, Args&& ...args)
    {
        auto std_binder = std::bind(method, wp.lock().get(), std::forward<Args>(args)...);
        using CallbackType = Callback<_Ret(T::*)(FnArgs...), decltype(std_binder), weak>;
        return CallbackType(std_binder, wp);
    }

    template <class _Ret, class T, class... FnArgs, class... Args>
    auto Bind(_Ret(T::*method)(FnArgs...)const, std::weak_ptr<T> wp, Args&& ...args)
    {
        auto std_binder = std::bind(method, wp.lock().get(), std::forward<Args>(args)...);
        using CallbackType = Callback<_Ret(T::*)(FnArgs...)const, decltype(std_binder), weak>;
        return CallbackType(std_binder, wp);
    }

    // shared pointer
    template <class _Ret, class T, class... FnArgs, class... Args>
    auto Bind(_Ret(T::* method)(FnArgs...), std::shared_ptr<T> sp, Args&& ...args)
    {
        auto std_binder = std::bind(method, sp, std::forward<Args>(args)...);
        using CallbackType = Callback<_Ret(T::*)(FnArgs...), decltype(std_binder), shared>;
        return CallbackType(std_binder, sp);
    }

    template <class _Ret, class T, class... FnArgs, class... Args>
    auto Bind(_Ret(T::* method)(FnArgs...)const, std::shared_ptr<T> sp, Args&& ...args)
    {
        auto std_binder = std::bind(method, sp, std::forward<Args>(args)...);
        using CallbackType = Callback<_Ret(T::*)(FnArgs...)const, decltype(std_binder), shared>;
        return CallbackType(std_binder, sp);
    }

    // raw pointer
    template <class _Ret, class T, class... FnArgs, class... Args>
    auto Bind(_Ret(T::*method)(FnArgs...), T* ptr, Args&& ...args)
    {
        auto std_binder = std::bind(method, ptr, std::forward<Args>(args)...);
        using CallbackType = Callback<_Ret(T::*)(FnArgs...), decltype(std_binder), raw>;
        return CallbackType(std_binder, ptr);
    }

    template <class _Ret, class T, class... FnArgs, class... Args>
    auto Bind(_Ret(T::*method)(FnArgs...)const, T* ptr, Args&& ...args)
    {
        auto std_binder = std::bind(method, ptr, std::forward<Args>(args)...);
        using CallbackType = Callback<_Ret(T::*)(FnArgs...)const, decltype(std_binder), raw>;
        return CallbackType(std_binder, ptr);
    }

    // global func
    template <class _Ret, class... FnArgs, class... Args>
    auto Bind(_Ret(*method)(FnArgs...), Args&& ...args)
    {
        auto std_binder = std::bind(method, std::forward<Args>(args)...);
        using CallbackType = Callback<_Ret(*)(FnArgs...), decltype(std_binder), raw>;
        return CallbackType(std_binder);
    }
}