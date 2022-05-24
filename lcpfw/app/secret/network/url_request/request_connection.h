#pragma once

#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"

enum class ReplyScheduler {
    CallingThread,
    TaskThread
};

class UrlRequestConnection : public base::RefCountedThreadSafe<UrlRequestConnection>
{
public:
    class RequestCompleteCallback
    {
    public:
        virtual ~RequestCompleteCallback() {}

        virtual void OnRequestEnd(UrlRequestConnection* instance) = 0;
    };

    void SetReplyScheduler(ReplyScheduler scheduler)
    {
        reply_scheduler_ = scheduler;
    }

    ReplyScheduler reply_scheduler() const
    {
        return reply_scheduler_;
    }

    void SetTimeoutDelta(int delta_in_ms)
    {
        timeout_delta_ = delta_in_ms;
    }

    int timeout_delta() const
    {
        return timeout_delta_;
    }

    // The module maintains another series copies of global/static members, even though they have
    // already been initialized in other modules; therefore, we need this interface to incorporate
    // in the task runner of calling thread.
    virtual void SetReplyThreadRunner(const scoped_refptr<base::SingleThreadTaskRunner>& runner) = 0;

    virtual void Start() = 0;

protected:
    virtual ~UrlRequestConnection() {}

    friend class base::RefCountedThreadSafe<UrlRequestConnection>;

private:
    ReplyScheduler reply_scheduler_ = ReplyScheduler::CallingThread;
    int timeout_delta_ = 60000; // 1min
};
