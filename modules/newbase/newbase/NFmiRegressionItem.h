// ======================================================================
/*!
 * \file NFmiRegressionItem.h
 * \brief Interface of class NFmiRegressionItem
 */
// ======================================================================

#pragma once

#include "NFmiInfoModifier.h"

class NFmiSuperSmartInfo;
class NFmiDataIdent;

//! Undocumented
class _FMI_DLL NFmiRegressionItem : public NFmiInfoModifier
{
 public:
  virtual ~NFmiRegressionItem();
  NFmiRegressionItem(double theCoefficient = 1.0,
                     NFmiDataIdent* theDataIdent = 0,
                     NFmiLevel* theLevel = 0,
                     NFmiQueryInfo* theData = 0);

  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(NFmiQueryInfo* theData);
  virtual void Calculate(float theValue) { NFmiInfoModifier::Calculate(theValue); }
  virtual float CalculationResult(void);
  virtual double FloatValue(void);

 protected:
  double itsReturnValue;

 private:
  double itsCoefficient;
  double itsConstant;

};  // class NFmiRegressionItem

// ======================================================================
