// ======================================================================
/*!
 * \file NFmiSpaceDescriptor.h
 * \brief Interface of class NFmiSpaceDescriptor
 */
// ======================================================================
/*!
 * \class NFmiSpaceDescriptor
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataDescriptor.h"
#include "NFmiHPlaceDescriptor.h"
#include "NFmiVPlaceDescriptor.h"

// ÄLÄ KÄYTÄ TÄTÄ, TESTI LUOKKA/18.2.2000/Marko

//! Undocumented
class _FMI_DLL NFmiSpaceDescriptor : public NFmiDataDescriptor
{
 public:
  //! Void constructor
  NFmiSpaceDescriptor(void) : itsLevels(0), itsPlaces(0) {}
  bool NextLevel(void) const { return itsLevels->Next(); }
  bool NextPlace(void) const { return itsPlaces->Next(); }

 private:
  NFmiVPlaceDescriptor* itsLevels;
  NFmiHPlaceDescriptor* itsPlaces;

};  // class NFmiSpaceDescriptor

// ======================================================================
