// ======================================================================
/*!
 * \file NFmiSoundingDataOpt1.h
 *
 * Apuluokka laskemaan ja tutkimaan luotaus dataa. Osaa täyttää itsensä
 * mm. mallipinta QueryDatasta (infosta).
 */
// ======================================================================

#pragma once

#include "NFmiSoundingData.h"

#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiLocation.h>
#include <newbase/NFmiParameterName.h>
#include <newbase/NFmiQueryDataUtil.h>
#include <newbase/NFmiFastInfoUtils.h>

#include <deque>
#include <unordered_map>

class NFmiFastQueryInfo;

class NFmiSoundingDataOpt1
{
 public:
  class LFCIndexCache
  {
   public:
    LFCIndexCache(void)
        : itsSurfaceValue(kFloatMissing),
          itsSurfaceELValue(kFloatMissing),
          its500mValue(kFloatMissing),
          its500mELValue(kFloatMissing),
          its500m2Value(kFloatMissing),
          its500m2ELValue(kFloatMissing),
          itsMostUnstableValue(kFloatMissing),
          itsMostUnstableELValue(kFloatMissing),
          fSurfaceValueInitialized(false),
          f500mValueInitialized(false),
          f500m2ValueInitialized(false),
          fMostUnstableValueInitialized(false)
    {
    }

    void Clear(void) { *this = LFCIndexCache(); }
    double itsSurfaceValue;
    double itsSurfaceELValue;
    double its500mValue;
    double its500mELValue;
    double its500m2Value;
    double its500m2ELValue;
    double itsMostUnstableValue;
    double itsMostUnstableELValue;
    bool fSurfaceValueInitialized;
    bool f500mValueInitialized;
    bool f500m2ValueInitialized;
    bool fMostUnstableValueInitialized;
  };

  NFmiSoundingDataOpt1();

  // TODO Fill-metodeille pitää laittaa haluttu parametri-lista parametriksi (jolla täytetään sitten
  // dynaamisesti NFmiDataMatrix-otus)
  bool FillSoundingData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                        const NFmiMetTime &theTime,
                        const NFmiMetTime &theOriginTime,
                        const NFmiLocation &theLocation,
                        int useStationIdOnly = false);
  bool FillSoundingData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                        const NFmiMetTime &theTime,
                        const NFmiMetTime &theOriginTime,
                        const NFmiPoint &theLatlon,
                        const NFmiString &theName,
                        const boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo,
                        bool useFastFill = false);
  bool FillSoundingData(const std::vector<FmiParameterName> &parametersInServerData, const std::string &theServerDataAsciiFormat,
      const NFmiMetTime &theTime,
      const NFmiMetTime &theOriginTime,
      const NFmiLocation &theLocation,
      const boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo);
  void CutEmptyData(void);  // tämä leikkaa Fill.. -metodeissa laskettuja data vektoreita niin että
                            // pelkät puuttuvat kerrokset otetaan pois
  static bool HasRealSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theSoundingLevelInfo);
  bool IsDataGood();

  // FillSoundingData-metodeilla täytetään kunkin parametrin vektorit ja tällä saa haluamansa
  // parametrin vektorin käyttöön
  std::deque<float> &GetParamData(FmiParameterName theId);
  const NFmiLocation &Location(void) const { return itsLocation; }
  void Location(const NFmiLocation &newValue) { itsLocation = newValue; }
  const NFmiMetTime &Time(void) const { return itsTime; }
  const NFmiMetTime &OriginTime(void) const { return itsOriginTime; }
  bool GetValuesStartingLookingFromPressureLevel(double &T, double &Td, double &P);
  float GetValueAtPressure(FmiParameterName theId, float P);
  float GetValueAtHeight(FmiParameterName theId, float H);
  bool CalcLCLAvgValues(
      double fromZ, double toZ, double &T, double &Td, double &P, bool fUsePotTandMix);
  bool CalcAvgWindComponentValues(double fromZ, double toZ, double &u, double &v);
  bool ObservationData(void) const { return fObservationData; }
  bool GetLowestNonMissingValues(float &H, float &U, float &V);
  float ZeroHeight(void) const { return itsZeroHeight; }
  int ZeroHeightIndex(void) const { return itsZeroHeightIndex; }
  bool IsSameSounding(const NFmiSoundingDataOpt1 &theOtherSounding);
  bool GetTandTdValuesFromNearestPressureLevel(double P,
                                               double &theFoundP,
                                               double &theT,
                                               double &theTd);
  bool SetValueToPressureLevel(float P, float theParamValue, FmiParameterName theId);
  bool FindHighestThetaE(double &T, double &Td, double &P, double &theMaxThetaE, double theMinP);
  float FindPressureWhereHighestValue(FmiParameterName theId, float theMaxP, float theMinP);
  bool ModifyT2DryAdiapaticBelowGivenP(double P, double T);
  bool ModifyTd2MoistAdiapaticBelowGivenP(double P, double Td);
  bool ModifyTd2MixingRatioBelowGivenP(double P, double T, double Td);
  bool Add2ParamAtNearestP(float P,
                           FmiParameterName parId,
                           float addValue,
                           float minValue,
                           float maxValue,
                           bool fCircularValue);
  void UpdateUandVParams(void);
  bool PressureDataAvailable(void) const { return fPressureDataAvailable; }
  bool HeightDataAvailable(void) const { return fHeightDataAvailable; }
  void SetTandTdSurfaceValues(float T, float Td);

  double CalcSHOWIndex(void);
  double CalcLIFTIndex(void);
  double CalcKINXIndex(void);
  double CalcCTOTIndex(void);
  double CalcVTOTIndex(void);
  double CalcTOTLIndex(void);
  double CalcLCLPressureLevel(FmiLCLCalcType theLCLCalcType);
  double CalcLCLIndex(FmiLCLCalcType theLCLCalcType);
  double CalcLCLHeightIndex(FmiLCLCalcType theLCLCalcType);
  //	double CalcLFCIndex(FmiLCLCalcType theLCLCalcType, double &EL);
  double CalcLFCIndex(FmiLCLCalcType theLCLCalcType, double &EL);
  double CalcLFCHeightIndex(FmiLCLCalcType theLCLCalcType, double &ELheigth);
  double CalcCAPE500Index(FmiLCLCalcType theLCLCalcType, double theHeightLimit = kFloatMissing);
  double CalcCAPE_TT_Index(FmiLCLCalcType theLCLCalcType, double Thigh, double Tlow);
  double CalcCINIndex(FmiLCLCalcType theLCLCalcType);
  double CalcBulkShearIndex(double startH, double endH);
  double CalcSRHIndex(double startH, double endH);
  double CalcThetaEDiffIndex(double startH, double endH);
  double CalcWSatHeightIndex(double theH);
  double CalcGDI();

  bool GetValuesNeededInLCLCalculations(FmiLCLCalcType theLCLCalcType,
                                        double &T,
                                        double &Td,
                                        double &P);
  NFmiString Get_U_V_ID_IndexText(const NFmiString &theText, FmiDirection theStormDirection);
  void Calc_U_and_V_IDs_left(double &u_ID, double &v_ID);
  void Calc_U_and_V_IDs_right(double &u_ID, double &v_ID);
  void Calc_U_and_V_mean_0_6km(double &u0_6, double &v0_6);
  double CalcWindBulkShearComponent(double startH, double endH, FmiParameterName theParId);
  double CalcBulkShearIndex(double startH, double endH, FmiParameterName theParId);
  void Calc_U_V_helpers(double &shr_0_6_u_n, double &shr_0_6_v_n, double &u0_6, double &v0_6);
  double CalcTOfLiftedAirParcel(double T, double Td, double fromP, double toP);

  bool GetTrValues(double &theTMinValue, double &theTMinPressure);
  bool GetMwValues(double &theMaxWsValue, double &theMaxWsPressure);
  bool MovingSounding() const { return fMovingSounding; }

 private:
  bool CheckLFCIndexCache(FmiLCLCalcType theLCLCalcTypeIn,
                          double &theLfcIndexValueOut,
                          double &theELValueOut);
  void FillLFCIndexCache(FmiLCLCalcType theLCLCalcType, double theLfcIndexValue, double theELValue);
  void FixPressureDataSoundingWithGroundData(
      const boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo);
  unsigned long GetHighestNonMissingValueLevelIndex(FmiParameterName theParaId);
  unsigned long GetLowestNonMissingValueLevelIndex(FmiParameterName theParaId);
  bool CheckForMissingLowLevelData(FmiParameterName theParaId, unsigned long theMissingIndexLimit);
  float GetPressureAtHeight(double H);
  void ClearDatas(void);
  bool FillParamData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                     FmiParameterName theId,
                     NFmiQueryDataUtil::SignificantSoundingLevels &theSoungingLevels);
  bool FillParamData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                     FmiParameterName theId,
                     const NFmiMetTime &theTime,
                     const NFmiPoint &theLatlon);
  bool FastFillParamData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         FmiParameterName theId);
  void FillWindData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
      NFmiQueryDataUtil::SignificantSoundingLevels &theSignificantSoundingLevels);
  void FillWindData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
      const NFmiMetTime &theTime,
      const NFmiPoint &theLatlon);
  void FastFillWindData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
  void FillRestOfWindData(NFmiFastInfoUtils::MetaWindParamUsage &metaWindParamUsage);
  void InitZeroHeight(void);  // tätä kutsutaan FillParamData-metodeista
  void CalculateHumidityData(void);
  std::string MakeCacheString(double T, double Td, double fromP, double toP);
  bool FillHeightDataFromLevels(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
  void SetVerticalParamStatus(void);
  void DoAfterFillChecks(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         std::deque<float> &data,
                         FmiParameterName theId);
  bool LookForFilledParamFromInfo(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                  FmiParameterName theId);
  std::deque<float> &GetResizedParamData(
      const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
      FmiParameterName theId,
      NFmiQueryDataUtil::SignificantSoundingLevels &theSoungingLevels);
  void FillParamDataNormally(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                             std::deque<float> &data);
  void FillParamDataFromSignificantLevels(
      const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
      std::deque<float> &data,
      NFmiQueryDataUtil::SignificantSoundingLevels &significantLevels);
  void MakeReverseLevelsChecks(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
  void MakeFillDataPostChecks(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo, const boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo = nullptr);
  void MakeFillDataPostChecksForServerData(const boost::shared_ptr<NFmiFastQueryInfo> &theGroundDataInfo);
  void FillMissingServerData();
  void SetServerDataFromGroundLevelUp();
  void ReverseAllData();

  NFmiLocation itsLocation;
  NFmiMetTime itsTime;
  NFmiMetTime itsOriginTime;  // tämä otetaan talteen IsSameSounding-metodia varten

  enum ParamDataIndex
  {
      kTemperatureIndex = 0,
      kDewPointIndex,
      kHumidityIndex,
      kPressureIndex,
      kGeomHeightIndex,
      kWindSpeedIndex,
      kWindDirectionIndex,
      kWindcomponentUIndex,
      kWindcomponentVIndex,
      kWindVectorIndex,
      kTotalCloudinessIndex,
      kDataVectorSize // viimeiseksi laitetaan data vektorin koko, joka on siis viimeistä parametria yhtä suurempi arvo
  };

  // TODO Laita käyttämään NFmiDataMatrix-luokkaa dynaamista datalistaa varten. Laita myös
  // param-lista (joka annetaan fillData-metodeissa) data osaksi
  std::vector<std::deque<float>> itsParamDataVector;

  float itsZeroHeight;  // tältä korkeudelta alkaa luotauksen 0-korkeus, eli vuoristossa luotaus
  // alkaa oikeasti korkeammalta ja se korkeus pitää käsitellä pintakorkeutena
  int itsZeroHeightIndex;  // edellisen indeksi (paikka vektorissa). Arvo on -1 jos ei löytynyt
                           // kunnollista 0-korkeutta
  bool fObservationData;
  bool fPressureDataAvailable;
  bool fHeightDataAvailable;

  LFCIndexCache itsLFCIndexCache;
  typedef std::unordered_map<std::string, double> LiftedAirParcelCacheType;
  LiftedAirParcelCacheType itsLiftedAirParcelCache;
  bool fMovingSounding = false;
};

