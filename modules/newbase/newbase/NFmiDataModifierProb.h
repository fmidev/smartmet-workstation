// ======================================================================
/*!
 * \file NFmiDataModifierProb.h
 * \brief Interface of class NFmiDataModifierProb
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"
#include "NFmiGlobals.h"  // Added by ClassView

//! Undocumented

class _FMI_DLL NFmiDataModifierProb : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierProb(void);
  NFmiDataModifierProb(FmiProbabilityCondition theCondition,
                       double theFirstLimit,
                       FmiJoinOperator theJoinOperator = kFmiAdd,
                       NFmiCombinedParam* theCombinedParam = 0);
  NFmiDataModifierProb(FmiProbabilityCondition theCondition,
                       double theFirstLimit,
                       double theSecondLimit,
                       FmiJoinOperator theJoinOperator,
                       NFmiCombinedParam* theCombinedParam = 0);

  void Clear(void);
  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  void Calculate(float theValue);
  float CalculationResult(void);

 protected:
  virtual bool CheckParams(double theValue);

 protected:
  void AddCounter(bool theCondition);
  unsigned long itsTotalCounter;
  unsigned long itsCounter;
  double its1Limit;
  double its2Limit;
  FmiProbabilityCondition itsCondition;

};  // class NFmiDataModifierProb

// ======================================================================
