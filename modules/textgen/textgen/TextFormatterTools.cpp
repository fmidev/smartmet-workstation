// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::TextFormatterTools
 */
// ======================================================================
/*!
 * \namespace TextGen::TextFormatterTools
 *
 * \brief Common utility functions for actual TextFormatter classes
 *
 */
// ======================================================================

#include "TextFormatterTools.h"
#include <calculator/WeatherPeriod.h>
#include <calculator/Settings.h>
#include <newbase/NFmiStringTools.h>
#include <calculator/TextGenPosixTime.h>
#include <boost/locale.hpp>
#include <boost/algorithm/string/replace.hpp>

using namespace std;
using namespace boost;
using namespace boost::locale::as;

namespace TextGen
{
namespace TextFormatterTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Capitalize the given string (UTF-8!!!)
 *
 * TODO: Note that we use fi_FI.UTF-8 by default, this should be improved.
 *
 * \param theString The string to capitalize
 */
// ----------------------------------------------------------------------

std::string capitalize(std::string& theString)
{
  using namespace boost::locale;
  using namespace boost::locale::boundary;

  generator gen;
  std::locale loc(gen("fi_FI.UTF-8"));
  ssegment_index wordmap(word, theString.begin(), theString.end(), loc);

  std::string ret;
  for (ssegment_index::iterator it = wordmap.begin(), e = wordmap.end(); it != e; ++it)
  {
    if (it == wordmap.begin())
    {
      ret += to_title(it->str(), loc);
    }
    else
    {
      ret += it->str();
    }
  }
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Punctuate the given string
 *
 * \param theString The string to punctuate
 */
// ----------------------------------------------------------------------

void punctuate(std::string& theString)
{
  if (!theString.empty()) theString += '.';
}

// ----------------------------------------------------------------------
/*!
 * \brief Create pattern string of form [N]
 *
 * \param n The pattern number
 */
// ----------------------------------------------------------------------

std::string make_needle(int n) { return "[" + boost::lexical_cast<std::string>(n) + "]"; }
// ----------------------------------------------------------------------
/*!
 * \brief Count patterns in order [1] ... [N]
 *
 * \param theString the string containing 0-N patterns
 */
// ----------------------------------------------------------------------

int count_patterns(const std::string& theString)
{
  int n = 0;
  std::string needle = "[1]";

  while (true)
  {
    if (!boost::algorithm::find_first(theString, needle)) return n;
    ++n;
    needle = make_needle(n + 1);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Format the timestamp using strftime style formatting
 *
 * \param theTime the timestamp
 * \param theFormattingString strftime style formatting string
 */
// ----------------------------------------------------------------------
std::string format_time(const TextGenPosixTime& theTime, const std::string& theFormattingString)
{
  if (theFormattingString.empty()) return "";

  std::ostringstream string_stream;

  string_stream << ftime(theFormattingString) << gmt << theTime.EpochTime();

  std::string formatted_string(string_stream.str());

  if (formatted_string.at(0) == '"')
    formatted_string = formatted_string.substr(1, formatted_string.length() - 2);

  return formatted_string;
}

// ----------------------------------------------------------------------
/*!
 * \brief Format the time period using strftime style formatting
 *
 * \param theTime TextGenPosixTime timestamp
 * \param theStoryVar the story variable
 * \param theFormatterName the formatter name
 */
// ----------------------------------------------------------------------
std::string format_time(const TextGenPosixTime& theTime,
                        const std::string& theStoryVar,
                        const std::string& theFormatterName)
{
  std::string timeformat =
      Settings::optional_string(theStoryVar + "::" + theFormatterName + "::timeformat", "");

  if (timeformat.empty()) timeformat = Settings::optional_string(theStoryVar + "::timeformat", "");

  return format_time(theTime, timeformat);
}

// ----------------------------------------------------------------------
/*!
 * \brief Format the time period using strftime style formatting
 *
 * \param thePeriod the weather period
 * \param theStartFormattingString strftime style formatting string for start time of the period
 * \param theEndFormattingString strftime style formatting string for end time of the period
 */
// ----------------------------------------------------------------------
std::string format_time(const WeatherPeriod& thePeriod,
                        const std::string& theStoryVar,
                        const std::string& theFormatterName)
{
  std::string startformat =
      Settings::optional_string(theStoryVar + "::" + theFormatterName + "::startformat", "");
  std::string endformat =
      Settings::optional_string(theStoryVar + "::" + theFormatterName + "::endformat", "");

  if (startformat.empty())
    startformat = Settings::optional_string(theStoryVar + "::startformat", "");
  if (endformat.empty()) endformat = Settings::optional_string(theStoryVar + "::endformat", "");

  std::string start_time_string(format_time(thePeriod.localStartTime(), startformat));
  std::string end_time_string(format_time(thePeriod.localEndTime(), endformat));

  return start_time_string + end_time_string;
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns value parameter for the story
 *
 * \param theStoryVar the story parameter
 * \param theProductName the ptoduct name
 */
// ----------------------------------------------------------------------
std::string get_story_value_param(const std::string& theStoryVar, const std::string& theProductName)
{
  std::string retval("");

  if (Settings::isset(theStoryVar + "::value::" + theProductName))
    retval = Settings::require_string(theStoryVar + "::value::" + theProductName);
  else
    retval = Settings::optional_string(theStoryVar + "::value", "");

  // line feed
  replace_all(retval, "\\n", "\n");
  // carrige return
  replace_all(retval, "\\r", "\r");
  // horizontal tabulator
  replace_all(retval, "\\t", "\t");

  // this is for raw text inclusions
  if (!retval.empty() && retval[0] == '@')
  {
    return "";
  }

  return retval;
}

}  // namespace TextFormatterTools
}  // namespace T
