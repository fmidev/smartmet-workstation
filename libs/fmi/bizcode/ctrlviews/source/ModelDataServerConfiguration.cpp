#include "ModelDataServerConfiguration.h"
#include "NFmiGlobals.h"
#include <boost/algorithm/string.hpp>

// CreateRegValue -funktio hakee ensin arvoa Win-rekisterist‰ ja sitten optionaalisesti konfiguraatioista ja lopuksi k‰ytt‰‰ oletusarvoa.
// Haluan tehd‰ homman nyt niin ett‰ rakennetaan normaalisti rekisteri-olio, mutta jos configurationOverride on true,
// t‰llˆin hataan lopullinen arvo pakotetusti konfiguraatioista.
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

// ***************************************************
// *****  GribDataParameterMapping   *****************
// ***************************************************

GribDataParameterMapping::GribDataParameterMapping() = default;

void GribDataParameterMapping::init(const std::string& mappingStr)
{
    std::vector<std::string> parts;
    boost::split(parts, mappingStr, boost::is_any_of(";"));
    if(parts.size() < 2)
        throw std::runtime_error(std::string(__FUNCTION__) + ": server sounding grib-data parameter mapping failing, too few parts in mapping string (parName;parId;[possibleConvFunc]) '" + mappingStr + "'");

    gribDataName_ = parts[0];
    boost::algorithm::trim(gribDataName_);
    if(gribDataName_.empty())
        throw std::runtime_error(std::string(__FUNCTION__) + ": server sounding grib-data parameter mapping failing, parName was empty (parName;parId;[possibleConvFunc]) '" + mappingStr + "'");

    try
    {
        auto parId = std::stoi(parts[1]);
        if(parId <= 0)
            throw std::runtime_error(std::string(__FUNCTION__) + ": server sounding grib-data parameter mapping failing, parId must be positive integer number (parName;parId;[possibleConvFunc]) '" + mappingStr + "'");

        newbaseParameterId_ = static_cast<FmiParameterName>(parId);
    }
    catch(...)
    {
        throw std::runtime_error(std::string(__FUNCTION__) + ": server sounding grib-data parameter mapping failing, can't convert parId part to integer number (parName;parId;[possibleConvFunc]) '" + mappingStr + "'");
    }

    if(parts.size() >= 3)
    {
        conversionFunctionName_ = parts[2];
        boost::algorithm::trim(conversionFunctionName_);
    }
}

// value:n pit‰‰ olla double, koska siin‰ voi olla origintime timestamp muodossa, ja sellainen ei mahdu float:iin.
double GribDataParameterMapping::doValueConversion(double value, const std::string& conversionFunctionName)
{
    if(std::isinf(value) || std::isnan(value) || value == kFloatMissing)
        return value;

    if(boost::iequals(conversionFunctionName, kelvinToCelsiusFunction))
    {
        return value - 273.15f;
    }

    if(boost::iequals(conversionFunctionName, normalToPercentFunction))
    {
        return value * 100.f;
    }

    return value;
}

// ***************************************************
// *****  ModelDataServerConfiguration ***************
// ***************************************************

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
    // Jostain syyst‰ string-olion saaminen CachedRegString oliosta on hankalaa, siksi k‰ytet‰‰n operator std::string():i‰...
    if((*dataNameOnServer_).operator std::string().empty())
        throw std::runtime_error(std::string(__FUNCTION__) + ": unable to get legal value for DataNameOnServer (non empty) with model '" + configurationModelName + "'");

    // HKEY_LOCAL_MACHINE -keys (HUOM! n‰m‰ vaatii Admin oikeuksia Vista/Win7/Win10)
    usedKey = HKEY_LOCAL_MACHINE;

    return initGribDataPart(usedConfigurationPath);
}

bool ModelDataServerConfiguration::initGribDataPart(const std::string& usedConfigurationPath)
{
    gribDataCase_ = NFmiSettings::Optional(usedConfigurationPath + "::GribDataCase", false);
    if(!gribDataCase_)
        return true;

    gribShortName_ = NFmiSettings::Require<std::string>(usedConfigurationPath + "::GribShortName");
    gribDataParams_ = NFmiSettings::Require<std::string>(usedConfigurationPath + "::GribDataParams");
    boost::algorithm::trim(gribDataParams_);
    if(gribDataParams_.empty())
        throw std::runtime_error(std::string(__FUNCTION__) + ": server sounding grib-data gribDataParams was empty string, it's required to have value when gribDataCase is set on");

    gribDataLevels_ = NFmiSettings::Optional<std::string>(usedConfigurationPath + "::GribDataLevels", "");
    boost::algorithm::trim(gribDataLevels_);
    finalGribDataParamsStr_ = makeFinalGribDataParamsStr(gribDataParams_, gribDataLevels_);
    if(finalGribDataParamsStr_.empty())
        throw std::runtime_error(std::string(__FUNCTION__) + ": server sounding grib-data finalGribDataParamsStr was empty string, it's required to have value when gribDataCase is set on");
    gribDataParamsMappingStr_ = NFmiSettings::Require<std::string>(usedConfigurationPath + "::GribDataParamsMapping");
    gribDataParamsMapping_ = makeGribDataParamsMappings(gribDataParamsMappingStr_);
    if(gribDataParamsMapping_.empty())
        throw std::runtime_error(std::string(__FUNCTION__) + ": server sounding grib-data gribDataParamsMapping was empty, it's required to have values when gribDataCase is set on");

    return true;
}

const std::string g_levelsString = "%levels%";

std::string ModelDataServerConfiguration::makeFinalGribDataParamsStr(const std::string& originalParamStr, const std::string& possibleLevelsStr)
{
    if(possibleLevelsStr.empty())
        return originalParamStr;

    auto finalParamStr = originalParamStr;
    boost::replace_all(finalParamStr, g_levelsString, possibleLevelsStr);
    return finalParamStr;
}

std::vector<GribDataParameterMapping> ModelDataServerConfiguration::makeGribDataParamsMappings(const std::string& gribDataParamsMappingStr)
{
    std::vector<std::string> paramParts;
    boost::split(paramParts, gribDataParamsMappingStr, boost::is_any_of(","));
    if(paramParts.size() < 2)
        throw std::runtime_error(std::string(__FUNCTION__) + ": server sounding grib-data parameter mappings failing, too few param parts in mapping string '" + gribDataParamsMappingStr + "'");

    std::vector<GribDataParameterMapping> parameterMappings;
    for(auto paramPartStr : paramParts)
    {
        GribDataParameterMapping parameterMapping;
        parameterMapping.init(paramPartStr);
        parameterMappings.push_back(parameterMapping);
    }
    return parameterMappings;
}

