// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::WeatherResult
 */
// ======================================================================
/*!
 * \class TextGen::WeatherResult
 *
 * \brief A storage class for an analysis result and its accuracy
 *
 * An analysis results consists of a value and an associated error
 * estimate. 0 error indicates an accurate result.
 *
 * This class provides storage only, creation and interpretation
 * is left for external classes and functions.
 *
 */
// ======================================================================

#include "WeatherResult.h"
#include <cassert>
#include <iostream>

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * Construction is possible only by explicitly stating the
 * value and accuracy, or by copy constructing. The void
 * constructor is intentionally disabled.
 *
 * \param theValue The value part of the result
 * \param theError The error part of the result
 */
// ----------------------------------------------------------------------

WeatherResult::WeatherResult(float theValue, float theError)
    : itsValue(theValue), itsError(theError)
{
  assert(itsError >= 0);
}

// ----------------------------------------------------------------------
/*!
 * \brief Value accessor
 *
 * Returns the value part of the result
 *
 * \return The value part of the result
 */
// ----------------------------------------------------------------------

float WeatherResult::value() const { return itsValue; }
// ----------------------------------------------------------------------
/*!
 * \brief Error accessor
 *
 * Returns the error part of the result
 *
 * \return The error part of the result
 */
// ----------------------------------------------------------------------

float WeatherResult::error() const { return itsError; }
}  // namespace TextGen

// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison for WeatherResult
 *
 * \param theLhs The first result
 * \param theRhs The second result
 * \return True if the results are equal
 */
// ----------------------------------------------------------------------

bool operator==(const TextGen::WeatherResult& theLhs, const TextGen::WeatherResult& theRhs)
{
  return (theLhs.value() == theRhs.value() && theLhs.error() == theRhs.error());
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison for WeatherResult
 *
 * \param theLhs The first result
 * \param theRhs The second result
 * \return True if the results are not equal
 */
// ----------------------------------------------------------------------

bool operator!=(const TextGen::WeatherResult& theLhs, const TextGen::WeatherResult& theRhs)
{
  return !(theLhs == theRhs);
}

// ----------------------------------------------------------------------
/*!
 * \brief Output operator for WeatherResult
 *
 * This is intended for logging results to MessageLogger instances.
 *
 * \param theResult The result to output
 * \param theOutput The output stream
 * \return The output stream
 */
// ----------------------------------------------------------------------

std::ostream& operator<<(std::ostream& theOutput, const TextGen::WeatherResult& theResult)
{
  theOutput << '(' << theResult.value() << ',' << theResult.error() << ')';
  return theOutput;
}

// ======================================================================
