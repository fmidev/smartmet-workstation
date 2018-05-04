// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::AndAcceptor
 */
// ======================================================================
/*!
 * \class TextGen::AndAcceptor
 *
 * \brief AndAcceptor combines two Acceptors with a logical AND
 *
 */
// ======================================================================

#include "AndAcceptor.h"
#include <newbase/NFmiGlobals.h>

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Copy Constructor
 *
 * \param theOther The acceptor
 */
// ----------------------------------------------------------------------

AndAcceptor::AndAcceptor(const AndAcceptor& theOther)
    : itsLhs(theOther.itsLhs->clone()), itsRhs(theOther.itsRhs->clone())
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theLhs The first acceptor
 * \param theRhs The second acceptor
 */
// ----------------------------------------------------------------------

AndAcceptor::AndAcceptor(const Acceptor& theLhs, const Acceptor& theRhs)
    : itsLhs(theLhs.clone()), itsRhs(theRhs.clone())
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

Acceptor* AndAcceptor::clone() const { return new AndAcceptor(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Accept or reject a value
 *
 * The value kFloatMissing is never accepted.

 * \param theValue The value to be accepted
 * \return True if the value is accepted
 */
// ----------------------------------------------------------------------

bool AndAcceptor::accept(float theValue) const
{
  return itsLhs->accept(theValue) && itsRhs->accept(theValue);
}

}  // namespace TextGen

// ======================================================================
