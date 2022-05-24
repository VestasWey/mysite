#include "secret/services/context_service.h"

#include <regex>
#include <tuple>

#include "base/guid.h"
#include "base/logging.h"
#include "base/strings/string_split.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/time/time.h"
#include "base/threading/thread_restrictions.h"
#include "base/values.h"

#include "net/http/http_request_headers.h"

#include "url/gurl.h"
#include "net/base/network_interfaces.h"
#include "net/base/escape.h"


namespace
{
    lcpfw::EthernetAddressInfo g_ethernet_address_info;

    std::string GetCurrentConnectionTypeName()
    {
        net::NetworkInterfaceList networks;
        if (net::GetNetworkList(&networks, net::INCLUDE_HOST_SCOPE_VIRTUAL_INTERFACES))
        {
            net::NetworkChangeNotifier::ConnectionType ct = net::NetworkChangeNotifier::ConnectionTypeFromInterfaceList(networks);
            switch (ct)
            {
            case net::NetworkChangeNotifier::CONNECTION_ETHERNET:
                return "Ethernet";
                break;
            case net::NetworkChangeNotifier::CONNECTION_WIFI:
                return "Wifi";
                break;
            case net::NetworkChangeNotifier::CONNECTION_2G:
            case net::NetworkChangeNotifier::CONNECTION_3G:
            case net::NetworkChangeNotifier::CONNECTION_4G:
            case net::NetworkChangeNotifier::CONNECTION_5G:
                return "Cellular ";
                break;
            case net::NetworkChangeNotifier::CONNECTION_BLUETOOTH:
                return "Bluetooth";
            case net::NetworkChangeNotifier::CONNECTION_NONE:
                return "offline";
                break;
            }
        }

        return "unknown";
    }

    using GetEthernetAddressResult = std::tuple<bool, lcpfw::EthernetAddressInfo>;
    using GetEthernetAddressParser = std::function<GetEthernetAddressResult(ResponseInfo, const std::string&)>;
    GetEthernetAddressResult ParseSetupEthernetAddressInfoResponse(ResponseInfo info, const std::string& data)
    {
        if (info.response_code == 200)
        {
            base::string16 raw = net::UnescapeForHTML(base::UTF8ToUTF16(data));

            //try
            {
                lcpfw::EthernetAddressInfo addr;

                static std::wregex ip_reg(LR"(IP\t: (\d+.\d+.\d+.\d+))");
                static std::wregex addr_reg(LR"(地址\t: (.+))");
                static std::wregex oper_reg(LR"(运营商\t: (.+))");

                auto fn = [&](const std::wregex& reg, std::wstring* pstr)
                {
                    std::wsmatch mat;
                    if (std::regex_search(raw, mat, reg))
                    {
                        if (mat.size() > 1)
                        {
                            *pstr = mat[1];
                        }
                    }
                };

                std::wstring ip;
                fn(ip_reg, &addr.ip);
                fn(addr_reg, &addr.addr);
                fn(oper_reg, &addr.communications_operator);

                return GetEthernetAddressResult(true, addr);
            }
            /*catch (std::regex_error& ex)
            {
                NOTREACHED() << ex.what();
            }*/
        }
        else
        {
            LOG(WARNING) << "Parse ethernet address info response failure: invalid status!\n"
                << "code: " << info.response_code;
        }

        return GetEthernetAddressResult(false, {});
    }

    using GetEthernetAddressHandler = std::function<void(bool, const lcpfw::EthernetAddressInfo&)>;
    void HandleGetEthernetAddressResponse(bool valid, const lcpfw::EthernetAddressInfo& info)
    {
        if (valid)
        {
            g_ethernet_address_info = info;

            LOG(INFO) << base::StringPrintf("ip: %s; region:%s; isp:%s; network:%s",
                info.ip.c_str(), 
                base::UTF16ToUTF8(info.addr).c_str(), 
                base::UTF16ToUTF8(info.communications_operator).c_str(),
                GetCurrentConnectionTypeName().c_str());
        }
        else
        {
            LOG(WARNING) << "Get ethernet address info failed.";
        }
    }

}   // namespace

class ContextServiceImpl : public ContextService
{
public:
    ContextServiceImpl(UrlRequestContext* request_context)
        : ContextService(request_context)
    {
        SetupEthernetAddressInfo();
    }

    void SetupEthernetAddressInfo() override
    {
        GURL url("http://www.cip.cc/");
        RequestHeaders headers;
        headers[net::HttpRequestHeaders::kUserAgent] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/90.0.4430.93 Safari/537.36";
        RequestProxy proxy =
            url_request_context_->CreateRequestConnection<GetEthernetAddressParser, GetEthernetAddressHandler>(
                url,
                RequestType::GET,
                headers,
                RequestRaw(""),
                ParseSetupEthernetAddressInfoResponse,
                HandleGetEthernetAddressResponse);
        proxy.Call();
    }
};

std::unique_ptr<ContextService> ContextService::Create(UrlRequestContext* request_context)
{
    return std::make_unique<ContextServiceImpl>(request_context);
}
