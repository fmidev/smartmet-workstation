// ======================================================================
/*!
 * \file NFmiDataModifierMaxMean.h
 * \brief Interface of class NFmiDataModifierMaxMean
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierMaxMean : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierMaxMean(void);
  NFmiDataModifierMaxMean(void);
  NFmiDataModifierMaxMean(float theWeightFactor);

  void Clear(void);

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult(void);

 private:
  long itsCounter;
  float itsAverage;
  float itsMaxValue;
  float itsWeightFactor;

};  // class NFmiDataModifierMaxMean

// ======================================================================
