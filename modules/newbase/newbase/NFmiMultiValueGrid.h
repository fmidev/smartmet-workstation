// ======================================================================
/*!
 * \file NFmiMultiValueGrid.h
 * \brief Interface of class NFmiMultiValueGrid
 */
// ======================================================================

#pragma once

#include "NFmiGrid.h"
#include "NFmiQueryInfo.h"

// infolta iteraattori-ominaisuudet

//! Undocumented

class _FMI_DLL NFmiMultiValuedGrid : public NFmiGrid, private NFmiQueryInfo
{
 public:
  virtual ~NFmiMultiValuedGrid(void);

  NFmiMultiValuedGrid(void);
  NFmiMultiValuedGrid(const NFmiGrid& theGrid,
                      const NFmiParamDescriptor& theParamDescriptor,
                      const NFmiTimeDescriptor& theTimeDescriptor,
                      const NFmiVPlaceDescriptor& theVPlaceDescriptor);

  bool FirstValue(void);  // Resets Param,Time and Level but not location

  // Ambiquos methods inhereted primarily from NFmiGrid

  bool First(void);                // resets Grid
  unsigned long Size(void) const;  // Grid Size
  NFmiArea* Area(void) const;      // GridArea

  float FloatValue(long dx = 0, long dy = 0);
  using NFmiQueryInfo::FloatValue;
  bool FloatValue(float data);
  bool InterpolateToLatLonPoint(double newLon, double newLat, double& theValue);

 private:
  NFmiMultiValuedGrid(const NFmiMultiValuedGrid& theGrid);
  NFmiMultiValuedGrid& operator=(const NFmiMultiValuedGrid& theGrid);

  void UpDate(void);
  bool Save(void);
  bool Edit(void);

  using NFmiGrid::Init;
  void Init(void);
  virtual bool Init(NFmiDataPool* theData) { return NFmiGrid::Init(theData); }
  virtual bool Init(NFmiGridBase* theData) { return NFmiGridBase::Init(theData); }
  NFmiDataPool* itsMultiData;
  unsigned long itsCurrentDataStatus;

 protected:
};  // class NFmiMultiValuedGrid

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

inline NFmiMultiValuedGrid::NFmiMultiValuedGrid(void) : itsMultiData(0), itsCurrentDataStatus() {}
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiMultiValuedGrid::First(void) { return NFmiGrid::First(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiMultiValuedGrid::Size(void) const { return NFmiGrid::Size(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline NFmiArea* NFmiMultiValuedGrid::Area(void) const { return NFmiGrid::Area(); }

// ======================================================================
