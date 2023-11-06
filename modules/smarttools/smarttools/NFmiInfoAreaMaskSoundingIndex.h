// ======================================================================
/*!
 * \file NFmiInfoAreaMaskSoundingIndex.h
 * \brief Interface of class NFmiInfoAreaMaskSoundingIndex
 */
// ======================================================================

#pragma once

#include "NFmiSoundingIndexCalculator.h"
#include <newbase/NFmiInfoAreaMask.h>

class NFmiFastQueryInfo;

//! Tämä luokka toimii kuten NFmiInfoAreaMask mutta kurkkaa halutun x-y hila pisteen yli arvoa
class NFmiInfoAreaMaskSoundingIndex : public NFmiInfoAreaMask
{
 public:
  virtual ~NFmiInfoAreaMaskSoundingIndex();
  NFmiInfoAreaMaskSoundingIndex(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                FmiSoundingParameters theSoundingParam,
                                unsigned long thePossibleMetaParamId);
  NFmiInfoAreaMaskSoundingIndex(const NFmiInfoAreaMaskSoundingIndex &theOther);
  NFmiAreaMask *Clone() const override;

  FmiSoundingParameters SoundingParam() const { return itsSoundingParam; }
  void SoundingParam(FmiSoundingParameters newValue) { itsSoundingParam = newValue; }
  // tätä kaytetaan smarttool-modifierin yhteydessä
  using NFmiInfoAreaMask::Value;
  double Value(const NFmiPoint &theLatlon,
               const NFmiMetTime &theTime,
               int theTimeIndex,
               bool fUseTimeInterpolationAlways);

 private:
  FmiSoundingParameters itsSoundingParam;

  NFmiInfoAreaMaskSoundingIndex &operator=(const NFmiInfoAreaMaskPeekXY &theMask);

};  // class NFmiInfoAreaMaskSoundingIndex
