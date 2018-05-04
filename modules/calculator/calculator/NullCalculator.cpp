// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::NullCalculator
 */
// ======================================================================
/*!
 * \class TextGen::NullCalculator
 *
 * \brief Dummy calculator
 *
 */
// ======================================================================

#include "NullCalculator.h"
#include "DefaultAcceptor.h"
#include "TextGenError.h"

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Integrate a new value
 *
 * \param theValue
 */
// ----------------------------------------------------------------------

void NullCalculator::operator()(float theValue) {}
// ----------------------------------------------------------------------
/*!
 * \brief Return the integrated value
 *
 * \return The integrated null value
 */
// ----------------------------------------------------------------------

float NullCalculator::operator()() const
{
  throw TextGenError("NullCalculator should never be asked for a result");
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the internal acceptor
 *
 * \param theAcceptor The acceptor to be used
 */
// ----------------------------------------------------------------------

void NullCalculator::acceptor(const Acceptor& theAcceptor) {}
// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Calculator> NullCalculator::clone() const
{
  return boost::shared_ptr<Calculator>(new NullCalculator(*this));
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset
 */
// ----------------------------------------------------------------------

void NullCalculator::reset() {}
}  // namespace TextGen

// ======================================================================
