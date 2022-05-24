#pragma once

#include <set>

#include "base/memory/weak_ptr.h"
#include "base/synchronization/lock.h"
#include "base/threading/thread.h"
#include "url/gurl.h"

#include "secret/network/common_types.h"
#include "secret/network/url_request/request_connection_common_types.h"
#include "secret/network/url_request/request_connection_impl.h"
#include "secret/network/url_request/request_connection_proxy.h"


class UrlRequestContext : public UrlRequestConnection::RequestCompleteCallback {
public:
    UrlRequestContext();

    ~UrlRequestContext();

    template<typename Parser, typename Handler>
    RequestProxy CreateRequestConnection(
        const GURL& url, 
        RequestType method, 
        const RequestData& req_data,
        const Parser& parser, 
        const Handler& handler)
    {
        auto req_conn = UrlRequestConnectionImpl<Parser, Handler>::Create(
            url,  method, {}, req_data, parser, handler,
            request_context_getter_,
            this);

        auto ptr = AddRequestConnection(req_conn);

        return RequestProxy(ptr);
    }

    template<typename Parser, typename Handler>
    RequestProxy CreateRequestConnection(
        const GURL& url,
        RequestType method,
        const RequestHeaders& extra_headers,
        const RequestData& req_data,
        const Parser& parser,
        const Handler& handler)
    {
        auto req_conn = UrlRequestConnectionImpl<Parser, Handler>::Create(
            url, method, extra_headers, req_data, parser, handler,
            request_context_getter_,
            this);

        auto ptr = AddRequestConnection(req_conn);

        return RequestProxy(ptr);
    }

    void SetCookie(const std::string& site, const std::string& cookie_line, const base::Time& creation_time);
    void SetCookie(const std::string& site, const lcpfw::cookies& cookie);

private:
    UrlRequestConnection* AddRequestConnection(const scoped_refptr<UrlRequestConnection>& req_conn);

    void RemoveRequestConnection(UrlRequestConnection* req_conn);

    // RequestConnectionBase::RequestCompleteCallback.
    void OnRequestEnd(UrlRequestConnection* req_conn) override;

    void OnSetCookies(net::CookieAccessResult access_result);

private:
    base::Lock conn_access_mutex_;
    base::Thread network_io_thread_;
    base::Thread result_parse_thread_;
    scoped_refptr<RequestConnectionContextGetter> request_context_getter_;
    std::set<scoped_refptr<UrlRequestConnection>> request_connections_;

    base::WeakPtrFactory<UrlRequestContext> weak_factory_;

    DISALLOW_COPY_AND_ASSIGN(UrlRequestContext);
};
