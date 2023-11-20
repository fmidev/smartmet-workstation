#include "NFmiMouseClickUrlActionData.h"
#include "SettingsFunctions.h"
#include "NFmiSettings.h"
#include "NFmiStringTools.h"
#include "catlog/catlog.h"

#include <boost/algorithm/string.hpp>
#include<stdexcept>
#include<tuple>
#include<algorithm>


namespace
{
    using IndexNameUrl = std::pair<int, std::string>;

    std::string makeUrlNameError(std::string errorStart, const std::string& urlName)
    {
        std::string str = errorStart;
        str += " '" + urlName + "' ";
        return str;
    }

    std::string parseCamelCaseName(std::string urlNameStart)
    {
        NFmiStringTools::Trim(urlNameStart);

        std::string usedUrlName;
        for(auto letter : urlNameStart)
        {
            if(!usedUrlName.empty() && (letter == '_' || std::isupper(letter)))
            {
                // Lisätään lopulliseen nimeen aina space jos usedUrlName ei ollut tyhjä
                // ja jos kyseinen kirjain oli joko alaviiva tai isokirjain, näin
                // saadaaan halutu sanat erilleen
                usedUrlName += " ";
            }
            if(letter != '_')
            {
                usedUrlName += letter;
            }
        }
        return usedUrlName;
    }

    IndexNameUrl parseUrlName(const std::string& urlName)
    {
        auto lastUnderScorePos = urlName.find_last_of('_');
        if(lastUnderScorePos == std::string::npos)
        {
            std::string errorStr = ::makeUrlNameError("Error with MouseClickUrlActionData: UrlName", urlName);
            errorStr += "didn't contain '_' character, UrlName must be in format 'MyUrlName_1', so after '_' there must be index number";
            throw std::runtime_error(errorStr);
        }

        auto indexStr = urlName.substr(lastUnderScorePos+1);
        int index = -1;
        try
        {
            index = std::stoi(indexStr);
        }
        catch(...)
        {
            std::string errorStr = ::makeUrlNameError("Error with MouseClickUrlActionData: UrlName", urlName);
            errorStr += "didn't contain acceptable index value after the last '_' character, UrlName must be in format 'MyUrlName_1', so after last '_' there must be index number";
            throw std::runtime_error(errorStr);
        }

        if(index < 1)
        {
            std::string errorStr = ::makeUrlNameError("Error with MouseClickUrlActionData: UrlName", urlName);
            errorStr += "index value after the last '_' character must be 1 or greater, UrlName must be in format 'MyUrlName_1', so after last '_' there must be positive index number";
            throw std::runtime_error(errorStr);
        }

        auto usedUrlName = ::parseCamelCaseName(urlName.substr(0, lastUnderScorePos));
        if(usedUrlName.empty())
        {
            std::string errorStr = ::makeUrlNameError("Error with MouseClickUrlActionData: UrlName", urlName);
            errorStr += "before the '_' character there was no actual name, UrlName must be in format 'MyUrlName_1', so before last '_' there must be some descriptive name without spaces";
            throw std::runtime_error(errorStr);
        }

        return std::make_pair(index, usedUrlName);
    }

    int SmartMetOpenUrlActionToIndex(SmartMetOpenUrlAction urlAction)
    {
        return static_cast<int>(urlAction);
    }

    std::string makeAreaInfoString(const NFmiPoint& latlon, float areaRadiusInKm, bool includeRadius)
    {
        std::string areaInfoStr = std::to_string(latlon.X());
        areaInfoStr += ",";
        areaInfoStr += std::to_string(latlon.Y());
        if(includeRadius)
        {
            areaInfoStr += ":";
            auto radiusStr = std::to_string(areaRadiusInKm);
            // Putsataan lopusta mahdolliset turhat desimaalit
            boost::trim_right_if(radiusStr, boost::is_any_of("0"));
            boost::trim_right_if(radiusStr, boost::is_any_of("."));
            areaInfoStr += radiusStr;
        }
        return areaInfoStr;
    }
}

std::map<unsigned int, SmartMetOpenUrlAction> NFmiMouseClickUrlActionData::itsOpenUrlActionKeyMappings =
{
    {'1', SmartMetOpenUrlAction::Url1Action},
    {'2', SmartMetOpenUrlAction::Url2Action},
    {'3', SmartMetOpenUrlAction::Url3Action},
    {'4', SmartMetOpenUrlAction::Url4Action},
    {'5', SmartMetOpenUrlAction::Url5Action},
    {'6', SmartMetOpenUrlAction::Url6Action},
    {'7', SmartMetOpenUrlAction::Url7Action},
    {'8', SmartMetOpenUrlAction::Url8Action},
    {'9', SmartMetOpenUrlAction::Url9Action},
    {'0', SmartMetOpenUrlAction::Url10Action}
};

const std::map<unsigned int, SmartMetOpenUrlAction>& NFmiMouseClickUrlActionData::OpenUrlActionKeyMappings()
{
    return itsOpenUrlActionKeyMappings;
}


NFmiMouseClickUrlActionData::NFmiMouseClickUrlActionData()
:itsAreaRadiusInKm(150)
{}

const std::string gBaseErrorMessage = "Error in NFmiMouseClickUrlActionData::InitFromSettings: ";

bool NFmiMouseClickUrlActionData::InitFromSettings(const std::string& baseNameSpace)
{
    if(!itsBaseNameSpace.empty())
        throw std::runtime_error(gBaseErrorMessage + "already initialized");

    itsBaseNameSpace = baseNameSpace;
    auto baseUrlKey = itsBaseNameSpace + "::Urls";
    auto dataKeys = NFmiSettings::ListChildren(baseUrlKey);
    for(const auto& urlKey : dataKeys)
    {
        auto tmpUrl = SettingsFunctions::GetUrlFromSettings(baseUrlKey + "::" + urlKey);
        NFmiStringTools::Trim(tmpUrl);
        if(tmpUrl.empty())
        {
            std::string errorMessage = gBaseErrorMessage + "no actual url after urlName '" + urlKey + "'";
            CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
        }
        else
        {
            try
            {
                auto indexUrlName = ::parseUrlName(urlKey);
                if(itsMouseActionUrls.find(indexUrlName.first) != itsMouseActionUrls.end())
                {
                    std::string errorMessage = gBaseErrorMessage + "given urlName index already exist from '" + urlKey + "', skipping this setting";
                    CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
                }
                else
                {
                    itsMouseActionUrls[indexUrlName.first] = std::make_pair(indexUrlName.second, tmpUrl);
                }
            }
            catch(std::exception& e)
            {
                CatLog::logMessage(e.what(), CatLog::Severity::Error, CatLog::Category::Configuration, true);
            }
        }
    }

    return true;
}

const std::string gEmptyUrlStr;

const std::string& NFmiMouseClickUrlActionData::GetMouseActionBaseUrl(int index) const
{
    auto iter = itsMouseActionUrls.find(index);
    if(iter != itsMouseActionUrls.end())
    {
        return iter->second.second;
    }
    return gEmptyUrlStr;
}

const std::string gDefaultPointInfoStr = "%pointinfo%";
const std::string gDefaultAreaInfoStr = "%areainfo%";
const std::string gRadiusUrlParameterStr = "&arearadius";

std::string NFmiMouseClickUrlActionData::GetMouseActionUrl(SmartMetOpenUrlAction urlAction, const NFmiPoint& latlon) const
{
    std::string baseUrl = GetMouseActionBaseUrl(SmartMetOpenUrlActionToIndex(urlAction));
    if(!baseUrl.empty())
    {
        bool hasPointInfo = boost::ifind_last(baseUrl, gDefaultPointInfoStr);
        bool hasRadiusUrlParameter = boost::ifind_last(baseUrl, gRadiusUrlParameterStr);
        auto includeRadius = !(hasRadiusUrlParameter || hasPointInfo);
        auto areaInfoStr = ::makeAreaInfoString(latlon, itsAreaRadiusInKm, includeRadius);
        if(hasPointInfo)
        {
            // Löytyi point-info place-holder, nyt pitää replace:ta se saadulla areaInfoStr:illa
            boost::ireplace_last(baseUrl, gDefaultPointInfoStr, areaInfoStr);
        }
        if(hasRadiusUrlParameter)
        {
            // Löytyi area-radius parameter urlista, nyt sen arvo pitää replace:ta se halutulla säteellä
            // Tehdään tämä myöhemmin!!!!
            // auto replacedUrlStr = ::getAreaRadiusStringFromUrl(baseUrl);
            // auto usedAreaRadiusStr = ::makeAreaRadiusString(replacedUrlStr, itsAreaRadiusInKm)
            // boost::ireplace_last(baseUrl, replacedUrlStr, usedAreaRadiusStr);
        }
        if(!hasPointInfo)
        {
            if(!boost::ifind_last(baseUrl, gDefaultAreaInfoStr))
            {
                // Ei löytynyt placeholderia baseUrl:ista, joten lisätään tehty area-info-string vain perään
                baseUrl += areaInfoStr;
            }
            else
            {
                // Löytyi placeholder, nyt pitää replace:ta se saadulla areaInfoStr:illa
                boost::ireplace_last(baseUrl, gDefaultAreaInfoStr, areaInfoStr);
            }
        }
    }
    return baseUrl;
}
