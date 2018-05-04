// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::RangeAcceptor
 */
// ======================================================================
/*!
 * \class TextGen::RangeAcceptor
 *
 * \brief RangeAcceptor accepths a range of values
 *
 */
// ======================================================================

#include "RangeAcceptor.h"
#include <newbase/NFmiGlobals.h>

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

RangeAcceptor::RangeAcceptor() : itsLoLimit(kFloatMissing), itsHiLimit(kFloatMissing) {}
// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

Acceptor* RangeAcceptor::clone() const { return new RangeAcceptor(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Accept or reject a value
 *
 * The value kFloatMissing is never accepted.

 * \param theValue The value to be accepted
 * \return True if the value is accepted
 */
// ----------------------------------------------------------------------

bool RangeAcceptor::accept(float theValue) const
{
  if (theValue == kFloatMissing) return false;
  if (itsLoLimit != kFloatMissing && theValue < itsLoLimit) return false;
  if (itsHiLimit != kFloatMissing && theValue > itsHiLimit) return false;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set a lower limit for acceptable values
 *
 * \param theLimit The limiting value
 */
// ----------------------------------------------------------------------

void RangeAcceptor::lowerLimit(float theLimit) { itsLoLimit = theLimit; }
// ----------------------------------------------------------------------
/*!
 * \brief Set an upper limit for acceptable values
 *
 * \param theLimit The limiting value
 */
// ----------------------------------------------------------------------

void RangeAcceptor::upperLimit(float theLimit) { itsHiLimit = theLimit; }
}  // namespace TextGen

// ======================================================================
