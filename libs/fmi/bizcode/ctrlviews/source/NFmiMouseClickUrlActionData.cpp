#include "NFmiMouseClickUrlActionData.h"
#include "SettingsFunctions.h"
#include "NFmiSettings.h"
#include "NFmiStringTools.h"
#include "NFmiMetTime.h"
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
                // Lis‰t‰‰n lopulliseen nimeen aina space jos usedUrlName ei ollut tyhj‰
                // ja jos kyseinen kirjain oli joko alaviiva tai isokirjain, n‰in
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

    std::string makePointInfoString(const NFmiPoint& latlon)
    {
        std::string pointInfoStr = std::to_string(latlon.Y());
        pointInfoStr += ",";
        pointInfoStr += std::to_string(latlon.X());
        return pointInfoStr;
    }

    std::string makeTimeInfoString(const NFmiMetTime & atime)
    {
        std::string timeInfoStr = atime.ToStr("YYYY-MM-DDTHH:mm:SS", kEnglish);
        return timeInfoStr;
    }

    std::string makeAreaRadiusString(float areaRadiusInKm)
    {
        std::string areaRadiusStr = std::to_string(areaRadiusInKm);
        return areaRadiusStr;
    }

    void replacePossibleSubstrings(std::string& modifiedStr, const std::string& seachStr, const std::string& replacementStr)
    {
        if(boost::icontains(modifiedStr, seachStr))
        {
            boost::ireplace_all(modifiedStr, seachStr, replacementStr);
        }
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
const std::string gDefaultAreaRadiusStr = "%arearadius%";
const std::string gDefaultTimeInfoStr = "%timeinfo%";

std::string NFmiMouseClickUrlActionData::GetMouseActionUrl(SmartMetOpenUrlAction urlAction, const NFmiPoint& latlon, const NFmiMetTime& atime, float areaRadiusInKm) const
{
    std::string baseUrl = GetMouseActionBaseUrl(SmartMetOpenUrlActionToIndex(urlAction));
    if(!baseUrl.empty())
    {
        auto pointInfoStr = ::makePointInfoString(latlon);
        ::replacePossibleSubstrings(baseUrl, gDefaultPointInfoStr, pointInfoStr);
        auto timeInfoStr = ::makeTimeInfoString(atime);
        ::replacePossibleSubstrings(baseUrl, gDefaultTimeInfoStr, timeInfoStr);
        auto areaRadiusStr = ::makeAreaRadiusString(areaRadiusInKm);
        ::replacePossibleSubstrings(baseUrl, gDefaultAreaRadiusStr, areaRadiusStr);
    }
    return baseUrl;
}
