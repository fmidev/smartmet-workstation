// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::RealRange
 */
// ======================================================================
/*!
 * \class TextGen::RealRange
 * \brief Representation of a generic RealRange
 */
// ======================================================================

#include "RealRange.h"
#include "Dictionary.h"
#include <calculator/TextGenError.h>

#include <boost/shared_ptr.hpp>
#include <sstream>
#include <iomanip>

using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

RealRange::~RealRange() {}
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

RealRange::RealRange(float theStartValue,
                     float theEndValue,
                     int thePrecision,
                     const std::string& theSeparator)
    : itsRangeSeparator(theSeparator),
      itsStartValue(theStartValue),
      itsEndValue(theEndValue),
      itsPrecision(thePrecision)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Glyph> RealRange::clone() const
{
  boost::shared_ptr<Glyph> ret(new RealRange(*this));
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

std::string RealRange::realize(const Dictionary& theDictionary) const
{
  std::ostringstream os;
  if (itsStartValue == itsEndValue)
    os << fixed << setprecision(itsPrecision) << itsStartValue;
  else
  {
    if (itsStartValue < 0 && itsEndValue > 0)
      os << fixed << setprecision(itsPrecision) << itsStartValue << itsRangeSeparator << "+"
         << itsEndValue;
    else if (itsStartValue > 0 && itsEndValue < 0)
      os << fixed << setprecision(itsPrecision) << "+" << itsStartValue << itsRangeSeparator
         << itsEndValue;
    else
      os << fixed << setprecision(itsPrecision) << itsStartValue << itsRangeSeparator
         << itsEndValue;
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

std::string RealRange::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns false since RealRange is not a separator
 */
// ----------------------------------------------------------------------

bool RealRange::isDelimiter() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the range separator string
 *
 * \return The range separator string
 */
// ----------------------------------------------------------------------

const std::string& RealRange::rangeSeparator() const { return itsRangeSeparator; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the start value of the range
 *
 * \return The start value
 */
// ----------------------------------------------------------------------

float RealRange::startValue() const { return itsStartValue; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the end value of the range
 *
 * \return The end value
 */
// ----------------------------------------------------------------------

float RealRange::endValue() const { return itsEndValue; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the precision of the range
 *
 * \return The precision
 */
// ----------------------------------------------------------------------

int RealRange::precision() const { return itsPrecision; }
}  // namespace TextGen

// ======================================================================
