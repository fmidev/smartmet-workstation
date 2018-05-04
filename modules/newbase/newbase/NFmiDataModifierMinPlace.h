// ======================================================================
/*!
 * \file NFmiDataModifierMinPlace.h
 * \brief Interface of class NFmiDataModifierMinPlace
 */
// ======================================================================
/*!
 * \class NFmiDataModifierMinPlace
 *
 * Undocumented
 *
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierExtremePlace.h"

//! Undocumented
class _FMI_DLL NFmiDataModifierMinPlace : public NFmiDataModifierExtremePlace
{
 public:
  virtual ~NFmiDataModifierMinPlace(void);
  NFmiDataModifierMinPlace(void);

  void Clear(void);

  using NFmiDataModifier::CalculationResult;
  virtual float CalculationResult(void);

 protected:
  virtual bool IsNewExtreme(float value);

 private:
  NFmiDataModifierMinPlace& operator=(const NFmiDataModifierMinPlace& theOther);

};  // class NFmiDataModifierMinPlace

// ======================================================================
