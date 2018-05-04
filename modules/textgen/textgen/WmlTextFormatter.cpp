// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::WmlTextFormatter
 */
// ======================================================================
/*!
 * \class TextGen::WmlTextFormatter
 *
 * \brief Glyph visitor generating WML output
 */
// ======================================================================

#include "WmlTextFormatter.h"
#include "Dictionary.h"
#include "Document.h"
#include "Real.h"
#include "Glyph.h"
#include "Header.h"
#include "Integer.h"
#include "IntegerRange.h"
#include "Paragraph.h"
#include "SectionTag.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "StoryTag.h"
#include "TextFormatterTools.h"
#include "WeatherTime.h"
#include "TimePeriod.h"

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Set the dictionary to be used while formatting
 *
 * \param theDict The dictionary (shared)
 */
// ----------------------------------------------------------------------

void WmlTextFormatter::dictionary(const boost::shared_ptr<Dictionary>& theDict)
{
  itsDictionary = theDict;
}

// ----------------------------------------------------------------------
/*!
 * \brief Format a glyph
 *
 * \param theGlyph The glyph
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::format(const Glyph& theGlyph) const { return theGlyph.realize(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Visit a glyph
 *
 * \param theGlyph The glyph
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const Glyph& theGlyph) const
{
  return theGlyph.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer number
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const Integer& theInteger) const
{
  return theInteger.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a float
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const Real& theReal) const
{
  return theReal.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer range
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const IntegerRange& theRange) const
{
  return theRange.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a sentence
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const Sentence& theSentence) const
{
  string ret = TextFormatterTools::realize(theSentence.begin(), theSentence.end(), *this, " ", "");
  ret = TextFormatterTools::capitalize(ret);
  TextFormatterTools::punctuate(ret);

  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a paragraph
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const Paragraph& theParagraph) const
{
  const string tags = Settings::optional_string(itsSectionVar + "::paragraph::wml::tags", "");

  string tmp =
      TextFormatterTools::realize(theParagraph.begin(), theParagraph.end(), *this, " ", "");
  ostringstream out;
  if (!tmp.empty())
  {
    out << "<p";
    if (!tags.empty()) out << ' ' << tags;
    out << '>' << tmp << "<br/></p>";
  }
  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a header
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const Header& theHeader) const
{
  const bool colon = Settings::optional_bool(itsSectionVar + "::header::colon", false);
  const int level = Settings::optional_int(itsSectionVar + "::header::wml::level", 1);

  string text = TextFormatterTools::realize(theHeader.begin(), theHeader.end(), *this, " ", "");
  text = TextFormatterTools::capitalize(text);

  if (text.empty()) return "";

  ostringstream out;

  if (level == 1)
    out << "<p><b>" << text << (colon ? ":" : "") << "</b><br/></p>";
  else if (level == 2)
    out << "<p>" << text << (colon ? ":" : "") << "</p>";

  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a document
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const Document& theDocument) const
{
  return TextFormatterTools::realize(theDocument.begin(), theDocument.end(), *this, "\n\n", "");
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a section tag
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const SectionTag& theSection) const
{
  itsSectionVar = theSection.realize(*itsDictionary);
  return "";
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a story tag
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const StoryTag& theStory) const
{
  itsStoryVar = theStory.realize(*itsDictionary);

  if (theStory.isPrefixTag())
  {
    return TextFormatterTools::get_story_value_param(itsStoryVar, itsProductName);
  }

  return "";
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit Time
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const WeatherTime& theTime) const
{
  return TextFormatterTools::format_time(theTime.nfmiTime(), itsStoryVar, "wml");
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit TimePeriod
 */
// ----------------------------------------------------------------------

string WmlTextFormatter::visit(const TimePeriod& thePeriod) const
{
  return TextFormatterTools::format_time(thePeriod.weatherPeriod(), itsStoryVar, "wml");
}

}  // namespace TextGen

// ======================================================================
