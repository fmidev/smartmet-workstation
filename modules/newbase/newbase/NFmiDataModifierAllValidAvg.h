// ======================================================================
/*!
 * \file NFmiDataModifierAllValidAvg.h
 * \brief Interface of class NFmiDataModifierAllValidAvg
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierAvg.h"

class _FMI_DLL NFmiDataModifierAllValidAvg : public NFmiDataModifierAvg
{
 public:
  virtual ~NFmiDataModifierAllValidAvg(void);
  NFmiDataModifierAllValidAvg(void);
  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

 private:
  NFmiDataModifierAllValidAvg& operator=(const NFmiDataModifierAllValidAvg& theOther);

};  // class NFmiDataModifierAllValidAvg

// ======================================================================
