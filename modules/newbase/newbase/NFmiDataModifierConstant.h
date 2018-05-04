// ======================================================================
/*!
 * \file NFmiDataModifierConstant.h
 * \brief Interface of class NFmiDataModifierConstant
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

//! Undocumented
class _FMI_DLL NFmiDataModifierConstant : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierConstant();
  NFmiDataModifierConstant(double theConstant, FmiJoinOperator theJoinOperator = kFmiAdd);
  virtual double FloatValue(void);
  virtual std::ostream& WriteOperand(std::ostream& file) const;

 protected:
  double itsConstant;

};  // class NFmiDataModifierConstant

// ======================================================================
