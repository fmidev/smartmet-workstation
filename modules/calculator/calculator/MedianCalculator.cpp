// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::MedianCalculator
 */
// ======================================================================
/*!
 * \class TextGen::MedianCalculator
 *
 * \brief Median intergator
 *
 */
// ======================================================================

#include "MedianCalculator.h"
#include "DefaultAcceptor.h"

#include <newbase/NFmiGlobals.h>

#include <algorithm>

using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

MedianCalculator::MedianCalculator() : itsAcceptor(new DefaultAcceptor()) {}
// ----------------------------------------------------------------------
/*!
 * \brief Integrate a new value
 *
 * \param theValue
 */
// ----------------------------------------------------------------------

void MedianCalculator::operator()(float theValue)
{
  if (itsAcceptor->accept(theValue))
  {
    theValueVector.push_back(theValue);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the integrated value
 *
 * \return The integrated mean value
 */
// ----------------------------------------------------------------------

float MedianCalculator::operator()() const
{
  if (theValueVector.empty())
  {
    return kFloatMissing;
  }
  else
  {
    sort(theValueVector.begin(), theValueVector.end());
    unsigned int vectorSize(theValueVector.size());
    const unsigned int medianIndex = (vectorSize < 2 ? 0 : ((vectorSize / 2) - 1));

    return theValueVector.at(medianIndex);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the internal acceptor
 *
 * \param theAcceptor The acceptor to be used
 */
// ----------------------------------------------------------------------

void MedianCalculator::acceptor(const Acceptor& theAcceptor)
{
  itsAcceptor = boost::shared_ptr<Acceptor>(theAcceptor.clone());
}

// ----------------------------------------------------------------------
/*!
 * \brief Clone
 */
// ----------------------------------------------------------------------

boost::shared_ptr<Calculator> MedianCalculator::clone() const
{
  return boost::shared_ptr<Calculator>(new MedianCalculator(*this));
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset
 */
// ----------------------------------------------------------------------

void MedianCalculator::reset() { theValueVector.clear(); }
}  // namespace TextGen

// ======================================================================
