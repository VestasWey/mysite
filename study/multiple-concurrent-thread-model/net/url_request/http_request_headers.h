#pragma once

namespace mctm
{
    class HttpRequestHeaders
    {
    public:

        static const char kGetMethod[];

        static const char kAcceptCharset[];
        static const char kAcceptEncoding[];
        static const char kAcceptLanguage[];
        static const char kAuthorization[];
        static const char kCacheControl[];
        static const char kConnection[];
        static const char kContentType[];
        static const char kCookie[];
        static const char kContentLength[];
        static const char kHost[];
        static const char kIfModifiedSince[];
        static const char kIfNoneMatch[];
        static const char kIfRange[];
        static const char kOrigin[];
        static const char kPragma[];
        static const char kProxyAuthorization[];
        static const char kProxyConnection[];
        static const char kRange[];
        static const char kReferer[];
        static const char kUserAgent[];
        static const char kTransferEncoding[];


        HttpRequestHeaders();
        virtual ~HttpRequestHeaders();
    };
}