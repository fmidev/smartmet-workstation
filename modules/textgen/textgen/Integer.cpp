// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::Integer
 */
// ======================================================================
/*!
 * \class TextGen::Integer
 * \brief Representation of a generic integer
 */
// ======================================================================

#include "Integer.h"
#include "Dictionary.h"
#include "TextFormatter.h"
#include <calculator/TextGenError.h>

#include <boost/shared_ptr.hpp>

#include <sstream>

using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

Integer::~Integer() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

Integer::Integer(int theInteger) : itsInteger(theInteger) {}
// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Glyph> Integer::clone() const
{
  boost::shared_ptr<Glyph> ret(new Integer(*this));
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the Integer
 *
 * \param theDictionary The dictionary to be used
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Integer::realize(const Dictionary& theDictionary) const
{
  ostringstream os;
  os << itsInteger;
  return os.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the number
 *
 * \param theFormatter The formatter
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Integer::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns false since Integer is not a separator
 */
// ----------------------------------------------------------------------

bool Integer::isDelimiter() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the value of the integer
 *
 * \return The integer
 */
// ----------------------------------------------------------------------

int Integer::value() const { return itsInteger; }
}  // namespace TextGen

// ======================================================================
