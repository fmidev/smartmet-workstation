// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::TrendCalculator
 */
// ======================================================================
/*!
 * \class TextGen::TrendCalculator
 *
 * \brief Trend intergator
 *
 */
// ======================================================================

#include "TrendCalculator.h"
#include "DefaultAcceptor.h"

#include <newbase/NFmiGlobals.h>

using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

TrendCalculator::TrendCalculator()
    : itsAcceptor(new DefaultAcceptor()),
      itsCounter(0),
      itsPositiveChanges(0),
      itsNegativeChanges(0),
      itsZeroChanges(0),
      itsLastValue(kFloatMissing)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Integrate a new value
 *
 * \param theValue
 */
// ----------------------------------------------------------------------

void TrendCalculator::operator()(float theValue)
{
  if (itsAcceptor->accept(theValue))
  {
    if (itsCounter > 0)
    {
      if (theValue > itsLastValue)
        ++itsPositiveChanges;
      else if (theValue < itsLastValue)
        ++itsNegativeChanges;
      else
        ++itsZeroChanges;
    }
    ++itsCounter;
    itsLastValue = theValue;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the integrated value
 *
 * \return The integrated trend value
 */
// ----------------------------------------------------------------------

float TrendCalculator::operator()() const
{
  // The total number of numbers is one greater than number of changes,
  // hence the -1 in the divisor itsCounter-1

  if (itsCounter < 1)
    return kFloatMissing;
  else if (itsCounter == 1)
    return 0.0;
  else
    return (itsPositiveChanges - itsNegativeChanges) / (itsCounter - 1.0) * 100;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the internal acceptor
 *
 * \param theAcceptor The acceptor to be used
 */
// ----------------------------------------------------------------------

void TrendCalculator::acceptor(const Acceptor& theAcceptor)
{
  itsAcceptor = shared_ptr<Acceptor>(theAcceptor.clone());
}

// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Calculator> TrendCalculator::clone() const
{
  return boost::shared_ptr<Calculator>(new TrendCalculator(*this));
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset
 */
// ----------------------------------------------------------------------

void TrendCalculator::reset()
{
  itsCounter = 0;
  itsPositiveChanges = 0;
  itsNegativeChanges = 0;
  itsZeroChanges = 0;
  itsLastValue = kFloatMissing;
}

}  // namespace TextGen

// ======================================================================
