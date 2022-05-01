#include "url_request_context.h"


namespace mctm
{
    URLRequestContext::URLRequestContext()
    {
    }

    URLRequestContext::~URLRequestContext()
    {
    }

    std::unique_ptr<URLRequest> URLRequestContext::CreateURLRequest(
        const CanonURL& url, std::weak_ptr<URLRequest::Delegate> delegate)
    {
        std::unique_ptr<URLRequest> request = std::make_unique<URLRequest>(url, delegate, this);

        //////////////////////////////////////////////////////////////////////////

        return request;
    }
}