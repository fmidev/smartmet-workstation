// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::StandardDeviationCalculator
 */
// ======================================================================
/*!
 * \class TextGen::StandardDeviationCalculator
 *
 * \brief StandardDeviation intergator
 *
 * The result is sample standard deviation, not plain population
 * standard deviation because we most often calculate the standard
 * deviation for data which is essentially discrete samples
 * of continuous data.
 */
// ======================================================================

#include "StandardDeviationCalculator.h"
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

StandardDeviationCalculator::StandardDeviationCalculator()
    : itsAcceptor(new DefaultAcceptor()), itsCounter(0), itsSum(0), itsSquaredSum(0)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate a new value
 *
 * \param theValue
 */
// ----------------------------------------------------------------------

void StandardDeviationCalculator::operator()(float theValue)
{
  if (itsAcceptor->accept(theValue))
  {
    ++itsCounter;
    itsSum += theValue;
    itsSquaredSum += theValue * theValue;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the integrated value
 *
 * \return The integrated sum value
 */
// ----------------------------------------------------------------------

float StandardDeviationCalculator::operator()() const
{
  if (itsCounter < 2)
    return kFloatMissing;
  else
  {
    const double tmp = itsSquaredSum - itsSum * itsSum / itsCounter;
    if (tmp < 0)
      return 0.0;
    else
      return sqrt(tmp / (itsCounter - 1));
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the internal acceptor
 *
 * \param theAcceptor The acceptor to be used
 */
// ----------------------------------------------------------------------

void StandardDeviationCalculator::acceptor(const Acceptor& theAcceptor)
{
  itsAcceptor = boost::shared_ptr<Acceptor>(theAcceptor.clone());
}

// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Calculator> StandardDeviationCalculator::clone() const
{
  return boost::shared_ptr<Calculator>(new StandardDeviationCalculator(*this));
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset
 */
// ----------------------------------------------------------------------

void StandardDeviationCalculator::reset()
{
  itsCounter = 0;
  itsSum = 0;
  itsSquaredSum = 0;
}

}  // namespace TextGen

// ======================================================================
