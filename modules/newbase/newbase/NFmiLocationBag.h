// ======================================================================
/*!
 * \file NFmiLocationBag.h
 * \brief Interface of class NFmiLocationBag
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiGeoTools.h"
#include "NFmiLocation.h"
#include "NFmiNearTree.h"
#include "NFmiNearTreeLocation.h"
#include "NFmiSaveBaseFactory.h"
#include "NFmiSize.h"
#include <boost/math/constants/constants.hpp>
#include <algorithm>
#include <cassert>
#include <set>

class NFmiStation;
class NFmiArea;

//! Undocumented
class _FMI_DLL NFmiLocationBag : public NFmiSize
{
 public:
  virtual ~NFmiLocationBag(void);
  NFmiLocationBag(void);
  NFmiLocationBag(const NFmiLocationBag &theBag);
  NFmiLocationBag(const NFmiLocation &theLocation);
  NFmiLocationBag(const NFmiLocation *theLocationArray, unsigned long theNumberOfLocations);
  NFmiLocationBag(const NFmiStation *theLocationArray, unsigned long theNumberOfLocations);

  NFmiLocationBag &operator=(const NFmiLocationBag &theLocationBag);
  bool operator==(const NFmiLocationBag &theLocationBag) const;

  virtual void Destroy(void);

  virtual bool Location(const NFmiLocation &theLocation);
  virtual const NFmiLocation *Location(void) const;
  virtual const NFmiLocation *Location(unsigned long theIndex) const;
  virtual bool AddLocation(const NFmiLocation &theLocation, bool theChecking = true);
  virtual bool IsLocations(void) const;
  virtual bool NearestLocation(const NFmiLocation &theLocation,
                               double theMaxDistance = kFloatMissing * 1000.);
  virtual bool NearestLocation(const NFmiLocation &theLocation,
                               const NFmiArea *theArea,
                               double theMaxDistance = kFloatMissing * 1000.);

  //! Hakee listan paikkaindeksi/etäisyys metreinä pareja. Listaan haetaan annettua paikkaa lähimmat
  //! datapisteet.
  const checkedVector<std::pair<int, double> > NearestLocations(
      const NFmiLocation &theLocation,
      int theMaxWantedLocations,
      double theMaxDistance = kFloatMissing) const;

  virtual const NFmiLocationBag Combine(const NFmiLocationBag &theBag);
  virtual NFmiLocationBag *Clone(void) const;

  virtual unsigned long ClassId(void) const { return kNFmiLocationBag; }
  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  bool IsInside(const NFmiPoint &theLatLon, double theRadius) const;

  std::size_t HashValue() const;

 protected:
  NFmiLocationBag(NFmiLocation **theLocationBag, unsigned long theNumberOfLocations);

  void Add(const NFmiLocation &theLocation);

  typedef checkedVector<NFmiLocation *> StorageType;
  StorageType itsLocations;

 private:
  // This could also have been a map to vector indices.
  // The pointer is the same as in the vector
  typedef std::set<NFmiLocation> SortedStorageType;
  SortedStorageType itsSortedLocations;

  // NearTree of the locations

  NFmiNearTree<NFmiNearTreeLocation, NFmiNearTreeLocationDistance> itsNearTree;

};  // class NFmiLocationBag

// ----------------------------------------------------------------------
/*!
 * Output operator for NFmiLocationBag class
 *
 * \param file The output stream to write to
 * \param ob The object to write
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

inline std::ostream &operator<<(std::ostream &file, const NFmiLocationBag &ob)
{
  return ob.Write(file);
}

// ----------------------------------------------------------------------
/*!
 * Input operator for NFmiLocationBag class
 *
 * \param file The input stream to read from
 * \param ob The object into which to read the new contents
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

inline std::istream &operator>>(std::istream &file, NFmiLocationBag &ob) { return ob.Read(file); }
// ----------------------------------------------------------------------
/*!
 * Returns the location with the given index. Note that the
 * pointer is const to ensure the validity of the internal
 * sorted containers.
 *
 * \param theIndex The index of the location
 * \return The requested location
 */
// ----------------------------------------------------------------------

inline const NFmiLocation *NFmiLocationBag::Location(unsigned long theIndex) const
{
  if (theIndex < itsLocations.size())
    return itsLocations[theIndex];
  else
    return 0;
}

// ----------------------------------------------------------------------
/*!
 * Clone the object
 *
 * \return A clone of the object
 * \todo Should return auto_ptr
 */
// ----------------------------------------------------------------------

inline NFmiLocationBag *NFmiLocationBag::Clone(void) const { return new NFmiLocationBag(*this); }
// ----------------------------------------------------------------------
/*!
 * Test if the bag contains any locations
 *
 * \return True if the bag contains atleast 1 location
 */
// ----------------------------------------------------------------------

inline bool NFmiLocationBag::IsLocations(void) const { return (!itsLocations.empty()); }

// ======================================================================
