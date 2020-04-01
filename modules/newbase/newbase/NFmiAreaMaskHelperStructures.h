#pragma once

#include "NFmiPoint.h"
#include "NFmiMetTime.h"

class NFmiMacroParamValue
{
 public:
  NFmiMacroParamValue();

  NFmiPoint itsLatlon;
  NFmiMetTime itsTime;
  // Tähän talletetaan laskennan lopullinen tulos
  float itsValue = kFloatMissing;
  // Poikkileikkaus laskuissa käytetään tätä painepintaa laskentapisteenä.
  float itsPressureHeight = kFloatMissing;
  // Tämä olio on käytössä vain jos tämä on true
  bool fSetValue = false;
  bool fDoCrossSectionCalculations = false;
};

class NFmiCalculationParams
{
 public:
  NFmiCalculationParams();
  NFmiCalculationParams(const NFmiPoint &theLatlon,
                        unsigned long theLocationIndex,
                        const NFmiMetTime &theTime,
                        unsigned long theTimeIndex,
                        bool crossSectionCase = false,
                        float thePressureHeight = kFloatMissing);

  NFmiPoint itsLatlon;
  unsigned long itsLocationIndex = gMissingIndex;
  NFmiMetTime itsTime;
  unsigned long itsTimeIndex = gMissingIndex;
  // Joskus pitää tietää että kyse on poikkileikkaus laskuista
  bool fCrossSectionCase = false;
  // Silloin (poikkileikkauslaskuissa) käytetään tätä painepintaa laskentapisteenä
  // Tein siitä mutablen, jotta ei tarvitse muuttaa niin monissa kohtaa const parametria ei const:iksi.
  mutable float itsPressureHeight = kFloatMissing;
};
