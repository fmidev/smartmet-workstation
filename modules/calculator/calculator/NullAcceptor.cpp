// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::NullAcceptor
 */
// ======================================================================
/*!
 * \class TextGen::NullAcceptor
 *
 * \brief Acceptor is a class which accepts everything
 *
 */
// ======================================================================

#include "NullAcceptor.h"
#include <newbase/NFmiGlobals.h>

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

Acceptor* NullAcceptor::clone() const { return new NullAcceptor(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Accept or reject a value
 *
 * NullAcceptor accepts all values
 *
 * \param theValue The value to be accepted
 * \return True if the value is accepted
 */
// ----------------------------------------------------------------------

bool NullAcceptor::accept(float theValue) const { return true; }
}  // namespace TextGen

// ======================================================================
