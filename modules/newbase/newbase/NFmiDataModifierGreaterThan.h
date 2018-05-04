// ======================================================================
/*!
 * \file NFmiDataModifierGreaterThan.h
 * \brief Interface of class NFmiDataModifierGreaterThan
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierGreaterThan : public NFmiDataModifier
{
 public:
  NFmiDataModifierGreaterThan(float theValue = kFloatMissing);
  virtual bool BoolOperation(float theValue);

 private:
  double itsLimit;

};  // class NFmiDataModifierGreaterThan

// ======================================================================
