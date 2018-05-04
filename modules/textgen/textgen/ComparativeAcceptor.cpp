// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::ComparativeAcceptor
 */
// ======================================================================
/*!
 * \class TextGen::ComparativeAcceptor
 *
 * \brief ComparativeAcceptor accepths a range of values
 *
 */
// ======================================================================

#include "ComparativeAcceptor.h"

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

ComparativeAcceptor::ComparativeAcceptor(
    const float& theLimit /*= kFloatMissing*/,
    const comparative_operator& theOperator /* = VOID_OPERATOR */)
    : itsLimit(theLimit), itsOperator(theOperator)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

Acceptor* ComparativeAcceptor::clone() const { return new ComparativeAcceptor(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Accept or reject a value
 *
 * The value kFloatMissing is never accepted.

 * \param theValue The value to be accepted
 * \return True if the comparison returns true
 */
// ----------------------------------------------------------------------

bool ComparativeAcceptor::accept(float theValue) const
{
  if (itsLimit == kFloatMissing) return false;

  bool retval = false;

  switch (itsOperator)
  {
    case LESS_THAN:
      retval = (theValue < itsLimit);
      break;
    case LESS_OR_EQUAL:
      retval = (theValue <= itsLimit);
      break;
    case EQUAL:
      retval = (theValue == itsLimit);
      break;
    case GREATER_OR_EQUAL:
      retval = (theValue >= itsLimit);
      break;
    case GREATER_THAN:
      retval = (theValue > itsLimit);
      break;
    case VOID_OPERATOR:
      retval = true;
      break;
  }

  return retval;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the limit for acceptable values
 *
 * \param theLimit The limiting value
 */
// ----------------------------------------------------------------------

void ComparativeAcceptor::setLimit(float theLimit) { itsLimit = theLimit; }
// ----------------------------------------------------------------------
/*!
 * \brief Set the operator to be applied in comparison
 *
 * \param theOperator The operator
 */
// ----------------------------------------------------------------------

void ComparativeAcceptor::setOperator(const comparative_operator& theOperator)
{
  itsOperator = theOperator;
}
}  // namespace TextGen

// ======================================================================
