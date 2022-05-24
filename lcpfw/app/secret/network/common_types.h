#pragma once

#include <string>
#include <vector>

namespace lcpfw
{

    // like _cef_cookie_t
    struct cookie_t
    {
        ///
        // The cookie name.
        ///
        std::string name;

        ///
        // The cookie value.
        ///
        std::string value;

        ///
        // If |domain| is empty a host cookie will be created instead of a domain
        // cookie. Domain cookies are stored with a leading "." and are visible to
        // sub-domains whereas host cookies are not.
        ///
        std::string domain;

        ///
        // If |path| is non-empty only URLs at or below the path will get the cookie
        // value.
        ///
        std::string path;

        ///
        // If |secure| is true the cookie will only be sent for HTTPS requests.
        ///
        int secure = 0;

        ///
        // If |httponly| is true the cookie will only be sent for HTTP requests.
        ///
        int httponly = 0;

        ///
        // The cookie creation date. This is automatically populated by the system on
        // cookie creation.
        ///
        time_t creation = 0;

        ///
        // The cookie last access date. This is automatically populated by the system
        // on access.
        ///
        time_t last_access = 0;

        ///
        // The cookie expiration date is only valid if |has_expires| is true.
        ///
        int has_expires = 0;
        time_t expires = 0;
    };
    typedef std::vector<cookie_t> cookies;

    struct EthernetAddressInfo
    {
        std::wstring ip;
        std::wstring addr;
        std::wstring communications_operator;    // 网络服务运营商
    };

}