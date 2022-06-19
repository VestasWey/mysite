#include "smart_pointer.h"


namespace mctm
{
    // ScopedHandle
    ScopedHandle::ScopedHandle()
    {
    }

    ScopedHandle::ScopedHandle(HANDLE handle)
        : handle_(handle)
    {
    }
    
    ScopedHandle::ScopedHandle(ScopedHandle& rht)
    {
        SetHandle(rht.Detach());
    }

    ScopedHandle::~ScopedHandle()
    {
        Close();
    }

    void ScopedHandle::SetHandle(HANDLE handle)
    {
        Close();

        handle_ = handle;
    }

    HANDLE ScopedHandle::handle() const
    {
        return handle_;
    }

    void ScopedHandle::Close()
    {
        if (!!handle_ && handle_ != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(handle_);
            handle_ = nullptr;
        }
    }


    HANDLE ScopedHandle::Detach()
    {
        HANDLE tmp = handle_;
        handle_ = nullptr;
        return tmp;
    }

    bool ScopedHandle::IsValid() const
    {
        return (!!handle_ && handle_ != INVALID_HANDLE_VALUE);
    }


    // ScopedCOMInitializer
    ScopedCOMInitializer::ScopedCOMInitializer(COINIT init /*= COINIT_APARTMENTTHREADED*/)
    {
        result_ = ::CoInitializeEx(nullptr, init);
    }

    ScopedCOMInitializer::~ScopedCOMInitializer()
    {
        if (SUCCEEDED(result_))
        {
            ::CoUninitialize();
        }
    }

}