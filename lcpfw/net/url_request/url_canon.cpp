#include "url_canon.h"


namespace mctm
{
    CanonURL::CanonURL(const std::string& url_string)
        : raw_url_(url_string)
    {
        //////////////////////////////////////////////////////////////////////////
        spec_ = raw_url_;
    }

    CanonURL::~CanonURL()
    {
    }

}