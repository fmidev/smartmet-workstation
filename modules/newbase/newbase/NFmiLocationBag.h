// ======================================================================
/*!
 * \file NFmiLocationBag.h
 * \brief Interface of class NFmiLocationBag
 */
// ======================================================================

#pragma once

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
class NFmiLocationBag : public NFmiSize
{
 public:
  virtual ~NFmiLocationBag();
  NFmiLocationBag();
  NFmiLocationBag(const NFmiLocationBag &theBag);
  NFmiLocationBag(const NFmiLocation &theLocation);
  NFmiLocationBag(const NFmiLocation *theLocationArray, unsigned long theNumberOfLocations);
  NFmiLocationBag(const NFmiStation *theLocationArray, unsigned long theNumberOfLocations);

  NFmiLocationBag &operator=(const NFmiLocationBag &theLocationBag);
  bool operator==(const NFmiLocationBag &theLocationBag) const;

  virtual void Destroy();

  virtual bool Location(const NFmiLocation &theLocation);
  virtual const NFmiLocation *Location() const;
  virtual const NFmiLocation *Location(unsigned long theIndex) const;
  virtual bool AddLocation(const NFmiLocation &theLocation, bool theChecking = true);
  virtual bool IsLocations() const;
  virtual bool NearestLocation(const NFmiLocation &theLocation,
                               double theMaxDistance = kFloatMissing * 1000.);
  virtual bool NearestLocation(const NFmiLocation &theLocation,
                               const NFmiArea *theArea,
                               double theMaxDistance = kFloatMissing * 1000.);

  //! Hakee listan paikkaindeksi/etäisyys metreinä pareja. Listaan haetaan annettua paikkaa lähimmat
  //! datapisteet.
  const std::vector<std::pair<int, double>> NearestLocations(
      const NFmiLocation &theLocation,
      int theMaxWantedLocations,
      double theMaxDistance = kFloatMissing) const;

  virtual const NFmiLocationBag Combine(const NFmiLocationBag &theBag);
  virtual NFmiLocationBag *Clone() const;

  virtual unsigned long ClassId() const { return kNFmiLocationBag; }
  virtual std::ostream &Write(std::ostream &file) const;
  virtual std::istream &Read(std::istream &file);

  bool IsInside(const NFmiPoint &theLatLon, double theRadius) const;

  std::size_t HashValue() const;
  bool AllowLocationCopyOptimization() const { return fAllowLocationCopyOptimization; }
  void AllowLocationCopyOptimization(bool newState) { fAllowLocationCopyOptimization = newState; }

 protected:
  void Add(const NFmiLocation &theLocation);
  void DoDeepLocationCopy(const NFmiLocationBag &theLocationBag);
  void DoActualCopyOperations(const NFmiLocationBag &theLocationBag);

  typedef std::vector<NFmiLocation *> StorageType;
  std::shared_ptr<StorageType> itsLocations;

 private:
  // This could also have been a map to vector indices.
  // The pointer is the same as in the vector
  typedef std::set<NFmiLocation> SortedStorageType;
  std::shared_ptr<SortedStorageType> itsSortedLocations;

  // NearTree of the locations

  std::shared_ptr<NFmiNearTree<NFmiNearTreeLocation, NFmiNearTreeLocationDistance>> itsNearTree;

  // Enable/disable optimizing when copying locationBags, when enable,
  // sharep_ptr's are used directly ,and when disabled, true copies of all the stations are
  // created (old and normal way). Can't enable this by default, because some usage would cause troubles.
  bool fAllowLocationCopyOptimization = false;
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
  if (theIndex < itsLocations->size())
    return (*itsLocations)[theIndex];
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

inline NFmiLocationBag *NFmiLocationBag::Clone() const { return new NFmiLocationBag(*this); }
// ----------------------------------------------------------------------
/*!
 * Test if the bag contains any locations
 *
 * \return True if the bag contains atleast 1 location
 */
// ----------------------------------------------------------------------

inline bool NFmiLocationBag::IsLocations() const { return (!itsLocations->empty()); }

// ======================================================================
