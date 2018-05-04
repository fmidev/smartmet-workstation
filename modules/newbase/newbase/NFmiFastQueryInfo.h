// ======================================================================
/*!
 * \file NFmiFastQueryInfo.h
 * \brief Interface of class NFmiFastQueryInfo
 */
// ======================================================================

#pragma once

#include "NFmiDataMatrix.h"
#include "NFmiQueryInfo.h"

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

class NFmiRect;

//! Undocumented
class _FMI_DLL NFmiFastQueryInfo : public NFmiQueryInfo
{
 public:
  ~NFmiFastQueryInfo(void);

  NFmiFastQueryInfo(const NFmiFastQueryInfo &theInfo);

  NFmiFastQueryInfo(const NFmiQueryInfo &theInfo);

  NFmiFastQueryInfo(double theInfoVersion = 7.);

  NFmiFastQueryInfo(const NFmiParamDescriptor &theParamDescriptor,
                    const NFmiTimeDescriptor &theTimeDescriptor,
                    const NFmiHPlaceDescriptor &theHPlaceDescriptor = NFmiHPlaceDescriptor(),
                    const NFmiVPlaceDescriptor &theVPlaceDescriptor = NFmiVPlaceDescriptor(),
                    double theInfoVersion = 7.);

  NFmiFastQueryInfo(NFmiQueryData *data,
                    NFmiParamDescriptor *theParamDescriptor = 0,
                    NFmiTimeDescriptor *theTimeDescriptor = 0,
                    NFmiHPlaceDescriptor *theHPlaceDescriptor = 0,
                    NFmiVPlaceDescriptor *theVPlaceDescriptor = 0);

  bool NextActiveParam(void);
  bool NextActiveParam(bool fIgnoreSubParam);  // = true);
  bool IsActiveParam(void);
  bool ActivateParam(bool newState, bool fIgnoreSubParam);  // = true);
  bool ActivateParam(bool newState);

  void Reset(void);
  void ResetParam(void);
  void ResetLocation(void);
  void ResetLevel(void);
  void ResetTime(void);

  bool First(void);

  // miksi first-metodit palauttavat booleanin?? (nfmiqueryinfon peruja), voidaanko muuttaa???

  bool FirstParam(bool fIgnoreSubParam = true);
  bool FirstLocation(void);
  bool FirstLevel(void);
  bool FirstTime(void);

  bool NextParam(bool fIgnoreSubParam = true);
  bool NextLocation(void);

  bool NextLevel(void);
  bool NextTime(void);

  // miksi last-metodit palauttavat booleanin?? (nfmiqueryinfon peruja), voidaanko muuttaa???

  bool LastParam(bool fIgnoreSubParam = true);
  bool LastLocation(void);
  bool LastLevel(void);
  virtual bool LastTime(void);

  bool PreviousParam(bool fIgnoreSubParam = true);
  bool PreviousLocation(void);
  bool PreviousLevel(void);
  bool PreviousTime(void);

  // HUOM!!! Nämä seuraavat metodit ovat todellisuudessa samaa luokkaa nopeudessa
  // kuin NFmiQueryInfo:nkin vastaavat metodit.

  bool Param(const NFmiParam &theParam);
  bool Param(const NFmiParam &theParam, const NFmiParam &theSubParam);
  bool Param(const NFmiDataIdent &theDataIdent);
  bool Param(FmiParameterName theParam);

  bool Location(const NFmiLocation &theLocation);
  bool Location(const unsigned long &theIdent);
  bool Location(const NFmiString &theLocationName);
  bool Location(const NFmiPoint &theLonLatPoint, NFmiPoint *theGridPoint = 0);
  bool NearestLocation(const NFmiLocation &theLocation,
                       double theMaxDistance = kFloatMissing * 1000.);
  bool NearestLocation(const NFmiLocation &theLocation,
                       const NFmiArea *theArea,
                       double theMaxDistance = kFloatMissing * 1000.);
  bool NearestPoint(const NFmiPoint &theLatLonPoint);
  bool Level(const NFmiLevel &theLevelValue);
  bool Time(const NFmiMetTime &theTime);

  const NFmiLocation *Location(void) const;
  NFmiPoint LatLon(void) const;
  const NFmiPoint &LatLonFast(void) const;  // Lisäsin nopean Latlon-metodin, joka ei ole
                                            // virtuaalinen. NFmiQueryInfo:n Latlon ei voi palauttaa
  // const referenssiä, koska se pyytää NFmiGrid-luokalta
  // latlon-pistettä ja se rakennetaan lennossa.
  const NFmiPoint &LatLon(unsigned long index) const;
  const NFmiPoint RelativePoint(void) const;
  const NFmiLevel *Level(void) const;
  FmiLevelType LevelType(void) const;
  NFmiProducer *Producer(void);
  const NFmiProducer &FirstParamProducer(void);

  NFmiDataIdent &Param(void) const;
  // *** vastaavan nopeuksiset loppuvat tähän *********************

  // HUOM!!! Nämä seuraavat metodit ovat todellisuudessa hitaita ja
  // niiden käyttöä pitäisi välttää!
  // Näistä voisi varmaan tehdä nopeampia!!!!

  const NFmiMetTime &Time(void) const;
  bool TimeToNearestStep(const NFmiMetTime &theTime,
                         FmiDirection theDirection,
                         long theTimeRangeInMinutes = kLongMissing);
  const NFmiMetTime &ValidTime(void) const;
  using NFmiQueryInfo::OriginTime;
  const NFmiMetTime &OriginTime(void) const;
  // ****** hitaat loppuvat tähän ********************************

  unsigned long TimeResolution(void);

  // ******* Halutaanko manipuloida infoa suoraan indeksien avulla ????? ******************

  unsigned long ParamIndex(void) const;
  unsigned long LocationIndex(void) const;
  unsigned long LevelIndex(void) const;
  unsigned long TimeIndex(void) const;
  bool ParamIndex(unsigned long theIndex);
  bool LocationIndex(unsigned long theIndex);
  bool LevelIndex(unsigned long theIndex);
  bool TimeIndex(unsigned long theIndex);
  // ******* Halutaanko manipuloida infoa suoraan indeksien avulla ????? ******************

  unsigned long SizeParams(void) const;
  unsigned long SizeLocations(void) const;
  unsigned long SizeLevels(void) const;
  unsigned long SizeTimes(void) const;

  bool IsParamUsable(void) const;
  bool IsLocationUsable(void) const;
  bool IsLevelUsable(void) const;
  bool IsTimeUsable(void) const;

  unsigned long PeekLocationIndex(int theXOffset, int theYOffset) const;
  float PeekLocationValue(int theXOffset, int theYOffset) const;  // lähinnä gridi-datan tutkimiseen
  float PeekLocationValue(int theXOffset, int theYOffset, const NFmiMetTime &theTime);
  const NFmiPoint PeekLocationLatLon(int theXOffset, int theYOffset);
  float PeekTimeValue(int theTimeIndexOffset);

  using NFmiQueryInfo::PeekValue;
  float PeekValue(int theTimeOffset, int theXOffset, int theYOffset);

  // 16.08.2001 Mika&Marko, datan & koordinaattien haku matriisiin:

  template <typename T>
  void Values(T &theMatrix) const;

  // 12.09.2013 Anssi.R changed method to virtual to be able to override in NFmiMultiQueryInfo
  virtual void Values(NFmiDataMatrix<float> &theMatrix, const NFmiMetTime &theInterpolatedTime);

  // 31.5.2017 Tavi high performance bulk query
  bool GetValues(size_t startIndex, size_t step, size_t count, std::vector<float> &values) const;
  bool GetValuesPartial(size_t startIndex, size_t rowCount, size_t rowStep, size_t columnCount, size_t columnStep, std::vector<float> &values) const;
  bool GetLevelToVec(std::vector<float> &values);
  bool GetLevelToVecPartial(size_t x1, size_t y1, size_t x2, size_t y2, std::vector<float> &values);
  bool GetCube(std::vector<float> &values);

  bool GetInterpolatedLevel(std::vector<float> &values, const NFmiMetTime &time);
  bool GetInterpolatedCube(std::vector<float> &values, const NFmiMetTime &t);

  void LandscapeValues(NFmiDataMatrix<float> &theMatrix,
                       const NFmiDataMatrix<float> &theDEMMatrix,
                       const NFmiDataMatrix<bool> &theWaterFlagMatrix);
  void LandscapeValues(NFmiDataMatrix<float> &theMatrix,
                       const NFmiMetTime &theInterpolatedTime,
                       const NFmiDataMatrix<float> &theDEMMatrix,
                       const NFmiDataMatrix<bool> &theWaterFlagMatrix);
  void LandscapeCroppedValues(NFmiDataMatrix<float> &theMatrix,
                              int x1,
                              int y1,
                              int x2,
                              int y2,
                              const NFmiDataMatrix<float> &theDEMMatrix,
                              const NFmiDataMatrix<bool> &theWaterFlagMatrix,
                              const NFmiDataMatrix<NFmiLocationCache> &theLocationCache =
                                  NFmiDataMatrix<NFmiLocationCache>());
  void LandscapeCachedInterpolation(NFmiDataMatrix<float> &theMatrix,
                                    const NFmiDataMatrix<NFmiLocationCache> &theLocationCache,
                                    const NFmiDataMatrix<float> &theDEMMatrix,
                                    const NFmiDataMatrix<bool> &theWaterFlagMatrix);

  void LandscapeCachedInterpolation(NFmiDataMatrix<float> &theMatrix,
                                    const NFmiDataMatrix<NFmiLocationCache> &theLocationCache,
                                    const NFmiTimeCache &theTimeCache,
                                    const NFmiDataMatrix<float> &theDEMMatrix,
                                    const NFmiDataMatrix<bool> &theWaterFlagMatrix);

  void Values(NFmiDataMatrix<float> &theMatrix,
              NFmiDataModifier *theFunction,
              const NFmiMetTime &theTime,
              int theBackwardOffsetInMinutes,
              int theForwardOffsetInMinutes);

  // 12.09.2013 Anssi.R changed method to virtual to be able to override in NFmiMultiQueryInfo
  virtual void Values(const NFmiDataMatrix<NFmiPoint> &theLatlonMatrix,
                      NFmiDataMatrix<float> &theValues,
                      const NFmiMetTime &theTime,
                      float P = kFloatMissing,
                      float H = kFloatMissing);

  void CroppedValues(NFmiDataMatrix<float> &theMatrix, int x1, int y1, int x2, int y2) const;
  void CroppedValues(NFmiDataMatrix<float> &theMatrix,
                     const NFmiMetTime &theInterpolatedTime,
                     int x1,
                     int y1,
                     int x2,
                     int y2);

  bool SetValues(const NFmiDataMatrix<float> &theMatrix);

  template <typename T>
  void Locations(T &theMatrix) const;

  void LocationsWorldXY(NFmiDataMatrix<NFmiPoint> &theMatrix, const NFmiArea &theArea) const;
  void LocationsXY(NFmiDataMatrix<NFmiPoint> &theMatrix, const NFmiArea &theArea) const;

  // Tähän tulee joukko funktioita, jotka palauttavat aktiivisen parametrin
  // arvon haluttuun korkeuteen [m]. Metodeja on neljä, jotka tekevät erilaisia
  // interpolointeja parametreista riippuen.
  float HeightValue(float theHeight);
  float HeightValue(float theHeight, const NFmiPoint &theLatlon);
  float HeightValue(float theHeight, const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
  float HeightValue(float theHeight,
                    const NFmiPoint &theLatlon,
                    const NFmiMetTime &theTime,
                    unsigned long theTimeRangeInMinutes);
  float HeightValue(float theHeight, const NFmiMetTime &theTime);

  // Tähän tulee joukko funktioita, jotka palauttavat aktiivisen parametrin
  // arvon haluttuun paine pintaan interpoloituna (log(p) interpolointi).
  // Metodeja on neljä, jotka tekevät erilaisia interpolointeja parametreista riippuen.
  float PressureLevelValue(float P);
  float PressureLevelValue(float P, const NFmiPoint &theLatlon);
  float PressureLevelValue(float P, const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
  float PressureLevelValue(float P,
                           const NFmiPoint &theLatlon,
                           const NFmiMetTime &theTime,
                           unsigned long theTimeRangeInMinutes);
  float PressureLevelValue(float P, const NFmiMetTime &theTime);

  // Tässä metodit millä saadaan dataan nopeita osoitus indeksejä, mitä voidaan hyödyntää kun
  // tehdään
  // Nopeita 4D interpolointeja (useita samaan paikkaan ja aikaa ja korkeuteen).
  // Kaikki double & parametrit (out-parametreja) ovat reaali lukuja joiden avulla voidaan päätellä
  // fastqueryinfosta
  // nopeasti laskuissa halutut arvot. Palauttaa false, jos ollaan datan ulkopuolella.
  bool GetLocationIndex(const NFmiPoint &theLatlon, double &xInd, double &yInd);
  bool GetTimeIndex(const NFmiMetTime &theTime, double &tInd);
  bool GetLevelIndex(const NFmiPoint &theLatlon,
                     const NFmiMetTime &theTime,
                     double pressure,
                     double &pInd);
  float FastPressureLevelValue(double xInd, double yInd, double tInd, double pInd);
  float FastPressureLevelValue(double xInd, double yInd, double pInd);
  float FastPressureLevelValue(double xInd, double yInd);

  void CrossSectionValues(NFmiDataMatrix<float> &theValues,
                          const NFmiMetTime &theInterpolatedTime,
                          const checkedVector<float> &theHeights,
                          const checkedVector<NFmiPoint> &theLatlonPoints);
  void TimeCrossSectionValues(NFmiDataMatrix<float> &theValues,
                              checkedVector<float> &theHeights,
                              const NFmiPoint &thePoint,
                              NFmiTimeBag &theWantedTimes);
  void RouteCrossSectionValues(NFmiDataMatrix<float> &theValues,
                               const checkedVector<float> &theHeights,
                               const checkedVector<NFmiPoint> &theLatlonPoints,
                               const checkedVector<NFmiMetTime> &thePointTimes);
  checkedVector<float> ConvertPressuresToHeights(const checkedVector<float> &thePressures);

  void CrossSectionValuesLogP(NFmiDataMatrix<float> &theValues,
                              const NFmiMetTime &theInterpolatedTime,
                              const checkedVector<float> &thePressures,
                              const checkedVector<NFmiPoint> &theLatlonPoints);
  void TimeCrossSectionValuesLogP(NFmiDataMatrix<float> &theValues,
                                  checkedVector<float> &thePressures,
                                  const NFmiPoint &thePoint,
                                  NFmiTimeBag &theWantedTimes,
                                  unsigned int theStartTimeIndex = 0);
  void RouteCrossSectionValuesLogP(NFmiDataMatrix<float> &theValues,
                                   const checkedVector<float> &thePressures,
                                   const checkedVector<NFmiPoint> &theLatlonPoints,
                                   const checkedVector<NFmiMetTime> &thePointTimes);

  // 05-Oct-2011 PKi: Cross mallipinnoille ja ground levelille
  void CrossSectionValuesHybrid(NFmiDataMatrix<float> &theValues,
                                const NFmiMetTime &theInterpolatedTime,
                                const checkedVector<NFmiLevel> &theLevels,
                                const checkedVector<NFmiPoint> &theLatlonPoints);
  void TimeCrossSectionValuesHybrid(NFmiDataMatrix<float> &theValues,
                                    const checkedVector<NFmiLevel> &theLevels,
                                    const NFmiPoint &thePoint,
                                    NFmiTimeBag &theWantedTimes);
  void RouteCrossSectionValuesHybrid(NFmiDataMatrix<float> &theValues,
                                     const checkedVector<NFmiLevel> &theLevels,
                                     const checkedVector<NFmiPoint> &theLatlonPoints,
                                     const checkedVector<NFmiMetTime> &thePointTimes);

  // 09-Mar-2015 PKi: FlightRoute
  void FlightRouteValues(NFmiDataMatrix<float> &theValues,
                         const checkedVector<float> &theHeights,
                         const checkedVector<NFmiPoint> &theLatlonPoints,
                         const checkedVector<NFmiMetTime> &thePointTimes);
  void FlightRouteValuesHybrid(NFmiDataMatrix<float> &theValues,
                               const checkedVector<NFmiLevel> &theLevels,
                               const checkedVector<NFmiPoint> &theLatlonPoints,
                               const checkedVector<NFmiMetTime> &thePointTimes);
  void FlightRouteValuesLogP(NFmiDataMatrix<float> &theValues,
                             const checkedVector<float> &thePressures,
                             const checkedVector<NFmiPoint> &theLatlonPoints,
                             const checkedVector<NFmiMetTime> &thePointTimes);

  // Hakee haluttuun hilaan interpoloitua dataa halutulta ajalta
  void GridValues(NFmiDataMatrix<float> &theValues,
                  const NFmiGrid &theWantedGrid,
                  const NFmiMetTime &theInterpolatedTime);

  // 12.09.2013 Anssi.R changed methods to virtual to be able to override in NFmiMultiQueryInfo
  // Tämä hakee hilan sellaisenaan (datan originaali hila ja alue) halutulle painepinnalle.
  virtual void PressureValues(NFmiDataMatrix<float> &theValues,
                              const NFmiMetTime &theInterpolatedTime,
                              float wantedPressureLevel);
  // Tämä hakee haluttuun hilaan ja alueeseen dataa.
  virtual void PressureValues(NFmiDataMatrix<float> &theValues,
                              const NFmiGrid &theWantedGrid,
                              const NFmiMetTime &theInterpolatedTime,
                              float wantedPressureLevel);
  // Tämä hakee hilan sellaisenaan (datan originaali hila ja alue) halutulle korkeudelle [m].
  // Jos haluat lentopinnoille dataa (Flight Level) on lentopinta -> metri kerroin = 30.5
  // eli esim. lentopinta 50 saadaan laskulla 50 * 30.5 eli 1525 [m].
  void HeightValues(NFmiDataMatrix<float> &theValues,
                    const NFmiMetTime &theInterpolatedTime,
                    float wantedHeightLevel);
  // Sama korkeus haku, mutta haluttuun hilaan ja projektioon.
  void HeightValues(NFmiDataMatrix<float> &theValues,
                    const NFmiGrid &theWantedGrid,
                    const NFmiMetTime &theInterpolatedTime,
                    float wantedHeightLevel);

  bool HeightDataAvailable(void) const
  {
    return fHeightValueAvailable || fHeightLevelDataAvailable;
  }  // tämä on uusi yleisfunktio, jota pitäisi käyttää kun halutaan tietää onko heightlevel-dataa
     // käytössä
  bool HeightValueAvailable(void) const { return fHeightValueAvailable; }
  bool HeightLevelDataAvailable(void) const { return fHeightLevelDataAvailable; }
  bool HeightParamIsRising(void) const { return fHeightParamIsRising; }
  unsigned long HeightParamIndex(void) const { return itsHeightParamIndex; }
  unsigned long PressureParamIndex(void) const { return itsPressureParamIndex; }
  bool PressureDataAvailable(void) const
  {
    return fPressureValueAvailable || fPressureLevelDataAvailable;
  }  // tämä on uusi yleisfunktio, jota pitäisi käyttää kun halutaan tietää onko painelevel-dataa
  // käytössä
  bool PressureValueAvailable(void) const
  {
    return fPressureValueAvailable;
  }  // Katso voiko PressureDataAvailable -funktiota käyttää mieluummin
  bool PressureLevelDataAvailable(void) const
  {
    return fPressureLevelDataAvailable;
  }  // Katso voiko PressureDataAvailable -funktiota käyttää mieluummin
  bool PressureParamIsRising(void) const { return fPressureParamIsRising; }
  bool FindNearestTime(const NFmiMetTime &theTime,
                       FmiDirection theDirection = kCenter,
                       unsigned long theTimeRangeInMinutes = kUnsignedLongMissing);

  NFmiGrid *GridData(bool fUseExisting = false);
  NFmiQueryInfo *Clone(void) const;
  NFmiFastQueryInfo &operator=(const NFmiFastQueryInfo &theInfo);
  bool SetDescriptors(NFmiQueryInfo *theQueryInfo, bool fIgnoreLevel = true);
  NFmiQueryInfo *CreateCombinedInfo(NFmiQueryInfo *theOtherInfo);
  bool IsSubParamUsed(void) const;

  // ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************
  // HUOM!! liikkumista ei voi sallia laatikon ulkopuolelle kuten esim. PeekValue-metodissa voi!!!
  // Move*** ovat liikkumista varten

  bool MoveUp(int moveBy = 1);     // toimii vain gridi datalle oikein!!!
  bool MoveDown(int moveBy = 1);   // toimii vain gridi datalle oikein!!!
  bool MoveLeft(int moveBy = 1);   // toimii vain gridi datalle oikein!!!
  bool MoveRight(int moveBy = 1);  // toimii vain gridi datalle oikein!!!

  // näillä asetetaan paikka suoraan johonkin laitaan (ei 'laatikon' ulkopuolelle!)

  bool Top(void);     // toimii vain gridi datalle oikein!!!
  bool Bottom(void);  // toimii vain gridi datalle oikein!!!
  bool Left(void);    // toimii vain gridi datalle oikein!!!
  bool Right(void);   // toimii vain gridi datalle oikein!!!
  // ***** 14.5.2001/MArko Uusia metodeja hilassa liikkumiseen ***************

  size_t Index(unsigned long theParamIndex,
               unsigned long theLocationIndex,
               unsigned long theLevelIndex,
               unsigned long theTimeIndex) const;

  virtual float GetFloatValue(size_t theIndex) const;

  // HUOM! tämä on viritys funktio, joka toimii oikeasti vain NFmiFastQueryInfo:ssa
  // Täälläkin tämän käyttö on vaarallista, ellei tiedä mitä tekee.
  // Käytetty InterpolatedValue-metodien virittämiseen W&C parametrin ali parametrien kanssa.
  void SetIsSubParamUsed(bool newState) { fUseSubParam = newState; }
  bool HasLatlonInfoInData() const;
  NFmiPoint GetLatlonFromData(void);  // jos datassa on kFmiLongitude ja kFmiLatitude parametrit,
                                      // lasketaan sijanti asetetun ajan ja paikan suhteen niiden
                                      // avulla
  float PeekParamValue(unsigned long theParamIndex);  // jos tiedetään jonkin parametrin indeksi,
                                                      // tämän avulla voidaan kurkata
  // sen parametrin arvoa (aika,paikka ja leveli jo asetettuja)
  // HUOM! Ei toimi aliparametrien (TotalWind ja WeatherAndCloudiness) kanssa!!!!

  // Nämä metodit määrätään fastInfossa koska lapsiluokissa käytetään näitä sitten.
  virtual const std::string &DataFileName(void) const
  {
    static std::string dummy;
    return dummy;
  }
  virtual void DataFileName(const std::string & /* theDataFileName */) {}
  virtual const std::string &DataFilePattern(void) const
  {
    static std::string dummy;
    return dummy;
  }
  virtual void DataFilePattern(const std::string & /* theDataFilePattern */) {}
  virtual boost::shared_ptr<NFmiQueryData> DataReference(void)
  {
    return boost::shared_ptr<NFmiQueryData>();
  }  // HUOM! ei saa palauttaa itsRefQueryData -data osaa shared-pointterissa, koska se deletoisi
     // lopuksi datan (TÄMÄ siis overridataan lapsessa!)
  NFmiInfoData::Type DataType(void) const { return itsDataType; };
  void DataType(NFmiInfoData::Type newType) { itsDataType = newType; };
  // Näillä Start/Restore -funktioilla otetaan nykyinen parametri tila talteen ja otetaan käyttöön
  // 'erikois' korkeus-parametri. Ja käytön jälkeen palautus.
  // HUOM! Jos Start-funktio palauttaa true:n, on kyseisen korkeus parametrin käyttö mahdollista ja
  // tällöin pitää kutsua jossain vaiheessa vastaavaa Restore-funktiota!
  bool StartUsingPressureParam(void);
  void RestoreAfterUsingPressureParam(void);
  bool StartUsingHeightParam(void);
  void RestoreAfterUsingHeightParam(void);
  // Universaali funktio, jolla saa halutunlaisen korkeus parametrin kyseiseltä leveliltä haluttuun
  // kohtaan ja aikaa interpoloituna
  float GetLevelHeightValue(FmiParameterName theParId,
                            const NFmiPoint &theLatlon,
                            const NFmiMetTime &theTime);
  float GetLevelHeightValue(FmiParameterName theParId,
                            const NFmiLocationCache &theLocationCache,
                            const NFmiTimeCache &theTimeCache);

  // Interpolate taking lapse rate and land/sea mask into account
  float LandscapeInterpolatedValue(float theHeight,
                                   const NFmiPoint &theLatLon,
                                   const NFmiMetTime &theTime,
                                   int theMaxMinuteRange = 0);

  float LandscapeInterpolatedValue(float theHeight,
                                   bool theWaterFlag,
                                   const NFmiPoint &theLatLon,
                                   const NFmiMetTime &theTime,
                                   int theMaxMinuteRange = 0);

  float LandscapeInterpolatedValueDewPoint(float theHeight,
                                           const NFmiPoint &theLatLon,
                                           const NFmiMetTime &theTime,
                                           int theMaxMinuteRange = 0);

  float LandscapeInterpolatedValueDewPoint(float theHeight,
                                           bool theWaterFlag,
                                           const NFmiPoint &theLatLon,
                                           const NFmiMetTime &theTime,
                                           int theMaxMinuteRange = 0);

  void LandscapeInterpolatedValuesDewPoint(
      NFmiDataMatrix<float> &theLandscapedMatrix,
      const NFmiDataMatrix<float> &theMatrix,
      const NFmiDataMatrix<float> &temperatureMatrix,
      NFmiDataMatrix<float> &humidityMatrix,
      const NFmiDataMatrix<NFmiLocationCache> &theLocationCache =
          NFmiDataMatrix<NFmiLocationCache>());

  void LandscapeInterpolatedValues(NFmiDataMatrix<float> &theLandscapedMatrix,
                                   const NFmiDataMatrix<float> &theMatrix,
                                   const NFmiDataMatrix<NFmiPoint> &gridPointMatrix,
                                   const NFmiDataMatrix<float> &demMatrix,
                                   const NFmiDataMatrix<bool> &waterFlagMatrix,
                                   const NFmiDataMatrix<float> &heightMatrix,
                                   const NFmiDataMatrix<float> &lapseRateMatrix,
                                   const NFmiDataMatrix<float> &maskMatrix);
  float GetCurrentLevelPressure(void);
  float GetCurrentLevelPressure(const NFmiPoint &theLatlon);
  float GetCurrentLevelPressure(const NFmiPoint &theLatlon, const NFmiMetTime &theTime);
 protected:
  size_t Index(void) const;

  float IndexFloatValue(size_t theIndex) const;
  bool IndexFloatValue(size_t theIndex, float theValue);

  bool AreIndividualIndexiesInside(const unsigned long &theParamIndex,
                                   const unsigned long &theLocationIndex,
                                   const unsigned long &theLevelIndex,
                                   const unsigned long &theTimeIndex) const;
  void InitFastInfo(void);
  std::vector<float> CalcPressureLevelDataPressures(void);
  std::vector<float> CalcHeightLevelDataHeights(void);
  float FindFirstPressureValue(void);
  float FindFirstHeightValue(void);
  void DoWindComponentFix(const NFmiGrid &usedGrid,
                          float u,
                          float v,
                          FmiParameterName id,
                          NFmiDataMatrix<float> &theValues);

  unsigned long itsParamIndex;  // indeksit 'resetoidaan' antamalla arvoksi -1, joka on isoin
                                // unsigned long, arvo on 4294967295
  unsigned long
      itsLocationIndex;  // toivotaan, että datan todellinen koko ei tule koskaan lähelle tätä arvoa
  unsigned long itsLevelIndex;  // tee ensin unsigned long versio (queryinfon peruja), ja yritä
                                // sitten konvertoida nämä pelkiksi int:eiksi
  unsigned long itsTimeIndex;
  unsigned long itsParamSize;
  unsigned long itsLocationSize;
  unsigned long itsLevelSize;
  unsigned long itsTimeSize;
  //  unsigned long itsDataArraySize;
  //  float * itsDataArray; // luokka ei omista tätä, taulun osoite on otettu suoraan queryinfon
  //  datapoolilta (oletus: data on float arvoida)
  NFmiGrid *itsTemporaryGridData;  // jos käyttäjä pyytää gridi dataa, se talletetaan tähän

  // This declaration is needed or NFmiMultiQueryInfo::TimeIndex will not compile
  friend class NFmiMultiQueryInfo;
  size_t itsLocLevTimSize;  // location*level*time size (käytetään FastIndex-metodissa)
  size_t itsLevTimSize;     // level*time size (käytetään FastIndex-metodissa)

  bool fUseSubParam;  // käytetäänkö parhaillaan yhdistelmä parametrin aliparametria

  // Nämä datat ovat tarkoitettu HeightValue-metodien optimointia varten.
  // Nämä alustetaan InitFastInfo-metodissa.
  bool fHeightValueAvailable;  // voidaanko datalta pyytää arvoja HeightValue-metodeilla (leveleitä
                               // pitää olla useita ja pitää olla korkeus parametri datassa)
  bool fHeightLevelDataAvailable;  // Jos level datassa ei ollut height parametria, oliko se kenties
                                   // heightlevel dataa, jolloin eri kerrosten korkeus tiedot
                                   // saadaan levelbagistä
  std::vector<float> itsHeightLevelDataHeights;  // optimointia kun data on heightlevel dataa, tähän
                                                 // on talletettu eri korkeustasot
  bool fHeightParamIsRising;  // kun mennään leveleitä NextLevel:eillä läpi, nouseeko vai laskeeko
                              // korkeus arvot
  unsigned long itsHeightParamIndex;  // Optimointia kun asetetaan välillä height parametri päälle,
                                      // kun tehdään korkeus interpolointia.
  // Nämä datat ovat tarkoitettu PressureLevelValue-metodien optimointia varten.(ja niihin liittyvät
  // poikkileikkaus funktiot)
  // Nämä alustetaan InitFastInfo-metodissa.
  bool fPressureValueAvailable;  // voidaanko datalta pyytää arvoja PressureLevelValue-metodeilla
  // (leveleitä pitää olla useita ja pitää olla korkeus parametri
  // datassa)
  bool fPressureLevelDataAvailable;  // Jos level datassa ei ollut paine parametria, oliko se
                                     // kenties painepinta dataa, jolloin eri kerrosten paine tiedot
                                     // saadaan levelbagistä
  bool fPressureParamIsRising;  // kun mennään leveleitä NextLevel:eillä läpi, nouseeko vai laskeeko
                                // korkeus arvot
  unsigned long itsPressureParamIndex;  // Optimointia kun asetetaan välillä height parametri
                                        // päälle, kun tehdään korkeus interpolointia.
  unsigned long itsOldParamIndex;  // nämä Old-valuet ovat tarpeen kun käytetään välillä eri korkeus
                                   // parametreja StartUsingPressureParam- ja Height -funktioita.
  bool fOldFSubParamUsed;  // Näistä palautetaan arvot sitten vastaavissa RestoreUsingPressureParam-
                           // ja Height -funktioissa.
  std::vector<float> itsPressureLevelDataPressures;  // optimointia kun data on painepinta dataa,
                                                     // tähän on talletettu eri painepinnat
  unsigned long itsLongitudeParamIndex;  // Optimointia GetLatlonFromData-metodin varalle.
  unsigned long itsLatitudeParamIndex;   // Optimointia GetLatlonFromData-metodin varalle.

  // Tähän tulee optimointi apua location ID hakuja varten. Eli kun haetaan havainto-aseman id:n
  // avulla paikkoja.
  // unorderd-mappiin laitetaan asema id avaimeksi ja location index arvoksi.
  typedef boost::unordered_map<unsigned long, unsigned long> StationIdSeekContainer;
  StationIdSeekContainer itsFastStationIdSeekList;
  NFmiInfoData::Type itsDataType;  // data tyyppiä käytetään lähinnä SmartMet koodeissa (esim.
                                   // 2=malli-pinta, 5=havainto, 15=mallihybridipinta), jouduin
                                   // pudottamaan sen fastInfon lapsi-luokalta (NFmiOwnerInfo -
                                   // smarttools-kirjasto)

  NFmiMetTime itsDummyTime;  // kun halutaan palauttaa const& time jota ei ole, palautetaan tämä,
                             // asetettu aikaan 1900.01.01: 1.1.1

};  // class NFmiFastQueryInfo

// ----------------------------------------------------------------------
/*!
 * \param theParamIndex Undocumented
 * \param theLocationIndex Undocumented
 * \param theLevelIndex Undocumented
 * \param theTimeIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::AreIndividualIndexiesInside(const unsigned long &theParamIndex,
                                                           const unsigned long &theLocationIndex,
                                                           const unsigned long &theLevelIndex,
                                                           const unsigned long &theTimeIndex) const
{
  //  if(!IsLocationMasked(theLocationIndex))
  //	return false;

  return ((theParamIndex < itsParamSize) && (theLocationIndex < itsLocationSize) &&
          (theLevelIndex < itsLevelSize) && (theTimeIndex < itsTimeSize));
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumenetd
 */
// ----------------------------------------------------------------------

inline size_t NFmiFastQueryInfo::Index(void) const
{
  if (AreIndividualIndexiesInside(itsParamIndex, itsLocationIndex, itsLevelIndex, itsTimeIndex))
    return (itsParamIndex * itsLocLevTimSize + itsLocationIndex * itsLevTimSize +
            itsLevelIndex * itsTimeSize + itsTimeIndex);
  else
    return static_cast<size_t>(-1);
}

// ----------------------------------------------------------------------
/*!
 * \param theParamIndex Undocumented
 * \param theLocationIndex Undocumented
 * \param theLevelIndex Undocumented
 * \param theTimeIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline size_t NFmiFastQueryInfo::Index(unsigned long theParamIndex,
                                       unsigned long theLocationIndex,
                                       unsigned long theLevelIndex,
                                       unsigned long theTimeIndex) const
{
  if (AreIndividualIndexiesInside(theParamIndex, theLocationIndex, theLevelIndex, theTimeIndex))
  {
    size_t idx = (theParamIndex * itsLocLevTimSize + theLocationIndex * itsLevTimSize +
                  theLevelIndex * itsTimeSize + theTimeIndex);
    return idx;
  }
  else
    return static_cast<size_t>(-1);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsSubParamUsed(void) const { return fUseSubParam; }
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiFastQueryInfo::ResetParam(void)
{
  itsParamIndex = static_cast<unsigned long>(-1);
  fUseSubParam = false;
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiFastQueryInfo::ResetLocation(void)
{
  itsLocationIndex = static_cast<unsigned long>(-1);
}

// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiFastQueryInfo::ResetLevel(void) { itsLevelIndex = static_cast<unsigned long>(-1); }
// ----------------------------------------------------------------------
/*!
 *
 */
// ----------------------------------------------------------------------

inline void NFmiFastQueryInfo::ResetTime(void) { itsTimeIndex = static_cast<unsigned long>(-1); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::FirstParam(bool fIgnoreSubParam)
{
  ResetParam();
  return NextParam(fIgnoreSubParam);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::NextLocation(void)
{
  itsLocationIndex++;
  if (itsLocationIndex < itsLocationSize)
    return true;
  else
  {
    itsLocationIndex = itsLocationSize;
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::FirstLocation(void)
{
  ResetLocation();
  return NextLocation();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::FirstLevel(void)
{
  ResetLevel();
  return NextLevel();
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::NextTime(void)
{
  itsTimeIndex++;
  if (itsTimeIndex < itsTimeSize)
    return true;
  else
  {
    itsTimeIndex = itsTimeSize;
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::FirstTime(void)
{
  ResetTime();
  return NextTime();
}

// ----------------------------------------------------------------------
/*!
 * \param theIndex Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline float NFmiFastQueryInfo::IndexFloatValue(size_t theIndex) const
{
  return itsRefRawData ? itsRefRawData->GetValue(theIndex) : kFloatMissing;
}


// ----------------------------------------------------------------------
/*!
* \param startIndex Undocumented
* \param step Undocumented
* \param count Undocumented
* \param values Vector to fill (and resize to count elements) with values startIndex, startIndex+step, startIndex+step*2, ..., startIndex+step*(count-1) - current iterators are invalidated by the resizing!
* \return false if out-of-range, true otherwise
*/
// ----------------------------------------------------------------------
inline bool NFmiFastQueryInfo::GetValues(size_t startIndex, size_t step, size_t count, std::vector<float> &values) const
{
	return itsRefRawData ? itsRefRawData->GetValues(startIndex, step, count, values) : false;
}

// ----------------------------------------------------------------------
/*!
* \param startIndex Undocumented
* \param rowCount Undocumented
* \param rowStep Undocumented
* \param columCount Undocumented
* \param columnStep Undocumented
* \param values Vector to fill (and resize to count elements) with values startIndex, startIndex+rowStep, startIndex+rowStep*2, ..., startIndex+rowStep*(count-1), startIndex+columnStep, startIndex+columnStep+rowStep ... - current iterators are invalidated by the resizing!
* \return false if out-of-range, true otherwise
*/
// ----------------------------------------------------------------------
inline bool NFmiFastQueryInfo::GetValuesPartial(size_t startIndex, size_t rowCount, size_t  rowStep, size_t columnCount, size_t  columnStep, std::vector<float> &values) const
{
	return itsRefRawData ? itsRefRawData->GetValuesPartial(startIndex, rowCount, rowStep, columnCount, columnStep, values) : false;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::LastLocation(void)
{
  itsLocationIndex = itsLocationSize - 1;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::LastLevel(void)
{
  itsLevelIndex = itsLevelSize - 1;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::LastTime(void)
{
  itsTimeIndex = itsTimeSize - 1;
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \param theParam Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::Param(FmiParameterName theParam)
{
  if (NFmiQueryInfo::Param(theParam))
  {
    itsParamIndex = NFmiQueryInfo::ParamIndex();
    fUseSubParam = itsParamDescriptor->IsSubParamUsed();
    return true;
  }
  else
  {
    // reset index if the param was not found
    itsParamIndex = static_cast<unsigned long>(-1);
    fUseSubParam = false;
    return false;
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::ParamIndex(void) const { return itsParamIndex; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::LocationIndex(void) const { return itsLocationIndex; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::LevelIndex(void) const { return itsLevelIndex; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::TimeIndex(void) const { return itsTimeIndex; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::SizeParams(void) const { return itsParamSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::SizeLocations(void) const { return itsLocationSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::SizeLevels(void) const { return itsLevelSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::SizeTimes(void) const { return itsTimeSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsParamUsable(void) const { return itsParamIndex < itsParamSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsLocationUsable(void) const
{
  return itsLocationIndex < itsLocationSize;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsLevelUsable(void) const { return itsLevelIndex < itsLevelSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline bool NFmiFastQueryInfo::IsTimeUsable(void) const { return itsTimeIndex < itsTimeSize; }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::TimeResolution(void)
{
  // piti overidata täällä, koska timeindex pitää asettaa
  // ennen Resolution kyselyä
  itsTimeDescriptor->Time(itsTimeIndex);
  return itsTimeDescriptor->Resolution();
}
// ----------------------------------------------------------------------
/*!
 * Palauttaa nykyisen indeksin koordinaatit. Koordinaatticache sijaitsee
 * QueryData-luokan cachessa
 *
 * \return Nykyisen pisteen koordinaatit
 */
// ----------------------------------------------------------------------

inline NFmiPoint NFmiFastQueryInfo::LatLon(void) const { return LatLon(itsLocationIndex); }
inline const NFmiPoint &NFmiFastQueryInfo::LatLonFast(void) const
{
  return LatLon(itsLocationIndex);
}

// ----------------------------------------------------------------------
/*!
 * \brief Location index for peek position
 */
// ----------------------------------------------------------------------

inline unsigned long NFmiFastQueryInfo::PeekLocationIndex(int theXOffset, int theYOffset) const
{
  unsigned long theHPlaceIndex = itsLocationIndex;

  if (IsGrid())
  {
    long currentXIndex = (theHPlaceIndex % itsGridXNumber) + theXOffset;
    long currentYIndex = (theHPlaceIndex / itsGridXNumber) + theYOffset;

    // voiko tämän seuraavan tarkistuksen poistaa, kun indeksi tarkistetaan kuitenkin
    // Index-metodissa??
    if (currentXIndex >= 0 &&
        currentXIndex <
            int(itsGridXNumber) &&  // x- ja y-indeksien pitää pysyä gridin sisällä offsettien kera!
        currentYIndex >= 0 &&
        currentYIndex < int(itsGridYNumber))
      theHPlaceIndex = theHPlaceIndex + theYOffset * itsGridXNumber + theXOffset;
    else
      theHPlaceIndex = static_cast<unsigned long>(-1);
  }
  else
    theHPlaceIndex = theHPlaceIndex + theXOffset;

  return theHPlaceIndex;
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa kaikki hilan data-arvot annettuun datamatriisiin.
 * Muutettu siten että hiladata menee kuten ennenkin, mutta
 * asemadata ladataankin 'yksiulotteiseen'-matriisiin (eli 1 x N).
 * Huom! x-dimension pitää olla 1 ja y-dimensioon laitetaan paikkojen
 * määrä ja sen pitää tapahtua näin jolloin matrix-luokka tekee
 * vain 1 + 1 vector-luokan instanssia (toisin päin tekisi 1 + N kpl).
 *
 * \param theMatrix The matrix in which to store the values
 */
// ----------------------------------------------------------------------

// Generic matrix with operator()(i,j) interface

template <typename T>
inline void NFmiFastQueryInfo::Values(T &theMatrix) const
{
  bool fIsGrid = IsGrid();
  int nx = fIsGrid ? itsGridXNumber : 1;
  int ny = fIsGrid ? itsGridYNumber : itsLocationSize;

  int idx = Index(itsParamIndex, 0, itsLevelIndex, itsTimeIndex);
  int offset = Index(itsParamIndex, 1, itsLevelIndex, itsTimeIndex) - idx;

  for (int j = 0; j < ny; j++)
    for (int i = 0; i < nx; i++)
    {
      theMatrix(i, j) = NFmiQueryInfo::PeekValue(idx);
      idx += offset;
    }
}

// Specialization for NFmiDataMatrix

template <>
inline void NFmiFastQueryInfo::Values(NFmiDataMatrix<float> &theMatrix) const
{
  bool fIsGrid = IsGrid();
  unsigned long nx = fIsGrid ? itsGridXNumber : 1;
  unsigned long ny = fIsGrid ? itsGridYNumber : itsLocationSize;

  theMatrix.Resize(nx, ny, kFloatMissing);

  size_t idx = Index(itsParamIndex, 0, itsLevelIndex, itsTimeIndex);
  size_t offset = Index(itsParamIndex, 1, itsLevelIndex, itsTimeIndex) - idx;

  // Mika: Must have this loop order so that the offset trick works

  for (unsigned long j = 0; j < ny; j++)
    for (unsigned long i = 0; i < nx;
         i++)  // pientä optimointia olisi jos for loopit saisi toisin päin
    {
      theMatrix[i][j] = NFmiQueryInfo::PeekValue(idx);
      idx += offset;
    }
}

// ----------------------------------------------------------------------
/*!
 * Palauttaa kaikki hilan LatLongit annettuun datamatriisiin
 *
 * \param theMatrix The matrix where to store the coordinates
 */
// ----------------------------------------------------------------------

// Generic matrix with operator()(i,j) interface

template <typename T>
inline void NFmiFastQueryInfo::Locations(T &theMatrix) const
{
  if (IsGrid())
  {
    const long unsigned int nx = itsGridXNumber;
    const long unsigned int ny = itsGridYNumber;

    for (long unsigned int j = 0; j < ny; j++)
      for (long unsigned int i = 0; i < nx; i++)
      {
        const NFmiPoint latlon = LatLon(j * nx + i);
        theMatrix(i, j) = typename T::value_type(latlon.X(), latlon.Y());
      }
  }
  else
    throw std::runtime_error(
        "NFmiFastQueryInfo::Locations: Cannot extract coordinate matrix from point data");
}

// Specialization for NFmiDataMatrix

template <>
inline void NFmiFastQueryInfo::Locations(NFmiDataMatrix<NFmiPoint> &theMatrix) const
{
  if (IsGrid())
  {
    unsigned long nx = itsGridXNumber;
    unsigned long ny = itsGridYNumber;

    theMatrix.Resize(nx, ny, NFmiPoint(kFloatMissing, kFloatMissing));

    for (unsigned long j = 0; j < ny; j++)
      for (unsigned long i = 0; i < nx; i++)
        theMatrix[i][j] = LatLon(j * nx + i);
  }
  else
    theMatrix = NFmiPoint(kFloatMissing, kFloatMissing);
}

// ======================================================================
