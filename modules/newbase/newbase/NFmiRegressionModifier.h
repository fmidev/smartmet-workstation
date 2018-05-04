// ======================================================================
/*!
 * \file NFmiRegressionModifier.h
 * \brief Interface of class NFmiRegressionModifier
 */
// ======================================================================

#pragma once

#include "NFmiInfoModifier.h"

//! Undocumented
class _FMI_DLL NFmiRegressionModifier : public NFmiInfoModifier
{
 public:
  virtual ~NFmiRegressionModifier();
  NFmiRegressionModifier(NFmiDataIdent* theParam, NFmiLevel* theLevel, NFmiQueryInfo* theData = 0);

  virtual std::ostream& WriteOperand(std::ostream& file) const;
  virtual double FloatValue(void);

 protected:
  NFmiDataModifierList* itsRegressionItems;

 private:
  NFmiRegressionModifier(const NFmiRegressionModifier& theMod);
  NFmiRegressionModifier& operator=(const NFmiRegressionModifier& theMod);

};  // class NFmiRegressionModifier

// ======================================================================
