// ======================================================================
/*!
 * \file NFmiStationBag.cpp
 * \brief Implementation of class NFmiStationBag
 */
// ======================================================================
/*!
 * \class NFmiStationBag
 *
 * Do not use this for anything, use NFmiLocation instead!
 *
 */
// ======================================================================

#include "NFmiStationBag.h"
#include "NFmiValueString.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiStationBag::NFmiStationBag() = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theStationArray Undocumented
 * \param numberOfStations Undocumented
 */
// ----------------------------------------------------------------------

NFmiStationBag::NFmiStationBag(unsigned long *theStationArray, unsigned long numberOfStations)
    : NFmiLocationBag()
{
  itsSize = numberOfStations;
  itsLocations.reserve(numberOfStations);
  for (unsigned long i = 0; i < numberOfStations; i++)
  {
    NFmiStation tmpStation(theStationArray[i]);
    AddLocation(tmpStation, false);
  }
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theBag The other object being copied
 */
// ----------------------------------------------------------------------

NFmiStationBag::NFmiStationBag(const NFmiStationBag &theBag) = default;

// ----------------------------------------------------------------------
/*!
 * \param theStation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStationBag::SetCurrent(long theStation)
{
  Reset();
  while (Next())
  {
    if (theStation == CurrentStation()) return true;
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theStation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStationBag::AddStation(const NFmiStation &theStation)
{
  return (NFmiLocationBag::AddLocation(theStation));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiLocationBag *NFmiStationBag::Clone() const { return new NFmiStationBag(*this); }

// ======================================================================
