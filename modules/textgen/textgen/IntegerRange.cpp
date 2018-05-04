// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::IntegerRange
 */
// ======================================================================
/*!
 * \class TextGen::IntegerRange
 * \brief Representation of a generic IntegerRange
 */
// ======================================================================

#include "IntegerRange.h"
#include "Dictionary.h"
#include <calculator/TextGenError.h>

#include <boost/shared_ptr.hpp>
#include <sstream>

using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

IntegerRange::~IntegerRange() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

IntegerRange::IntegerRange(int theStartValue, int theEndValue, const std::string& theSeparator)
    : itsRangeSeparator(theSeparator), itsStartValue(theStartValue), itsEndValue(theEndValue)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Glyph> IntegerRange::clone() const
{
  boost::shared_ptr<Glyph> ret(new IntegerRange(*this));
  return ret;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the text for the range
 *
 * \param theDictionary The dictionary to be used
 * \return The text
 */
// ----------------------------------------------------------------------

std::string IntegerRange::realize(const Dictionary& theDictionary) const
{
  std::ostringstream os;
  if (itsStartValue == itsEndValue)
    os << itsStartValue;
  else
  {
    if (itsStartValue < 0 && itsEndValue > 0)
      os << itsStartValue << itsRangeSeparator << "+" << itsEndValue;
    else if (itsStartValue > 0 && itsEndValue < 0)
      os << "+" << itsStartValue << itsRangeSeparator << itsEndValue;
    else
      os << itsStartValue << itsRangeSeparator << itsEndValue;
  }

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

std::string IntegerRange::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns false since IntegerRange is not a separator
 */
// ----------------------------------------------------------------------

bool IntegerRange::isDelimiter() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the range separator string
 *
 * \return The range separator string
 */
// ----------------------------------------------------------------------

const std::string& IntegerRange::rangeSeparator() const { return itsRangeSeparator; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the start value of the range
 *
 * \return The start value
 */
// ----------------------------------------------------------------------

int IntegerRange::startValue() const { return itsStartValue; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the end value of the range
 *
 * \return The end value
 */
// ----------------------------------------------------------------------

int IntegerRange::endValue() const { return itsEndValue; }
}  // namespace TextGen

// ======================================================================
