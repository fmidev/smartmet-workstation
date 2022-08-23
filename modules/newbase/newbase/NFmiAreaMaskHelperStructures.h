#pragma once

#include "NFmiPoint.h"
#include "NFmiMetTime.h"

class NFmiFastQueryInfo;

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
  // Jos laskuissa on käytetty CalculationPoint = synop tyyliin laskenta pisteitä, tähän talletetaan sen originaali 
  // latlon pisteen pointteri, jota sitten voidaan käyttää tarkemmissa etäisyyslaskuissa, sen sijaan että käytettäisiin
  // laskentahilan lähimmän pisteen koordinaatteja. Tämä estää tulosten satunnaisen oloisen vaihtelun, kun laskenta hila 
  // muuttaa ja etäisyys perusteiset laskennan tulokset muuttuvat (esim. etäisyydet occurance laskentojen hakusäteen suhteen).
  const NFmiPoint *itsActualCalculationPoint = nullptr;
  // Synop datoilla voi olla monia eri datoja (esim. Suomessa suomi+euro+maailma datat).
  // Ainakin SimpleCondition laskuihin näitä olisi muuten vaikea saada käyttöön, joten tässä on 'kuljetus' laskenta paikalle.
  mutable NFmiFastQueryInfo *itsCurrentMultiInfoData = nullptr;
  // Joskus havaintoasemien maksimi sallittua etäisyyttä laskentapisteeseen halutaan
  // rajoittaa smarttool kielessä halutulla km määrällä "observationradius = 5", 
  // tällöin jos asema on kauempana, sitä ei käytetä ollenkaan kyseiselle laskenta pisteelle.
  float itsObservationRadiusInKm = kFloatMissing;
};
