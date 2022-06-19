#pragma once

#include <wtypes.h>

namespace mctm
{
    class WaitableEvent
    {
    public:
        WaitableEvent(bool manual, bool initial, const wchar_t* name = nullptr);
        ~WaitableEvent();

        void Signal();
        bool Wait() const;
        bool TimedWait(DWORD dwMilliseconds) const;
        void Close();
        void Reset();
        bool IsSignaled() const;

        operator HANDLE() const { return event_handle_; }

    private:
        HANDLE event_handle_ = nullptr;
    };
}

