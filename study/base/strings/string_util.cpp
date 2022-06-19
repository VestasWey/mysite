#include "string_util.h"

#include <xlocbuf>
#include <codecvt>
#include <wchar.h>

#include "logging/logging.h"

namespace mctm
{
    // to 16 or 32
    std::wstring UTF8ToUnicode(const std::string& str)
    {
        std::wstring ret;
        try
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv;
            ret = wcv.from_bytes(str);
        }
        catch (const std::exception & e)
        {
            NOTREACHED() << e.what();
            //std::cerr << e.what() << std::endl;
        }
        return ret;
    }

    std::string UnicodeToUTF8(const std::wstring& wstr)
    {
        std::string ret;
        try
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> wcv;
            ret = wcv.to_bytes(wstr);
        }
        catch (const std::exception & e)
        {
            NOTREACHED() << e.what();
            //std::cerr << e.what() << std::endl;
        }
        return ret;
    }

    std::string UnicodeToANSI(const std::wstring& wstr)
    {
        std::string ret;
        mbstate_t state = {};
        const wchar_t *src = wstr.data();
        size_t len = -1;
        errno_t err_code = wcsrtombs_s(&len, nullptr, 0, &src, 0, &state);
        if (0 == err_code)
        {
            if (static_cast<size_t>(-1) != len)
            {
                std::unique_ptr<char[]> buff(new char[len + 1]);
                err_code = wcsrtombs_s(&len, buff.get(), len + 1, &src, len, &state);
                if (0 == err_code)
                {
                    if (static_cast<size_t>(-1) != len)
                    {
                        ret.assign(buff.get(), len);
                    }
                }
            }
        }
        else
        {
            NOTREACHED() << err_code;
        }
        return ret;
    }

    std::wstring ANSIToUnicode(const std::string& str)
    {
        std::wstring ret;
        mbstate_t state = {};
        const char *src = str.data();
        size_t len = -1;
        errno_t err_code = mbsrtowcs_s(&len, nullptr, 0, &src, 0, &state);
        if (0 == err_code)
        {
            if (static_cast<size_t>(-1) != len)
            {
                std::unique_ptr<wchar_t[]> buff(new wchar_t[len + 1]);
                err_code = mbsrtowcs_s(&len, buff.get(), len + 1, &src, len, &state);
                if (static_cast<size_t>(-1) != len)
                {
                    ret.assign(buff.get(), len);
                }
            }
        }
        else
        {
            NOTREACHED() << err_code;
        }
        return ret;
    }

    std::string UTF8ToANSI(const std::string& str)
    {
        std::wstring uni = UTF8ToUnicode(str);
        return UnicodeToANSI(uni);
    }

    std::string ANSIToUTF8(const std::string& str)
    {
        std::wstring uni = ANSIToUnicode(str);
        return UnicodeToUTF8(uni);
    }

    std::wstring UTF8ToWide(const std::string& str)
    {
        return UTF8ToUnicode(str);
    }

    std::string WideToUTF8(const std::wstring& str)
    {
        return UnicodeToUTF8(str);
    }

}