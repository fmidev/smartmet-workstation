// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::MeanCalculator
 */
// ======================================================================
/*!
 * \class TextGen::MeanCalculator
 *
 * \brief Mean intergator
 *
 */
// ======================================================================

#include "MeanCalculator.h"
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

MeanCalculator::MeanCalculator() : itsAcceptor(new DefaultAcceptor()), itsCounter(0), itsSum(0) {}
// ----------------------------------------------------------------------
/*!
 * \brief Integrate a new value
 *
 * \param theValue
 */
// ----------------------------------------------------------------------

void MeanCalculator::operator()(float theValue)
{
  if (itsAcceptor->accept(theValue))
  {
    ++itsCounter;
    itsSum += theValue;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the integrated value
 *
 * \return The integrated mean value
 */
// ----------------------------------------------------------------------

float MeanCalculator::operator()() const
{
  if (itsCounter == 0)
    return kFloatMissing;
  else
    return itsSum / static_cast<double>(itsCounter);
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the internal acceptor
 *
 * \param theAcceptor The acceptor to be used
 */
// ----------------------------------------------------------------------

void MeanCalculator::acceptor(const Acceptor& theAcceptor)
{
  itsAcceptor = shared_ptr<Acceptor>(theAcceptor.clone());
}

// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Calculator> MeanCalculator::clone() const
{
  return boost::shared_ptr<Calculator>(new MeanCalculator(*this));
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset
 */
// ----------------------------------------------------------------------

void MeanCalculator::reset()
{
  itsCounter = 0;
  itsSum = 0;
}

}  // namespace TextGen

// ======================================================================
