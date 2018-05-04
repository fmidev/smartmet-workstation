// ======================================================================
/*!
 * \file NFmiHPlaceDescriptor.cpp
 * \brief Implementation of class NFmiHPlaceDescriptor
 */
// ======================================================================
/*!
 * \class NFmiHPlaceDescriptor
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiHPlaceDescriptor.h"
#include "NFmiGrid.h"
#include "NFmiLocationBag.h"
#include "NFmiSaveBaseFactory.h"
#include "NFmiStation.h"
#include "NFmiValueString.h"

#include <boost/functional/hash.hpp>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiHPlaceDescriptor::NFmiHPlaceDescriptor()
    : itsLocationBag(nullptr),
      itsArea(nullptr),
      itsSelectedType(kAll),
      itsMaxNumberOfSources(0),
      itsGrid(nullptr),
      itsActivity(nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theLocationBag Undocumented
 * \param theSelectedType Undocumented
 * \param theMaxNumberOfSources Undocumented
 */
// ----------------------------------------------------------------------

NFmiHPlaceDescriptor::NFmiHPlaceDescriptor(const NFmiLocationBag &theLocationBag,
                                           FmiStationType theSelectedType,
                                           unsigned long theMaxNumberOfSources)
    : itsLocationBag(theLocationBag.Clone()),
      itsArea(nullptr),
      itsSelectedType(theSelectedType),
      itsMaxNumberOfSources(theMaxNumberOfSources),
      itsGrid(nullptr),
      itsActivity(new bool[theLocationBag.GetSize()])
{
  for (int i = 0; i < static_cast<int>(theLocationBag.GetSize()); i++)
    itsActivity[i] = true;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theArea Undocumented
 * \param theSelectedType Undocumented
 * \param theMaxNumberOfSources Undocumented
 */
// ----------------------------------------------------------------------

NFmiHPlaceDescriptor::NFmiHPlaceDescriptor(const NFmiArea &theArea,
                                           FmiStationType theSelectedType,
                                           unsigned long theMaxNumberOfSources)
    : itsLocationBag(nullptr),
      itsArea(theArea.Clone()),
      itsSelectedType(theSelectedType),
      itsMaxNumberOfSources(theMaxNumberOfSources),
      itsGrid(nullptr),
      itsActivity(nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theGrid Undocumented
 * \param theSelectedType Undocumented
 * \param theMaxNumberOfSources Undocumented
 */
// ----------------------------------------------------------------------

NFmiHPlaceDescriptor::NFmiHPlaceDescriptor(const NFmiGrid &theGrid,
                                           FmiStationType theSelectedType,
                                           unsigned long theMaxNumberOfSources)
    : itsLocationBag(nullptr),
      itsArea(nullptr),
      itsSelectedType(theSelectedType),
      itsMaxNumberOfSources(theMaxNumberOfSources),
      itsGrid(new NFmiGrid(theGrid)),
      itsActivity(nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theLocationBag Undocumented
 * \param theArea Undocumented
 * \param theSelectedType Undocumented
 * \param theMaxNumberOfSources Undocumented
 */
// ----------------------------------------------------------------------

NFmiHPlaceDescriptor::NFmiHPlaceDescriptor(const NFmiLocationBag &theLocationBag,
                                           const NFmiArea &theArea,
                                           FmiStationType theSelectedType,
                                           unsigned long theMaxNumberOfSources)
    : itsLocationBag(theLocationBag.Clone()),
      itsArea(theArea.Clone()),
      itsSelectedType(theSelectedType),
      itsMaxNumberOfSources(theMaxNumberOfSources),
      itsGrid(nullptr),
      itsActivity(new bool[theLocationBag.GetSize()])
{
  for (int i = 0; i < static_cast<int>(theLocationBag.GetSize()); i++)
    itsActivity[i] = true;
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theArea Undocumented
 * \param theGrid Undocumented
 * \param theSelectedType Undocumented
 * \param theMaxNumberOfSources Undocumented
 */
// ----------------------------------------------------------------------

NFmiHPlaceDescriptor::NFmiHPlaceDescriptor(const NFmiArea &theArea,
                                           const NFmiGrid &theGrid,
                                           FmiStationType theSelectedType,
                                           unsigned long theMaxNumberOfSources)
    : itsLocationBag(nullptr),
      itsArea(theArea.Clone()),
      itsSelectedType(theSelectedType),
      itsMaxNumberOfSources(theMaxNumberOfSources),
      itsGrid(new NFmiGrid(theGrid)),
      itsActivity(nullptr)
{
}

// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theHPlaceDescriptor The object being copied
 */
// ----------------------------------------------------------------------

NFmiHPlaceDescriptor::NFmiHPlaceDescriptor(const NFmiHPlaceDescriptor &theHPlaceDescriptor)
    : NFmiDataDescriptor(),
      itsLocationBag(theHPlaceDescriptor.itsLocationBag
                         ? theHPlaceDescriptor.itsLocationBag->Clone()
                         : nullptr),
      itsArea(theHPlaceDescriptor.itsArea ? theHPlaceDescriptor.itsArea->Clone() : nullptr),
      itsSelectedType(theHPlaceDescriptor.SelectedType()),
      itsMaxNumberOfSources(theHPlaceDescriptor.itsMaxNumberOfSources),
      itsGrid(theHPlaceDescriptor.itsGrid ? new NFmiGrid(*(theHPlaceDescriptor.itsGrid)) : nullptr),
      itsActivity(theHPlaceDescriptor.itsLocationBag
                      ? new bool[theHPlaceDescriptor.itsLocationBag->GetSize()]
                      : nullptr)
{
  if (itsActivity)
    for (int i = 0; i < static_cast<int>(itsLocationBag->GetSize()); i++)
      itsActivity[i] = true;
}

// ----------------------------------------------------------------------
/*!
 * \todo Iffit ovar turhia, nollapointterin voi deletoida turvallisesti
 */
// ----------------------------------------------------------------------

void NFmiHPlaceDescriptor::Destroy()
{
  if (itsLocationBag)
  {
    delete itsLocationBag;
    itsLocationBag = nullptr;
  }
  if (itsArea)
  {
    delete itsArea;
    itsArea = nullptr;
  }
  if (itsGrid)
  {
    delete itsGrid;
    itsGrid = nullptr;
  }
  if (itsActivity)
  {
    delete[] itsActivity;
    itsActivity = nullptr;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param theLocationBag Undocumented
 */
// ----------------------------------------------------------------------

void NFmiHPlaceDescriptor::LocationList(const NFmiLocationBag &theLocationBag)
{
  if (IsLocation()) itsLocationBag->Destroy();
  itsLocationBag = theLocationBag.Clone();

  itsActivity = new bool[Size()];
  for (int i = 0; i < static_cast<int>(Size()); i++)
    itsActivity[i] = true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiLocation *NFmiHPlaceDescriptor::Location() const
{
  if (itsLocationBag) return itsLocationBag->Location();
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiLocation *NFmiHPlaceDescriptor::LocationWithIndex(unsigned long theIndex) const
{
  if (itsLocationBag) return itsLocationBag->Location(theIndex);
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiHPlaceDescriptor::LatLon() const
{
  if (itsLocationBag)
  {
    const NFmiLocation *loc = Location();
    if (loc) return loc->GetLocation();
  }
  else if (itsGrid)
  {
    return itsGrid->LatLon();
  }

  return NFmiPoint::gMissingLatlon;
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiHPlaceDescriptor::LatLon(unsigned long theIndex) const
{
  if (itsLocationBag)
  {
    const NFmiLocation *location = LocationWithIndex(theIndex);
    return NFmiPoint(location->GetLongitude(), location->GetLatitude());
  }
  else if (itsGrid)
  {
    return itsGrid->LatLon(theIndex);
  }
  else
  {
    return NFmiPoint(kFloatMissing, kFloatMissing);
  }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa currentin paikan suhteellisen sijainnin (esim. gridin areassa)
 *
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiHPlaceDescriptor::RelativePoint() const
{
  if (itsLocationBag || !itsGrid)
    return NFmiPoint(kFloatMissing,
                     kFloatMissing);  // Marko En tiedä voidaanko tätä saada laskettua mitenkään?
  return itsGrid->RelativePoint();
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa currentin paikan suhteellisen sijainnin (esim. gridin areassa)
 *
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiHPlaceDescriptor::RelativePoint(unsigned long theIndex) const
{
  if (itsLocationBag || !itsGrid)
    return NFmiPoint(kFloatMissing,
                     kFloatMissing);  // Marko En tiedä voidaanko tätä saada laskettua mitenkään?
  return itsGrid->RelativePoint(theIndex);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::Next()
{
  if (IsLocation())
    return (itsLocationBag->Next());
  else if (IsGrid())
    return (itsGrid->Next());
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::Previous()
{
  if (IsLocation())
    return (itsLocationBag->Previous());
  else if (IsGrid())
    return (itsGrid->Previous());
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiHPlaceDescriptor::Size() const
{
  if (IsLocation()) return (itsLocationBag->GetSize());
  if (IsArea()) return 1ul;
  if (itsGrid) return itsGrid->OriginalSize();  // Marko/23.11.1998, myös time, param, level jutut
  return 0;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

unsigned long NFmiHPlaceDescriptor::Index() const
{
  if (IsLocation()) return itsLocationBag->CurrentIndex();
  if (IsArea()) return static_cast<unsigned long>(0);
  if (IsGrid())
    //	return static_cast<unsigned long>(itsGrid->DataIndex()) - itsGrid->Base();
    return static_cast<unsigned long>(itsGrid->DataIndex());
  return static_cast<unsigned long>(-1);
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::Index(unsigned long theIndex)
{
  if (IsLocation()) return itsLocationBag->SetCurrentIndex(theIndex);
  if (IsArea()) return static_cast<unsigned long>(0);
  if (IsGrid()) return itsGrid->Index(theIndex);
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theIdent Undocumented
 * \return Undocumented
 * \todo Const long reference as input???
 * \todo Siirrä koodi NFmiLocationBag:iin!
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::Location(long theIdent)
{
  if (IsLocation())
  {
    bool tempBoolean;
    itsLocationBag->Reset();

    do
    {
      tempBoolean = itsLocationBag->Next();
      if (!tempBoolean) break;
    } while (!(itsLocationBag->Location()->GetIdent() == theIdent));
    if (tempBoolean)
      return true;
    else
      return false;
  }
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theName Undocumented
 * \return Undocumented
 * \todo Siirrä koodi NFmiLocationBag:iin!
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::Location(const NFmiString &theName)
{
  NFmiString theLocationUpperCase;
  NFmiString theUpperName(theName);
  theUpperName.UpperCase();

  if (IsLocation())
  {
    bool tempBoolean;
    itsLocationBag->Reset();

    do
    {
      do
      {
        tempBoolean = itsLocationBag->Next();
        if (!tempBoolean) return false;
      } while (theName.GetLen() != itsLocationBag->Location()->GetName().GetLen());

      theLocationUpperCase = itsLocationBag->Location()->GetName();
      theLocationUpperCase.UpperCase();
    } while (!(theLocationUpperCase == theUpperName));

    if (tempBoolean)
      return true;
    else
      return false;
  }
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theLonLatPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::Location(const NFmiPoint &theLonLatPoint, NFmiPoint *theGridPoint)
{
  if (IsLocation())  // Muutin käyttämään locationbagin omaa koodia ("saman niminen asema" -bugin
                     // takia) /Marko
    return itsLocationBag->NearestLocation(theLonLatPoint);
  else if (IsGrid())
  {
    if (itsGrid->NearestLatLon(
            theLonLatPoint.X(), theLonLatPoint.Y(), kFloatMissing * 1000., theGridPoint))
      return true;
  }

  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \return Undocumented
 * \todo Siirrä koodi NFmiLocationBag:iin!
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::Location(const NFmiLocation &theLocation)
{
  if (IsLocation())
  {
    bool tempBoolean;
    itsLocationBag->Reset();

    do
    {
      tempBoolean = itsLocationBag->Next();
      if (!tempBoolean) break;
    } while (!(theLocation == *itsLocationBag->Location()));  // Marko: kun käännetään järjestys,
                                                              // voidaan etsiä myös locationilla
                                                              // stationeita

    if (tempBoolean)
      return true;
    else
      return false;
  }
  else if (IsGrid())
    return itsGrid->NearestLatLon(theLocation.GetLongitude(), theLocation.GetLatitude());
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::IsActive() const
{
  if (itsActivity)
    return (itsActivity[Index()]);
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theActivityState Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::SetActivity(bool theActivityState)
{
  if (itsActivity)
  {
    bool temp = itsActivity[Index()];
    itsActivity[Index()] = theActivityState;
    return temp;
  }
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::NextActive()
{
  while (Next())
    if (IsActive()) return true;

  return false;
}

// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theHPlaceDescriptor The object being copied
 * \return The object assigned to
 * \bug Must protect from self assignment
 */
// ----------------------------------------------------------------------

NFmiHPlaceDescriptor &NFmiHPlaceDescriptor::operator=(
    const NFmiHPlaceDescriptor &theHPlaceDescriptor)
{
  if (this != &theHPlaceDescriptor)
  {
    Destroy();

    itsLocationBag =
        theHPlaceDescriptor.itsLocationBag ? theHPlaceDescriptor.itsLocationBag->Clone() : nullptr;
    itsArea = theHPlaceDescriptor.itsArea ? theHPlaceDescriptor.itsArea->Clone() : nullptr;
    itsGrid = theHPlaceDescriptor.itsGrid ? new NFmiGrid(*theHPlaceDescriptor.itsGrid) : nullptr;

    itsSelectedType = theHPlaceDescriptor.itsSelectedType;

    if (itsLocationBag)
    {
      itsActivity = new bool[itsLocationBag->GetSize()];
      for (int i = 0; i < static_cast<int>(itsLocationBag->GetSize()); i++)
        itsActivity[i] = theHPlaceDescriptor.itsActivity[i];
    }
  }

  return *this;
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theHPlaceDescriptor The object being compared to
 * \return True if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::operator==(const NFmiHPlaceDescriptor &theHPlaceDescriptor) const
{
  if (this->itsSelectedType == theHPlaceDescriptor.itsSelectedType)
  {
    if (this->IsGrid() && theHPlaceDescriptor.IsGrid())
    {
      if (*(this->Grid()) == *(theHPlaceDescriptor.Grid())) return true;
    }
    else if (this->IsLocation() && theHPlaceDescriptor.IsLocation())
    {
      return (*(this->itsLocationBag) == *(theHPlaceDescriptor.itsLocationBag));
    }
  }
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theCombine Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiHPlaceDescriptor NFmiHPlaceDescriptor::Combine(const NFmiHPlaceDescriptor &theCombine)
{
  if (itsLocationBag && theCombine.itsLocationBag)
    return NFmiHPlaceDescriptor(itsLocationBag->Combine(*(theCombine).itsLocationBag));
  // jos ei voi yhdistää locationbagejä, palauttaa this-descriptorin (voitaisiin ehkä laittaa jotain
  // gridi yhdistelyjä?)
  return *this;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

void NFmiHPlaceDescriptor::Reset()
{
  if (IsLocation()) itsLocationBag->Reset();
  if (IsGrid()) itsGrid->Reset();
}

// ----------------------------------------------------------------------
/*!
 * \param theLocation Undocumented
 * \param theMaxDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::NearestLocation(const NFmiLocation &theLocation, double theMaxDistance)
{
  if (itsLocationBag) return itsLocationBag->NearestLocation(theLocation, theMaxDistance);
  if (itsGrid)
    return itsGrid->NearestLatLon(
        theLocation.GetLongitude(), theLocation.GetLatitude(), theMaxDistance);
  return false;
}

bool NFmiHPlaceDescriptor::NearestLocation(const NFmiLocation &theLocation,
                                           const NFmiArea *theArea,
                                           double theMaxDistance)
{
  if (itsLocationBag)
  {
    if (theArea)
      return itsLocationBag->NearestLocation(theLocation, theArea, theMaxDistance);
    else
      return itsLocationBag->NearestLocation(theLocation, theMaxDistance);
  }
  if (itsGrid)
    return itsGrid->NearestLatLon(
        theLocation.GetLongitude(), theLocation.GetLatitude(), theMaxDistance);
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::NearestPoint(const NFmiPoint &theLatLonPoint)
{
  if (itsLocationBag)
    return itsLocationBag->NearestLocation(NFmiLocation(theLatLonPoint.X(), theLatLonPoint.Y()));
  if (itsGrid) return itsGrid->NearestLatLon(theLatLonPoint.X(), theLatLonPoint.Y());
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Liikkuu xSteps:in ja ySteps:in osoittaman verran gridissä.
 * Jos ei ole gridi tai gridissä ei päästä liikkumaan haluttua
 * askelmäärää, palautuu false.
 *
 * \param xSteps Undocumented
 * \param ySteps Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::MoveInGrid(long xSteps, long ySteps)
{
  if (IsGrid())
  {
    NFmiPoint originalXY = itsGrid->XY();
    int check = 0;

    if (xSteps > 0)
      if (!(itsGrid->Right(xSteps))) check++;
    if (xSteps < 0)
      if (!(itsGrid->Left(abs(xSteps)))) check++;
    if (ySteps > 0)
      if (!(itsGrid->Up(ySteps))) check++;
    if (ySteps < 0)
      if (!(itsGrid->Down(abs(ySteps)))) check++;
    if (check != 0)
    {
      double help = 0;
      itsGrid->InterpolateToXYPoint(originalXY, help);
      return false;
    }
    return true;
  }
  else
    return false;
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiHPlaceDescriptor::Write(std::ostream &file) const
{
  long theDummy = 0;
  file << static_cast<long>(itsSelectedType) << " " << itsMaxNumberOfSources << " " << theDummy
       << " " << theDummy << std::endl;

  if (itsLocationBag)
  {
    file << itsLocationBag->ClassId() << " NFmiLocationBag" << std::endl;
    file << *itsLocationBag;

    for (unsigned long i = 0; i < itsLocationBag->GetSize(); i++)
      file << itsActivity[i] << " ";
    file << std::endl;
  }
  else
  {
    file << 0 << " NFmiLocationBag" << std::endl;
  }

  if (itsArea)
  {
    file << itsArea->ClassId() << " " << itsArea->ClassName() << std::endl;
    file << *itsArea;
  }
  else
  {
    file << 0 << " NFmiArea" << std::endl;
  }

  if (itsGrid)
  {
    file << itsGrid->ClassId() << " " << itsGrid->ClassName() << std::endl;
    file << *itsGrid;
  }
  else
  {
    file << 0 << " NFmiGrid" << std::endl;
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

std::istream &NFmiHPlaceDescriptor::Read(std::istream &file)
{
  unsigned long classId;
  unsigned char className[30];
  long theDummy2, theDummy3;
  long theSelectedTypeValue;

  Destroy();

  try
  {
    file >> theSelectedTypeValue;
    itsSelectedType = FmiStationType(theSelectedTypeValue);
    file >> itsMaxNumberOfSources >> theDummy2 >> theDummy3;

    file >> classId >> className;
    if (classId >= KNFmiMaxClass)
      throw runtime_error("NFmiHPlaceDescriptor::Read().classID(NFmiLocation) Error");

    if (classId)
    {
      itsLocationBag = static_cast<NFmiLocationBag *>(CreateSaveBase(classId));
      file >> *itsLocationBag;

      itsActivity = new bool[itsLocationBag->GetSize()];
      for (unsigned long i = 0; i < itsLocationBag->GetSize(); i++)
        file >> itsActivity[i];
    }

    file >> classId;
    if (classId > KNFmiMaxClass)
      throw runtime_error("NFmiHPlaceDescriptor::Read().classID(NFmiArea) Error");
    file >> className;  // NFmiArea
    if (classId)
    {
      itsArea = static_cast<NFmiArea *>(CreateSaveBase(classId));
      file >> *itsArea;
    }

    file >> classId;
    if (classId > KNFmiMaxClass)
      throw runtime_error("NFmiHPlaceDescriptor::Read().classID(NFmiGrid) Error");
    file >> className;  // NFmiGrid
    if (classId)
    {
      itsGrid = static_cast<NFmiGrid *>(CreateSaveBase(classId));
      file >> *itsGrid;
    }

  }  // try

  catch (...)
  {
    Destroy();
    throw;
  }

  return file;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiArea *NFmiHPlaceDescriptor::Area() const
{
  return itsArea ? itsArea : itsGrid ? itsGrid->Area() : nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

FmiInterpolationMethod NFmiHPlaceDescriptor::InterpolationMethod() const
{
  return itsGrid ? itsGrid->InterpolationMethod() : kNoneInterpolation;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiHPlaceDescriptor::GridPoint() const
{
  return itsGrid ? itsGrid->GridPoint() : NFmiPoint(kFloatMissing, kFloatMissing);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::First()
{
  return IsLocation() ? itsLocationBag->First() : itsGrid->First();
}

// ----------------------------------------------------------------------
/*!
 * Hakee listan paikkaindeksi/etäisyys metreinä pareja.
 * Listaan haetaan annettua paikkaa lähimmat datapisteet järjestyksessä
 * lähimmästä kauimpaan. Listaan haetaan joko haluttu määrä lähimpiä pisteitä
 * tai hakua voi myös rajoittaa maksimi etäisyydellä. Jos maksimi määräksi
 *  laitetaan -1, haetaan paikkoja niin paljon kuin löytyy (maxEtäisyys
 *  rajoitus huomiooon ottaen).
 *
 * \param theLocation Undocumented
 * \param theMaxWantedLocations Undocumented
 * \param theMaxDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const checkedVector<std::pair<int, double> > NFmiHPlaceDescriptor::NearestLocations(
    const NFmiLocation &theLocation, int theMaxWantedLocations, double theMaxDistance) const
{
  if (IsLocation())
  {
    return itsLocationBag->NearestLocations(theLocation, theMaxWantedLocations, theMaxDistance);
  }
  else
  {
    return itsGrid->NearestLocations(theLocation, theMaxWantedLocations, theMaxDistance);
  }
}

// ----------------------------------------------------------------------
/*!
 * Calculates coordinates in cache
 *
 * \param v The cache vector
 */
// ----------------------------------------------------------------------

void NFmiHPlaceDescriptor::CreateLatLonCache(std::vector<NFmiPoint> &v)
{
  NFmiPoint point;

  v.clear();
  if (IsLocation())
  {
    v.reserve(itsLocationBag->GetSize());
    for (itsLocationBag->Reset(); itsLocationBag->Next();)
    {
      point.Set(itsLocationBag->Location()->GetLongitude(),
                itsLocationBag->Location()->GetLatitude());
      v.push_back(point);
    }
  }
  else
  {
    v.reserve(itsGrid->Size());
    for (itsGrid->Reset(); itsGrid->Next();)
      v.push_back(itsGrid->LatLon());
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the given point is in the data
 */
// ----------------------------------------------------------------------

bool NFmiHPlaceDescriptor::IsInside(const NFmiPoint &theLatLon, double theRadius) const
{
  if (IsLocation())
    return itsLocationBag->IsInside(theLatLon, theRadius);
  else
    return itsGrid->IsInside(theLatLon);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for caching unique grids or sets of locations
 */
// ----------------------------------------------------------------------

std::size_t NFmiHPlaceDescriptor::HashValue() const
{
  std::size_t hash = 0;

  if (itsLocationBag != nullptr) boost::hash_combine(hash, itsLocationBag->HashValue());

  if (itsGrid != nullptr) boost::hash_combine(hash, itsGrid->HashValue());

  return hash;
}
