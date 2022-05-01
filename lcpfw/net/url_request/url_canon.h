#pragma once
#include <string>

namespace mctm
{
    class CanonURL
    {
    public:
        explicit CanonURL(const std::string& url_string);
        virtual ~CanonURL();

        const std::string& spec() const
        {
            return spec_;
        }

    private:
        std::string raw_url_;
        // The actual text of the URL, in canonical ASCII form.
        std::string spec_;
    };
}