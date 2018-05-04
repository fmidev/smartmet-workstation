// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::Paragraph
 */
// ======================================================================
/*!
 * \class TextGen::Paragraph
 *
 * \brief A sequence of sentences.
 *
 * The responsibility of the Paragraph class is to contain
 * a list of sentences, which can then be converted into text.
 *
 */
// ======================================================================

#include "Paragraph.h"

#include "Dictionary.h"
#include "PlainTextFormatter.h"
#include "TextFormatter.h"
#include <calculator/TextGenError.h>

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

boost::shared_ptr<Glyph> Paragraph::clone() const
{
  boost::shared_ptr<Glyph> ret(new Paragraph(*this));
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the paragraph
 *
 * \param theDictionary The dictionary to be used
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Paragraph::realize(const Dictionary& theDictionary) const
{
  throw TextGenError("Paragraph::realize(Dictionary) should not be called");
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the sentence
 *
 * \param theFormatter The formatter
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Paragraph::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns false since paragraph is not a separator
 */
// ----------------------------------------------------------------------

bool Paragraph::isDelimiter() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Add a paragraph to the end of this paragraph
 *
 * \param theParagraph The paragraph to be added
 * \result The paragraph added to
 */
// ----------------------------------------------------------------------

Paragraph& Paragraph::operator<<(const Paragraph& theParagraph)
{
  if (this != &theParagraph)
  {
    copy(theParagraph.itsData.begin(), theParagraph.itsData.end(), back_inserter(itsData));
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
 * \brief Adding a glyph to a paragraph
 *
 * \param theGlyph The glyph to be added
 * \result The paragraph added to
 */
// ----------------------------------------------------------------------

Paragraph& Paragraph::operator<<(const Glyph& theGlyph)
{
  itsData.push_back(theGlyph.clone());
  return *this;
}

}  // namespace TextGen

// ======================================================================
