#include "SoundingDataServerConfigurations.h"
#include "NFmiSettings.h"
#include "catlog/catlog.h"

static const std::string g_VersionNumberName = "VersionNumber";

static bool IsModelNameLegit(const std::string &modelName)
{
    if(!modelName.empty())
    {
        if(modelName != g_VersionNumberName)
            return true;
    }

    return false;
}

// CreateRegValue -funktio hakee ensin arvoa Win-rekisteristä ja sitten optionaalisesti konfiguraatioista ja lopuksi käyttää oletusarvoa.
// Haluan tehdä homman nyt niin että rakennetaan normaalisti rekisteri-olio, mutta jos configurationOverride on true,
// tällöin hataan lopullinen arvo pakotetusti konfiguraatioista.
template<typename RegValueType, typename ValueType>
boost::shared_ptr<RegValueType> CreateOverrideRegValue(const std::string &baseRegistryPath, const std::string &registrySection, const std::string &parameterName, HKEY usedKey, ValueType defaultValue, const std::string &baseConfigurationPath, bool configurationOverride)
{
    std::string parameterRegistryName = std::string("\\") + parameterName;
    std::string parameterConfigurationName = std::string("::") + parameterName;
    std::string usedConfigurationPath = baseConfigurationPath + parameterConfigurationName;
    auto regValuePtr = ::CreateRegValue<RegValueType>(baseRegistryPath, registrySection, parameterRegistryName, usedKey, defaultValue, usedConfigurationPath);
    if(configurationOverride)
    {
        *regValuePtr = NFmiSettings::Require<ValueType>(usedConfigurationPath);
    }
    return regValuePtr;
}

bool ModelSoundingDataServerConfigurations::init(const std::string &configurationModelName, const std::string &baseRegistryPath, const std::string &baseConfigurationPath, bool configurationOverride)
{
    if(initialized_)
        throw std::runtime_error(std::string(__FUNCTION__) + ": all ready initialized with model '" + configurationModelName + "'");

    initialized_ = true;
    producerName_ = configurationModelName;
    baseRegistryPath_ = baseRegistryPath;
    baseConfigurationPath_ = baseConfigurationPath;
    auto usedConfigurationPath = baseConfigurationPath_ + "::" + configurationModelName;
    std::string usedRegistrySectionName = "\\" + configurationModelName;

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    int nonLegalProducerId = 0;
    producerId_ = ::CreateOverrideRegValue<CachedRegInt>(baseRegistryPath, usedRegistrySectionName, "ProducerId", usedKey, nonLegalProducerId, usedConfigurationPath, configurationOverride);
    if(*producerId_ == nonLegalProducerId)
        throw std::runtime_error(std::string(__FUNCTION__) + ": unable to get legal value for ProducerId (non 0 value) with model '" + configurationModelName + "'");
    modelNameOnServer_ = ::CreateOverrideRegValue<CachedRegString>(baseRegistryPath, usedRegistrySectionName, "ModelNameOnServer", usedKey, std::string(""), usedConfigurationPath, configurationOverride);
    // Jostain syystä string-olion saaminen CachedRegString oliosta on hankalaa, siksi käytetään operator std::string():iä...
    if((*modelNameOnServer_).operator std::string().empty())
        throw std::runtime_error(std::string(__FUNCTION__) + ": unable to get legal value for ModelNameOnServer (non empty) with model '" + configurationModelName + "'");
    // useServerData asetusta ei oteta konffeista vaikka configurationOverride olisi true
    useServerData_ = ::CreateOverrideRegValue<CachedRegBool>(baseRegistryPath, usedRegistrySectionName, "UseServerData", usedKey, false, usedConfigurationPath, false);

    // HKEY_LOCAL_MACHINE -keys (HUOM! nämä vaatii Admin oikeuksia Vista/Win7/Win10)
    usedKey = HKEY_LOCAL_MACHINE;

    return true;
}

bool SoundingDataServerConfigurations::init(const std::string &baseRegistryPath, const std::string &baseConfigurationPath)
{
    if(initialized_)
        throw std::runtime_error(std::string(__FUNCTION__) + ": all ready initialized");

    initialized_ = true;
    baseRegistryPath_ = baseRegistryPath;
    baseConfigurationPath_ = baseConfigurationPath;

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    // 1. Hae kaikkien eri mallien nimet konffeista
    auto modelNames = NFmiSettings::ListChildren(baseConfigurationPath);
    if(modelNames.empty())
        return false;
    else
    {
        bool catchedAnyExceptions = false;
        // 2. Pitää tehdä versionumeron perusteella tehdä pakotettu päivitys konffeista?
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

        // HKEY_LOCAL_MACHINE -keys (HUOM! nämä vaatii Admin oikeuksia Vista/Win7/Win10)
        usedKey = HKEY_LOCAL_MACHINE;

        if(modelConfigurations_.empty() && catchedAnyExceptions)
            return false;
        else
            return true;
    }
}

// Jos rekisterissä ollut SoundingDataServerConfigurations versionumero on pienempi kuin
// mitä on konffeissa, pitää arvot ottaa käyttöön konffeista.
bool SoundingDataServerConfigurations::mustDoConfigurationOverride(HKEY usedKey)
{
    std::string parameterRegistryName = std::string("\\") + g_VersionNumberName;
    std::string finalParameterConfigurationPath = baseConfigurationPath_ + "::" + g_VersionNumberName;
    int nonLegalVersionNumber = 0;
    versionNumber_ = ::CreateRegValue<CachedRegInt>(baseRegistryPath_, registrySectionName_, parameterRegistryName, usedKey, nonLegalVersionNumber, finalParameterConfigurationPath);
    int versionNumberFromConfiguration = NFmiSettings::Optional<int>(finalParameterConfigurationPath, nonLegalVersionNumber);
    if(*versionNumber_ < versionNumberFromConfiguration)
        return true;
    else
        return false;
}

ModelSoundingDataServerConfigurations SoundingDataServerConfigurations::MakeModelConfiguration(const std::string &modelName, bool configurationOverride)
{
    ModelSoundingDataServerConfigurations modelConfiguration;
    modelConfiguration.init(modelName, baseRegistryPath_ + registrySectionName_, baseConfigurationPath_, configurationOverride);
    return modelConfiguration;
}
