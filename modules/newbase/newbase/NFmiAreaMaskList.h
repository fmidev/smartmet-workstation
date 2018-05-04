// ======================================================================
/*!
 * \file NFmiAreaMaskList.h
 * \brief Interface for class NFmiAreaMaskList
 */
// ======================================================================

#pragma once

#include "NFmiAreaMask.h"
#include "boost/shared_ptr.hpp"
#include <vector>

class _FMI_DLL NFmiAreaMaskList
{
 public:
  NFmiAreaMaskList(void);
  NFmiAreaMaskList(const NFmiAreaMaskList &theOther);
  virtual ~NFmiAreaMaskList(void);
  static boost::shared_ptr<NFmiAreaMaskList> CreateShallowCopy(
      const boost::shared_ptr<NFmiAreaMaskList> &theOther);

  void Add(boost::shared_ptr<NFmiAreaMask> &theMask);
  bool Remove(void);
  void Clear(void);
  unsigned long NumberOfItems(void);

  bool Reset(void);
  bool Next(void);
  boost::shared_ptr<NFmiAreaMask> Current(void);

  bool UseMask(void) { return fMaskInUse; }
  bool IsMasked(const NFmiPoint &theLatLon);
  double MaskValue(const NFmiPoint &theLatLon);
  bool CheckIfMaskUsed(void);
  bool SyncronizeMaskTime(const NFmiMetTime &theTime);

  bool Index(unsigned long index);    // 1:sta alkava indeksi, sisäinen muuttuja itsCurrentIndex on
                                      // taas 0:sta alkava
  bool Find(unsigned long theIndex);  // 1:sta alkava indeksi, sisäinen muuttuja itsCurrentIndex on
                                      // taas 0:sta alkava
  bool Find(const NFmiDataIdent &theParam);
  bool Find(const NFmiDataIdent &theParam, const NFmiLevel *theLevel);

 private:
  bool IsValidIndex(int theIndex);

  std::vector<boost::shared_ptr<NFmiAreaMask> > itsMaskVector;
  int itsCurrentIndex;  // Reset laittaa tämän -1:ksi, 1. maski löytyy 0:sta ja viimeinen size-1:stä
  bool fMaskInUse;      // Arvo asetetaan kun tarkastetaan onko mikään

};  // class NFmiAreaMaskList

// ======================================================================
