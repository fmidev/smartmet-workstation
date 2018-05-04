// ======================================================================
/*!
 * \file NFmiDataModifierBase.h
 * \brief Interface of class NFmiDataModifierBase
 */
// ======================================================================

#pragma once

#include "NFmiString.h"  // Added by ClassView

//! Undocumented
enum FmiProbabilityCondition
{
  kValueLessThanLimit,
  kValueGreaterThanLimit,
  kValueLessOrEqualThanLimit,
  kValueGreaterOrEqualThanLimit,
  kValueBetweenLimits,
  kValueOutOfLimits,
  kValueIsEqual,
  kNoCondition
};

//! Undocumented
enum FmiJoinOperator
{
  kFmiVal,
  kFmiAdd,
  kFmiSub,
  kFmiMul,
  kFmiDiv,
  kFmiPow,
  kFmiAnd,
  kFmiOr,
  kFmiNot
};

//! Undocumented
class _FMI_DLL NFmiDataModifierBase
{
 public:
  virtual ~NFmiDataModifierBase(void);
  NFmiDataModifierBase(const NFmiDataModifierBase& other);
  NFmiDataModifierBase(FmiJoinOperator theJoinOperator = kFmiAdd);

  FmiJoinOperator JoinOperator(void);

  virtual std::ostream& Write(std::ostream& file);
  virtual std::ostream& WriteOperator(std::ostream& file) const;
  virtual std::ostream& WriteOperand(std::ostream& file) const;
  virtual std::ostream& WriteExpressionStart(std::ostream& file) const;
  virtual std::ostream& WriteExpressionBody(std::ostream& file);
  virtual std::ostream& WriteExpressionEnd(std::ostream& file) const;

 protected:
  FmiJoinOperator itsJoinOperator;
  NFmiString itsExpressionType;

};  // class NFmiDataModifierBase

// ======================================================================
