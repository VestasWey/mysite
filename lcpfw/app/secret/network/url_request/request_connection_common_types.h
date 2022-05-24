#pragma once

#include <map>
#include <string>

#include "net/url_request/url_fetcher.h"

#include "url/gurl.h"

using RequestType = net::URLFetcher::RequestType;

using RequestHeaders = std::map<std::string, std::string>;

// `query_string` should be already escaped.
GURL AppendQueryStringToGURL(const GURL& original, const std::string& query_string);

// (content-type, content)
using RequestContent = std::pair<std::string, std::string>;

struct RequestData {
    virtual ~RequestData() = default;

    virtual bool empty() const = 0;

    virtual RequestContent ToRequestContent() const = 0;

    virtual std::unique_ptr<RequestData> Clone() const = 0;
};

struct RequestRaw : RequestData {
    std::string raw_data;

    explicit RequestRaw(const std::string& raw);

    // RequestData
    bool empty() const override;
    RequestContent ToRequestContent() const override;
    std::unique_ptr<RequestData> Clone() const override;
};

struct RequestUniqueParams : RequestData {
    using Params = std::map<std::string, std::string>;
    using key_compare = Params::key_compare;

    Params params;

    RequestUniqueParams() = default;

    explicit RequestUniqueParams(const Params& params);

    RequestUniqueParams(std::initializer_list<Params::value_type> init);

    Params::mapped_type& operator[](const Params::key_type& key);

    bool Get(const Params::key_type& key, Params::mapped_type& value) const;

    // RequestData
    bool empty() const override;
    RequestContent ToRequestContent() const override;
    std::unique_ptr<RequestData> Clone() const override;
};


struct RequestRepeatParams : RequestData {
    using Params = std::vector<std::pair<std::string, std::string>>;

    Params params;

    RequestRepeatParams() = default;

    explicit RequestRepeatParams(const Params& params);

    RequestRepeatParams(std::initializer_list<Params::value_type> init);

    std::string& operator[](const std::string& key);

    // RequestData
    bool empty() const override;
    RequestContent ToRequestContent() const override;
    std::unique_ptr<RequestData> Clone() const override;
};

struct RequestJSON : RequestData {
    std::string json_data;

    explicit RequestJSON(const std::string& json);

    // RequestData
    bool empty() const override;
    RequestContent ToRequestContent() const override;
    std::unique_ptr<RequestData> Clone() const override;
};

struct RequestUpload : RequestData {
    using Data = std::string;
    using ExtraParams = std::map<std::string, std::string>;

    std::string data_name;
    std::string filename;
    std::string mime_type;
    Data data;

    ExtraParams extra_params;

    RequestUpload::RequestUpload(const std::string& filename, const Data& data);

    RequestUpload::RequestUpload(const std::string& filename, const Data& data, const ExtraParams& params);

    // RequestData
    bool empty() const override;
    RequestContent ToRequestContent() const override;
    std::unique_ptr<RequestData> Clone() const override;

private:
    std::string GenerateMultipartRequestBody(const std::string& boundary) const;
};

struct ResponseInfo {
    // net::Error
    int error_code;

    // HTTP response code.
    int response_code;

    // HTTP response headers.
    const net::HttpResponseHeaders* response_headers;

    // Some request information for diagnosis.

    const GURL* request_url;

    const RequestData* request_data;

    ResponseInfo(int error_code, int response_code, net::HttpResponseHeaders* headers, const GURL* url,
        const RequestData* request_data);
};

std::ostream& operator<<(std::ostream& os, const ResponseInfo& info);

