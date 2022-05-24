#include "secret/network/url_request/request_context.h"


#include "base/bind.h"
#include "base/memory/scoped_refptr.h"
#include "base/strings/string_split.h"
#include "net/cookies/cookie_monster.h"

namespace {

    const char kNetworkIOThreadName[] = "UrlRequestNetworkIOThread";
    const char kRelayThreadName[] = "UrlRequestParseResponseThread";

}

UrlRequestContext::UrlRequestContext()
    : network_io_thread_(kNetworkIOThreadName),
    result_parse_thread_(kRelayThreadName),
    weak_factory_(this)
{
    base::Thread::Options options;
    options.message_pump_type = base::MessagePumpType::IO;

    CHECK(network_io_thread_.StartWithOptions(options));
    LOG(INFO) << "UrlRequest Network IO Thread ID: " << network_io_thread_.GetThreadId();

    CHECK(result_parse_thread_.StartWithOptions(options));
    LOG(INFO) << "UrlRequest Response Parse Thread ID: " << result_parse_thread_.GetThreadId();

    request_context_getter_ = base::MakeRefCounted<RequestConnectionContextGetter>(
        network_io_thread_.task_runner(),
        result_parse_thread_.task_runner()
    );
}

UrlRequestContext::~UrlRequestContext()
{
    // context-getter may be owned by request connections, and which could be owned by active
    // threads; thus they may not be destroyed immediately in here.

    {
        base::AutoLock lock(conn_access_mutex_);
        request_connections_.clear();
    }

    request_context_getter_->MarkAsShuttingDown();
    request_context_getter_ = nullptr;
}

void UrlRequestContext::SetCookie(const std::string& site, const std::string& cookie_line, const base::Time& creation_time)
{
    net::CookieStore* cookie_store = request_context_getter_->GetURLRequestContext()->cookie_store();

    net::CookieStore::SetCookiesCallback callback(
        base::BindOnce(&UrlRequestContext::OnSetCookies, weak_factory_.GetWeakPtr()));

    auto ck = net::CanonicalCookie::Create(
        GURL(site),
        cookie_line,
        creation_time,
        {});

    cookie_store->SetCanonicalCookieAsync(
        std::move(ck),
        GURL(site),
        net::CookieOptions::MakeAllInclusive(),
        std::move(callback));
}

void UrlRequestContext::SetCookie(const std::string& site, const lcpfw::cookies& cookie)
{
    net::CookieStore* cookie_store = request_context_getter_->GetURLRequestContext()->cookie_store();

    net::CookieStore::SetCookiesCallback callback(
        base::BindOnce(&UrlRequestContext::OnSetCookies, weak_factory_.GetWeakPtr()));

    for (auto& iter : cookie)
    {
        auto ck = net::CanonicalCookie::CreateSanitizedCookie(
            GURL(site),
            iter.name,
            iter.value,
            iter.domain,
            iter.path,
            base::Time::FromTimeT(iter.creation),
            base::Time::FromTimeT(iter.expires),
            base::Time::FromTimeT(iter.last_access),
            iter.secure,
            iter.httponly,
            net::CookieSameSite::UNSPECIFIED,
            net::CookiePriority::COOKIE_PRIORITY_DEFAULT,
            false);

        cookie_store->SetCanonicalCookieAsync(
            std::move(ck),
            GURL(site),
            net::CookieOptions::MakeAllInclusive(),
            std::move(callback));
    }
}

void UrlRequestContext::OnSetCookies(net::CookieAccessResult access_result)
{
}

UrlRequestConnection* UrlRequestContext::AddRequestConnection(const scoped_refptr<UrlRequestConnection>& req_conn)
{
    auto view_ptr = req_conn.get();

    {
        base::AutoLock lock(conn_access_mutex_);
        request_connections_.insert(req_conn);
    }

    return view_ptr;
}

void UrlRequestContext::RemoveRequestConnection(UrlRequestConnection* req_conn)
{
    base::AutoLock lock(conn_access_mutex_);
    auto it = std::find_if(request_connections_.cbegin(), request_connections_.cend(),
        [req_conn](const scoped_refptr<UrlRequestConnection>& req) {
            return req_conn == req.get();
        });
    if (it != request_connections_.cend()) {
        request_connections_.erase(it);
    }
}

// RequestConnectionBase::RequestCompleteCallback.
void UrlRequestContext::OnRequestEnd(UrlRequestConnection* req_conn)
{
    RemoveRequestConnection(req_conn);
}
