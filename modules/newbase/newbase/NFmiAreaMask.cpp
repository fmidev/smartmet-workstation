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

// ******** ShallowLocationCopyEnabler ********************************************

ShallowLocationCopyEnabler::ShallowLocationCopyEnabler(
    const boost::shared_ptr<NFmiFastQueryInfo> &theFastInfoPtr)
    : fastInfoPtr(theFastInfoPtr)
{
  possibleLocationBag = fastInfoPtr->HPlaceDescriptor().LocationBag();
  if (possibleLocationBag)
  {
    possibleLocationBag->AllowLocationCopyOptimization(true);
  }
}

ShallowLocationCopyEnabler::~ShallowLocationCopyEnabler()
{
  if (possibleLocationBag)
  {
    possibleLocationBag->AllowLocationCopyOptimization(false);
  }
}

// ******** NFmiAreaMask *******************************************************

//! Destructor
NFmiAreaMask::~NFmiAreaMask() = default;

boost::shared_ptr<NFmiFastQueryInfo> NFmiAreaMask::DoShallowCopy(
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  if (theInfo)
  {
    ShallowLocationCopyEnabler enabler(theInfo);
    return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*theInfo));
  }
  else
    return theInfo;
}

std::vector<boost::shared_ptr<NFmiFastQueryInfo>> NFmiAreaMask::DoShallowCopy(
    const std::vector<boost::shared_ptr<NFmiFastQueryInfo>> &infoVector)
{
  // tehd‰‰n matala kopio info-vektorista
  std::vector<boost::shared_ptr<NFmiFastQueryInfo>> shallowCopyVector;
  for (const auto &info : infoVector)
  {
    ShallowLocationCopyEnabler enabler(info);
    shallowCopyVector.push_back(boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*info)));
  }
  return shallowCopyVector;
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

std::vector<boost::shared_ptr<NFmiAreaMask>> NFmiAreaMask::DoShallowCopy(
    const std::vector<boost::shared_ptr<NFmiAreaMask>> &theMaskVector)
{
  std::vector<boost::shared_ptr<NFmiAreaMask>> returnVector(theMaskVector.size());
  for (size_t i = 0; i < theMaskVector.size(); i++)
    returnVector[i] = NFmiAreaMask::DoShallowCopy(theMaskVector[i]);
  return returnVector;
}

// ======================================================================
