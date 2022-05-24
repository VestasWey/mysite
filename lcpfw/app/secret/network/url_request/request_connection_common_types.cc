#include "secret/network/url_request/request_connection_common_types.h"

#include "base/rand_util.h"
#include "base/strings/stringprintf.h"

#include "net/base/escape.h"
#include "net/base/mime_util.h"
#include "net/base/net_errors.h"

namespace {

const char kDefaultFileMimeType[] = "application/octet-stream";
const char kDefaultDataName[] = "file";

const char kContentForm[] = "application/x-www-form-urlencoded";
const char kContentJSON[] = "application/json";
const char kContentUpload[] = "multipart/form-data; boundary=";

const char kUploadFormFile[] = "%s\"; filename=\"%s";

// Although RFC says that we could use arbitrary value for boundary, for security reasons,
// we generate random multipart boundary each time.
std::string GenerateMultipartBoundary()
{
    const char* kBoundaryPrefix = "---------------------------";

    int r0 = base::RandInt(0, std::numeric_limits<int>::max());
    int r1 = base::RandInt(0, std::numeric_limits<int>::max());

    return base::StringPrintf("%s%08X%08X", kBoundaryPrefix, r0, r1);
}

template <typename Container>
std::string StringifyRequestParams(const Container& params)
{
    const size_t kReservedSize = 32U;

    if (params.empty()) {
        return std::string();
    }

    std::string content;
    content.reserve(kReservedSize);
    for (const auto& param : params) {
        // Be compatible with the case in which each request param only contains values,
        // e.g. event tracking service.
        if (!param.first.empty()) {
            content.append(net::EscapeQueryParamValue(param.first, false)).append(1, '=');
        }

        content.append(net::EscapeQueryParamValue(param.second, false)).append(1, '&');
    }

    // Discards tailing '&'.
    content.pop_back();

    return content;
}

}   // namespace

GURL AppendQueryStringToGURL(const GURL& original, const std::string& query_string)
{
    GURL::Replacements replacements;
    replacements.SetQueryStr(query_string);
    return original.ReplaceComponents(replacements);
}

// -*- RequestRaw implementation -*-

RequestRaw::RequestRaw(const std::string& raw)
    : raw_data(raw)
{}

bool RequestRaw::empty() const
{
    return raw_data.empty();
}

RequestContent RequestRaw::ToRequestContent() const
{
    return RequestContent(kContentForm, raw_data);
}

std::unique_ptr<RequestData> RequestRaw::Clone() const
{
    return std::make_unique<RequestRaw>(*this);
}

// -*- RequestUniqueParams implementation -*-

RequestUniqueParams::RequestUniqueParams(const Params& params)
    : params(params)
{}

RequestUniqueParams::RequestUniqueParams(std::initializer_list<Params::value_type> init)
    : params(init)
{}

auto RequestUniqueParams::operator[](const Params::key_type& key)->Params::mapped_type&
{
    return params[key];
}

bool RequestUniqueParams::Get(const Params::key_type& key, Params::mapped_type& value) const
{
    auto it = params.find(key);
    if (it == params.end()) {
        return false;
    }

    value = it->second;

    return true;
}

bool RequestUniqueParams::empty() const
{
    return params.empty();
}

RequestContent RequestUniqueParams::ToRequestContent() const
{
    return RequestContent(kContentForm, StringifyRequestParams(params));
}

std::unique_ptr<RequestData> RequestUniqueParams::Clone() const
{
    return std::make_unique<RequestUniqueParams>(*this);
}

// -*- RequestParams2 implementation -*-

RequestRepeatParams::RequestRepeatParams(const Params& params)
    : params(params)
{}

RequestRepeatParams::RequestRepeatParams(std::initializer_list<Params::value_type> init)
    : params(init)
{}

std::string& RequestRepeatParams::operator[](const std::string& key)
{
    params.push_back({ key, {} });
    return params.back().second;
}

bool RequestRepeatParams::empty() const
{
    return params.empty();
}

RequestContent RequestRepeatParams::ToRequestContent() const
{
    return RequestContent(kContentForm, StringifyRequestParams(params));
}

std::unique_ptr<RequestData> RequestRepeatParams::Clone() const
{
    return std::make_unique<RequestRepeatParams>(*this);
}

// -*- RequestJSON implementation -*-

RequestJSON::RequestJSON(const std::string& json)
    : json_data(json)
{}

bool RequestJSON::empty() const
{
    return json_data.empty();
}

RequestContent RequestJSON::ToRequestContent() const
{
    return RequestContent(kContentJSON, json_data);
}

std::unique_ptr<RequestData> RequestJSON::Clone() const
{
    return std::make_unique<RequestJSON>(*this);
}

// -*- RequestUpload implementation -*-

RequestUpload::RequestUpload(const std::string& filename, const Data& data)
    : data_name(kDefaultDataName), filename(filename), mime_type(kDefaultFileMimeType), data(data)
{}

RequestUpload::RequestUpload(const std::string& filename, const Data& data, const ExtraParams& params)
    : data_name(kDefaultDataName), filename(filename), mime_type(kDefaultFileMimeType), data(data),
      extra_params(params)
{}

bool RequestUpload::empty() const
{
    return data_name.empty() || filename.empty() || data.empty();
}

RequestContent RequestUpload::ToRequestContent() const
{
    auto boundary = GenerateMultipartBoundary();

    std::string content_type(kContentUpload);
    content_type += boundary;

    auto content = GenerateMultipartRequestBody(boundary);

    return RequestContent(std::move(content_type), std::move(content));
}

std::string RequestUpload::GenerateMultipartRequestBody(const std::string& boundary) const
{
    const size_t kBodyReserviedSize = data.size() + 1024;
    std::string body;
    body.reserve(kBodyReserviedSize);

    std::string upload_data_name = base::StringPrintf(kUploadFormFile, data_name.c_str(), filename.c_str());
    net::AddMultipartValueForUpload(upload_data_name, data, boundary, mime_type, &body);

    for (const auto& param : extra_params) {
        net::AddMultipartValueForUpload(param.first, param.second, boundary, std::string(), &body);
    }

    net::AddMultipartFinalDelimiterForUpload(boundary, &body);

    return body;
}

std::unique_ptr<RequestData> RequestUpload::Clone() const
{
    return std::make_unique<RequestUpload>(*this);
}

// -*- ResponseInfo implementation -*-

ResponseInfo::ResponseInfo(int error_code, int response_code, net::HttpResponseHeaders* headers,
                           const GURL* url, const RequestData* request_data)
    : error_code(error_code), response_code(response_code), response_headers(headers), request_url(url),
      request_data(request_data)
{}

std::ostream& operator<<(std::ostream& os, const ResponseInfo& info)
{
    std::string spec(info.request_url->spec());
#ifdef NDEBUG
    spec = spec.substr(0, spec.find('?'));
#endif
    os << "\nHTTP response code: " << info.response_code << "\n"
        << "Error code: " << info.error_code << "(" << net::ErrorToString(info.error_code) << ")\n"
        << "Request url: " << spec;

#ifndef NDEBUG
    if (info.request_data)
    {
        std::string diagno = info.request_data->ToRequestContent().second.substr(0, 255);
        os << "\nRequest content data: " << diagno;
    }
#endif

    return os;
}
