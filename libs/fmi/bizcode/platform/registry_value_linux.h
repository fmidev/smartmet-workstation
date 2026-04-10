#pragma once
#define SMARTMET_REGISTRY_VALUE_LINUX_H
// Linux replacement for registry_value.h
// Uses NFmiSettings (file-based key-value store) instead of Windows Registry.
// Provides the same template class interfaces so NFmiCachedRegistryValue.h
// works unchanged on both platforms.

#ifdef UNIX

#include "linux_compat.h"
#include "NFmiSettings.h"
#include <string>
#include <sstream>
#include <iomanip>

// HKEY stub -- NFmiSettings is a global singleton, no hive concept needed
using HKEY = void*;
inline HKEY HKEY_CURRENT_USER = nullptr;
inline HKEY HKEY_LOCAL_MACHINE = nullptr;

// Windows Registry type constants (unused but referenced in NFmiCachedRegistryValue.h)
constexpr unsigned long REG_SZ = 1;
constexpr unsigned long REG_DWORD = 4;
constexpr unsigned long REG_BINARY = 3;
constexpr unsigned long ERROR_SUCCESS = 0;

class registry_value
{
public:
    registry_value(const std::string& name, HKEY /*base*/)
    {
        // Convert Windows backslash-delimited registry path to NFmiSettings key
        // e.g. "Software\Fmi\SmartMet\General\Key" -> "Software::Fmi::SmartMet::General::Key"
        settingsKey_ = name;
        for(auto& c : settingsKey_)
        {
            if(c == '\\')
                c = ':';
        }
        // Replace single colons with double (the loop above gives us single colons)
        std::string result;
        result.reserve(settingsKey_.size() * 2);
        for(size_t i = 0; i < settingsKey_.size(); ++i)
        {
            result += settingsKey_[i];
            if(settingsKey_[i] == ':' && (i + 1 >= settingsKey_.size() || settingsKey_[i + 1] != ':'))
                result += ':';
        }
        settingsKey_ = result;
    }

    virtual ~registry_value() = default;

    bool exists()
    {
        return NFmiSettings::IsSet(settingsKey_);
    }

    void remove_value()
    {
        if(NFmiSettings::IsSet(settingsKey_))
            NFmiSettings::Set(settingsKey_, "");
    }

    void remove_key() {}

protected:
    const std::string& key() const { return settingsKey_; }

private:
    std::string settingsKey_;
};


template<class T>
class registry_string : public registry_value
{
public:
    using value_type = T;

    registry_string(const std::string& name, HKEY base) : registry_value(name, base) {}

    operator T()
    {
        return NFmiSettings::Optional<T>(key(), T());
    }

    const registry_string& operator=(const T& value)
    {
        std::stringstream ss;
        ss << std::setprecision(20) << value;
        NFmiSettings::Set(key(), ss.str());
        return *this;
    }
};


template<>
class registry_string<std::string> : public registry_value
{
public:
    using value_type = std::string;

    registry_string(const std::string& name, HKEY base) : registry_value(name, base) {}

    operator std::string()
    {
        return NFmiSettings::Optional<std::string>(key(), "");
    }

    const registry_string& operator=(const std::string& value)
    {
        NFmiSettings::Set(key(), value);
        return *this;
    }
};


template<class T>
class registry_int : public registry_value
{
public:
    using value_type = T;

    registry_int(const std::string& name, HKEY base) : registry_value(name, base) {}

    operator T()
    {
        return static_cast<T>(NFmiSettings::Optional<int>(key(), 0));
    }

    const registry_int& operator=(T value)
    {
        NFmiSettings::Set(key(), std::to_string(static_cast<int>(value)));
        return *this;
    }
};


template<>
class registry_int<bool> : public registry_value
{
public:
    using value_type = bool;

    registry_int(const std::string& name, HKEY base) : registry_value(name, base) {}

    operator bool()
    {
        return NFmiSettings::Optional<int>(key(), 0) != 0;
    }

    const registry_int& operator=(bool value)
    {
        NFmiSettings::Set(key(), value ? "1" : "0");
        return *this;
    }
};


// Binary registry values -- serialize structs to comma-separated strings.
// On Linux RECT and POINT are not real Win32 structs, so this is mainly
// a stub that preserves the interface. Specializations can be added as needed.
template<class T>
class registry_binary : public registry_value
{
public:
    using value_type = T;

    registry_binary(const std::string& name, HKEY base) : registry_value(name, base) {}

    operator T()
    {
        return T();  // Default-constructed value if not overridden
    }

    const registry_binary& operator=(const T& /*value*/)
    {
        // No-op for unknown binary types on Linux
        return *this;
    }
};

#endif // UNIX
