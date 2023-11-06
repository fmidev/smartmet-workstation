#pragma once
//**********************************************************
// C++ Class Name : NFmiAreaMaskInfo
// ---------------------------------------------------------
//  Author         : pietarin
//  Creation Date  : 8.11. 2010
//
//**********************************************************

#include "NFmiSoundingIndexCalculator.h"
#include <newbase/NFmiAreaMask.h>
#include <newbase/NFmiCalculationCondition.h>
#include <newbase/NFmiDataIdent.h>
#include <newbase/NFmiPoint.h>

class NFmiLevel;
class NFmiSimpleConditionInfo;

class NFmiAreaMaskInfo
{
 public:
  NFmiAreaMaskInfo(const std::string& theOrigLineText = "");
  NFmiAreaMaskInfo(const NFmiAreaMaskInfo& theOther);
  ~NFmiAreaMaskInfo();

  void SetDataIdent(const NFmiDataIdent& value) { itsDataIdent = value; }
  const NFmiDataIdent& GetDataIdent() const { return itsDataIdent; }
  void SetUseDefaultProducer(bool value) { fUseDefaultProducer = value; }
  bool GetUseDefaultProducer() const { return fUseDefaultProducer; }
  void SetMaskCondition(const NFmiCalculationCondition& value) { itsMaskCondition = value; }
  const NFmiCalculationCondition& GetMaskCondition() const { return itsMaskCondition; }
  NFmiAreaMask::CalculationOperationType GetOperationType() const { return itsOperationType; }
  void SetOperationType(NFmiAreaMask::CalculationOperationType newValue)
  {
    itsOperationType = newValue;
  }
  NFmiAreaMask::CalculationOperator GetCalculationOperator() const
  {
    return itsCalculationOperator;
  }
  void SetCalculationOperator(NFmiAreaMask::CalculationOperator newValue)
  {
    itsCalculationOperator = newValue;
  }
  NFmiAreaMask::BinaryOperator GetBinaryOperator() const { return itsBinaryOperator; }
  void SetBinaryOperator(NFmiAreaMask::BinaryOperator theValue) { itsBinaryOperator = theValue; }
  NFmiInfoData::Type GetDataType() const { return itsDataType; }
  void SetDataType(NFmiInfoData::Type newValue) { itsDataType = newValue; }
  NFmiLevel* GetLevel() const { return itsLevel; }
  void SetLevel(NFmiLevel* theLevel);
  const std::string& GetMaskText() const { return itsMaskText; }
  void SetMaskText(const std::string& theText) { itsMaskText = theText; }
  const std::string& GetOrigLineText() const { return itsOrigLineText; }
  void SetOrigLineText(const std::string& theText) { itsOrigLineText = theText; }
  NFmiAreaMask::FunctionType GetFunctionType() const { return itsFunctionType; }
  void SetFunctionType(NFmiAreaMask::FunctionType newType) { itsFunctionType = newType; }
  NFmiAreaMask::FunctionType GetSecondaryFunctionType() const { return itsSecondaryFunctionType; }
  void SetSecondaryFunctionType(NFmiAreaMask::FunctionType newType)
  {
    itsSecondaryFunctionType = newType;
  }
  NFmiAreaMask::MetFunctionDirection MetFunctionDirection() const
  {
    return itsMetFunctionDirection;
  }
  void MetFunctionDirection(NFmiAreaMask::MetFunctionDirection newValue)
  {
    itsMetFunctionDirection = newValue;
  }
  const NFmiPoint& GetOffsetPoint1() const { return itsOffsetPoint1; }
  void SetOffsetPoint1(const NFmiPoint& newValue) { itsOffsetPoint1 = newValue; }
  const NFmiPoint& GetOffsetPoint2() const { return itsOffsetPoint2; }
  void SetOffsetPoint2(const NFmiPoint& newValue) { itsOffsetPoint2 = newValue; }
  NFmiAreaMask::MathFunctionType GetMathFunctionType() const { return itsMathFunctionType; };
  void SetMathFunctionType(NFmiAreaMask::MathFunctionType newValue)
  {
    itsMathFunctionType = newValue;
  };
  int IntegrationFunctionType() const { return itsIntegrationFunctionType; }
  void IntegrationFunctionType(int newValue) { itsIntegrationFunctionType = newValue; }
  int FunctionArgumentCount() const { return itsFunctionArgumentCount; }
  void FunctionArgumentCount(int newValue) { itsFunctionArgumentCount = newValue; }
  FmiSoundingParameters SoundingParameter() const { return itsSoundingParameter; }
  void SoundingParameter(FmiSoundingParameters newValue) { itsSoundingParameter = newValue; }
  int ModelRunIndex() const { return itsModelRunIndex; }
  void ModelRunIndex(int newValue) { itsModelRunIndex = newValue; }
  bool AllowSimpleCondition() const;
  NFmiAreaMask::SimpleConditionRule SimpleConditionRule() const { return itsSimpleConditionRule; }
  void SimpleConditionRule(NFmiAreaMask::SimpleConditionRule newValue)
  {
    itsSimpleConditionRule = newValue;
  }
  boost::shared_ptr<NFmiSimpleConditionInfo> SimpleConditionInfo() const
  {
    return itsSimpleConditionInfo;
  }
  void SimpleConditionInfo(boost::shared_ptr<NFmiSimpleConditionInfo>& theSimpleConditionInfo);
  float TimeOffsetInHours() const { return itsTimeOffsetInHours; }
  void TimeOffsetInHours(float newValue) { itsTimeOffsetInHours = newValue; }
  void SetSecondaryParam(const NFmiDataIdent& value) { itsSecondaryParam = value; }
  const NFmiDataIdent& GetSecondaryParam() const { return itsSecondaryParam; }
  NFmiLevel* GetSecondaryParamLevel() const { return itsSecondaryParamLevel; }
  void SetSecondaryParamLevel(NFmiLevel* theLevel);
  NFmiInfoData::Type GetSecondaryParamDataType() const { return itsSecondaryParamDataType; }
  void SetSecondaryParamDataType(NFmiInfoData::Type newValue)
  {
    itsSecondaryParamDataType = newValue;
  }
  bool GetSecondaryParamUseDefaultProducer() const { return fSecondaryParamUseDefaultProducer; }
  void SetSecondaryParamUseDefaultProducer(bool newValue)
  {
    fSecondaryParamUseDefaultProducer = newValue;
  }

 private:
  NFmiDataIdent itsDataIdent;
  bool fUseDefaultProducer;
  NFmiCalculationCondition itsMaskCondition;
  NFmiAreaMask::CalculationOperationType itsOperationType;  // tämä menee päällekkäin erilaisten
                                                            // maski ja info tyyppien kanssa, mutta
                                                            // piti tehdä
  // smarttooleja varten vielä tämmöinen mm. hoitamaan laskujärjestyksiä ja sulkuja jne.
  NFmiAreaMask::CalculationOperator
      itsCalculationOperator;  // jos operation, tässä tieto mistä niistä on kyse esim. +, -, * jne.
  NFmiAreaMask::BinaryOperator itsBinaryOperator;
  NFmiInfoData::Type itsDataType;  // jos kyseessä infoVariable, tarvitaan vielä datan tyyppi, että
                                   // parametri saadaan tietokannasta (=infoOrganizerista)
  NFmiLevel* itsLevel;  // mahd. level tieto, omistaa ja tuhoaa
  std::string itsMaskText;  // originaali teksti, mistä tämä maskinfo on tulkittu, tämä on siis vain
                            // yksi sana tai luku
  std::string itsOrigLineText;  // originaali koko rivin teksti, mistä tämä currentti sana
                                // (itsMaskText) on otettu (tätä käytetään virhe teksteissä)
  NFmiAreaMask::FunctionType itsFunctionType;  // onko mahd. funktio esim. min, max jne. (ei
                                               // matemaattisia funktioita kuten sin, cos, pow,
                                               // jne.)
  NFmiAreaMask::FunctionType itsSecondaryFunctionType;  // Tähän laitetaan mm. vertikaali
  // funktioissa käytetty korkeus tyyppi esim.
  // VertP tai VertZ
  NFmiAreaMask::MetFunctionDirection itsMetFunctionDirection;  // grad, adv, div rot ja lap
                                                               // -funktioille (ja näiden
                                                               // 2-versioille) määrätään myös
                                                               // suunta, joka voi olla X, Y tai
                                                               // molemmat
  NFmiPoint itsOffsetPoint1;  // esim. aikaoffset (x alku ja y loppu) tai paikkaoffset (alku x ja y
                              // offset)
  NFmiPoint itsOffsetPoint2;  // paikkaoffset (loppu x ja y offset)
  NFmiAreaMask::MathFunctionType itsMathFunctionType;
  int itsIntegrationFunctionType;  // 1=SumT tyylinen ja 2=SumZ tyylinen ja 3=MinH tyylinen funktio
  int itsFunctionArgumentCount;    // kuinka monta pilkulla eroteltua argumenttia on odotettavissa
  // tähän 'meteorologiseen' funktioon (mm. grad, adv, div, lap, rot
  // jne....).
  FmiSoundingParameters itsSoundingParameter;
  int itsModelRunIndex;
  // Onko jollekin areaMask funktiolla sallittua tai pakollista olla ns. SimpleCondition ehto
  // lauseke, oletuksena ei.
  NFmiAreaMask::SimpleConditionRule itsSimpleConditionRule =
      NFmiAreaMask::SimpleConditionRule::NotAllowed;
  // Tietyillä funktioilla voi olla simple-condition-info osio, joka talletetaan tähän
  boost::shared_ptr<NFmiSimpleConditionInfo> itsSimpleConditionInfo;
  float itsTimeOffsetInHours = 0;
  // Jos laskuissa on käytössä sekundaari parametri, tässä on sen par+prod+level tiedot
  NFmiDataIdent itsSecondaryParam;
  NFmiLevel* itsSecondaryParamLevel = nullptr;  // mahd. level tieto, omistaa ja tuhoaa
  NFmiInfoData::Type itsSecondaryParamDataType = NFmiInfoData::kNoDataType;
  bool fSecondaryParamUseDefaultProducer = true;
};
