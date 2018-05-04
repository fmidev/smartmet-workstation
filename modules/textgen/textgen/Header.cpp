// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::Header
 */
// ======================================================================
/*!
 * \class TextGen::Header
 *
 * \brief A sequence of glyphs forming a header
 *
 */
// ======================================================================

#include "Header.h"

#include "Dictionary.h"
#include "Integer.h"
#include "Phrase.h"
#include "PlainTextFormatter.h"
#include <calculator/TextGenError.h>

using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Glyph> Header::clone() const
{
  boost::shared_ptr<Glyph> ret(new Header(*this));
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the header
 *
 * \param theDictionary The dictionary to be used
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Header::realize(const Dictionary& theDictionary) const
{
  throw TextGenError("Header::realize(Dictionary) should not be called");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the header
 *
 * \param theFormatter The formatter
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Header::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns false since header is not a separator
 */
// ----------------------------------------------------------------------

bool Header::isDelimiter() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Adding a glyph to a header
 *
 * \param theGlyph The glyph to be added
 * \result The header added to
 */
// ----------------------------------------------------------------------

Header& Header::operator<<(const Glyph& theGlyph)
{
  itsData.push_back(theGlyph.clone());
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a phrase to a header with automatic conversion
 *
 * \param thePhrase The string initializer for the phrase
 * \return The header added to
 */
// ----------------------------------------------------------------------

Header& Header::operator<<(const string& thePhrase)
{
  if (!thePhrase.empty())
  {
    boost::shared_ptr<Phrase> phrase(new Phrase(thePhrase));
    itsData.push_back(phrase);
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a number to a header with automatic conversion
 *
 * \param theNumber The integer initializer for the number
 * \return The header added to
 */
// ----------------------------------------------------------------------

Header& Header::operator<<(int theNumber)
{
  boost::shared_ptr<Integer> number(new Integer(theNumber));
  itsData.push_back(number);
  return *this;
}

}  // namespace TextGen

// ======================================================================
