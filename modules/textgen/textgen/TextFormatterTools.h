// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::TextFormatterTools
 */
// ======================================================================

#ifndef TEXTGEN_TEXTFORMATTERTOOLS_H
#define TEXTGEN_TEXTFORMATTERTOOLS_H

#include "TextFormatter.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <string>
#include <iostream>

class TextGenPosixTime;

namespace TextGen
{
class WeatherPeriod;

namespace TextFormatterTools
{
std::string capitalize(std::string& theString);
void punctuate(std::string& theString);
std::string make_needle(int n);
int count_patterns(const std::string& theString);
std::string format_time(const TextGenPosixTime& theTime, const std::string& theFormattingString);
std::string format_time(const TextGenPosixTime& theTime,
                        const std::string& theStoryVar,
                        const std::string& theFormatterName);
std::string format_time(const WeatherPeriod& thePeriod,
                        const std::string& theStoryVar,
                        const std::string& theFormatterName);
std::string get_story_value_param(const std::string& theStoryVar,
                                  const std::string& theProductName);

// ----------------------------------------------------------------------
/*!
 * \brief Realize the given Glyphs and join them
 *
 * \param it The begin iterator
 * \param end The end iterator
 * \param theFormatter The text formatter
 * \param thePrefix The string joining prefix
 * \param theSuffix The string joining prefix
 * \return The realized string
 */
// ----------------------------------------------------------------------

template <typename Iterator>
const std::string realize(Iterator it,
                          Iterator end,
                          const TextFormatter& theFormatter,
                          const std::string& thePrefix,
                          const std::string& theSuffix)
{
  std::string ret, tmp;

  // Number of patterns to replace
  int patterns = 0;
  // Next pattern to replace
  int pattern = 1;

  for (; it != end; ++it)
  {
    bool isdelim = (*it)->isDelimiter();

    tmp = theFormatter.format(**it);  // iterator -> shared_ptr -> object

    if (patterns > 0)
    {
      std::string needle = make_needle(pattern++);

      // Normal replace for normal glyphs
      if (tmp.empty())
      {
        boost::algorithm::replace_first(ret, " " + needle, tmp);
        boost::algorithm::replace_first(ret, needle + " ", tmp);
        boost::algorithm::replace_first(ret, needle, tmp);
      }
      else if (!isdelim)
        boost::algorithm::replace_first(ret, needle, tmp);
      else
      {
        // Try replacing " [N]" first for delimiters
        // We should test if the first one succeeds to avoid
        // the second replace, but replace does not return
        /// a boolean on success.
        boost::algorithm::replace_first(ret, " " + needle, tmp);
        boost::algorithm::replace_first(ret, needle, tmp);
      }
      if (pattern > patterns)
      {
        patterns = 0;
        pattern = 1;
      }
    }

    else if (!tmp.empty())
    {
      patterns = count_patterns(tmp);

      if (!ret.empty() && !isdelim) ret += thePrefix;
      ret += tmp;
      if (!isdelim) ret += theSuffix;
    }
  }

  return ret;
}

}  // namespace TextFormatterTools

}  // namespace TextGen

#endif  // TEXTGEN_TEXTFORMATTERTOOLS_H
// =====================================================================
