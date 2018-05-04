// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::Real
 */
// ======================================================================
/*!
 * \class TextGen::Real
 * \brief Representation of a generic floating point number
 */
// ======================================================================

#include "Real.h"
#include "Dictionary.h"
#include "TextFormatter.h"
#include <calculator/TextGenError.h>

#include <newbase/NFmiStringTools.h>

#include <boost/shared_ptr.hpp>

#include <sstream>
#include <iomanip>

using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

Real::~Real() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

Real::Real(float theReal, int thePrecision, bool theComma)
    : itsReal(theReal), itsPrecision(thePrecision), itsComma(theComma)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Glyph> Real::clone() const
{
  boost::shared_ptr<Glyph> ret(new Real(*this));
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the Real
 *
 * \param theDictionary The dictionary to be used
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Real::realize(const Dictionary& theDictionary) const
{
  ostringstream os;
  os << fixed << setprecision(itsPrecision) << itsReal;
  string result = os.str();
  if (!itsComma) NFmiStringTools::ReplaceChars(result, '.', ',');
  return result;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the number
 *
 * \param theFormatter The formatter
 * \return The text
 */
// ----------------------------------------------------------------------

std::string Real::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns false since Real is not a separator
 */
// ----------------------------------------------------------------------

bool Real::isDelimiter() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the value of the integer
 *
 * \return The integer
 */
// ----------------------------------------------------------------------

float Real::value() const { return itsReal; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the precision of the real
 *
 * \return The precision
 */
// ----------------------------------------------------------------------

int Real::precision() const { return itsPrecision; }
// ----------------------------------------------------------------------
/*!
 * \brief Return true if comma is in use
 *
 * \return True, if comma is in use
 */
// ----------------------------------------------------------------------

bool Real::comma() const { return itsComma; }
}  // namespace TextGen

// ======================================================================
