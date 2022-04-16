#pragma once
#include <string>

namespace mctm
{
    // to 16 or 32
    std::wstring UTF8ToUnicode(const std::string& str);

    std::string UnicodeToUTF8(const std::wstring& wstr);

    std::string UnicodeToANSI(const std::wstring& wstr);

    std::wstring ANSIToUnicode(const std::string& str);

    std::string UTF8ToANSI(const std::string& str);

    std::string ANSIToUTF8(const std::string& str);

    std::wstring UTF8ToWide(const std::string& str);
    std::string WideToUTF8(const std::wstring& str);
}
