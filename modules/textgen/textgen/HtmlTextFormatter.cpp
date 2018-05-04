// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::HtmlTextFormatter
 */
// ======================================================================
/*!
 * \class TextGen::HtmlTextFormatter
 *
 * \brief Glyph visitor generating HTML output
 */
// ======================================================================

#include "HtmlTextFormatter.h"
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

void HtmlTextFormatter::dictionary(const boost::shared_ptr<Dictionary>& theDict)
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

string HtmlTextFormatter::format(const Glyph& theGlyph) const { return theGlyph.realize(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Visit a glyph
 *
 * \param theGlyph The glyph
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const Glyph& theGlyph) const
{
  return theGlyph.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer number
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const Integer& theInteger) const
{
  return theInteger.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a real
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const Real& theReal) const
{
  return theReal.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer range
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const IntegerRange& theRange) const
{
  return theRange.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a sentence
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const Sentence& theSentence) const
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

string HtmlTextFormatter::visit(const Paragraph& theParagraph) const
{
  const string tags = Settings::optional_string(itsSectionVar + "::paragraph::html::tags", "");

  string tmp =
      TextFormatterTools::realize(theParagraph.begin(), theParagraph.end(), *this, " ", "");

  ostringstream out;
  if (!tmp.empty())
  {
    out << "<p";
    if (!tags.empty()) out << ' ' << tags;
    out << '>' << tmp << "</p>";
  }
  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a header
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const Header& theHeader) const
{
  const bool colon = Settings::optional_bool(itsSectionVar + "::header::colon", false);
  const int level = Settings::optional_int(itsSectionVar + "::header::html::level", 1);
  const string tags = Settings::optional_string(itsSectionVar + "::header::html::tags", "");

  string text = TextFormatterTools::realize(theHeader.begin(), theHeader.end(), *this, " ", "");
  text = TextFormatterTools::capitalize(text);

  if (text.empty()) return "";

  ostringstream out;
  out << "<h" << level;
  if (!tags.empty()) out << ' ' << tags;
  out << '>' << text << (colon ? ":" : "") << "</h" << level << '>';
  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a document
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const Document& theDocument) const
{
  const string tags = Settings::optional_string("textgen::document::html::tags", "");

  ostringstream out;
  if (tags.empty())
    out << "<div>";
  else
    out << "<div " << tags << '>';

  out << TextFormatterTools::realize(theDocument.begin(), theDocument.end(), *this, "\n\n", "");
  out << "</div>";
  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a section tag
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const SectionTag& theSection) const
{
  itsSectionVar = theSection.realize(*itsDictionary);
  return "";
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a story tag
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const StoryTag& theStory) const
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

string HtmlTextFormatter::visit(const WeatherTime& theTime) const
{
  return TextFormatterTools::format_time(theTime.nfmiTime(), itsStoryVar, "html");
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit TimePeriod
 */
// ----------------------------------------------------------------------

string HtmlTextFormatter::visit(const TimePeriod& thePeriod) const
{
  return TextFormatterTools::format_time(thePeriod.weatherPeriod(), itsStoryVar, "html");
}

}  // namespace TextGen

// ======================================================================
