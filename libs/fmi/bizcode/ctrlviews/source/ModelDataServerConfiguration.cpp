#include "ModelDataServerConfiguration.h"

// CreateRegValue -funktio hakee ensin arvoa Win-rekisteristä ja sitten optionaalisesti konfiguraatioista ja lopuksi käyttää oletusarvoa.
// Haluan tehdä homman nyt niin että rakennetaan normaalisti rekisteri-olio, mutta jos configurationOverride on true,
// tällöin hataan lopullinen arvo pakotetusti konfiguraatioista.
template<typename RegValueType, typename ValueType>
static boost::shared_ptr<RegValueType> CreateOverrideRegValue(const std::string &baseRegistryPath, const std::string &registrySection, const std::string &parameterName, HKEY usedKey, ValueType defaultValue, const std::string &baseConfigurationPath, bool configurationOverride)
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


bool ModelDataServerConfiguration::init(const std::string &configurationModelName, const std::string &baseRegistryPath, const std::string &baseConfigurationPath, bool configurationOverride)
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
    dataNameOnServer_ = ::CreateOverrideRegValue<CachedRegString>(baseRegistryPath, usedRegistrySectionName, "DataNameOnServer", usedKey, std::string(""), usedConfigurationPath, configurationOverride);
    // Jostain syystä string-olion saaminen CachedRegString oliosta on hankalaa, siksi käytetään operator std::string():iä...
    if((*dataNameOnServer_).operator std::string().empty())
        throw std::runtime_error(std::string(__FUNCTION__) + ": unable to get legal value for DataNameOnServer (non empty) with model '" + configurationModelName + "'");

    // HKEY_LOCAL_MACHINE -keys (HUOM! nämä vaatii Admin oikeuksia Vista/Win7/Win10)
    usedKey = HKEY_LOCAL_MACHINE;

    return true;
}
