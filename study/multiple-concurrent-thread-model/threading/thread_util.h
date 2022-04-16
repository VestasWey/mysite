#pragma once
#include <map>
#include <mutex>
#include <string>
#include <processthreadsapi.h>


namespace mctm
{
    void SetThreadName(DWORD dwThreadID, const char* name);

    class ThreadLocalStorage
    {
    public:
        ThreadLocalStorage()
        {
            slot_ = ::TlsAlloc();
            Set(0);
        }

        ~ThreadLocalStorage()
        {
            if (TLS_OUT_OF_INDEXES != slot_)
            {
                ::TlsFree(slot_);
            }
        }

        void* Get()
        {
            if (TLS_OUT_OF_INDEXES != slot_)
            {
                return ::TlsGetValue(slot_);
            }
            return nullptr;
        }

        void Set(void* ptr)
        {
            if (TLS_OUT_OF_INDEXES != slot_)
            {
                ::TlsSetValue(slot_, ptr);
            }
        }

    private:
        DWORD slot_ = TLS_OUT_OF_INDEXES;
    };

    template <class Type>
    class ThreadLocalPointer
    {
    public:
        ThreadLocalPointer()
        {
        }

        Type* Get()
        {
            return static_cast<Type*>(tls_.Get());
        }

        void Set(Type* ptr)
        {
            tls_.Set((void*)ptr);
        }

    private:
        ThreadLocalStorage tls_;
    };

    template <class Type>
    class ThreadSingletonInstance
    {
    public:
        explicit ThreadSingletonInstance(const char* prefix_key)
            : instance_prefix_key_(prefix_key)
            , slots_mutex_(std::make_unique<std::recursive_mutex>())
        {
        }

        ThreadSingletonInstance(const ThreadSingletonInstance<Type>& rht)
        {
            *this = rht;
        }

        ThreadSingletonInstance<Type>& operator=(const ThreadSingletonInstance<Type>& rht)
        {
            *this = std::move(rht);
            return *this;
        }

        Type* Pointer()
        {
            DWORD tid = ::GetCurrentThreadId();
            std::string key = instance_prefix_key_ + "_" + std::to_string(tid);
            std::lock_guard<std::recursive_mutex> lock(*slots_mutex_);
            if (slots_.find(key) == slots_.end())
            {
                slots_[key] = std::make_unique<Type>();
            }
            return slots_[key].get();
        }

        void RemoveInstance()
        {
            DWORD tid = ::GetCurrentThreadId();
            std::string key = instance_prefix_key_ + "_" + std::to_string(tid);
            std::lock_guard<std::recursive_mutex> lock(*slots_mutex_);
            slots_.erase(key);
        }

    private:
        std::string instance_prefix_key_;
        std::unique_ptr<std::recursive_mutex> slots_mutex_;
        std::map<std::string, std::unique_ptr<Type>> slots_;
    };
}

#define THREAD_SINGLETON_INSTANCE_INITIALIZER(type) \
    mctm::ThreadSingletonInstance<type>(std::string(__FILE__":" + std::to_string(__LINE__)).c_str());