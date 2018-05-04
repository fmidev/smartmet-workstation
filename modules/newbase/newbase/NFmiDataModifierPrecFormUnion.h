// ======================================================================
/*!
 * \file NFmiDataModifierPrecFormUnion.h
 * \brief Interface of class NFmiDataModifierPrecFormUnion
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiWeatherAndCloudiness.h"

class _FMI_DLL NFmiDataModifierPrecFormUnion : public NFmiDataModifier
{
  // HUOM missingValuesAlloved = true koska pouta tuo puuttuvan !!!

 public:
  virtual ~NFmiDataModifierPrecFormUnion(void);
  NFmiDataModifierPrecFormUnion(FmiJoinOperator theJoinOperator = kFmiAdd,
                                bool missingValuesAlloved = true);

  void Clear(void);
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult(void);
  float Result(void);

 private:
  bool fIsRain;
  bool fIsSleet;
  bool fIsSnow;
  bool fIsFreezing;

};  // class NFmiDataModifierPrecFormUnion

// ======================================================================
