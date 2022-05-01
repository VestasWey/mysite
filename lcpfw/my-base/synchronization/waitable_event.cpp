#include "waitable_event.h"

#include <handleapi.h>
#include <synchapi.h>
#include <WinBase.h>


namespace mctm
{
    WaitableEvent::WaitableEvent(bool manual, bool initial, const wchar_t* name/* = nullptr*/)
    {
        event_handle_ = ::CreateEventW(nullptr, manual, initial, name);
    }

    WaitableEvent::~WaitableEvent()
    {
        Close();
    }

    void WaitableEvent::Signal()
    {
        if (event_handle_)
        {
            ::SetEvent(event_handle_);
        }
    }

    bool WaitableEvent::Wait() const
    {
        return TimedWait(INFINITE);
    }

    bool WaitableEvent::TimedWait(DWORD dwMilliseconds) const
    {
        if (event_handle_)
        {
            DWORD dw = ::WaitForSingleObject(event_handle_, dwMilliseconds);
            return (dw == WAIT_OBJECT_0);
        }
        return false;
    }

    void WaitableEvent::Close()
    {
        if (event_handle_)
        {
            ::CloseHandle(event_handle_);
            event_handle_ = nullptr;
        }
    }

    void WaitableEvent::Reset()
    {
        if (event_handle_)
        {
            ::ResetEvent(event_handle_);
        }
    }

    bool WaitableEvent::IsSignaled() const
    {
        if (event_handle_)
        {
            return TimedWait(0);
        }
        return false;
    }

}
