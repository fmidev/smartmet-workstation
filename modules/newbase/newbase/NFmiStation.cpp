// ======================================================================
/*!
 * \file NFmiStation.cpp
 * \brief Implementation of class NFmiStation
 */
// ======================================================================
/*!
 * \class NFmiStation
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiStation.h"
#include <fstream>

#include "NFmiVersion.h"

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

NFmiStation::~NFmiStation() = default;
// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------
NFmiStation::NFmiStation() : NFmiLocation(), itsMaxDistance(kFloatMissing), itsIdentType(kWMO) {}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theIdent Undocumented
 * \param theName Undocumented
 * \param theLongitude Undocumented
 * \param theLatitude Undocumented
 * \param theMaxDistance Undocumented
 * \param theIdentType Undocumented
 */
// ----------------------------------------------------------------------

NFmiStation::NFmiStation(long theIdent,
                         const NFmiString &theName,
                         double theLongitude,
                         double theLatitude,
                         double theMaxDistance,
                         FmiStationType theIdentType)
    : NFmiLocation(theIdent, theName, theLongitude, theLatitude),
      itsMaxDistance(theMaxDistance),
      itsIdentType(theIdentType)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theIdent Undocumented
 * \param theLocation Undocumented
 * \param theMaxDistance Undocumented
 * \param theIdentType Undocumented
 */
// ----------------------------------------------------------------------

NFmiStation::NFmiStation(NFmiIndividual theIdent,
                         NFmiLocation theLocation,
                         double theMaxDistance,
                         FmiStationType theIdentType)
    : NFmiLocation(theIdent.GetIdent(),
                   theIdent.GetName(),
                   theLocation.GetLongitude(),
                   theLocation.GetLatitude()),
      itsMaxDistance(theMaxDistance),
      itsIdentType(theIdentType)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theStation The other object being copied
 */
// ----------------------------------------------------------------------

NFmiStation::NFmiStation(const NFmiStation &theStation)
    : NFmiLocation(theStation.GetIdent(),
                   theStation.GetName(),
                   theStation.GetLongitude(),
                   theStation.GetLatitude()),
      itsMaxDistance(theStation.MaxDistance()),
      itsIdentType(theStation.IdentType())
{
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiStation::IsEqual(const NFmiSortable &theLocation) const
{
  return ((GetIdent() ==
           (static_cast<const NFmiStation *>(static_cast<const NFmiLocation *>(&theLocation))
                ->GetIdent())) &&
          (GetName() ==
           (static_cast<const NFmiStation *>(static_cast<const NFmiLocation *>(&theLocation))
                ->GetName())) &&
          (IdentType() ==
           (static_cast<const NFmiStation *>(static_cast<const NFmiLocation *>(&theLocation))
                ->IdentType())));
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Always false
 * \bug This is not implemented properly
 */
// ----------------------------------------------------------------------

bool NFmiStation::IsLessThan(const NFmiSortable & /* theLocation */) const { return false; }
// ----------------------------------------------------------------------
/*!
 * \param theStation Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------
bool NFmiStation::IsEqualName(const NFmiStation &theStation) const
{
  return GetName().IsEqual(theStation.GetName());
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theStation The other object being copied
 * \return The object assigned to
 */
// ----------------------------------------------------------------------

NFmiStation &NFmiStation::operator=(const NFmiStation &theStation)
{
  SetLocation(theStation);
  itsMaxDistance = theStation.itsMaxDistance;
  itsIdentType = theStation.itsIdentType;
  return *this;
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStation::SetLocation(const NFmiLocation &theLocation)
{
  NFmiIndividual::operator=(
      *(static_cast<const NFmiIndividual *>(static_cast<const NFmiStation *>(&theLocation))));

  NFmiLocation::SetLocation(theLocation);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiLocation *NFmiStation::Clone() const { return new NFmiStation(*this); }
// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiStation::Write(std::ostream &file) const
{
  NFmiIndividual::Write(file);
  NFmiLocation::Write(file);

  if (FmiInfoVersion >= 4)
  {
    file << itsMaxDistance << " ";
    file << static_cast<unsigned int>(itsIdentType) << std::endl;
  }

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

std::istream &NFmiStation::Read(std::istream &file)
{
  NFmiIndividual::Read(file);
  NFmiLocation::Read(file);

  itsMaxDistance = kFloatMissing;

  if (FmiInfoVersion >= 4)
  {
    unsigned int theIdentType;
    file >> itsMaxDistance;
    file >> theIdentType;
    itsIdentType = FmiStationType(theIdentType);
  }

  return file;
}

// ======================================================================
