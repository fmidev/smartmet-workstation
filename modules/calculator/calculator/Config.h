// ======================================================================
/*!
 * \brief A thread safe configuration object
 */
// ======================================================================

#pragma once

#include <macgyver/Cast.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <map>
#include <stdexcept>
#include <string>

namespace Fmi
{
class Config
{
 public:
  Config();

  static bool isset(const std::string& theName);
  static void set(const std::string& theName, const std::string& theValue);
  static void set(const std::string& theSettingsString);
  static void clear();
  static void release();

  template <typename T>
  T require(const std::string& theName) const;

  template <typename T>
  T require(const std::string& theName, const T& theLoLimit, const T& theHiLimit) const;

  template <typename T>
  T optional(const std::string& theName, const T& theDefault) const;

  template <typename T>
  T optional(const std::string& theName,
             const T& theDefault,
             const T& theLoLimit,
             const T& theHiLimit) const;

 private:
  std::string requireString(const std::string& theName) const;
  std::string optionalString(const std::string& theName, const std::string& theDefault) const;
  bool requireBoolean(const std::string& theName) const;
  bool optionalBoolean(const std::string& theName, bool theDefault) const;
  int requireInteger(const std::string& theName) const;
  int optionalInteger(const std::string& theName, int theDefault) const;
  double requireDouble(const std::string& theName) const;
  double optionalDouble(const std::string& theName, double theDefault) const;

  //	std::map<std::string,std::string> itsSettings;	// the settings
  //	boost::mutex itsMutex;							// settings mutex

};  // class Config

// ----------------------------------------------------------------------
/*!
 * \brief Template methods
 */
// ----------------------------------------------------------------------

template <>
inline std::string Config::require<std::string>(const std::string& theName) const
{
  return requireString(theName);
}

template <>
inline bool Config::require<bool>(const std::string& theName) const
{
  return requireBoolean(theName);
}

template <>
inline int Config::require<int>(const std::string& theName) const
{
  return requireInteger(theName);
}

template <>
inline double Config::require<double>(const std::string& theName) const
{
  return requireDouble(theName);
}

template <typename T>
inline T Config::require(const std::string& theName) const
{
  try
  {
    return number_cast<T>(requireString(theName));
  }
  catch (std::exception& e)
  {
    throw std::runtime_error("Variable '" + theName + "': " + e.what());
  }
}

template <typename T>
inline T Config::require(const std::string& theName, const T& theLoLimit, const T& theHiLimit) const
{
  T value = require<T>(theName);
  if (value >= theLoLimit && value <= theHiLimit) return value;
  throw std::runtime_error("Variable '" + theName + "' value is out of range " +
                           boost::lexical_cast<std::string>(theLoLimit) + "..." +
                           boost::lexical_cast<std::string>(theHiLimit));
}

template <>
inline std::string Config::optional<std::string>(const std::string& theName,
                                                 const std::string& theDefault) const
{
  return optionalString(theName, theDefault);
}

template <>
inline bool Config::optional<bool>(const std::string& theName, const bool& theDefault) const
{
  return optionalBoolean(theName, theDefault);
}

template <>
inline int Config::optional<int>(const std::string& theName, const int& theDefault) const
{
  return optionalInteger(theName, theDefault);
}

template <>
inline double Config::optional<double>(const std::string& theName, const double& theDefault) const
{
  return optionalDouble(theName, theDefault);
}

template <typename T>
inline T Config::optional(const std::string& theName,
                          const T& theDefault,
                          const T& theLoLimit,
                          const T& theHiLimit) const
{
  T value = optional<T>(theName, theDefault);
  if (value >= theLoLimit && value <= theHiLimit) return value;
  throw std::runtime_error("Variable '" + theName + "' value is out of range " +
                           boost::lexical_cast<std::string>(theLoLimit) + "..." +
                           boost::lexical_cast<std::string>(theHiLimit));
}

}  // namespace Fmi
