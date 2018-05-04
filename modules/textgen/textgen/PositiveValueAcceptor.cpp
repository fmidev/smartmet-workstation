// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::PositiveValueAcceptor
 */
// ======================================================================
/*!
 * \class TextGen::PositiveValueAcceptor
 *
 * \brief PositiveValueAcceptor accepts only positive values
 *
 */
// ======================================================================

#include "PositiveValueAcceptor.h"
#include <newbase/NFmiGlobals.h>

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

Acceptor* PositiveValueAcceptor::clone() const { return new PositiveValueAcceptor(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Accept or reject a value
 *
 * Only positive value is accepted, , but value kFloatMissing is never accepted.

 * \param theValue The value to be accepted
 * \return True if the value is accepted
 */
// ----------------------------------------------------------------------

bool PositiveValueAcceptor::accept(float theValue) const
{
  return (theValue != kFloatMissing && theValue > 0.0);
}

}  // namespace TextGen

// ======================================================================
