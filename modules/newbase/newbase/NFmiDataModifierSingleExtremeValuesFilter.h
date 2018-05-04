// ======================================================================
/*!
 * \file NFmiDataModifierSingleExtremeValuesFilter.h
 * \brief Interface of class NFmiDataModifierSingleExtremeValuesFilter
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierAvg.h"

class _FMI_DLL NFmiDataModifierSingleExtremeValuesFilter : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierSingleExtremeValuesFilter(void);
  NFmiDataModifierSingleExtremeValuesFilter(void);
  NFmiDataModifierSingleExtremeValuesFilter(float thePointValue, float theLimit);
  void Clear(void);
  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult(void);

 private:
  NFmiDataModifierAvg itsAvgModifier;
  float itsValue;
  float itsLimit;

};  // class NFmiDataModifierSingleExtremeValuesFilter

// ======================================================================
