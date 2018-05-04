// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::CssTextFormatter
 */
// ======================================================================
/*!
 * \class TextGen::CssTextFormatter
 *
 * \brief Glyph visitor generating HTML output
 */
// ======================================================================

#include "CssTextFormatter.h"
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
#include <set>

using namespace std;
using namespace boost;

namespace TextGen
{
std::set<string> css_class_set;

// ----------------------------------------------------------------------
/*!
 * \brief Set the dictionary to be used while formatting
 *
 * \param theDict The dictionary (shared)
 */
// ----------------------------------------------------------------------

void CssTextFormatter::dictionary(const boost::shared_ptr<Dictionary>& theDict)
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

string CssTextFormatter::format(const Glyph& theGlyph) const { return theGlyph.realize(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Visit a glyph
 *
 * \param theGlyph The glyph
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const Glyph& theGlyph) const
{
  return theGlyph.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer number
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const Integer& theInteger) const
{
  return theInteger.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a real
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const Real& theReal) const
{
  return theReal.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer range
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const IntegerRange& theRange) const
{
  return theRange.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a sentence
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const Sentence& theSentence) const
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

string CssTextFormatter::visit(const Paragraph& theParagraph) const
{
  const string content = Settings::optional_string(itsSectionVar + "::content", "");

  if (content.compare("none") == 0) return "";

  string text =
      TextFormatterTools::realize(theParagraph.begin(), theParagraph.end(), *this, "", "\n");

  ostringstream out;
  if (!text.empty())
  {
    const string css_tag = Settings::optional_string(itsSectionVar + "::css::tag", "div");
    const string css_class = Settings::optional_string(itsSectionVar + "::css::class", "");

    // add tag if class is not empty and starting-tag for the class has not been already defined
    bool addCssTag = !css_class.empty() && !(css_class_set.find(css_class) != css_class_set.end());

    if (addCssTag)
    {
      out << '<' << css_tag << (" class=\"" + css_class + "\"") << ">";

      css_class_set.insert(css_class);
    }

    out << text;

    if (addCssTag)
    {
      out << "</" << css_tag << ">\n";

      css_class_set.erase(css_class);
    }
  }

  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a header
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const Header& theHeader) const
{
  const bool colon = Settings::optional_bool(itsSectionVar + "::header::colon", false);

  string text = TextFormatterTools::realize(theHeader.begin(), theHeader.end(), *this, " ", "");
  text = TextFormatterTools::capitalize(text);

  if (text.empty()) return "";

  ostringstream out;
  const string css_tag = Settings::optional_string(itsSectionVar + "::header::css::tag", "div");
  const string css_class = Settings::optional_string(itsSectionVar + "::header::css::class", "");

  if (!css_tag.empty())
    out << "<" << css_tag << (!css_class.empty() ? (" class=\"" + css_class + "\"") : "") << ">\n";

  out << text << (colon ? ":" : "") << "\n";

  if (!css_tag.empty()) out << "</" << css_tag << ">\n";

  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a document
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const Document& theDocument) const
{
  ostringstream out;

  //	out << "<head>\n";
  //	out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"styles.css\" />\n";
  //	out << "</head>\n\n";

  const string css_class = Settings::optional_string("textgen::css::class", "forecast");

  out << "<div class=\"" << css_class << "\">\n\n";

  out << TextFormatterTools::realize(theDocument.begin(), theDocument.end(), *this, "\n", "");
  out << "\n</div>";

  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a section tag
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const SectionTag& theSection) const
{
  itsSectionVar = theSection.realize(*itsDictionary);

  //	const string content = Settings::optional_string(itsSectionVar+"::content","");
  const string css_class = Settings::optional_string(itsSectionVar + "::css::class", "");

  // if class name not defined or starting-tag for the class already defined but not terminated
  if (css_class.empty() ||
      (css_class_set.find(css_class) != css_class_set.end() && theSection.isPrefixTag()))
    return "";

  ostringstream out;
  const string css_tag = Settings::optional_string(itsSectionVar + "::css::tag", "div");

  if (theSection.isPrefixTag())
  {
    out << "<" << css_tag;
    out << " class=\"" << css_class << "\"";
    out << ">\n";

    css_class_set.insert(css_class);
  }
  else
  {
    out << "</" << css_tag << ">\n";

    css_class_set.erase(css_class);
  }

  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a story tag
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const StoryTag& theStory) const
{
  itsStoryVar = theStory.realize(*itsDictionary);

  ostringstream out;
  const string css_tag = Settings::optional_string(itsStoryVar + "::css::tag", "span");
  const string css_class = Settings::optional_string(itsStoryVar + "::css::class", "");

  if (theStory.isPrefixTag())
  {
    if (!css_tag.empty())
    {
      out << '<' << css_tag;
      if (!css_class.empty()) out << " class=\"" << css_class << "\"";
      out << ">\n";
    }
    out << TextFormatterTools::get_story_value_param(itsStoryVar, itsProductName);
  }
  else
  {
    if (!css_tag.empty()) out << "</" << css_tag << ">";
  }

  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit Time
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const WeatherTime& theTime) const
{
  return TextFormatterTools::format_time(theTime.nfmiTime(), itsStoryVar, "css");
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit TimePeriod
 */
// ----------------------------------------------------------------------

string CssTextFormatter::visit(const TimePeriod& thePeriod) const
{
  return TextFormatterTools::format_time(thePeriod.weatherPeriod(), itsStoryVar, "css");
}

}  // namespace TextGen

// ======================================================================
