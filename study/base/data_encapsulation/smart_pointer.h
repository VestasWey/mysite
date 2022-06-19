#pragma once

#include <memory>
#include <wtypes.h>
#include <objbase.h>

#define DEFINE_SMART_POINTER(type) \
    class type;\
    typedef std::shared_ptr<type> ##type##Ref;\
    typedef std::weak_ptr<type> ##type##Weak;\
    typedef std::unique_ptr<type> Scoped##type##;

namespace mctm
{
    class ScopedHandle
    {
    public:
        ScopedHandle();
        explicit ScopedHandle(HANDLE handle);
        ScopedHandle(ScopedHandle& rht);
        ~ScopedHandle();

        void SetHandle(HANDLE handle);
        HANDLE handle() const;
        void Close();
        HANDLE Detach();
        bool IsValid() const;

        operator HANDLE() const
        {
            return handle_;
        }

    private:
        HANDLE handle_ = nullptr;
    };

    class ScopedCOMInitializer
    {
    public:
        // COINIT_APARTMENTTHREADED COINIT_MULTITHREADED
        ScopedCOMInitializer(COINIT init = COINIT_APARTMENTTHREADED);
        ~ScopedCOMInitializer();

    private:
        HRESULT result_ = RPC_E_CHANGED_MODE;
    };

    DEFINE_SMART_POINTER(MessageLoop);
    DEFINE_SMART_POINTER(MessagePump);
    DEFINE_SMART_POINTER(IOBuffer);
    using SingleThreadTaskRunner = MessageLoopRef;
}

