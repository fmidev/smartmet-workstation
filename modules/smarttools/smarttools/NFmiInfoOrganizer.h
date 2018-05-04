#pragma once

// Luokan tehtävä on toimia eri queryDatojen ja niiden iteraattoreiden
// säilytyspaikkana SmartMet ja SmartToolFilter ohjelmille.
// Alkuperäinen NFmiInfoOrganizer-luokka on kauheaa sylttyä ja se pitää
// tehdä tässä uusiksi:
// --------------------
// 1. Rajapinta on siivottava.
// 2. Tehtävä selkeämmät data/info -pyynti funktiot.
//
// Lisäksi tulee uusia ominaisuuksia:
// ----------------------------------
// 1. Tuettava multi-threddausta
// 2. Osattava hanskata vanhoja malliajo datoja (esim. hirlam[-1])
// 3. Muistin siivous ja datojen tuhoamisen automatisointi.
// 4. Data voidaan tuhota infoOrganizerista vaikka sitä vielä joku käyttää
// omassa threadissaa ja threadi sitten lopuksi itse tuhoaa automaattisesti datan.
//
// Toteutus: NFmiOwnerInfo on tavallisten queryDatojen säilytys luokka.
// NFmiSmartInfo on käytössä vain editoitavalle datalle.
// Huom! Myös SmartMetiin tiputetut datat pitää lisätä tänne fileFiltterin kanssa.
// TODO: keksi parempi nimi tai muuta lopuksi NFmiInfoOrganizer-nimiseksi ja
// tuhoa alkuperäinen luokka.

#include <newbase/NFmiPoint.h>
#include <newbase/NFmiDataMatrix.h>
#include <newbase/NFmiInfoData.h>
#include <newbase/NFmiParamBag.h>
#include <newbase/NFmiProducerName.h>
#include <boost/shared_ptr.hpp>
#include <map>

class NFmiSmartInfo;
class NFmiOwnerInfo;
class NFmiFastQueryInfo;
class NFmiDrawParamFactory;
class NFmiDrawParam;
class NFmiQueryData;
class NFmiQueryDataSetKeeper;
class NFmiQueryDataKeeper;
class NFmiDataIdent;
class NFmiLevel;
class NFmiProducer;
class NFmiQueryInfo;
class NFmiTimeDescriptor;
class NFmiMetTime;
class NFmiArea;

class NFmiInfoOrganizer
{
 public:
  typedef std::map<std::string, boost::shared_ptr<NFmiQueryDataSetKeeper> > MapType;

  struct ParamCheckFlags
  {  // Tämän rakenteen avulla voidaan halutessa varmistaa ainakin FindSoundingInfo -metodissa, että
    // palautettavassa datassa on tiettyjä parametreja, esim.:
    // fSounding -> P, T, Td, RH, WS, WD
    // fTrajectory -> P, WS, WD, w
    ParamCheckFlags(bool sounding = false, bool trajectory = false)
        : fSounding(sounding), fTrajectory(trajectory)
    {
    }

    bool fSounding;
    bool fTrajectory;
  };

  NFmiInfoOrganizer(void);
  ~NFmiInfoOrganizer(void);

  bool Init(const std::string &theDrawParamPath,
            bool createDrawParamFileIfNotExist,
            bool createEditedDataCopy,
            bool fUseOnePressureLevelDrawParam);
  bool AddData(NFmiQueryData *theData,
               const std::string &theDataFileName,
               const std::string &theDataFilePattern,
               NFmiInfoData::Type theDataType,
               int theUndoLevel,
               int theMaxLatestDataCount,
               int theModelRunTimeGap,
               bool &fDataWasDeletedOut);
  int CleanUnusedDataFromMemory(void);
  static boost::shared_ptr<NFmiFastQueryInfo> DoDynamicShallowCopy(
      const boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
  static bool IsTempData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo);
  static bool IsTempData(unsigned long theProducerId, bool includeRawTemp = false);
  static int CalcWantedParameterCount(boost::shared_ptr<NFmiFastQueryInfo> &info,
                                      const std::vector<FmiParameterName> &wantedParameters);
  static boost::shared_ptr<NFmiFastQueryInfo> GetInfoWithMostWantedParams(
      checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > &infos,
      const std::vector<FmiParameterName> &wantedParameters);

  // ***************************************************************************************************************
  // Tässä perässä on pienin mahdollinen julkinen rajapinta, jonka sain siivottua originaali
  // NFmiInfoOrganizr:ista.
  // Ne pitää muuttaa vielä share_ptr ja uusien Info-luokkien mukaisiksi.
  // HUOM! Että multi-threaddaus onnistuisi, pitää kaikki info-olioiden palautus funktioiden
  // palauttaa kopio
  // iteraattori-infosta. Pitäisikö jokaiselle datalle luoda on-demandina kopio-info-olio lista,
  // josta palautus olisi nopea?
  // Pitääkö lisäksi laittaa extra-infoa eri datoista, että siivous rutiini tietää milloin niitä on
  // viimeksi käytetty,
  // mitä muuta infoa tarvitaan kirjanpitoon ja muuhin?
  // ***************************************************************************************************************
  boost::shared_ptr<NFmiFastQueryInfo> Info(boost::shared_ptr<NFmiDrawParam> &theDrawParam,
                                            bool fCrossSectionInfoWanted,
                                            bool fGetLatestIfArchiveNotFound,
                                            bool &fGetDataFromServer);
  boost::shared_ptr<NFmiFastQueryInfo> Info(boost::shared_ptr<NFmiDrawParam> &theDrawParam,
                                            bool fCrossSectionInfoWanted,
                                            bool fGetLatestIfArchiveNotFound);
  boost::shared_ptr<NFmiFastQueryInfo> Info(const NFmiDataIdent &theIdent,
                                            const NFmiLevel *theLevel,
                                            NFmiInfoData::Type theType,
                                            bool fUseParIdOnly = false,
                                            bool fLevelData = false,
                                            int theModelRunIndex = 0);
  checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > GetInfos(
      const std::string &theFileNameFilter,
      int theModelRunIndex = 0);  // palauttaa vectorin halutunlaisia infoja
  checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > GetInfos(
      int theProducerId,
      int theProducerId2 = -1,
      int theProducerId3 = -1,
      int theProducerId4 = -1);  // palauttaa vectorin halutun tuottajan infoja
  checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > GetInfos(NFmiInfoData::Type theDataType);
  checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > GetInfos(
      NFmiInfoData::Type theType,
      bool fGroundData,
      int theProducerId,
      int theProducerId2 = -1);  // palauttaa vectorin halutun tuottajan infoja
  boost::shared_ptr<NFmiFastQueryInfo> FindInfo(
      NFmiInfoData::Type theDataType,
      int theIndex = 0);  // Hakee indeksin mukaisen tietyn datatyypin infon
  boost::shared_ptr<NFmiFastQueryInfo> FindInfo(
      NFmiInfoData::Type theDataType,
      const NFmiProducer &theProducer,
      bool fGroundData,
      int theIndex = 0);  // Hakee indeksin mukaisen tietyn datatyypin infon
  boost::shared_ptr<NFmiFastQueryInfo> FindSoundingInfo(
      const NFmiProducer &theProducer,
      int theIndex = 0,
      ParamCheckFlags paramCheckFlags =
          ParamCheckFlags());  // Hakee parhaan luotaus infon tuottajalle
  boost::shared_ptr<NFmiFastQueryInfo> FindSoundingInfo(
      const NFmiProducer &theProducer,
      const NFmiMetTime &theDataTime,
      int theIndex = 0,
      ParamCheckFlags paramCheckFlags =
      ParamCheckFlags());  // Hakee parhaan luotaus infon tuottajalle
  boost::shared_ptr<NFmiFastQueryInfo> GetPrioritizedSoundingInfo(
      ParamCheckFlags paramCheckFlags =
          ParamCheckFlags());  // Hakee tietyn prioriteetin mukaisesti parhaan luotaus-infon

  // HUOM! Nämä makroParamData jutut pitää miettiä uusiksi, jos niitä aletaan käsittelemään eri
  // säikeissä. Tällöin
  // Niistä pitää luoda aina ilmeisesti paikalliset kopiot?!?!
  boost::shared_ptr<NFmiFastQueryInfo> MacroParamData(void);
  boost::shared_ptr<NFmiFastQueryInfo> CrossSectionMacroParamData(void);

  NFmiParamBag GetParams(int theProducerId1);
  int GetNearestUnRegularTimeIndex(boost::shared_ptr<NFmiDrawParam> &theDrawParam,
                                   const NFmiMetTime &theTime);

  boost::shared_ptr<NFmiDrawParam> CreateDrawParam(const NFmiDataIdent &theDataIdent,
                                                   const NFmiLevel *theLevel,
                                                   NFmiInfoData::Type theType);
  boost::shared_ptr<NFmiDrawParam> CreateCrossSectionDrawParam(const NFmiDataIdent &theDataIdent,
                                                               NFmiInfoData::Type theType);

  bool Clear(void);
  void ClearData(NFmiInfoData::Type theDataType);
  void ClearThisKindOfData(NFmiQueryInfo *theInfo,
                           NFmiInfoData::Type theDataType,
                           const std::string &theFileNamePattern,
                           NFmiTimeDescriptor &theRemovedDatasTimesOut);
  void ClearDynamicHelpData();

  const std::string &WorkingDirectory(void) const { return itsWorkingDirectory; };
  void WorkingDirectory(const std::string &newValue) { itsWorkingDirectory = newValue; };
  void UpdateEditedDataCopy(void);  // 28.09.1999/Marko

  void SetDrawParamPath(const std::string &theDrawParamPath);
  const std::string GetDrawParamPath(void);
  void SetMacroParamDataGridSize(int x, int y);
  void SetMacroParamDataMinGridSize(int x, int y);
  void SetMacroParamDataMaxGridSize(int x, int y);

  boost::shared_ptr<NFmiFastQueryInfo> CreateNewMacroParamData(int x,
                                                               int y,
                                                               NFmiInfoData::Type theDataType);

  const NFmiPoint &GetMacroParamDataGridSize(void) const { return itsMacroParamGridSize; }
  const NFmiPoint &GetMacroParamDataMaxGridSize(void) const { return itsMacroParamMaxGridSize; }
  const NFmiPoint &GetMacroParamDataMinGridSize(void) const { return itsMacroParamMinGridSize; }
  int CountData(void);
  double CountDataSize(void);
  void UpdateCrossSectionMacroParamDataSize(int x, int y);
  void UpdateMacroParamDataSize(int x, int y);
  static bool HasGoodParamsForSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                           const ParamCheckFlags &paramCheckFlags);
  static boost::shared_ptr<NFmiFastQueryInfo> CreateNewMacroParamData_checkedInput(
      int x, int y, NFmiInfoData::Type theDataType, boost::shared_ptr<NFmiArea> wantedArea = nullptr);
  // ***************************************************************************************************************

 private:
  // estetään kopi konstruktorin ja sijoitus operaattoreiden luonti
  NFmiInfoOrganizer &operator=(const NFmiInfoOrganizer &);
  NFmiInfoOrganizer(const NFmiInfoOrganizer &);

  bool AddEditedData(NFmiSmartInfo *theEditedData, int theUndoLevel);
  bool Add(NFmiOwnerInfo *theInfo,
           int theMaxLatestDataCount,
           int theModelRunTimeGap,
           bool &fDataWasDeletedOut);

  boost::shared_ptr<NFmiFastQueryInfo> Info(boost::shared_ptr<NFmiDrawParam> &theDrawParam,
                                            bool fCrossSectionInfoWanted);
  boost::shared_ptr<NFmiFastQueryInfo> GetInfo(const NFmiDataIdent &theIdent,
                                               const NFmiLevel *theLevel,
                                               NFmiInfoData::Type theType,
                                               bool fUseParIdOnly,
                                               int theModelRunIndex = 0);
  boost::shared_ptr<NFmiFastQueryInfo> GetInfo(boost::shared_ptr<NFmiDrawParam> &theDrawParam);
  boost::shared_ptr<NFmiFastQueryInfo> CrossSectionInfo(const NFmiDataIdent &theDataIdent,
                                                        NFmiInfoData::Type theType,
                                                        int theModelRunIndex = 0);
  boost::shared_ptr<NFmiFastQueryInfo> GetSynopPlotParamInfo(NFmiInfoData::Type theType);
  boost::shared_ptr<NFmiFastQueryInfo> GetSoundingPlotParamInfo(NFmiInfoData::Type theType);
  boost::shared_ptr<NFmiFastQueryInfo> GetMetarPlotParamInfo(NFmiInfoData::Type theType);
  boost::shared_ptr<NFmiFastQueryInfo> GetWantedProducerInfo(NFmiInfoData::Type theType,
                                                             FmiProducerName theProducerName);
  boost::shared_ptr<NFmiDrawParam> CreateSynopPlotDrawParam(const NFmiDataIdent &theDataIdent,
                                                            const NFmiLevel *theLevel,
                                                            NFmiInfoData::Type theType);
  bool IsInfosTwoOfTheKind(NFmiQueryInfo *theInfo1,
                           NFmiInfoData::Type theType1,
                           const std::string &theFileNamePattern,
                           const boost::shared_ptr<NFmiFastQueryInfo> &theInfo2);
  int IsGoodSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                         const NFmiProducer &theProducer,
                         bool ignoreProducer,
                         const ParamCheckFlags &paramCheckFlags);
  void FixMacroParamDataGridSize(int &x, int &y);

  boost::shared_ptr<NFmiQueryDataKeeper>
      itsEditedDataKeeper;  // pitää sisällään oikeasti NFmiSmartInfo-olion
  boost::shared_ptr<NFmiQueryDataKeeper> itsCopyOfEditedDataKeeper;
  MapType itsDataMap;  // sijoitus mappiin tapahtuu filepatternin avulla

  boost::shared_ptr<NFmiDrawParamFactory> itsDrawParamFactory;
  std::string itsWorkingDirectory;
  NFmiPoint itsMacroParamGridSize;
  NFmiPoint itsMacroParamMinGridSize;
  NFmiPoint itsMacroParamMaxGridSize;
  boost::shared_ptr<NFmiFastQueryInfo> itsMacroParamData;  // makro-parametrien laskuja varten pitää
  // pitää yllä yhden hilan kokoista dataa
  // (yksi aika,param ja level, editoitavan
  // datan hplaceDesc)
  boost::shared_ptr<NFmiFastQueryInfo> itsCrossSectionMacroParamData;  // poikkileikkaus
                                                                       // makro-parametrien laskuja
  // varten pitää pitää yllä
  // yhden hilan kokoista dataa
  // (yksi aika,param ja level,
  // editoitavan datan
  // hplaceDesc)
  bool fCreateEditedDataCopy;  // luodaanko vai eikö luoda kopiota editoidusta datasta
  static std::vector<FmiParameterName> itsWantedSoundingParams;
  static std::vector<FmiParameterName> itsWantedTrajectoryParams;
  static bool fCheckParamsInitialized;
  static void InitializeCheckParams(void);
};
