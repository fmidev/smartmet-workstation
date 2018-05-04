#include "Utf8ConversionFunctions.h"
#include "boost/locale.hpp"

std::string fromLocaleStringToUtf8(const std::string &localeStr)
{
    boost::locale::generator g;
    g.locale_cache_enabled(true);
    std::locale loc = g(boost::locale::util::get_system_locale());
    return boost::locale::conv::to_utf<char>(localeStr,loc);
}

std::string fromUtf8toLocaleString(const std::string &utf8Str)
{
    boost::locale::generator g;
    g.locale_cache_enabled(true);
    std::locale loc = g(boost::locale::util::get_system_locale());
    return boost::locale::conv::from_utf<char>(utf8Str,loc);
}
