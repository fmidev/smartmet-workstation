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

NFmiStationBag::NFmiStationBag() : NFmiLocationBag() {}
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
    itsLocations.push_back(NFmiStation(theStationArray[i]).Clone());
}

// ----------------------------------------------------------------------
/*!
 * \param theStationArray Undocumented
 * \param numberOfStations Undocumented
 */
// ----------------------------------------------------------------------

NFmiStationBag::NFmiStationBag(NFmiStation *theStationArray, unsigned long numberOfStations)
    : NFmiLocationBag(theStationArray, numberOfStations)
{
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
 *
 */
// ----------------------------------------------------------------------

void NFmiStationBag::Destroy()
{
  for (unsigned long i = 0; i < itsLocations.size(); i++)
    delete itsLocations[i];
  itsLocations.clear();
  itsSize = 0;
}

// ----------------------------------------------------------------------
/*!
 * \param theBag Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiLocationBag NFmiStationBag::Combine(const NFmiLocationBag &theBag)
{
  NFmiLocationBag outbag(theBag);

  StorageType::const_iterator begin = itsLocations.begin();
  StorageType::const_iterator end = itsLocations.end();
  StorageType::const_iterator iter;
  for (iter = begin; iter != end; ++iter)
    outbag.AddLocation(*(*iter), true);

  return outbag;
}

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
 * \param theLocation Undocumented
 * \param theChecking Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStationBag::AddLocation(const NFmiLocation &theLocation, bool theChecking)
{
  return (NFmiLocationBag::AddLocation(theLocation, theChecking));
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
// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiStationBag::Write(std::ostream &file) const
{
  NFmiLocationBag::Write(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiStationBag::Read(std::istream &file)
{
  NFmiLocationBag::Read(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theStationBag The other object being copied
 */
// ----------------------------------------------------------------------

NFmiStationBag &NFmiStationBag::operator=(const NFmiStationBag &theStationBag)
{
  NFmiLocationBag::operator=(*static_cast<const NFmiLocationBag *>(&theStationBag));
  return *this;
}

// ======================================================================
