// ======================================================================
/*!
 * \file NFmiLocationFinder.h
 * \brief Interface of class NFmiLocationFinder
 */
// ======================================================================

#pragma once

#include "NFmiPoint.h"
#include "NFmiString.h"
#include <map>
#include <string>

//! Undocumented
class _FMI_DLL NFmiLocationFinder
{
 public:
  ~NFmiLocationFinder(void);
  NFmiLocationFinder(void);
  NFmiLocationFinder(const NFmiLocationFinder& theLocationFinder);

  bool AddFile(const NFmiString& theFileName, bool lonFirst = true, bool optionalType = false);
  const NFmiPoint Find(const NFmiString& theName);
  unsigned long FindWmo(const NFmiString& theName);

  bool Empty(void) const;
  void Clear(void);
  bool LastSearchFailed(void);

 private:
  bool AddFileOfOptionalType(const NFmiString& theFileName, bool lonFirst = true);

  //! The data container consists of location names and coordinates.
  typedef std::map<std::string, NFmiPoint> LocationFinderMap;

  LocationFinderMap itsPrimaryPoints;
  LocationFinderMap itsSecondaryPoints;

  bool itsLastSearchFailed;  //!< Success indicator for the latest search

};  // class NFmiLocationFinder

// ----------------------------------------------------------------------
/*!
 * Test if the object is empty
 *
 * \return True, if the object contains no locations
 */
// ----------------------------------------------------------------------

inline bool NFmiLocationFinder::Empty(void) const
{
  return (itsPrimaryPoints.empty() && itsSecondaryPoints.empty());
}

// ----------------------------------------------------------------------
/*!
 * Delete all locations from the object
 */
// ----------------------------------------------------------------------

inline void NFmiLocationFinder::Clear(void)
{
  itsPrimaryPoints.clear();
  itsSecondaryPoints.clear();
}

// ----------------------------------------------------------------------
/*!
 * Test if the last search failed
 *
 * \return True if the last search failed
 */
// ----------------------------------------------------------------------

inline bool NFmiLocationFinder::LastSearchFailed(void) { return itsLastSearchFailed; }

// ======================================================================
