#include "thread_checker.h"
#include <processthreadsapi.h>

namespace
{
    static const DWORD kInvalidThreadId = 0;
}

namespace mctm
{
    ThreadChecker::ThreadChecker()
        : valid_thread_id_(kInvalidThreadId)
    {
        EnsureThreadIdAssigned();
    }

    ThreadChecker::~ThreadChecker()
    {
    }

    bool ThreadChecker::CalledOnValidThread() const
    {
        EnsureThreadIdAssigned();
        std::lock_guard<std::mutex> lock(lock_);
        return valid_thread_id_ == ::GetCurrentThreadId();
    }

    void ThreadChecker::DetachFromThread()
    {
        std::lock_guard<std::mutex> lock(lock_);
        valid_thread_id_ = kInvalidThreadId;
    }

    void ThreadChecker::EnsureThreadIdAssigned() const
    {
        std::lock_guard<std::mutex> lock(lock_);
        if (valid_thread_id_ != kInvalidThreadId)
            return;
        valid_thread_id_ = ::GetCurrentThreadId();
    }

}