// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace Settings
 */
// ======================================================================
/*!
 * \namespace Settings
 *
 *†\brief Provides parsed and checked access to Config
 *
 */
// ----------------------------------------------------------------------

// boost included laitettava ennen newbase:n NFmiGlobals-includea,
// muuten MSVC:ss‰ min max m‰‰rittelyt jo tehty

#include <boost/lexical_cast.hpp>

#include "Settings.h"
#include "WeatherResult.h"
#include "Config.h"
#include <newbase/NFmiStringTools.h>
#include "TextGenPosixTime.h"

#include <cctype>  // for std::isdigit
#include <list>
#include <stdexcept>
#include <sstream>  // std::stringstream

using namespace std;
using namespace boost;

namespace Settings
{
// ----------------------------------------------------------------------
/*!
 * \brief Test whether the given variable is set
 *
 * \param theName The variable name
 * \return True if the variable is set
 */
// ----------------------------------------------------------------------

bool isset(const std::string& theName) { return Fmi::Config().isset(theName); }
void set(const std::string& theName, const std::string& theValue)
{
  Fmi::Config().set(theName, theValue);
}

void set(const std::string& theSettingsString) { Fmi::Config().set(theSettingsString); }
// ----------------------------------------------------------------------
/*!
 * \brief Require the string value of the given variable
 *
 * Throws if the variable is not set
 *
 * \param theName The variable name
 * \return The value of the variable
 */
// ----------------------------------------------------------------------

std::string require(const std::string& theName)
{
  return Fmi::Config().require<std::string>(theName.c_str());
}

// ----------------------------------------------------------------------
/*!
 * \brief Require the nonempty string value of the given variable
 *
 * Throws if the variable is not set or if the value is empty
 *
 * \param theName The variable name
 * \return The integer value
 */
// ----------------------------------------------------------------------

std::string require_string(const std::string& theName)
{
  const string value = require(theName);
  if (value.empty()) throw runtime_error("Value of " + theName + " must be nonempty");
  return value;
}

// ----------------------------------------------------------------------
/*!
 * \brief Require the integer value of the given variable
 *
 * Throws if the variable is not set or if the value is not an integer.
 *
 * \param theName The variable name
 * \return The integer value
 */
// ----------------------------------------------------------------------

int require_int(const std::string& theName) { return Fmi::Config().require<int>(theName.c_str()); }
// ----------------------------------------------------------------------
/*!
 * \brief Require the boolean value of the given variable
 *
 * Throws if the variable is not set or if the value is not
 * "true" or "false" (as strings).
 *
 * \param theName The variable name
 * \return The integer value
 */
// ----------------------------------------------------------------------

bool require_bool(const std::string& theName)
{
  return Fmi::Config().require<bool>(theName.c_str());
}

// ----------------------------------------------------------------------
/*!
 * \brief Require the double value of the given variable
 *
 * Throws if the variable is not set or if the value is not an integer.
 *
 * \param theName The variable name
 * \return The integer value
 */
// ----------------------------------------------------------------------

double require_double(const std::string& theName)
{
  return Fmi::Config().require<double>(theName.c_str());
}

// ----------------------------------------------------------------------
/*!
 * \brief Require the hour value of the given variable
 *
 * Throws if the variable is not set or if the value is not 0-23.
 *
 * \param theName The variable name
 * \return The integer value
 */
// ----------------------------------------------------------------------

int require_hour(const std::string& theName)
{
  return Fmi::Config().require(theName.c_str(), 0, 23);
}

// ----------------------------------------------------------------------
/*!
 * \brief Require the days value of the given variable
 *
 * Throws if the variable is not set or if the value is not >= 0.
 *
 * \param theName The variable name
 * \return The integer value
 */
// ----------------------------------------------------------------------

int require_days(const std::string& theName)
{
  const int maxdays = 100000;
  return Fmi::Config().require(theName.c_str(), 0, maxdays);
}

// ----------------------------------------------------------------------
/*!
 * \brief Require the percentage value of the given variable
 *
 * Throws if the variable is not set or if the value is not 0-100.
 *
 * \param theName The variable name
 * \return The integer value
 */
// ----------------------------------------------------------------------

int require_percentage(const std::string& theName)
{
  return Fmi::Config().require(theName.c_str(), 0, 100);
}

// ----------------------------------------------------------------------
/*!
 * \brief Require the time value of the given variable
 *
 * Throws if the variable is not of the form YYYYMMDDHHMI
 *
 * \param theName The variable name
 * \return The integer value
 */
// ----------------------------------------------------------------------

TextGenPosixTime require_time(const std::string& theName)
{
  const string value = require_string(theName.c_str());

  const string msg = theName + " value " + value + " is not of form YYYYMMDDHHMI";

  if (value.size() != 12) throw runtime_error(msg);

  for (string::const_iterator it = value.begin(); it != value.end(); ++it)
    if (!isdigit(*it)) throw runtime_error(msg);

  try
  {
    const int yy = lexical_cast<int>(value.substr(0, 4));
    const int mm = lexical_cast<int>(value.substr(4, 2));
    const int dd = lexical_cast<int>(value.substr(6, 2));
    const int hh = lexical_cast<int>(value.substr(8, 2));
    const int mi = lexical_cast<int>(value.substr(10, 2));

    if (mm < 1 || mm > 12) throw runtime_error(msg);
    if (dd < 1 || dd > 31) throw runtime_error(msg);
    if (hh < 0 || hh > 23) throw runtime_error(msg);
    if (mi < 0 || mi > 59) throw runtime_error(msg);

    if (dd > TextGenPosixTime::DaysInMonth(mm, yy)) throw runtime_error(msg);

    return TextGenPosixTime(yy, mm, dd, hh, mi);
  }
  catch (std::exception&)
  {
    throw runtime_error(msg);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Require a WeatherResult value from the given variable
 *
 * \param theName The variable name
 * \return The result
 */
// ----------------------------------------------------------------------

TextGen::WeatherResult require_result(const std::string& theName)
{
  const string value = require_string(theName.c_str());

  const string msg(theName + " value " + value + " is not of form A,B");
  try
  {
    vector<string> values = NFmiStringTools::Split(value);
    if (values.size() != 2) throw runtime_error(msg);
    const float result = lexical_cast<float>(values[0]);
    const float accuracy = lexical_cast<float>(values[1]);
    return TextGen::WeatherResult(result, accuracy);
  }
  catch (std::exception&)
  {
    throw runtime_error(msg);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return optional string valued variable, or the given value
 *
 * \param theName The variable name
 * \param theDefault The value to return if the variable is not set
 * \return The optional value
 */
// ----------------------------------------------------------------------

std::string optional_string(const std::string& theName, const std::string& theDefault)
{
  return Fmi::Config().optional<string>(theName, theDefault);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return optional integer valued variable, or the given value
 *
 * \param theName The variable name
 * \param theDefault The value to return if the variable is not set
 * \return The optional value
 */
// ----------------------------------------------------------------------

int optional_int(const std::string& theName, int theDefault)
{
  stringstream ss;
  ss << theDefault;
  return boost::lexical_cast<int>(Fmi::Config().optional<string>(theName, ss.str()));

  // return Fmi::Config().optional<int>(theName ,theDefault);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return optional boolean valued variable, or the given value
 *
 * \param theName The variable name
 * \param theDefault The value to return if the variable is not set
 * \return The optional value
 */
// ----------------------------------------------------------------------

bool optional_bool(const std::string& theName, bool theDefault)
{
  return Fmi::Config().optional(theName, theDefault);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return optional double valued variable, or the given value
 *
 * \param theName The variable name
 * \param theDefault The value to return if the variable is not set
 * \return The optional value
 */
// ----------------------------------------------------------------------

double optional_double(const std::string& theName, double theDefault)
{
  /*
  stringstream ss;
  ss << theDefault;
  return boost::lexical_cast<double>(Fmi::Config().optional<string>(theName ,ss.str()));
*/
  return Fmi::Config().optional<double>(theName, theDefault);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return optional hour valued variable, or the given value
 *
 * \param theName The variable name
 * \param theDefault The value to return if the variable is not set
 * \return The optional value
 */
// ----------------------------------------------------------------------

int optional_hour(const std::string& theName, int theDefault)
{
  return Fmi::Config().optional(theName, theDefault, 0, 23);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return optional percentage valued variable, or the given value
 *
 * \param theName The variable name
 * \param theDefault The value to return if the variable is not set
 * \return The optional value
 */
// ----------------------------------------------------------------------

int optional_percentage(const std::string& theName, int theDefault)
{
  return Fmi::Config().optional(theName, theDefault, 0, 100);
}

void clear() { Fmi::Config::clear(); }
void release() { Fmi::Config::release(); }
}

// ======================================================================
