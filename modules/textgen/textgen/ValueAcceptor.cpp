// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::ValueAcceptor
 */
// ======================================================================
/*!
 * \class TextGen::ValueAcceptor
 *
 * \brief ValueAcceptor accepts only a single value
 *
 * This is typically used to calculate the percentage of some
 * enumerated type out of all types.
 */
// ======================================================================

#include "ValueAcceptor.h"
#include <newbase/NFmiGlobals.h>

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

ValueAcceptor::ValueAcceptor() : itsValue(kFloatMissing) {}
// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

Acceptor* ValueAcceptor::clone() const { return new ValueAcceptor(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Accept or reject a value
 *
 * The value kFloatMissing is never accepted.

 * \param theValue The value to be accepted
 * \return True if the value is accepted
 */
// ----------------------------------------------------------------------

bool ValueAcceptor::accept(float theValue) const
{
  if (theValue == kFloatMissing) return false;
  return (theValue == itsValue);
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the accepted value
 *
 * \param theValue The value
 */
// ----------------------------------------------------------------------

void ValueAcceptor::value(float theValue) { itsValue = theValue; }
}  // namespace TextGen

// ======================================================================
