// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::ModChangeCalculator
 */
// ======================================================================
/*!
 * \class TextGen::ModChangeCalculator
 *
 * \brief Modular change intergator
 *
 */
// ======================================================================

#include "ModChangeCalculator.h"
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

ModChangeCalculator::ModChangeCalculator(int theModulo)
    : itsAcceptor(new DefaultAcceptor()),
      itsModulo(theModulo),
      itsCounter(0),
      itsCumulativeChange(0),
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

void ModChangeCalculator::operator()(float theValue)
{
  if (itsAcceptor->accept(theValue))
  {
    if (itsCounter > 0)
    {
      const double diff = theValue - itsLastValue;
      if (diff < -itsModulo / 2)
        itsCumulativeChange += diff + itsModulo;
      else if (diff > itsModulo / 2)
        itsCumulativeChange += diff - itsModulo;
      else
        itsCumulativeChange += diff;
    }
    ++itsCounter;
    itsLastValue = theValue;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the integrated value
 *
 * \return The integrated change value
 */
// ----------------------------------------------------------------------

float ModChangeCalculator::operator()() const
{
  if (itsCounter == 0)
    return kFloatMissing;
  else
    return itsCumulativeChange;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the internal acceptor
 *
 * \param theAcceptor The acceptor to be used
 */
// ----------------------------------------------------------------------

void ModChangeCalculator::acceptor(const Acceptor& theAcceptor)
{
  itsAcceptor = shared_ptr<Acceptor>(theAcceptor.clone());
}

// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Calculator> ModChangeCalculator::clone() const
{
  return boost::shared_ptr<Calculator>(new ModChangeCalculator(*this));
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset
 */
// ----------------------------------------------------------------------

void ModChangeCalculator::reset()
{
  itsCounter = 0;
  itsCumulativeChange = 0;
  itsLastValue = kFloatMissing;
}

}  // namespace TextGen

// ======================================================================
