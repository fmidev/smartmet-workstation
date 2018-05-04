// ======================================================================
/*!
 * \file NFmiDataModifierDummy.h
 * \brief Interface of class NFmiDataModifierDummy
 */
// ======================================================================

#pragma once

#include "NFmiDataModifier.h"

// tallettaa vain arvon. Voidaan käyttää windChill-modifierista jos ei
// mitään varsinaista modifieria/Lasse

//! Undocumented
class _FMI_DLL NFmiDataModifierDummy : public NFmiDataModifier
{
 public:
  virtual ~NFmiDataModifierDummy(void);
  NFmiDataModifierDummy(void);
  using NFmiDataModifier::Calculate;
  virtual void Calculate(float theValue);

  // Lasse ei kutsuta mistään, aikaisemmat kutsut QI::CalcInterpolatedTimeData ja
  // QI::CalcTimeData:sta jouduttu poistamaan

  virtual void Calculate(NFmiQueryInfo* theQI);
  using NFmiDataModifier::CalculationResult;
  float CalculationResult(void);
  virtual void Clear(void);

 protected:
  float itsValue;

};  // class NFmiDataModifierDummy

// ======================================================================
