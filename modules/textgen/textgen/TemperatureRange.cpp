// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::TemperatureRange
 */
// ======================================================================
/*!
 * \class TextGen::TemperatureRange
 * \brief Representation of a generic TemperatureRange
 */
// ======================================================================

#include "TemperatureRange.h"
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
 * \brief Constructor
 */
// ----------------------------------------------------------------------

TemperatureRange::TemperatureRange(int theStartValue,
                                   int theEndValue,
                                   const std::string& theSeparator)
    : IntegerRange(theStartValue, theEndValue, theSeparator)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Glyph> TemperatureRange::clone() const
{
  boost::shared_ptr<Glyph> ret(new TemperatureRange(*this));
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

std::string TemperatureRange::realize(const Dictionary& theDictionary) const
{
  std::ostringstream os;
  if (itsStartValue == itsEndValue)
    os << itsStartValue;
  else
  {
    if (itsStartValue <= 0 && itsEndValue > 0)
      os << itsStartValue << itsRangeSeparator << "+" << itsEndValue;
    else if (itsStartValue > 0 && itsEndValue <= 0)
      os << "+" << itsStartValue << itsRangeSeparator << itsEndValue;
    else
      os << itsStartValue << itsRangeSeparator << itsEndValue;
  }

  return os.str();
}

std::string TemperatureRange::realize(const TextFormatter& theFormatter) const
{
  return theFormatter.visit(*this);
}

}  // namespace TextGen

// ======================================================================
