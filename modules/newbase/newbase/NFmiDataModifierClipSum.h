//************** NFmiDataModifierClipSum ************************
// 19.9.2000/Lasse; puolittaa alku- ja loppuhetken arvot
//   soveltuu esim sadesumman llskemiseen kun lähdetään hetkellisistä intensiteeteistä
// AllValid= vaatii että kaikki arvot ovat käypiä

#pragma once

#include "NFmiDataModifierAllValidSum.h"

class _FMI_DLL NFmiDataModifierAllValidClipSum : public NFmiDataModifierAllValidSum
{
 public:
  using NFmiDataModifierAllValidSum::Calculate;

  virtual void Calculate(NFmiQueryInfo* theQI)
  {
    // PITÄIS TARKISTAA VOIKO TOIMIA NÄIN ja tässähän ei ole arvovälin tsekkausta
    float value = theQI->FloatValue();
    float halfValue;
    if (CheckMissingValues(value))
    {
      halfValue = value * .5;
      if (itsCounter == 0) fCalculationResultOk = true;
      itsSum += halfValue + itsHalfValue;
    }
    else
    {
      fCalculationResultOk = false;
    }
    itsHalfValue = halfValue;
    itsCounter++;
  };

  NFmiDataModifierAllValidClipSum(void) : NFmiDataModifierAllValidSum() { itsHalfValue = 0.; };
  virtual ~NFmiDataModifierAllValidClipSum(void){};

 private:
  float itsHalfValue;
};

//************** NFmiDataModifierAllValidsClipSum ************************
