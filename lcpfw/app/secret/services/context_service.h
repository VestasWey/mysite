
#pragma once

#include "base/memory/weak_ptr.h"

#include "secret/network/url_request/request_context.h"


class ContextService
{
public:
    virtual ~ContextService() = default;

    static std::unique_ptr<ContextService> Create(UrlRequestContext* request_context);

    virtual void SetupEthernetAddressInfo() = 0;

protected:
    explicit ContextService(UrlRequestContext* request_context)
        : url_request_context_(request_context)
    {}

protected:
    UrlRequestContext* url_request_context_ = nullptr;

    DISALLOW_COPY_AND_ASSIGN(ContextService);
};
