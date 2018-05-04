// ======================================================================
/*!
 * \file NFmiDataModifierAllValidSum.h
 * \brief Interface of class NFmiDataModifierAllValidSum
 */
// ======================================================================

#pragma once

#include "NFmiDataModifierSum.h"

class _FMI_DLL NFmiDataModifierAllValidSum : public NFmiDataModifierSum
{
 public:
  virtual ~NFmiDataModifierAllValidSum(void);
  NFmiDataModifierAllValidSum(void);

  virtual void Calculate(float theValue);
  virtual void Calculate(NFmiQueryInfo* theQI);

  void Clear(void);

 protected:
  long itsCounter;

 private:
  NFmiDataModifierAllValidSum& operator=(const NFmiDataModifierAllValidSum& theOther);

};  // class NFmiDataModifierAllValidSum

// ======================================================================
