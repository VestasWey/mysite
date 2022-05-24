#include "secret/network/url_request/request_connection_context_getter.h"

#include "base/command_line.h"
#include "net/url_request/url_request_context_builder.h"

namespace {

const char kSwitchIgnoreCertificateErrors[] = "ignore-certificate-errors";
const char kSwitchDisableSystemProxy[] = "disable-system-proxy";

bool ShouldUseSystemProxy()
{
    return !base::CommandLine::ForCurrentProcess()->HasSwitch(kSwitchDisableSystemProxy);
}

}   // namespace

RequestConnectionContextGetter::RequestConnectionContextGetter(
    const scoped_refptr<base::SingleThreadTaskRunner>& network_task_runner,
    const scoped_refptr<base::SingleThreadTaskRunner>& result_parse_task_runner)
    : network_task_runner_(network_task_runner),
    result_parse_task_runner_(result_parse_task_runner),
    use_system_proxy_(ShouldUseSystemProxy()),
    shutting_down_(false),
    shutdown_event_(base::WaitableEvent::ResetPolicy::AUTOMATIC)
{}

RequestConnectionContextGetter::~RequestConnectionContextGetter()
{
    Cleanup();
    shutdown_event_.Wait();
}

net::URLRequestContext* RequestConnectionContextGetter::GetURLRequestContext()
{
    // on network io thread
    // context_ create in GetURLRequestContext() on network thread while the first UrlRequest dowork.
    if (!shutting_down_.load(std::memory_order_acquire) && !context_) {
        net::URLRequestContextBuilder builder;

        builder.DisableHttpCache();

        net::HttpNetworkSession::Params session_params;
        session_params.ignore_certificate_errors = true;
        builder.set_http_network_session_params(session_params);

        context_ = builder.Build();
    }

    return context_.get();
}

scoped_refptr<base::SingleThreadTaskRunner> RequestConnectionContextGetter::GetNetworkTaskRunner() const
{
    return network_task_runner_;
}

scoped_refptr<base::SingleThreadTaskRunner> RequestConnectionContextGetter::GetResultParseThreadRunner() const
{
    return result_parse_task_runner_;
}

void RequestConnectionContextGetter::Cleanup()
{
    // context_ create in GetURLRequestContext() on network thread while the first UrlRequest dowork,
    // so we should release context_ on network thread.
    if (!network_task_runner_->BelongsToCurrentThread())
    {
        network_task_runner_->PostTask(FROM_HERE, base::Bind(&RequestConnectionContextGetter::Cleanup, this));
        return;
    }

    context_ = nullptr;

    shutdown_event_.Signal();
}

void RequestConnectionContextGetter::MarkAsShuttingDown()
{
    shutting_down_.store(true, std::memory_order_release);
}

bool RequestConnectionContextGetter::IsShuttingDown() const
{
    return shutting_down_.load(std::memory_order_acquire);
}

bool RequestConnectionContextGetter::use_system_proxy() const
{
    return use_system_proxy_;
}
