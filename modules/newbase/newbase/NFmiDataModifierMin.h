// ======================================================================
/*!
 * \file NFmiDataModifierMin.h
 * \brief Interface of class NFmiDataModifierMin
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtreme.h"

class _FMI_DLL NFmiDataModifierMin : public NFmiDataModifierExtreme
{
 public:
  virtual ~NFmiDataModifierMin(void);
  NFmiDataModifierMin(void);
  NFmiDataModifierMin(const NFmiDataModifierMin& theOther);
  NFmiDataModifier* Clone(void) const;

  //  virtual void Calculate(float theValue);

  void Clear(void);
  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult(void);

 protected:
  virtual bool IsNewExtreme(float value);

 private:
  NFmiDataModifierMin& operator=(const NFmiDataModifierMin& theOther);

};  // class NFmiDataModifierMin

// ======================================================================
