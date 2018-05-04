// ======================================================================
/*!
 * \file NFmiDataModifierReplace.h
 * \brief Interface of class NFmiDataModifierReplace
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierReplace : public NFmiDataModifier
{
 public:
  NFmiDataModifierReplace(float theOrigValue, float theNewValue);

  float FloatOperation(float theValue);

 private:
  float itsOriginalValue;
  float itsNewValue;

};  // class NFmiDataModifierReplace

// ======================================================================
