// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::WeatherFact
 */
// ======================================================================
/*!
 * \class TextGen::WeatherFact
 *
 * \brief Representation of a single fact about weather
 *
 * The purpose of a single WeatherFact object is to specify
 * a particular analysis type on the weather, and to provide
 * the result of the analysis along with its accuracy estimate.
 *
 */
// ======================================================================

#include "WeatherFact.h"

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Destructor
 */
// ----------------------------------------------------------------------

WeatherFact::~WeatherFact() {}
// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 *
 * \param theFact The object to copy
 */
// ----------------------------------------------------------------------

WeatherFact::WeatherFact(const WeatherFact& theFact)
    : itsParameter(theFact.itsParameter),
      itsFunction(theFact.itsFunction),
      itsPeriod(theFact.itsPeriod),
      itsArea(theFact.itsArea),
      itsResult(theFact.itsResult)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theParameter The weather analysis parameter
 * \param theFunction The weather analysis function
 * \param thePeriod The weather analysis period
 * \param theArea The weather analysis area
 * \param theResult The weather analysis result
 */
// ----------------------------------------------------------------------

WeatherFact::WeatherFact(WeatherParameter theParameter,
                         WeatherFunction theFunction,
                         const WeatherPeriod& thePeriod,
                         const WeatherArea& theArea,
                         const WeatherResult& theResult)
    : itsParameter(theParameter),
      itsFunction(theFunction),
      itsPeriod(thePeriod),
      itsArea(theArea),
      itsResult(theResult)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 *
 * \param theFact The object to copy
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

WeatherFact& WeatherFact::operator=(const WeatherFact& theFact)
{
  if (this != &theFact)
  {
    itsParameter = theFact.itsParameter;
    itsFunction = theFact.itsFunction;
    itsPeriod = theFact.itsPeriod;
    itsArea = theFact.itsArea;
    itsResult = theFact.itsResult;
  }
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \brief Parameter accessor
 *
 * \return The weather analysis parameter
 */
// ----------------------------------------------------------------------

WeatherParameter WeatherFact::parameter() const { return itsParameter; }
// ----------------------------------------------------------------------
/*!
 * \brief Function accessor
 *
 * \return The weather analysis function
 */
// ----------------------------------------------------------------------

WeatherFunction WeatherFact::function() const { return itsFunction; }
// ----------------------------------------------------------------------
/*!
 * \brief Period accessor
 *
 * \return The weather analysis period
 */
// ----------------------------------------------------------------------

const WeatherPeriod& WeatherFact::period() const { return itsPeriod; }
// ----------------------------------------------------------------------
/*!
 * \brief Area accessor
 *
 * \return The weather analysis area
 */
// ----------------------------------------------------------------------

const WeatherArea& WeatherFact::area() const { return itsArea; }
// ----------------------------------------------------------------------
/*!
 * \brief Result accessor
 *
 * \return The weather analysis result
 */
// ----------------------------------------------------------------------

const WeatherResult& WeatherFact::result() const { return itsResult; }
}  // namespace TextGen

// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison for WeatherFact
 *
 * \param theLhs The first fact
 * \param theRhs The second fact
 * \return True if the facts are equal
 */
// ----------------------------------------------------------------------

bool operator==(const TextGen::WeatherFact& theLhs, const TextGen::WeatherFact& theRhs)
{
  return (theLhs.parameter() == theRhs.parameter() && theLhs.function() == theRhs.function() &&
          theLhs.period() == theRhs.period() && theLhs.area() == theRhs.area() &&
          theLhs.result() == theRhs.result());
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison for WeatherFact
 *
 * \param theLhs The first fact
 * \param theRhs The second fact
 * \return True if the facts are not equal
 */
// ----------------------------------------------------------------------

bool operator!=(const TextGen::WeatherFact& theLhs, const TextGen::WeatherFact& theRhs)
{
  return !(theLhs == theRhs);
}

// ======================================================================
