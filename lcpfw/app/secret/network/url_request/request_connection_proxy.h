#pragma once

#include "base/threading/thread_task_runner_handle.h"

#include "secret/network/url_request/request_connection.h"


class RequestProxy {
public:
    explicit RequestProxy(UrlRequestConnection* request)
        : request_(request)
    {
        DCHECK(request_);
    }

    ~RequestProxy() = default;

    const RequestProxy& ReplyOn(ReplyScheduler scheduler) const
    {
        request_->SetReplyScheduler(scheduler);

        return *this;
    }

    void Call(int timeout_delta_in_ms = 60000) const
    {
        request_->SetReplyThreadRunner(base::ThreadTaskRunnerHandle::Get());
        request_->SetTimeoutDelta(timeout_delta_in_ms);
        request_->Start();
    }

private:
    UrlRequestConnection* request_;
};
