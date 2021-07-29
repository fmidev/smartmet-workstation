// ======================================================================
/*!
 * \file NFmiTimeZoneFinder.h
 * \brief Interface of class NFmiTimeZoneFinder
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include <boost/shared_ptr.hpp>
#include <string>

class NFmiPoint;
class NFmiTimeZoneFinderPimple;

//! Undocumented
class NFmiTimeZoneFinder
{
 public:
  ~NFmiTimeZoneFinder();
  NFmiTimeZoneFinder();
  NFmiTimeZoneFinder(const NFmiTimeZoneFinder& theTimeZoneFinder);
  NFmiTimeZoneFinder& operator=(const NFmiTimeZoneFinder& theTimeZoneFinder);

  bool ReadFile(const std::string& theFileName);
  float Find(const NFmiPoint& theLatLon) const;

  bool Empty() const;
  void Clear();

#ifndef NDEBUG
  void Check(const NFmiPoint& theLatLon) const;
#endif

 private:
  boost::shared_ptr<NFmiTimeZoneFinderPimple> itsPimple;

};  // class NFmiTimeZoneFinder

// ======================================================================
