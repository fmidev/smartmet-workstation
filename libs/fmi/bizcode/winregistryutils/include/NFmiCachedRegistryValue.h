#pragma once

#include "registry_value.h"
#include "boost/shared_ptr.hpp"

typedef registry_string<double> regdouble;
typedef registry_binary<RECT> regrect;
typedef registry_int<bool> regbool;
typedef registry_int<int> regint;
typedef registry_binary<POINT> regpoint;
typedef registry_string<std::string> regstring;

// Luokka joka lukee rekisteri arvon kerran ja k�ytt�� sit� ajon aikana.
// Kaikki arvon muutokset laitetaan talteen takaisin rekisteriin.
// SmartMet haluaa lukea arvot rekisterist� vain kerran, koska voi olla muitakin
// SmartMet instansseja k�ynniss� ja niiss� tehdyjen rekisteri muutosten ei
// haluta n�kyv�n ajan aikana.
template<typename RegClass>
class NFmiCachedRegistryValue : public RegClass
{
public:
    typedef typename RegClass::value_type value_type;

    NFmiCachedRegistryValue(const std::string & name, HKEY base)
    :RegClass(name, base)
    ,mCacheInitialized(false)
    {
    }

    operator value_type()
    {
        if(!mCacheInitialized)
        {
            mCachedValue = RegClass::operator value_type();
            mCacheInitialized = true;
        }

        return mCachedValue;
    }

    const NFmiCachedRegistryValue & operator=(const value_type & value)
    {
        if((!mCacheInitialized) || (mCacheInitialized && mCachedValue != value))
        { // jos cachea ei ole alustettu TAI jos arvo on muuttunut, tehd��n asetukset
            mCachedValue = value;
            mCacheInitialized = true;
            RegClass::operator=(value);
        }
        return *this;
    }

private:
    value_type mCachedValue; // t�h�n talletetaan rekisterist� haettu arvo, jota k�ytet��n ohjelman suorituksen ajan
    bool mCacheInitialized;
};

typedef NFmiCachedRegistryValue<regdouble> CachedRegDouble;
typedef NFmiCachedRegistryValue<regrect> CachedRegRect;
typedef NFmiCachedRegistryValue<regbool> CachedRegBool;
typedef NFmiCachedRegistryValue<regint> CachedRegInt;
typedef NFmiCachedRegistryValue<regpoint> CachedRegPoint;
typedef NFmiCachedRegistryValue<regstring> CachedRegString;

template<typename RegValue>
static boost::shared_ptr<RegValue> CreateRegValue(const std::string &baseRegistryPath, const std::string &sectionName, const std::string &keyName, HKEY usedKey, typename RegValue::value_type defaultValue, const char *settingsKeyForDefaultValue = 0)
{
    boost::shared_ptr<RegValue> regValueKey(new RegValue(baseRegistryPath + sectionName + keyName, usedKey));
    if(regValueKey && regValueKey->exists() == false)
    {
        // kokeillaan l�ytyyk� oletusarvoa asetuksista konffi tiedostoista, ja jos ei, annetaan oletus arvo
        if(settingsKeyForDefaultValue)
            *regValueKey = NFmiSettings::Optional(std::string(settingsKeyForDefaultValue), defaultValue);
        else
            *regValueKey = defaultValue;
    }
    return regValueKey;
}
