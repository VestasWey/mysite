#pragma once
#include <mutex>
#include <windows.h>

namespace mctm
{
    class ThreadChecker
    {
    public:
        ThreadChecker();
        ~ThreadChecker();

        bool CalledOnValidThread() const;

        // Changes the thread that is checked for in CalledOnValidThread.  This may
        // be useful when an object may be created on one thread and then used
        // exclusively on another thread.
        void DetachFromThread();

    private:
        void EnsureThreadIdAssigned() const;

        mutable std::mutex lock_;
        // This is mutable so that CalledOnValidThread can set it.
        // It's guarded by |lock_|.
        mutable DWORD valid_thread_id_ = 0;
    };
}