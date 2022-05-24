#pragma once

#include <atomic>
#include <memory>

#include "base/memory/ref_counted.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "net/url_request/url_request_context.h"
#include "net/url_request/url_request_context_getter.h"

class RequestConnectionContextGetter : public net::URLRequestContextGetter {
public:
    RequestConnectionContextGetter(const scoped_refptr<base::SingleThreadTaskRunner>& network_task_runner,
                                   const scoped_refptr<base::SingleThreadTaskRunner>& result_parse_task_runner);

    ~RequestConnectionContextGetter();

    // net::URLRequestContextGetter
    net::URLRequestContext* GetURLRequestContext() override;
    scoped_refptr<base::SingleThreadTaskRunner> GetNetworkTaskRunner() const override;

    scoped_refptr<base::SingleThreadTaskRunner> GetResultParseThreadRunner() const;

    void MarkAsShuttingDown();

    bool IsShuttingDown() const;

    bool use_system_proxy() const;

private:
    void Cleanup();

    DISALLOW_COPY_AND_ASSIGN(RequestConnectionContextGetter);

private:
    scoped_refptr<base::SingleThreadTaskRunner> network_task_runner_;
    scoped_refptr<base::SingleThreadTaskRunner> result_parse_task_runner_;
    std::unique_ptr<net::URLRequestContext> context_;
    bool use_system_proxy_;
    std::atomic<bool> shutting_down_;
    base::WaitableEvent shutdown_event_;
};
