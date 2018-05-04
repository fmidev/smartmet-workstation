#pragma once
//**********************************************************
// C++ Class Name : NFmiCalculationConstantValue
// ---------------------------------------------------
//  Author         : pietarin
//  Creation Date  : 9.11. 2010
//
//**********************************************************

#include <newbase/NFmiAreaMaskImpl.h>
#include <newbase/NFmiInfoAreaMask.h>
#include <boost/shared_ptr.hpp>

class NFmiDataModifier;
class NFmiDataIterator;
class NFmiFastQueryInfo;

// Tämä luokka antaa laskuihin mukaan ajan mukana muuttuvat kertoimet. Aikasarja-editorista huom!
class NFmiCalculationConstantValue : public NFmiAreaMaskImpl
{
 public:
  double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways);

  NFmiCalculationConstantValue(double value = 0);
  ~NFmiCalculationConstantValue();
  NFmiCalculationConstantValue(const NFmiCalculationConstantValue &theOther);
  NFmiAreaMask *Clone(void) const;

  void SetValue(double value) { itsValue = value; }
  double GetValue(void) const { return itsValue; }
 private:
  double itsValue;
};

// Tämä luokka antaa laskuihin mukaan korkeus laskuissa (esim. SumZ tai MinH tyyliset funktiot)
// käytetyn kerroksen paksuuden. Korkeus laskut jakavat laskuissaan korkeus akselin vaihtuvan
// paksuisiin kerroksiin siten että alhaalla on tarkempi resoluutio kuin ylhäällä. Tätä voi käyttää
// vain ed. mainittujen korkeus laskujen yhteydessä.
class NFmiCalculationDeltaZValue : public NFmiAreaMaskImpl
{
 public:
  double Value(const NFmiCalculationParams & /* theCalculationParams */,
               bool /* fUseTimeInterpolationAlways */)
  {
    return NFmiCalculationDeltaZValue::itsHeightValue;
  }

  NFmiCalculationDeltaZValue(void);
  ~NFmiCalculationDeltaZValue(){};
  NFmiCalculationDeltaZValue(const NFmiCalculationDeltaZValue &theOther);
  NFmiAreaMask *Clone(void) const;

  // tätä funktiota käyttämällä asetetaan korkeus 'siivun' paksuus. HUOM! se on staattinen kuten on
  // itsHeightValue-dataosakin, joten se tulee kaikille 'DeltaZ':oille yhteiseksi arvoksi.
  static void SetDeltaZValue(double value) { itsHeightValue = value; }
 private:
  static double itsHeightValue;
};

// Tämä on taas pika suunnittelua/toteutusta, mutta laitoin tälläisen luokan
// jonka avulla hoidetaan smarttools laskuissa mm. sulut ja lasku- ja vertailu operaattorit.
// Kun laitoin uuden smarttool systeemin laskemaan rekursiivisesti laskut ja
// kaikki (muuttujat, operatorit, sulut jne.) pitää laittaa samaan putkeen.
class NFmiCalculationSpecialCase : public NFmiAreaMaskImpl
{
 public:
  NFmiCalculationSpecialCase(NFmiAreaMask::CalculationOperator theValue = NotOperation);
  ~NFmiCalculationSpecialCase(void){};
  NFmiCalculationSpecialCase(const NFmiCalculationSpecialCase &theOther);
  NFmiAreaMask *Clone(void) const;

 private:
};

// Ramppifunktioiden laskut.
class NFmiCalculationRampFuction : public NFmiInfoAreaMask
{
 public:
  double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways);

  NFmiCalculationRampFuction(const NFmiCalculationCondition &theOperation,
                             Type theMaskType,
                             NFmiInfoData::Type theDataType,
                             boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                             BinaryOperator thePostBinaryOperator = kNoValue);
  ~NFmiCalculationRampFuction(void);
  NFmiCalculationRampFuction(const NFmiCalculationRampFuction &theOther);
  NFmiAreaMask *Clone(void) const;

 private:
};

// Ramppifunktioiden laskut AreaMask:ien avulla (mm. lat, lon ja elevationangle tapaukset).
class NFmiCalculationRampFuctionWithAreaMask : public NFmiAreaMaskImpl
{
 public:
  double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways);

  NFmiCalculationRampFuctionWithAreaMask(const NFmiCalculationCondition &theOperation,
                                         Type theMaskType,
                                         NFmiInfoData::Type theDataType,
                                         boost::shared_ptr<NFmiAreaMask> &theAreaMask,
                                         BinaryOperator thePostBinaryOperator = kNoValue);
  ~NFmiCalculationRampFuctionWithAreaMask(void);
  NFmiCalculationRampFuctionWithAreaMask(const NFmiCalculationRampFuctionWithAreaMask &theOther);
  NFmiAreaMask *Clone(void) const;

 private:
  boost::shared_ptr<NFmiAreaMask> itsAreaMask;
  bool fIsTimeIntepolationNeededInValue;  // erikois optimointia Value-metodin ja Time-metodin
                                          // käytössä
};

// Avg, min ja max funktioiden laskut.
class NFmiCalculationIntegrationFuction : public NFmiInfoAreaMask
{
 public:
  double Value(const NFmiCalculationParams &theCalculationParams, bool fUseTimeInterpolationAlways);

  NFmiCalculationIntegrationFuction(boost::shared_ptr<NFmiDataIterator> &theDataIterator,
                                    boost::shared_ptr<NFmiDataModifier> &theDataModifier,
                                    Type theMaskType,
                                    NFmiInfoData::Type theDataType,
                                    boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
  ~NFmiCalculationIntegrationFuction(void);

 private:
  NFmiCalculationIntegrationFuction(const NFmiCalculationIntegrationFuction &theOther);

  boost::shared_ptr<NFmiDataModifier> itsDataModifier;
  boost::shared_ptr<NFmiDataIterator> itsDataIterator;
};

