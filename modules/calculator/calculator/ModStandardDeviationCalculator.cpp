// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::ModStandardDeviationCalculator
 */
// ======================================================================
/*!
 * \class TextGen::ModStandardDeviationCalculator
 *
 * \brief Modular standard deviation intergator
 *
 * The modular standard deviation is calculated using the Mitsuta algorithm
 * for wind direction standard deviations.
 *
 * Reference: Mori, Y., 1986.<br>
 * <em>Evaluation of Several Single-Pass Estimators of the StandardDeviation and
 *     the Standard Deviation of Wind Direction.</em><br>
 * J Climate Appl. Metro., 25, 1387-1397.
 *
 * Some information can also be found with Google (Mitsuta wind direction).
 */
// ======================================================================

#include "ModStandardDeviationCalculator.h"
#include "DefaultAcceptor.h"

#include <newbase/NFmiGlobals.h>

#include <cmath>

using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

ModStandardDeviationCalculator::ModStandardDeviationCalculator(int theModulo)
    : itsAcceptor(new DefaultAcceptor()),
      itsModulo(theModulo),
      itsCounter(0),
      itsSum(0),
      itsSquaredSum(0),
      itsPreviousDirection(kFloatMissing)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate a new value
 *
 * Uses the Mitsuta algorithm.
 *
 * \param theValue
 */
// ----------------------------------------------------------------------

void ModStandardDeviationCalculator::operator()(float theValue)
{
  if (itsAcceptor->accept(theValue))
  {
    ++itsCounter;
    if (itsCounter == 1)
    {
      itsSum = theValue;
      itsSquaredSum = theValue * theValue;
      itsPreviousDirection = theValue;
    }
    else
    {
      const double diff = theValue - itsPreviousDirection;
      double dir = itsPreviousDirection + diff;
      if (diff < -itsModulo / 2)
      {
        while (dir < itsModulo / 2)
          dir += itsModulo;
      }
      else if (diff > itsModulo / 2)
      {
        while (dir > itsModulo / 2)
          dir -= itsModulo;
      }
      itsSum += dir;
      itsSquaredSum += dir * dir;

      itsPreviousDirection = dir;
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the integrated value
 *
 * \return The integrated mean value
 */
// ----------------------------------------------------------------------

float ModStandardDeviationCalculator::operator()() const
{
  if (itsCounter < 1)
    return kFloatMissing;
  else
  {
    const double tmp = itsSquaredSum - itsSum * itsSum / itsCounter;
    if (tmp < 0)
      return 0.0;
    else
      return sqrt(tmp / itsCounter);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the internal acceptor
 *
 * \param theAcceptor The acceptor to be used
 */
// ----------------------------------------------------------------------

void ModStandardDeviationCalculator::acceptor(const Acceptor& theAcceptor)
{
  itsAcceptor = boost::shared_ptr<Acceptor>(theAcceptor.clone());
}

// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Calculator> ModStandardDeviationCalculator::clone() const
{
  return boost::shared_ptr<Calculator>(new ModStandardDeviationCalculator(*this));
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset
 */
// ----------------------------------------------------------------------

void ModStandardDeviationCalculator::reset()
{
  itsCounter = 0;
  itsSum = 0;
  itsSquaredSum = 0;
  itsPreviousDirection = kFloatMissing;
}

}  // namespace TextGen

// ======================================================================
