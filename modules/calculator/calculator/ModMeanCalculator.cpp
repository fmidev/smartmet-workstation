// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::ModMeanCalculator
 */
// ======================================================================
/*!
 * \class TextGen::ModMeanCalculator
 *
 * \brief Modular mean intergator
 *
 * The modular mean is calculated using the Mitsuta algorithm
 * for wind direction means.
 *
 * Reference: Mori, Y., 1986.<br>
 * <em>Evaluation of Several Single-Pass Estimators of the Mean and
 *     the Standard Deviation of Wind Direction.</em><br>
 * J Climate Appl. Metro., 25, 1387-1397.
 *
 * Some information can also be found with Google (Mitsuta wind direction).
 */
// ======================================================================

#include "ModMeanCalculator.h"
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

ModMeanCalculator::ModMeanCalculator(int theModulo)
    : itsAcceptor(new DefaultAcceptor()),
      itsModulo(theModulo),
      itsCounter(0),
      itsSum(0),
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

void ModMeanCalculator::operator()(float theValue)
{
  if (itsAcceptor->accept(theValue))
  {
    ++itsCounter;
    if (itsCounter == 1)
    {
      itsSum = theValue;
      itsPreviousDirection = theValue;
    }
    else
    {
      const double diff = theValue - itsPreviousDirection;
      double dir = itsPreviousDirection + diff;
      if (diff < -itsModulo / 2)
        dir += itsModulo;
      else if (diff > itsModulo / 2)
        dir -= itsModulo;
      itsSum += dir;

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

float ModMeanCalculator::operator()() const
{
  if (itsCounter == 0) return kFloatMissing;

  // Floats do not support %, it is an integer operator
  double mean = itsSum / itsCounter;
  mean -= itsModulo * floor(mean / itsModulo);
  return mean;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the internal acceptor
 *
 * \param theAcceptor The acceptor to be used
 */
// ----------------------------------------------------------------------

void ModMeanCalculator::acceptor(const Acceptor& theAcceptor)
{
  itsAcceptor = boost::shared_ptr<Acceptor>(theAcceptor.clone());
}

// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Calculator> ModMeanCalculator::clone() const
{
  return boost::shared_ptr<Calculator>(new ModMeanCalculator(*this));
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset
 */
// ----------------------------------------------------------------------

void ModMeanCalculator::reset()
{
  itsCounter = 0;
  itsSum = 0;
  itsPreviousDirection = kFloatMissing;
}

}  // namespace TextGen

// ======================================================================
