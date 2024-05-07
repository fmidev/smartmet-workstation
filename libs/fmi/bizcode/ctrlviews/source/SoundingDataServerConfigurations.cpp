#include "SoundingDataServerConfigurations.h"
#include "NFmiSettings.h"
#include "NFmiEnumConverter.h"
#include "NFmiSoundingData.h"
#include "catlog/catlog.h"
#include "SettingsFunctions.h"
#include "ModelDataServerConfiguration.h"

static const std::string g_VersionNumberName = "VersionNumber";
static const std::string g_SmartmetServerBaseUriName = "SmartmetServerBaseUri";
static const std::string g_SmartmetServerBaseUrlStart = "SmartmetServerBaseUrl_";

// Kun pyydet‰‰n konffeista NFmiSettings::ListChildren:ill‰ mallidata kohtaista listaa, meid‰n pit‰‰ 
// ohittaa tietyt sanat, koska niill‰ nimill‰ on asetukset 'p‰‰tasolla', ja ne tullaan lis‰‰m‰‰n tuohon
// ListChildren listaan. Lis‰t‰‰n myˆs tyhj‰‰ sana listaan, jotta tarkastelut  yksinkertaistuvat.
static const std::vector<std::string> g_IgnoredConfigurationVariableNames{ "", g_VersionNumberName, g_SmartmetServerBaseUriName };

// Seuraavan alkuiset sanat ignoorataan myˆs (n‰ill‰ listataan n kpl server url:eja)
static const std::vector<std::string> g_IgnoredConfigurationVariablesWithStart{ g_SmartmetServerBaseUrlStart };

static bool IsModelNameLegit(const std::string& modelName)
{
	if (std::find(g_IgnoredConfigurationVariableNames.begin(), g_IgnoredConfigurationVariableNames.end(), modelName) != g_IgnoredConfigurationVariableNames.end())
		return false;
	else
	{
		for (const auto& ignoredVariableNameStart : g_IgnoredConfigurationVariablesWithStart)
		{
			if (modelName.find(ignoredVariableNameStart) != std::string::npos)
				return false;
		}

		return true;
	}
}

bool SoundingDataServerConfigurations::init(const std::string &baseRegistryPath, const std::string &baseConfigurationPath)
{
    if(initialized_)
        throw std::runtime_error(std::string(__FUNCTION__) + ": all ready initialized");

    initialized_ = true;
    baseRegistryPath_ = baseRegistryPath;
    baseConfigurationPath_ = baseConfigurationPath;
    // Huom. 1. kFmiModelLevel parametri on vain debuggaus tarkoituksessa haettu parametri
    // Huom. 2. OriginTimeParameterId on 'feikki' parametri, jonka sijasta haetaan mallidatan origintime:a, t‰lle erikoisk‰sittely
    wantedParameters_ = std::vector<FmiParameterName>{ kFmiTemperature, kFmiDewPoint, kFmiHumidity, kFmiPressure, kFmiGeomHeight, kFmiTotalCloudCover
		, kFmiWindSpeedMS, kFmiWindDirection, kFmiModelLevel, NFmiSoundingData::OriginTimeParameterId, NFmiSoundingData::LevelParameterId
		, kFmiWindUMS, kFmiWindVMS, kFmiGeopHeight };
    wantedParametersString_ = makeWantedParametersString();
    initBaseUrlVector();

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    // 1. Hae kaikkien eri mallien nimet konffeista
    auto modelNames = NFmiSettings::ListChildren(baseConfigurationPath);
    if(modelNames.empty())
        return false;
    else
    {
        bool catchedAnyExceptions = false;
        // 2. Pit‰‰ tehd‰ versionumeron perusteella tehd‰ pakotettu p‰ivitys konffeista?
        bool configurationOverride = mustDoConfigurationOverride(usedKey);
        // 3. Luo kaikille malleille omat datat
        for(const auto &modelName : modelNames)
        {
            try
            {
                if(::IsModelNameLegit(modelName))
                    modelConfigurations_.push_back(MakeModelConfiguration(modelName, configurationOverride));
            }
            catch(std::exception &e)
            {
                catchedAnyExceptions = true;
                std::string errorMessage = __FUNCTION__;
                errorMessage += " error: ";
                errorMessage += e.what();
                CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
            }
        }

        // 4. Mik‰ oli valittu serveri
        selectedBaseUrlIndex_ = ::CreateRegValue<CachedRegInt>(baseRegistryPath_, registrySectionName_, "\\SelectedBaseUrlIndex", usedKey, 0);

        // HKEY_LOCAL_MACHINE -keys (HUOM! n‰m‰ vaatii Admin oikeuksia Vista/Win7/Win10)
        usedKey = HKEY_LOCAL_MACHINE;

        if(modelConfigurations_.empty() && catchedAnyExceptions)
            return false;
        else
            return true;
    }
}

void SoundingDataServerConfigurations::initBaseUrlVector()
{
    for(int index = 1; index <= 20; index++)
    {
        try
        {
			std::string usedKey = baseConfigurationPath_ + "::" + g_SmartmetServerBaseUrlStart;
			usedKey += std::to_string(index);
			auto serverUrl = SettingsFunctions::GetUrlFromSettings(usedKey, true);
			if (!serverUrl.empty())
            serverBaseUrls_.push_back(serverUrl);
        }
        catch(std::exception &e)
        {
            std::string errorMessage = __FUNCTION__;
            errorMessage += " error: ";
            errorMessage += e.what();
            CatLog::logMessage(errorMessage, CatLog::Severity::Error, CatLog::Category::Configuration, true);
        }
    }

    if(serverBaseUrls_.empty())
    {
        const std::string defaultSoundingDataServerUrl = "http ://smartmet.fmi.fi/timeseries";
        CatLog::logMessage(std::string("Using default sounding data server: ") + defaultSoundingDataServerUrl, CatLog::Severity::Warning, CatLog::Category::Configuration);
        serverBaseUrls_.push_back(defaultSoundingDataServerUrl);
    }
}

std::string SoundingDataServerConfigurations::makeWantedParametersString() const
{
    NFmiEnumConverter enumConverter;
    std::string str;
    for(auto paramId : wantedParameters_)
    {
        if(!str.empty())
            str += ",";
        if(paramId == NFmiSoundingData::OriginTimeParameterId)
            str += "origintime";
        else if(paramId == NFmiSoundingData::LevelParameterId)
            str += "level";
        else
            str += enumConverter.ToString(paramId);
    }
    return str;
}

// Jos rekisteriss‰ ollut SoundingDataServerConfigurations versionumero on pienempi kuin
// mit‰ on konffeissa, pit‰‰ arvot ottaa k‰yttˆˆn konffeista.
bool SoundingDataServerConfigurations::mustDoConfigurationOverride(HKEY usedKey)
{
    std::string parameterRegistryName = std::string("\\") + g_VersionNumberName;
    std::string finalParameterConfigurationPath = baseConfigurationPath_ + "::" + g_VersionNumberName;
    int nonLegalVersionNumber = 0;
    versionNumber_ = ::CreateRegValue<CachedRegInt>(baseRegistryPath_, registrySectionName_, parameterRegistryName, usedKey, nonLegalVersionNumber, finalParameterConfigurationPath);
    int versionNumberFromConfiguration = NFmiSettings::Optional<int>(finalParameterConfigurationPath, nonLegalVersionNumber);
    if(*versionNumber_ < versionNumberFromConfiguration)
    {
        // Uusi versio numero pit‰‰ ottaa myˆs talteen Win-rekisteriin
        *versionNumber_ = versionNumberFromConfiguration;
        return true;
    }
    else
        return false;
}

ModelDataServerConfiguration SoundingDataServerConfigurations::MakeModelConfiguration(const std::string &modelName, bool configurationOverride)
{
    ModelDataServerConfiguration modelConfiguration;
    modelConfiguration.init(modelName, baseRegistryPath_ + registrySectionName_, baseConfigurationPath_, configurationOverride);
    return modelConfiguration;
}

static auto makeFindProducerLambda(int producerId)
{
    return [producerId](const auto &modelConfiguration) {return producerId == modelConfiguration.producerId(); };
}

template<typename Container, typename IterCallback, typename ReturnType>
static ReturnType getInfoFromProducer(int producerId, const Container &container, ReturnType notFoundReturnValue, IterCallback iterCallback)
{
    auto findProducerLambda = makeFindProducerLambda(producerId);
    auto iter = std::find_if(container.begin(), container.end(), findProducerLambda);
    if(iter != container.end())
    {
        return iterCallback(iter);
    }
    else
        return notFoundReturnValue;
}

std::string SoundingDataServerConfigurations::dataNameOnServer(int producerId) const
{
    return ::getInfoFromProducer(producerId, modelConfigurations_, std::string(), [](auto &iter) {return iter->dataNameOnServer(); });
}

const ModelDataServerConfiguration* SoundingDataServerConfigurations::getServerConfiguration(int producerId) const
{
    auto findProducerLambda = makeFindProducerLambda(producerId);
    auto iter = std::find_if(modelConfigurations_.begin(), modelConfigurations_.end(), findProducerLambda);
    if(iter == modelConfigurations_.end())
    {
        return nullptr;
    }

    return &(*iter);
}

static std::string makeLonlatString(const NFmiPoint &latlon)
{
    std::string str = std::to_string(latlon.X());
    str += ",";
    str += std::to_string(latlon.Y());
    return str;
}

std::string SoundingDataServerConfigurations::makeFinalServerRequestUrl(int producerId, const NFmiMetTime &validTime, const NFmiPoint &latlon) const
{
    try
    {
        auto serverConfigurationPtr = getServerConfiguration(producerId);
        if(!serverConfigurationPtr)
            throw std::runtime_error(std::string("Error in ") + __FUNCTION__ + ", can't find server-configuration from given producer id " + std::to_string(producerId));

        auto baseRequestStr = getSelectedBaseUrl();
        if(baseRequestStr.empty())
            throw std::runtime_error("Error in SoundingDataServerConfigurations::makeFinalServerRequestUri, illegal server url selected (empty)");

        if(serverConfigurationPtr->gribDataCase())
        {
            return makeFinalGribDataServerRequestUrl(baseRequestStr, *serverConfigurationPtr, validTime, latlon);
        }

        std::string requestStr = baseRequestStr;
        requestStr += "producer=";
        requestStr += dataNameOnServer(producerId);
        requestStr += "&param=";
        requestStr += wantedParametersString_;
        requestStr += makeBaseUrlParameterString(validTime, latlon);
        return requestStr;
    }
    catch(std::exception &e)
    {
        std::string errorStr = __FUNCTION__;
        errorStr += ": unable to construct sounding data request URI for smartmet-server: ";
        errorStr += e.what();
        CatLog::logMessage(errorStr, CatLog::Severity::Error, CatLog::Category::NetRequest, true);
    }
    return "";
}

std::string SoundingDataServerConfigurations::makeBaseUrlParameterString(const NFmiMetTime& validTime, const NFmiPoint& latlon) const
{
    std::string urlParameterStr;
    urlParameterStr += "&lonlat=";
    urlParameterStr += makeLonlatString(latlon);
    urlParameterStr += "&timesteps=1";
    urlParameterStr += "&format=ascii";
    urlParameterStr += "&precision=double";
    // Oletus paluu formaatti on YYYYMMDDTHHMISS eli siin‰ on 'T' kirjain p‰iv‰yksen ja kellon v‰liss‰
    // Nyt halutaan k‰ytt‰‰ timestamp formaattia, jossa iso integer luku ilman sekunteja ja 'T' kirjainta eli muotoa: YYYYMMDDHHmm
    urlParameterStr += "&timeformat=timestamp";
    // Kaikki requestissa ja palauuarvoissa olevat ajat halutaan UTC ajassa
    urlParameterStr += "&tz=utc";
    // Ei haeta origintime:n avulla, haetaan toistaiseksi vain viimeisint‰ mallidataa, joka lˆytyy serverilt‰
    //urlParameterStr += "&origintime=";
    //urlParameterStr += originTime.ToStr(kYYYYMMDDHHMM);
    urlParameterStr += "&starttime=";
    urlParameterStr += validTime.ToStr(kYYYYMMDDHHMM);

    return urlParameterStr;
}

std::string SoundingDataServerConfigurations::makeFinalGribDataServerRequestUrl(const std::string &baseRequestStr, const ModelDataServerConfiguration& gribDataServerConfiguration, const NFmiMetTime& validTime, const NFmiPoint& latlon) const
{
    std::string requestStr = baseRequestStr;
    requestStr += "&param=";
    requestStr += gribDataServerConfiguration.finalGribDataParamsStr();
    requestStr += makeBaseUrlParameterString(validTime, latlon);
    return requestStr;
}

int SoundingDataServerConfigurations::selectedBaseUrlIndex() const
{
    return *selectedBaseUrlIndex_;
}

void SoundingDataServerConfigurations::setSelectedBaseUrlIndex(int newValue)
{
    *selectedBaseUrlIndex_ = newValue;
}

const std::string& SoundingDataServerConfigurations::getSelectedBaseUrl() const
{
    auto selectedIndex = *selectedBaseUrlIndex_;
    if(selectedIndex < serverBaseUrls_.size())
        return serverBaseUrls_[selectedIndex];
    else
    {
        static const std::string emptyValue;
        return emptyValue;
    }
}
