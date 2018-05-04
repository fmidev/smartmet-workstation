// ======================================================================
/*!
 * \file NFmiDataModifierSum.h
 * \brief Interface of class NFmiDataModifierSum
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierSum : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierSum(void);
  NFmiDataModifierSum(FmiJoinOperator theJoinOperator = kFmiAdd, bool missingValuesAlloved = true);
  NFmiDataModifierSum(const NFmiDataModifierSum& theOther);
  NFmiDataModifier* Clone(void) const;

  float Sum(void);
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

  void Clear(void);
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult(void);

 protected:
  float itsSum;

};  // class NFmiDataModifierSum

// ======================================================================
