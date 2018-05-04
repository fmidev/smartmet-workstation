// ======================================================================
/*!
 * \file NFmiHPlaceDescriptor.h
 * \brief Interface of class NFmiHPlaceDescriptor
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"
#include "NFmiDataMatrix.h"
#include "NFmiGlobals.h"
#include "NFmiLocation.h"
#include "NFmiPoint.h"
#include "NFmiSaveBaseFactory.h"
#include <algorithm>

class NFmiLocationBag;
class NFmiArea;
class NFmiGrid;
class NFmiLocation;

//! Undocumented
class _FMI_DLL NFmiHPlaceDescriptor : public NFmiDataDescriptor
{
 public:
  virtual ~NFmiHPlaceDescriptor(void);
  NFmiHPlaceDescriptor(void);
  NFmiHPlaceDescriptor(const NFmiHPlaceDescriptor &theHPlaceDescriptor);

  NFmiHPlaceDescriptor(const NFmiLocationBag &theLocationBag,
                       FmiStationType theSelectedType = kWMO,
                       unsigned long theMaxNumberOfSources = 0);

  NFmiHPlaceDescriptor(const NFmiArea &theArea,
                       FmiStationType theSelectedType = kWMO,
                       unsigned long theMaxNumberOfSources = 0);

  NFmiHPlaceDescriptor(const NFmiGrid &theGrid,
                       FmiStationType theSelectedType = kLatLon,
                       unsigned long theMaxNumberOfSources = 0);

  NFmiHPlaceDescriptor(const NFmiLocationBag &theLocationBag,
                       const NFmiArea &theArea,
                       FmiStationType theSelectedType = kAll,
                       unsigned long theMaxNumberOfSources = 0);

  NFmiHPlaceDescriptor(const NFmiArea &theArea,
                       const NFmiGrid &theGrid,
                       FmiStationType theSelectedType = kLatLon,
                       unsigned long theMaxNumberOfSources = 0);

  bool IsLocation(void) const;
  bool IsArea(void) const;
  bool IsGrid(void) const;

  const NFmiArea *Area(void) const;
  const NFmiGrid *Grid(void) const;
  NFmiGrid *NonConstGrid(void) const;
  const NFmiLocation *Location(void) const;
  const NFmiLocation *LocationWithIndex(unsigned long theIndex) const;

  NFmiPoint LatLon(void) const;
  NFmiPoint LatLon(unsigned long theIndex) const;
  NFmiPoint RelativePoint(void) const;
  NFmiPoint RelativePoint(unsigned long theIndex) const;
  NFmiPoint GridPoint(void) const;

  bool NearestLocation(const NFmiLocation &theLocation,
                       double theMaxDistance = kFloatMissing * 1000.);
  bool NearestLocation(const NFmiLocation &theLocation,
                       const NFmiArea *theArea,
                       double theMaxDistance = kFloatMissing * 1000.);
  bool NearestPoint(const NFmiPoint &theLatLonPoint);

  //! Hakee listan paikkaindeksi/etäisyys metreinä pareja. Listaan haetaan annettua paikkaa lähimmat
  //! datapisteet.
  const checkedVector<std::pair<int, double> > NearestLocations(
      const NFmiLocation &theLocation,
      int theMaxWantedLocations,
      double theMaxDistance = kFloatMissing) const;

  bool MoveInGrid(long xSteps, long ySteps);

  FmiInterpolationMethod InterpolationMethod(void) const;

  void LocationList(const NFmiLocationBag &theLocationBag);
  bool Location(const NFmiLocation &theLocation);
  bool Location(long theIdent);
  bool Location(const NFmiString &theLocationName);
  bool Location(const NFmiPoint &theLonLatPoint, NFmiPoint *theGridPoint = 0);

  void Reset(void);
  bool First(void);
  bool Next(void);
  bool Previous(void);

  void CreateLatLonCache(std::vector<NFmiPoint> &v);

  virtual unsigned long Index(void) const;
  bool Index(unsigned long theIndex);
  virtual unsigned long Size(void) const;

  virtual FmiStationType SelectedType(void) const;
  virtual void SelectedType(FmiStationType thelocationType);

  virtual unsigned long MaxNumberOfSources(void) const;
  virtual void MaxNumberOfSources(unsigned long theMaxNumberOfSources);
  virtual bool IsMaxNumberOfSources(void) const;

  virtual bool IsActive(void) const;
  virtual bool SetActivity(bool);
  virtual bool NextActive(void);

  virtual const NFmiHPlaceDescriptor Combine(const NFmiHPlaceDescriptor &theCombine);

  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  virtual unsigned long ClassId(void) const;
  virtual const char *ClassName(void) const;

  NFmiHPlaceDescriptor &operator=(const NFmiHPlaceDescriptor &theHPlaceDescriptor);
  bool operator==(const NFmiHPlaceDescriptor &theHPlaceDescriptor) const;

  bool IsInside(const NFmiPoint &theLatLon, double theRadius) const;

  std::size_t HashValue() const;

 protected:
  void Destroy(void);

 private:
  NFmiLocationBag *itsLocationBag;

  NFmiArea *itsArea;
  FmiStationType itsSelectedType;
  unsigned long itsMaxNumberOfSources;

  NFmiGrid *itsGrid;

  bool *itsActivity;

};  // class NFmiHPlaceDescriptor

// ----------------------------------------------------------------------
/*!
 * Destructor
 */
// ----------------------------------------------------------------------

inline NFmiHPlaceDescriptor::~NFmiHPlaceDescriptor(void) { Destroy(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiHPlaceDescriptor::IsLocation(void) const { return (itsLocationBag != 0); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiHPlaceDescriptor::IsArea(void) const { return (itsArea != 0); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiHPlaceDescriptor::IsGrid(void) const { return (itsGrid != 0); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const NFmiGrid *NFmiHPlaceDescriptor::Grid(void) const { return itsGrid; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiGrid *NFmiHPlaceDescriptor::NonConstGrid(void) const { return itsGrid; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline FmiStationType NFmiHPlaceDescriptor::SelectedType(void) const { return itsSelectedType; }
// ----------------------------------------------------------------------
/*!
 * \param theStationType Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiHPlaceDescriptor::SelectedType(FmiStationType theStationType)
{
  itsSelectedType = theStationType;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiHPlaceDescriptor::MaxNumberOfSources(void) const
{
  return itsMaxNumberOfSources;
}

// ----------------------------------------------------------------------
/*!
 * \param theMaxNumberOfSources Undocumented
 */
// ----------------------------------------------------------------------

inline void NFmiHPlaceDescriptor::MaxNumberOfSources(unsigned long theMaxNumberOfSources)
{
  itsMaxNumberOfSources = theMaxNumberOfSources;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiHPlaceDescriptor::IsMaxNumberOfSources(void) const
{
  return itsMaxNumberOfSources ? true : false;
}

// ----------------------------------------------------------------------
/*!
 * Output operator for class NFmiHPlaceDescriptor
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiHPlaceDescriptor &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for class NFmiHPlaceDescriptor
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiHPlaceDescriptor &ob)
{
  return ob.Read(file);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiHPlaceDescriptor::ClassId(void) const { return kNFmiHPlaceDescriptor; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline const char *NFmiHPlaceDescriptor::ClassName(void) const { return "NFmiHPlaceDescriptor"; }

// ======================================================================
