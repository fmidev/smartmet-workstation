// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::Phrase
 */
// ======================================================================
/*!
 * \class TextGen::Phrase
 * \brief Representation of a dictionary word
 */
// ======================================================================

#include "Phrase.h"
#include "Dictionary.h"
#include "TextFormatter.h"

using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

Phrase::~Phrase() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theWord The word
 */
// ----------------------------------------------------------------------

Phrase::Phrase(const std::string& theWord) : itsWord(theWord) {}
// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Glyph> Phrase::clone() const
{
  boost::shared_ptr<Glyph> ret(new Phrase(*this));
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Realize using the given dictionary
 *
 * \param theDictionary The dictionary to realize with
 * \return The realized string
 */
// ----------------------------------------------------------------------

std::string Phrase::realize(const Dictionary& theDictionary) const
{
  return theDictionary.find(itsWord);
}

// ----------------------------------------------------------------------
/*!
 * \brief Realize using the given text formatter
 *
 * \param theFormatter The formatter
 * \return The formatter string
 */
// ----------------------------------------------------------------------

std::string Phrase::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns false since phrase is not a separator
 */
// ----------------------------------------------------------------------

bool Phrase::isDelimiter() const { return false; }
}  // namespace TextGen

// ======================================================================
