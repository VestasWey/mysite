#pragma once

#include "url_request.h"

namespace mctm
{
    class URLRequestContext
    {
    public:
        URLRequestContext();
        virtual ~URLRequestContext();

        std::unique_ptr<URLRequest> CreateURLRequest(const CanonURL& url,
            std::weak_ptr<URLRequest::Delegate> delegate);

    private:

    };
}