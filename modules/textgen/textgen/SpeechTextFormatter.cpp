// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::SpeechTextFormatter
 */
// ======================================================================
/*!
 * \class TextGen::SpeechTextFormatter
 *
 * \brief Glyph visitor generating input for BitLips speech generator
 */
// ======================================================================

#include "SpeechTextFormatter.h"
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

void SpeechTextFormatter::dictionary(const boost::shared_ptr<Dictionary>& theDict)
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

string SpeechTextFormatter::format(const Glyph& theGlyph) const { return theGlyph.realize(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Visit a glyph
 *
 * \param theGlyph The glyph
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const Glyph& theGlyph) const
{
  return theGlyph.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer number
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const Integer& theInteger) const
{
  return theInteger.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a float
 *
 * Note: The generator understands only numbers of the form "2,8"
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const Real& theReal) const
{
  Real dummy(theReal.value(), theReal.precision(), false);

  return dummy.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer range
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const IntegerRange& theRange) const
{
  return theRange.realize(*itsDictionary);
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a sentence
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const Sentence& theSentence) const
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

string SpeechTextFormatter::visit(const Paragraph& theParagraph) const
{
  string ret =
      TextFormatterTools::realize(theParagraph.begin(), theParagraph.end(), *this, " ", "");
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a header
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const Header& theHeader) const
{
  const bool colon = Settings::optional_bool(itsSectionVar + "::header::colon", false);

  string ret = TextFormatterTools::realize(theHeader.begin(), theHeader.end(), *this, " ", "");
  ret = TextFormatterTools::capitalize(ret);
  if (!ret.empty())
  {
    if (colon)
      ret += ':';
    else
      ret += '.';
  }

  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a document
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const Document& theDocument) const
{
  string ret = TextFormatterTools::realize(theDocument.begin(), theDocument.end(), *this, " ", "");
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a section tag
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const SectionTag& theSection) const
{
  itsSectionVar = theSection.realize(*itsDictionary);
  return "";
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a story tag
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const StoryTag& theStory) const
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

string SpeechTextFormatter::visit(const WeatherTime& theTime) const
{
  return TextFormatterTools::format_time(theTime.nfmiTime(), itsStoryVar, "speech");
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit TimePeriod
 */
// ----------------------------------------------------------------------

string SpeechTextFormatter::visit(const TimePeriod& thePeriod) const
{
  return TextFormatterTools::format_time(thePeriod.weatherPeriod(), itsStoryVar, "speech");
}
}  // namespace TextGen

// ======================================================================
