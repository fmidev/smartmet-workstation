// ======================================================================
/*!
 * \file NFmiAreaMask.cpp
 * \brief Implementation of class NFmiAreaMask
 */
// ======================================================================
/*!
 * \class NFmiAreaMask
 *
 * A basic area mask class. Also contains a list for possible submasks,
 * which can be indexed.
 *
 */
// ======================================================================

#include "NFmiAreaMask.h"
#include "NFmiFastQueryInfo.h"

#include <cmath>

//! Destructor
NFmiAreaMask::~NFmiAreaMask() = default;
boost::shared_ptr<NFmiFastQueryInfo> NFmiAreaMask::DoShallowCopy(
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  if (theInfo)
  {
    return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*theInfo));
  }
  else
    return theInfo;
}

boost::shared_ptr<NFmiAreaMask> NFmiAreaMask::DoShallowCopy(
    const boost::shared_ptr<NFmiAreaMask> &theMask)
{
  if (theMask)
  {
    return boost::shared_ptr<NFmiAreaMask>(theMask->Clone());
  }
  else
    return theMask;
}

checkedVector<boost::shared_ptr<NFmiAreaMask> > NFmiAreaMask::DoShallowCopy(
    const checkedVector<boost::shared_ptr<NFmiAreaMask> > &theMaskVector)
{
  checkedVector<boost::shared_ptr<NFmiAreaMask> > returnVector(theMaskVector.size());
  for (size_t i = 0; i < theMaskVector.size(); i++)
    returnVector[i] = NFmiAreaMask::DoShallowCopy(theMaskVector[i]);
  return returnVector;
}

// ======================================================================
