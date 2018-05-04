// ======================================================================
/*!
 * \file NFmiDataModifierStandardDeviation.h
 * \brief Interface of class NFmiDataModifierStandardDeviation
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierStandardDeviation : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierStandardDeviation(void);
  NFmiDataModifierStandardDeviation(void);

  void Clear(void);

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult(void);

 protected:
  long itsCounter;
  float itsSum;
  float itsSquaredSum;

};  // class NFmiDataModifierStandardDeviation

// ======================================================================
