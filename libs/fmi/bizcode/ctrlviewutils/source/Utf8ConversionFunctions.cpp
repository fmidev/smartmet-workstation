#include "Utf8ConversionFunctions.h"
#include "catlog/catlog.h"
#include "boost/locale.hpp"
#include <locale>
#include <codecvt>

namespace
{
    void makeErrorLogging(std::string parameterName, std::string functionName, const std::string& parameterValue, const std::exception* exceptionPtr = nullptr)
    {
        std::string errorMessage = exceptionPtr ? "Error in " : "Unknown error in ";
        errorMessage += functionName + " function with " + parameterName + " '";
        errorMessage += parameterValue + "'";
        if(exceptionPtr)
        {
            errorMessage += " => ";
            errorMessage += exceptionPtr->what();
        }
        CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Operational, true);
    }
}

std::string fromLocaleStringToUtf8(const std::string& localeStr)
{
    try
    {
        boost::locale::generator g;
        g.locale_cache_enabled(true);
        std::locale loc = g(boost::locale::util::get_system_locale());
        return boost::locale::conv::to_utf<char>(localeStr, loc);
    }
    catch(std::exception& e)
    {
        ::makeErrorLogging("localeStr", __FUNCTION__, localeStr, &e);
    }
    catch(...)
    {
        ::makeErrorLogging("localeStr", __FUNCTION__, localeStr);
    }

    // Virhetilanteissa palautetaan vain originaali localeStr
    return localeStr;
}

std::string fromUtf8toLocaleString(const std::string& utf8Str)
{
    try
    {
        boost::locale::generator g;
        g.locale_cache_enabled(true);
        std::locale loc = g(boost::locale::util::get_system_locale());
        return boost::locale::conv::from_utf<char>(utf8Str, loc);
    }
    catch(std::exception& e)
    {
        ::makeErrorLogging("utf8Str", __FUNCTION__, utf8Str, &e);
    }
    catch(...)
    {
        ::makeErrorLogging("utf8Str", __FUNCTION__, utf8Str);
    }

    // Virhetilanteissa palautetaan vain originaali utf8Str
    return utf8Str;
}
