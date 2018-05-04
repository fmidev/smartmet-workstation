// ======================================================================
/*!
 * \file NFmiDataModifierExtremePlace.h
 * \brief abstrakti luokka paikkamin/max:ia varten joka myös antaa äärirvon
 *  sattumispaikan. /Lasse
 *  vrt NFmiDataModifierExtreme joka antaa ajan kun ajan yly ääriarvo
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiLocation.h"

//! Undocumented
class _FMI_DLL NFmiDataModifierExtremePlace : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierExtremePlace(void);
  NFmiDataModifierExtremePlace(void);

  void SetLocation(NFmiQueryInfo *theQI);
  const NFmiLocation GetLocation(void);
  // virtual void Calculate(NFmiQueryInfo * theQI);
  // virtual void Calculate(float theValue);
  using NFmiDataModifier::Calculate;
  virtual void Calculate(float theValue, NFmiQueryInfo *theQI);

 protected:
  virtual bool IsNewExtreme(float) = 0;

 public:
  float itsExtremeValue;
  NFmiLocation itsExtremeLocation;

};  // class NFmiDataModifierExtremePlace

// ======================================================================
