// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::Sentence
 */
// ======================================================================
/*!
 * \class TextGen::Sentence
 *
 * \brief A sequence of sentences.
 *
 * The responsibility of the Sentence class is to contain
 * a list of sentences, which can then be converted into text.
 *
 */
// ======================================================================

#include "Sentence.h"

#include "Dictionary.h"
#include "Integer.h"
#include "Phrase.h"
#include "PlainTextFormatter.h"
#include "TextFormatter.h"
#include <calculator/TextGenError.h>
#include <algorithm>
#include <iterator>

using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Glyph> Sentence::clone() const
{
  boost::shared_ptr<Glyph> ret(new Sentence(*this));
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the sentence
 *
 * \param theDictionary The dictionary to be used
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Sentence::realize(const Dictionary& theDictionary) const
{
  throw TextGenError("Sentence::realize(Dictionary) should not be called");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the sentence
 *
 * \param theFormatter The formatter
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Sentence::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns false since sentence is not a separator
 */
// ----------------------------------------------------------------------

bool Sentence::isDelimiter() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Add a sentence to the end of this sentence
 *
 * \param theSentence The sentence to be added
 * \result The sentence added to
 */
// ----------------------------------------------------------------------

Sentence& Sentence::operator<<(const Sentence& theSentence)
{
  if (this != &theSentence)
  {
    copy(theSentence.itsData.begin(), theSentence.itsData.end(), back_inserter(itsData));
  }
  else
  {
    storage_type tmp(itsData);
    copy(tmp.begin(), tmp.end(), back_inserter(itsData));
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a glyph to a sentence
 *
 * \param theGlyph The glyph to be added
 * \result The sentence added to
 */
// ----------------------------------------------------------------------

Sentence& Sentence::operator<<(const Glyph& theGlyph)
{
  itsData.push_back(theGlyph.clone());
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Add a phrase to a sentence with automatic conversion
 *
 * \param thePhrase The string initializer for the phrase
 * \return The sentence added to
 */
// ----------------------------------------------------------------------

Sentence& Sentence::operator<<(const string& thePhrase)
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
 * \brief Add a number to a sentence with automatic conversion
 *
 * \param theNumber The integer initializer for the number
 * \return The sentence added to
 */
// ----------------------------------------------------------------------

Sentence& Sentence::operator<<(int theNumber)
{
  *this << Integer(theNumber);
  return *this;
}

}  // namespace TextGen

// ======================================================================
