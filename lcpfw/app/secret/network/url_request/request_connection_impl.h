#pragma once

#include <map>
#include <memory>
#include <utility>

#include "base/strings/stringprintf.h"
#include "base/synchronization/waitable_event.h"
#include "base/system/sys_info.h"
#include "base/threading/thread_checker.h"
#include "net/base/load_flags.h"
#include "net/url_request/url_fetcher.h"
#include "net/url_request/url_fetcher_delegate.h"
#include "url/gurl.h"

#include "common/app_context.h"
#include "secret/network/url_request/request_connection_common_types.h"
#include "secret/network/url_request/request_connection.h"
#include "secret/network/url_request/request_connection_context_getter.h"
#include "utils/app_tuple.h"


template<typename ResponseParser, typename Handler>
class UrlRequestConnectionImpl
    : public UrlRequestConnection,
    public net::URLFetcherDelegate
{
public:
    static scoped_refptr<UrlRequestConnectionImpl> Create(
        const GURL& url,
        net::URLFetcher::RequestType method,
        const RequestHeaders& extra_request_headers,
        const RequestData& request_data,
        const ResponseParser& parser,
        const Handler& handler,
        const scoped_refptr<RequestConnectionContextGetter>& context_getter,
        RequestCompleteCallback* callback)
    {
        DCHECK(url.is_valid());
        DCHECK(context_getter);
        DCHECK(callback);

        return scoped_refptr<UrlRequestConnectionImpl>(
            new UrlRequestConnectionImpl(url,
                method,
                extra_request_headers,
                request_data,
                parser,
                handler,
                context_getter,
                callback));
    }

    // UrlRequestConnection
    void Start() override
    {
        CHECK(reply_thread_runner_);

        auto result_parse_task_runner = request_context_getter_->GetResultParseThreadRunner();
        result_parse_task_runner->PostTask(FROM_HERE,
            base::Bind(&UrlRequestConnectionImpl::CreateAndStartRequestOnParseThread, this));
    }

    void SetReplyThreadRunner(const scoped_refptr<base::SingleThreadTaskRunner>& runner) override
    {
        reply_thread_runner_ = runner;
    }

private:
    using parser_result_package_type =
        decltype(std::declval<ResponseParser>()({ 0, 0, nullptr, nullptr, nullptr }, {}));

    UrlRequestConnectionImpl(const GURL& url,
        net::URLFetcher::RequestType method,
        const RequestHeaders& extra_request_headers,
        const RequestData& request_data,
        const ResponseParser& parser,
        const Handler& handler,
        const scoped_refptr<RequestConnectionContextGetter>& context_getter,
        RequestCompleteCallback* callback)
        : url_(url),
        method_(method),
        extra_request_headers_(extra_request_headers),
        request_data_(request_data.Clone()),
        response_parser_(parser),
        handler_(handler),
        complete_callback_(callback),
        request_context_getter_(context_getter)
    {
        if (method == net::URLFetcher::GET && !request_data.empty())
        {
            auto query_string = request_data.ToRequestContent().second;
            url_ = AppendQueryStringToGURL(url, query_string);
        }
    }

    ~UrlRequestConnectionImpl() override
    {
        if (url_request_)
        {
            auto response_parse_task_runner = request_context_getter_->GetResultParseThreadRunner();
            if (response_parse_task_runner->BelongsToCurrentThread())
            {
                DismissRequestOnParseThread();
            }
            else
            {
                // Once we get into destructor, the ref-count of the instance has already been
                // down to 0. Don't sabotage it.
                response_parse_task_runner->PostTask(FROM_HERE,
                    base::Bind(&UrlRequestConnectionImpl::DismissRequestOnParseThread, base::Unretained(this)));
            }

            // request_(net::URLFetcher) will wait until it's network shutdown in it's destructor,
            // wait here until request_'s work thorough shutdown.
            request_dismissed_event_.Wait();
        }
    }

    // caller post request task to relay thread (i.e. ParseThread);
    // relay thread post actual request task to network io thread (via net::URLFetcher);
    // network io thread do request sequenced and reply to relay thread with response;
    // relay thread parse the response, then post the parsed result obj (deep copy from base::Value in a general way) to caller thread.
    void CreateAndStartRequestOnParseThread()
    {
        DCHECK(request_context_getter_->GetResultParseThreadRunner()->BelongsToCurrentThread());

        // URLFetcherCore will take current thread task_runner via Create(), 
        // URLFetcherCore do request on network io thread, and post result to Create() thread (i.e. result-parse thread) via URLFetcherDelegate callbacks. 
        url_request_ = net::URLFetcher::Create(url_, method_, this, MISSING_TRAFFIC_ANNOTATION);

        if (!request_context_getter_->use_system_proxy())
        {
            url_request_->SetLoadFlags(net::LOAD_BYPASS_PROXY);
        }

        url_request_->SetRequestContext(request_context_getter_.get());

        static std::string ua;
        if (ua.empty())
        {
            auto os_version = base::SysInfo::OperatingSystemVersion();
            auto os_arch = base::SysInfo::OperatingSystemArchitecture();
            std::string app_ver = AppContext::Current()->GetExecutableVersion();
            ua = base::StringPrintf("lcpfw application appVer/%s osVer/%s_%s", app_ver.c_str(), os_version.c_str(), os_arch.c_str());
        }

        // Set User-Agent identity.
        url_request_->AddExtraRequestHeader(net::HttpRequestHeaders::kUserAgent, ua);

        if (!extra_request_headers_.empty())
        {
            for (const auto& header : extra_request_headers_)
            {
                url_request_->AddExtraRequestHeader(header.first, header.second);
            }
        }

        // Fill the request body.
        // HTTP PUT passes parameters via request body as POST does.
        if (method_ != net::URLFetcher::GET && !request_data_->empty())
        {
            auto request_content = request_data_->ToRequestContent();
            url_request_->SetUploadData(request_content.first, request_content.second);
        }

        url_request_->Start();
    }

    void DismissRequestOnParseThread()
    {
        DCHECK(request_context_getter_->GetResultParseThreadRunner()->BelongsToCurrentThread());

        // request_(net::URLFetcher) will wait until it's network shutdown in it's destructor.
        url_request_ = nullptr;
        request_dismissed_event_.Signal();
    }

    // URLFetcherDelegate
    // belongs to result-parse thread
    void OnURLFetchComplete(const net::URLFetcher* source) override
    {
        DCHECK(request_context_getter_->GetResultParseThreadRunner()->BelongsToCurrentThread());

        // As the matter of fact, we have no idea how to handle unsuccessful cases while with
        // such little knowledge about the request context.
        // Thus, let the response-parser make the right call.
        ResponseInfo response_info(source->GetError(), source->GetResponseCode(),
            source->GetResponseHeaders(), &url_, request_data_.get());

        std::string response_data;
        if (!source->GetResponseAsString(&response_data))
        {
            LOG(WARNING) << "Failed to read response as string!";
        }

        ParseResponseAndReply(response_info, response_data);
    }

    // belongs to result-parse thread
    void ParseResponseAndReply(ResponseInfo response_info, const std::string& response_data)
    {
        DCHECK(request_context_getter_->GetResultParseThreadRunner()->BelongsToCurrentThread());

        DCHECK(url_request_);
        if (!url_request_)
        {
            return;
        }

        // parse url-request result 
        auto result_package = response_parser_(response_info, response_data);

        // post url-request result to request caller
        auto* package = new parser_result_package_type(std::move(result_package));

        // `request_` must be released on the same thread as where it was created, i.e. result-parse thread. So we just do it here.
        url_request_ = nullptr;

        // reply to caller
        if (reply_scheduler() == ReplyScheduler::TaskThread)
        {
            auto network_task_runner = request_context_getter_->GetNetworkTaskRunner();
            network_task_runner->PostTaskAndReply(
                FROM_HERE,
                base::Bind(&UrlRequestConnectionImpl::CompleteRequestOnNetworkThread, this, base::Owned(package)),
                base::Bind(&UrlRequestConnectionImpl::TransitionToNotifyRequestEnd, this));
        }
        else
        {
            reply_thread_runner_->PostTask(
                FROM_HERE,
                base::Bind(&UrlRequestConnectionImpl::CompleteRequestOnCallingThread, this, base::Owned(package)));
        }
    }

    void CompleteRequestOnCallingThread(parser_result_package_type* package)
    {
        DCHECK(calling_thread_checker_.CalledOnValidThread());

        // reply caller with the callback he posted by request task
        lcpfw::apply(handler_, *package);

        NotifyRequestEnd();
    }

    void CompleteRequestOnNetworkThread(parser_result_package_type* package)
    {
        DCHECK(request_context_getter_->GetNetworkTaskRunner()->BelongsToCurrentThread());

        // reply caller with the callback he posted by request task
        lcpfw::apply(handler_, *package);
    }

    void TransitionToNotifyRequestEnd()
    {
        DCHECK(request_context_getter_->GetResultParseThreadRunner()->BelongsToCurrentThread());

        reply_thread_runner_->PostTask(FROM_HERE,
            base::Bind(&UrlRequestConnectionImpl::NotifyRequestEnd, this));
    }

    void NotifyRequestEnd()
    {
        DCHECK(calling_thread_checker_.CalledOnValidThread());

        if (!request_context_getter_->IsShuttingDown() && complete_callback_)
        {
            complete_callback_->OnRequestEnd(this);
        }
    }

private:
    GURL url_;
    net::URLFetcher::RequestType method_;
    RequestHeaders extra_request_headers_;
    std::unique_ptr<RequestData> request_data_;
    ResponseParser response_parser_;
    Handler handler_;
    RequestCompleteCallback* complete_callback_;    // just for UrlRequestContext remove this when request done.
    base::ThreadChecker calling_thread_checker_;
    scoped_refptr<RequestConnectionContextGetter> request_context_getter_;
    scoped_refptr<base::SingleThreadTaskRunner> reply_thread_runner_;
    base::WaitableEvent request_dismissed_event_;

    // net::URLFetcher已经处于deprecated状态，
    // 要嘛直接使用net::URLRequest，参照URLFetcher自己简化处理回调、存储response，要嘛使用SimpleURLLoader。
    // 采用net::URLRequest，还是需要参照URLFetcherCore的封装方式对URLRequest::Delegate的种种回调进行处理和数据封装，繁杂且冗余；
    // 采用service::SimpleURLLoader需要引入mojo等等一堆库。
    // 所以现在还是继续使用net::URLFetcher，修改其源码，将net::URLFetcher::Create()函数进行public化而非private化。
    std::unique_ptr<net::URLFetcher> url_request_;

    DISALLOW_COPY_AND_ASSIGN(UrlRequestConnectionImpl);
};
