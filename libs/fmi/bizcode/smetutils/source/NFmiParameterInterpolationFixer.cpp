#include "NFmiParameterInterpolationFixer.h"
#include "NFmiQueryData.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiSettings.h"
#include "catlog/catlog.h"
#include <boost/algorithm/string.hpp>

NFmiParameterInterpolationFixer::NFmiParameterInterpolationFixer() = default;
NFmiParameterInterpolationFixer::~NFmiParameterInterpolationFixer() = default;

namespace
{
    bool modifyParametersInterpolationToLinear(NFmiDataIdent& editedParameter, bool doForceFix)
    {
        auto interpolationMethod = editedParameter.GetParam()->InterpolationMethod();
        if(interpolationMethod == kNoneInterpolation || interpolationMethod == kNearestPoint || interpolationMethod == kNearestNonMissing)
        {
            if(doForceFix)
            {
                editedParameter.GetParam()->InterpolationMethod(kLinearly);
            }
            return true;
        }
        return false;
    }

    std::vector<NFmiParam> makeCheckedParametersFromString(const std::string& parametersString)
    {
        std::vector<std::string> parts;
        boost::split(parts, parametersString, boost::is_any_of(","));
        std::vector<NFmiParam> params;
        for(auto iter = parts.begin(); iter != parts.end(); ++iter)
        {
            auto paramIdString = *iter;
            ++iter;
            if(iter != parts.end())
            {
                auto paramNameString = *iter;
                if(!paramIdString.empty() && !paramNameString.empty())
                {
                    try
                    {
                        auto parId = std::stoul(paramIdString);
                        params.push_back(NFmiParam(parId, paramNameString));
                    }
                    catch(std::exception& e)
                    {
                        std::string singleParamFailedLogMessage = "Forced linear interpolation parameter interpretation failed for following id-name pair: '";
                        singleParamFailedLogMessage += paramIdString;
                        singleParamFailedLogMessage += "','";
                        singleParamFailedLogMessage += paramNameString;
                        singleParamFailedLogMessage += "', with error: ";
                        singleParamFailedLogMessage += e.what();
                        CatLog::logMessage(singleParamFailedLogMessage, CatLog::Severity::Error, CatLog::Category::Configuration);
                    }
                }
                else
                {
                    std::string logMessage = "Error in makeCheckedParametersFromString: forced linear interpolation parameter had parId and/or parname empty: id='";
                    logMessage += paramIdString;
                    logMessage += "', name='";
                    logMessage += paramIdString;
                    logMessage += "', skipping this parameter, whole configuration string was: '";
                    logMessage += parametersString;
                    logMessage += "'";
                    CatLog::logMessage(logMessage, CatLog::Severity::Warning, CatLog::Category::Configuration);
                }
            }
            else
            {
                std::string logMessage = "Error in makeCheckedParametersFromString: given forced linear interpolation parameter list had uneven number of parameters, last one is omitted from given string: '";
                logMessage += parametersString;
                logMessage += "'";
                CatLog::logMessage(logMessage, CatLog::Severity::Warning, CatLog::Category::Configuration);
            }
        }
        return params;
    }

    std::vector<NFmiParam> makeCheckedParametersFromConfigurations(const std::string& configurationKey)
    {
        std::string loggedConfigurationKey = "'";
        loggedConfigurationKey += configurationKey + "'";
        const std::string emptyResultLogMessage = "no parameters are forced to linear interpolation";

        auto configurationValue = NFmiSettings::Optional(configurationKey, std::string(""));
        std::string configurationValueLogMessage = configurationKey;
        configurationValueLogMessage += " had following 'raw' values: ";
        configurationValueLogMessage += configurationValue;
        CatLog::logMessage(configurationValueLogMessage, CatLog::Severity::Debug, CatLog::Category::Configuration);
        try
        {
            if(configurationValue.empty())
            {
                std::string emptyValueMessage = loggedConfigurationKey;
                emptyValueMessage += " configuration has no value, " + emptyResultLogMessage;
                CatLog::logMessage(emptyValueMessage, CatLog::Severity::Debug, CatLog::Category::Configuration);
                return std::vector<NFmiParam>();
            }
            else
            {
                return ::makeCheckedParametersFromString(configurationValue);
            }
        }
        catch(std::exception &e)
        { 
            std::string emptyValueMessage = loggedConfigurationKey;
            emptyValueMessage += " configuration had value = '";
            emptyValueMessage += configurationValue;
            emptyValueMessage += "' and that resulted following error: '";
            emptyValueMessage += e.what();
            emptyValueMessage += "' and ";
            emptyValueMessage += emptyResultLogMessage;
            throw std::runtime_error(emptyValueMessage);
        }
        catch(...)
        {
            std::string emptyValueMessage = loggedConfigurationKey;
            emptyValueMessage += " configuration had value = '";
            emptyValueMessage += configurationValue;
            emptyValueMessage += "' and that resulted unknown error ";
            emptyValueMessage += " and ";
            emptyValueMessage += emptyResultLogMessage;
            throw std::runtime_error(emptyValueMessage);
        }
    }

    // TotalWind parametrin wind-vector pit‰‰ aina korjata lineaariseksi ja ilman mit‰‰n lokituksia.
    bool fixTotalWindSubparamWindVector(NFmiParamDescriptor& paramDescriptor)
    {
        if(paramDescriptor.Param(kFmiTotalWindMS))
        {
            if(paramDescriptor.Param(kFmiWindVectorMS))
            {
                return ::modifyParametersInterpolationToLinear(paramDescriptor.EditParam(false), true);
            }
        }
        return false;
    }

    std::string makeParameterLogName(const NFmiParam& param)
    {
        std::string logName = std::to_string(param.GetIdent());
        logName += ",";
        logName += param.GetName();
        return logName;
    }

    void makePossibleParameterChangeLogging(const std::string& dataFileName, const std::string& forceFixedParameterNames, bool doForcedParameterInterpolationChanges)
    {
        if(!forceFixedParameterNames.empty())
        {
            std::string startOfLogMessage = "In query-data file '";
            startOfLogMessage += dataFileName;
            startOfLogMessage += "' following parameters ";

            if(doForcedParameterInterpolationChanges)
            {
                std::string logMessage = startOfLogMessage;
                logMessage += "were changed to linear interpolation: ";
                logMessage += forceFixedParameterNames;
                CatLog::logMessage(logMessage, CatLog::Severity::Debug, CatLog::Category::Data);
            }
            else
            {
                std::string logMessage = startOfLogMessage;
                logMessage += "were in non-linear interpolation mode, but doForcedParameterInterpolationChanges option (in Settings dialog) was off: ";
                logMessage += forceFixedParameterNames;
                CatLog::logMessage(logMessage, CatLog::Severity::Warning, CatLog::Category::Data);
            }
        }
    }

} // end-of nameless namespace

// Throws exceptions in error cases
void NFmiParameterInterpolationFixer::init()
{
    if(initialized_)
        throw std::runtime_error("NFmiParameterInterpolationFixer::Init: already initialized.");

    initialized_ = true;
    auto baseRegistryPath = NFmiApplicationWinRegistry::MakeBaseRegistryPath();
    auto sectionName = NFmiApplicationWinRegistry::MakeGeneralSectionName();
    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    // Laitetaan oletus arvo true:ksi, jotta sit‰ ei tarvitse erikseen s‰‰t‰ p‰‰lle kaikissa koneissa
    doForcedParameterInterpolationChanges_ = ::CreateRegValue<CachedRegBool>(baseRegistryPath, sectionName, "\\DoForcedParameterInterpolationChanges", usedKey, true);
    // Itse oletus checkedParameters_ -lista on taas tyhj‰, kun sit‰ asetetaan makeCheckedParametersFromConfigurations 
    // funktiossa, joten se pit‰‰ laittaa konfiguraatioista aina erikseen k‰yttˆˆn
    checkedParameters_ = makeCheckedParametersFromConfigurations(makeConfigurationKey());
    doFinalChecksForCheckedParameters();
}

const std::string& NFmiParameterInterpolationFixer::makeConfigurationKey() const
{
    static const std::string configurationKey = "SmartMet::ForcedParameterInterpolationChangesList";
    return configurationKey;
}

void NFmiParameterInterpolationFixer::doFinalChecksForCheckedParameters() const
{
    if(checkedParameters_.empty())
    {
        std::string noParametersObtainedLogMessage = "No actual parameters were obtained from ";
        noParametersObtainedLogMessage += makeConfigurationKey();
        noParametersObtainedLogMessage += " option, only illegal values given?";
        CatLog::logMessage(noParametersObtainedLogMessage, CatLog::Severity::Warning, CatLog::Category::Configuration);
    }
    else
    {
        std::string followingParametersObtainedLogMessage = "Following parameters were obtained from ";
        followingParametersObtainedLogMessage += makeConfigurationKey();
        followingParametersObtainedLogMessage += " option: ";
        std::string parameterList;
        for(const auto& param : checkedParameters_)
        {
            if(!parameterList.empty())
            {
                // Lis‰t‰‰n 1. j‰lkeen aina pilkku erotin
                parameterList += ", ";
            }
            parameterList += ::makeParameterLogName(param);
        }
        followingParametersObtainedLogMessage += parameterList;
        CatLog::logMessage(followingParametersObtainedLogMessage, CatLog::Severity::Debug, CatLog::Category::Configuration);
    }
}

bool NFmiParameterInterpolationFixer::doForcedParameterInterpolationChanges()
{
    return *doForcedParameterInterpolationChanges_;
}

void NFmiParameterInterpolationFixer::doForcedParameterInterpolationChanges(bool newValue)
{
    *doForcedParameterInterpolationChanges_ = newValue;
}

void NFmiParameterInterpolationFixer::fixCheckedParametersInterpolation(NFmiQueryData* data, const std::string& dataFileName)
{
    // Huom! asema/havainto dataa ei muuteta mitenk‰‰n, data->IsGrid() pit‰‰ olla true.
    if(data && data->IsGrid())
    {
        auto paramDescriptor = data->Info()->ParamDescriptor();
        auto totalWindSubparamWindVectorrFixed = ::fixTotalWindSubparamWindVector(paramDescriptor);

        // Tutkitaan kaikki checkedParameters_ listalla olevat parametrit. Jos niit‰ lˆytyy annetusta 
        // datasta, tehd‰‰n seuraavaa:
        // 1. Jos doForcedParameterInterpolationChanges_ optio on pois p‰‰lt‰, tehd‰‰n seuraavaa:
        // 1.1. Tehd‰‰n varoitus viesti ett‰ kyseisen datan tarkastettavalla parametrilla oli ei-lineaarinen 
        //      interpolaatio k‰ytˆss‰
        // 2. Jos doForcedParameterInterpolationChanges_ optio on p‰‰ll‰, tehd‰‰n seuraavaa:
        // 2.1. Muutetaan parametrin interpolaatio lineaariseksi
        // 2.2. Lokitetaan ett‰ kyseinen muutos on tehty parametreille debug tasolla
        bool parameterModified = false;
        std::string forceFixedParameterNames;
        for(const auto& param : checkedParameters_)
        {
            if(paramDescriptor.Param(param))
            {
                auto& checkedParam = paramDescriptor.EditParam(false);
                if(::modifyParametersInterpolationToLinear(checkedParam, doForcedParameterInterpolationChanges()))
                {
                    parameterModified = true;
                    if(!forceFixedParameterNames.empty())
                    {
                        // Lis‰t‰‰n 1. j‰lkeen aina pilkku erotin
                        forceFixedParameterNames += ", ";
                    }
                    forceFixedParameterNames += ::makeParameterLogName(*checkedParam.GetParam());
                }
            }
        }

        if(totalWindSubparamWindVectorrFixed || parameterModified)
        {
            ::makePossibleParameterChangeLogging(dataFileName, forceFixedParameterNames, doForcedParameterInterpolationChanges());
            data->Info()->SetParamDescriptor(paramDescriptor);
        }

    }
}
