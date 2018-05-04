// ======================================================================
/*!
 * \file NFmiDataModifierAvg.h
 * \brief Interface of class NFmiDataModifierAvg
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierAvg : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierAvg(void);
  NFmiDataModifierAvg(void);
  NFmiDataModifierAvg(const NFmiDataModifierAvg& theOther);
  NFmiDataModifier* Clone(void) const;

  float Avg(void);
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

  void Clear(void);
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult(void);
  long Counter(void);

 protected:
  long itsCounter;
  float itsAverage;

};  // class NFmiDataModifierAvg

// ======================================================================
