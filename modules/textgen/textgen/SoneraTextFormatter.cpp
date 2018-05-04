// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::SoneraTextFormatter
 */
// ======================================================================
/*!
 * \class TextGen::SoneraTextFormatter
 *
 * \brief Glyph visitor generating Sonera tailored output
 *
 * The text formatter utilizes the following variables
 * \code
 * textgen::soneraformatter::pause::paragraph = [number]
 * textgen::soneraformatter::pause::header = [number]
 * textgen::soneraformatter::pause::sentence = [number]
 * textgen::soneraformatter::pause::delimiter = [number]
 * \endcode
 * If a variable is not set, no pause phrase is output.
 */
// ======================================================================

#include "TextFormatterTools.h"
#include "SoneraTextFormatter.h"
#include "Dictionary.h"
#include "Document.h"
#include "Real.h"
#include "Glyph.h"
#include "Header.h"
#include "Integer.h"
#include "IntegerRange.h"
#include "Paragraph.h"
#include "Phrase.h"
#include "SectionTag.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "StoryTag.h"
#include <calculator/TextGenError.h>
#include "WeatherTime.h"
#include "TimePeriod.h"

#include <newbase/NFmiStringTools.h>

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Check number validity for Sonera
 *
 * Throws if the number is out of range -100...1000
 *
 * \param theNumber The number to test
 */
// ----------------------------------------------------------------------

void sonera_check(int theNumber)
{
  if (theNumber < -100 || theNumber > 100)
    throw TextGen::TextGenError("SoneraTextFormatter supports only numbers -100...100");
}

// ----------------------------------------------------------------------
/*!
 * \brief Pad a number with leading zeros
 *
 * \param theString The string to pad
 * \param theCount The desired string size
 */
// ----------------------------------------------------------------------

string padzeros(const string& theString, unsigned int theCount)
{
  if (theString.size() >= theCount) return theString;

  string ret(theCount - theString.size(), '0');
  ret += theString;
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add the given pause prompt
 *
 * \param theVar The variable containing the optional pause
 * \param theList The prompt list
 */
// ----------------------------------------------------------------------

void addpause(const std::string& theVar, list<string>& theList)
{
  if (Settings::isset(theVar))
  {
    const int num = Settings::require_int(theVar);
    theList.push_back(lexical_cast<string>(num));
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief A helper function for containers
 */
// ----------------------------------------------------------------------

template <typename Iterator>
void sonera_realize(Iterator it,
                    Iterator end,
                    const TextGen::TextFormatter& theFormatter,
                    list<string>& theList)
{
  static const string var = "textgen::soneraformatter::pause::delimiter";

  for (; it != end; ++it)
  {
    if (!((*it)->isDelimiter()))
      theFormatter.format(**it);
    else
      addpause(var, theList);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief A helper for realizing Sonera numbers
 */
// ----------------------------------------------------------------------

template <typename Container>
void sonera_realize(int theNumber,
                    Container& theContainer,
                    const TextGen::Dictionary& theDictionary)
{
  static TextGen::Phrase miinus("miinus");

  ostringstream os;

  sonera_check(theNumber);
  if (theNumber < 0) theContainer.push_back(miinus.realize(theDictionary));
  os << abs(theNumber);
  theContainer.push_back(os.str());
}
}

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

SoneraTextFormatter::SoneraTextFormatter()
    : TextFormatter(), itsParts(), itsDepth(0), itsDictionary(), itsSectionVar()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the dictionary to be used while formatting
 *
 * \param theDict The dictionary (shared)
 */
// ----------------------------------------------------------------------

void SoneraTextFormatter::dictionary(const boost::shared_ptr<Dictionary>& theDict)
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

string SoneraTextFormatter::format(const Glyph& theGlyph) const
{
  static string dummy("dummy");

  ++itsDepth;
  theGlyph.realize(*this);
  --itsDepth;

  if (itsDepth > 0) return dummy;

  const int max_words_on_line = 19;  // specified by Sonera

  int lines = 1;
  int words_on_line = 0;
  string ret;
  for (container_type::const_iterator it = itsParts.begin(); it != itsParts.end(); ++it)
  {
    if (!it->empty())
    {
      if (words_on_line >= max_words_on_line)
      {
        ret += ";\n";
        ++lines;
        words_on_line = 0;
      }
      if (words_on_line == 0) ret += 'r' + lexical_cast<string>(lines) + ',';
      ret += padzeros(*it, 3);
      ret += ',';
      ++words_on_line;
    }
  }

  if (words_on_line > 0) ret += ";\n";

  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a glyph
 *
 * \param theGlyph The glyph
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const Glyph& theGlyph) const
{
  static string dummy("glyph");
  string tokens = theGlyph.realize(*itsDictionary);
  vector<string> numbers = NFmiStringTools::Split(tokens);
  copy(numbers.begin(), numbers.end(), back_inserter(itsParts));
  return dummy;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer number
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const Integer& theInteger) const
{
  static string dummy("integer");

  sonera_realize(theInteger.value(), itsParts, *itsDictionary);

  return dummy;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a float
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const Real& theReal) const
{
  throw TextGen::TextGenError("Cannot use Reals in Sonera phone service");
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit an integer range
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const IntegerRange& theRange) const
{
  static string dummy("integerrange");

  static Phrase viiva("viiva");
  sonera_realize(theRange.startValue(), itsParts, *itsDictionary);
  if (theRange.startValue() != theRange.endValue())
  {
    viiva.realize(*this);
    sonera_realize(theRange.endValue(), itsParts, *itsDictionary);
  }

  return dummy;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a sentence
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const Sentence& theSentence) const
{
  static string dummy("sentence");
  const container_type::size_type oldsize = itsParts.size();
  sonera_realize(theSentence.begin(), theSentence.end(), *this, itsParts);

  if (itsParts.size() > oldsize)
  {
    static const string var = "textgen::soneraformatter::pause::sentence";
    addpause(var, itsParts);
  }

  return dummy;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a paragraph
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const Paragraph& theParagraph) const
{
  static string dummy("paragraph");
  const container_type::size_type oldsize = itsParts.size();
  sonera_realize(theParagraph.begin(), theParagraph.end(), *this, itsParts);

  if (itsParts.size() > oldsize)
  {
    static const string var = "textgen::soneraformatter::pause::paragraph";
    addpause(var, itsParts);
  }

  return dummy;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a header
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const Header& theHeader) const
{
  static string dummy("header");
  const container_type::size_type oldsize = itsParts.size();
  sonera_realize(theHeader.begin(), theHeader.end(), *this, itsParts);

  if (itsParts.size() > oldsize)
  {
    static const string var = "textgen::soneraformatter::pause::header";
    addpause(var, itsParts);
  }

  return dummy;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a document
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const Document& theDocument) const
{
  static string dummy("document");
  sonera_realize(theDocument.begin(), theDocument.end(), *this, itsParts);
  return dummy;
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a section tag
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const SectionTag& theSection) const
{
  itsSectionVar = theSection.realize(*itsDictionary);
  return "";
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit a story tag
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const StoryTag& theStory) const
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

string SoneraTextFormatter::visit(const WeatherTime& theTime) const
{
  return TextFormatterTools::format_time(theTime.nfmiTime(), itsStoryVar, "sonera");
}

// ----------------------------------------------------------------------
/*!
 * \brief Visit TimePeriod
 */
// ----------------------------------------------------------------------

string SoneraTextFormatter::visit(const TimePeriod& thePeriod) const
{
  return TextFormatterTools::format_time(thePeriod.weatherPeriod(), itsStoryVar, "sonera");
}

}  // namespace TextGen

// ======================================================================
