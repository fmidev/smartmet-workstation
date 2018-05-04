// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::Delimiter
 */
// ======================================================================
/*!
 * \class TextGen::Delimiter
 * \brief Representation of a sentence delimiter, such as ","
 *
 */
// ======================================================================

#include "Delimiter.h"
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

Delimiter::~Delimiter() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theSeparator The word
 */
// ----------------------------------------------------------------------

Delimiter::Delimiter(const std::string& theSeparator) : itsSeparator(theSeparator) {}
// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Glyph> Delimiter::clone() const
{
  boost::shared_ptr<Glyph> ret(new Delimiter(*this));
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Realize using the given dictionary
 *
 * Note that the dictionary is not needed in the implementation,
 * the returned string is always the one given in the constructor.
 *
 * \param theDictionary The dictionary to realize with
 * \return The realized string
 */
// ----------------------------------------------------------------------

std::string Delimiter::realize(const Dictionary& theDictionary) const { return itsSeparator; }
// ----------------------------------------------------------------------
/*!
 * \brief Realize using the given text formatter
 *
 * \param theFormatter The text formatter
 * \return The realized string
 */
// ----------------------------------------------------------------------

std::string Delimiter::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns true
 */
// ----------------------------------------------------------------------

bool Delimiter::isDelimiter() const { return true; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the delimiter as is without realization
 *
 * \return The delimiter string
 */
// ----------------------------------------------------------------------

const string& Delimiter::value() const { return itsSeparator; }
}  // namespace TextGen

// ======================================================================
