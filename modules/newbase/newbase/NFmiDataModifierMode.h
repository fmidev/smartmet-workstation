// ======================================================================
/*!
 * \file NFmiDataModifierMode.h
 * \brief Interface of class NFmiDataModifierMode
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiDataModifier.h"

class _FMI_DLL NFmiDataModifierMode : public NFmiDataModifier
{
 public:
  void Clear(void);

  using NFmiDataModifier::Calculate;
  using NFmiDataModifier::CalculationResult;
  virtual void Calculate(float theValue);
  virtual float CalculationResult();

  NFmiDataModifierMode(void);
  virtual ~NFmiDataModifierMode(void);
  NFmiDataModifierMode(const NFmiDataModifierMode &theOther);
  NFmiDataModifier *Clone(void) const;

 private:
  class Data
  {
   public:
    Data(void) : itsValue(kFloatMissing), itsCounter(0){};
    Data(float theValue) : itsValue(theValue), itsCounter(1){};
    void Increase(void) { itsCounter++; };
    float Value(void) const { return itsValue; };
    void Value(float theValue) { itsValue = theValue; };
    int Counter(void) const { return itsCounter; };
    bool operator==(const Data &theData) const { return itsValue == theData.itsValue; };
    bool operator<(const Data &theData) const { return itsValue < theData.itsValue; };

   private:
    float itsValue;
    int itsCounter;
  };
  checkedVector<Data> itsDataVector;
};

// ======================================================================
