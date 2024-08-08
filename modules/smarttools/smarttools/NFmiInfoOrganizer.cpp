
#include "NFmiInfoOrganizer.h"
#include "NFmiDrawParam.h"
#include "NFmiDrawParamFactory.h"
#include "NFmiQueryDataKeeper.h"
#include "NFmiSmartInfo.h"
#include <newbase/NFmiFastInfoUtils.h>
#include <newbase/NFmiGrid.h>
#include <newbase/NFmiLatLonArea.h>
#include <newbase/NFmiQueryDataUtil.h>
#include <newbase/NFmiQueryInfo.h>

#ifdef _MSC_VER
#pragma warning(disable : 4239)  // poistaa VC++ 2010 varoituksen: warning C4239: nonstandard
                                 // extension used : 'argument' : conversion from
                                 // 'boost::shared_ptr<T>' to 'boost::shared_ptr<T> &'
#endif

namespace
{
// Kun infoOrganizerille annetaan uusia datoja, normaalisti niitä käsitellään kuten uusia datoja.
// On kuitenkin tilanteita missä ladattua data halutaan merkitä vanhaksi:
// 1) Kun SmartMet käynnistyy ja dataa ladataan lokaalilevyltä muistiin ensi kerran
// 2) Kun ladataan CaseStudy dataa
// 3) Kun palataan CaseStudy tilasta takaisin ja luetaan lokaalidatat uudestaan.
bool gMarkLoadedDataAsOld = false;
}  // namespace

std::vector<FmiParameterName> NFmiInfoOrganizer::itsWantedSoundingParams;
std::vector<FmiParameterName> NFmiInfoOrganizer::itsWantedTrajectoryParams;
bool NFmiInfoOrganizer::fCheckParamsInitialized = false;
NFmiPoint NFmiInfoOrganizer::itsMacroParamMinGridSize(5, 5);
NFmiPoint NFmiInfoOrganizer::itsMacroParamMaxGridSize(2000, 2000);

void NFmiInfoOrganizer::InitializeCheckParams()
{
  if (!fCheckParamsInitialized)
  {
    fCheckParamsInitialized = true;
    // Tuuliparametreja ei tarvitse lisätä näihin listoihin, ne tarkastetaan erikseen
    // CheckForSoundingParams funktiossa
    itsWantedSoundingParams.push_back(kFmiTemperature);
    itsWantedSoundingParams.push_back(kFmiDewPoint);
    itsWantedSoundingParams.push_back(kFmiHumidity);

    itsWantedTrajectoryParams.push_back(kFmiVelocityPotential);
    itsWantedTrajectoryParams.push_back(kFmiVerticalVelocityMMS);
  }
}

NFmiInfoOrganizer::NFmiInfoOrganizer()
    : itsEditedDataKeeper(),
      itsCopyOfEditedDataKeeper(),
      itsDataMap(),
      itsDrawParamFactory(),
      itsWorkingDirectory(),
      itsMacroParamGridSize(50, 50),
      itsOptimizedVisualizationGridSize(50, 50),
      itsMacroParamData(),
      itsCrossSectionMacroParamData(),
      fCreateEditedDataCopy(false)
{
  InitializeCheckParams();
}

NFmiInfoOrganizer::~NFmiInfoOrganizer() {}

bool NFmiInfoOrganizer::Init(const std::string &theDrawParamPath,
                             bool createDrawParamFileIfNotExist,
                             bool createEditedDataCopy,
                             bool fUseOnePressureLevelDrawParam)
{
  fCreateEditedDataCopy = createEditedDataCopy;
  itsDrawParamFactory = boost::shared_ptr<NFmiDrawParamFactory>(
      new NFmiDrawParamFactory(createDrawParamFileIfNotExist, fUseOnePressureLevelDrawParam));
  itsDrawParamFactory->LoadDirectory(theDrawParamPath);
  UpdateMacroParamDataSize(static_cast<int>(itsMacroParamGridSize.X()),
                           static_cast<int>(itsMacroParamGridSize.Y()));
  UpdateOptimizedVisualizationMacroParamDataSize(
      static_cast<int>(itsOptimizedVisualizationGridSize.X()),
      static_cast<int>(itsOptimizedVisualizationGridSize.Y()),
      nullptr);
  return itsDrawParamFactory->Init();
}

// ***** Datan lisäykseen liittyviä metodeja/funktioita ****************

bool NFmiInfoOrganizer::AddData(NFmiQueryData *theData,
                                const std::string &theDataFileName,
                                const std::string &theDataFilePattern,
                                NFmiInfoData::Type theDataType,
                                int theUndoLevel,
                                int theMaxLatestDataCount,
                                int theModelRunTimeGap,
                                int theFakeProducerId,
                                bool &fDataWasDeletedOut,
                                bool reloadCaseStudyData)
{
  bool status = false;
  fDataWasDeletedOut = false;
  if (theData)
  {
    if (theDataType == NFmiInfoData::kEditable)
    {
      status = AddEditedData(
          new NFmiSmartInfo(theData, theDataType, theDataFileName, theDataFilePattern),
          theUndoLevel);
    }
    else
    {
      // muun tyyppiset datat kuin editoitavat menevät mappiin
      status =
          Add(new NFmiOwnerInfo(
                  theData, theDataType, theDataFileName, theDataFilePattern, gMarkLoadedDataAsOld),
              theMaxLatestDataCount,
              theModelRunTimeGap,
              theFakeProducerId,
              fDataWasDeletedOut,
              reloadCaseStudyData);
    }
  }
  return status;
}

bool NFmiInfoOrganizer::AddEditedData(NFmiSmartInfo *theEditedData, int theUndoLevel)
{
  if (theEditedData)
  {
    boost::shared_ptr<NFmiOwnerInfo> dataPtr(theEditedData);
    theEditedData->First();
    try
    {
      if (theUndoLevel)
        theEditedData->UndoLevel(theUndoLevel);
    }
    catch (...)
    {
      // jos undo-levelin asetus epäonnistui syystä tai toisesta (esim. muisti loppui), asetetaan
      // syvyydeksi 0 ja koitetaan jatkaa...
      theEditedData->UndoLevel(0);
    }

    itsEditedDataKeeper = boost::shared_ptr<NFmiQueryDataKeeper>(new NFmiQueryDataKeeper(dataPtr));
    fCreateEditedDataCopy =
        theUndoLevel
            ? true
            : false;  // pitää päivittää kopion luomiseen vaikuttavaa muuttujaa undo-levelin mukaan
    UpdateEditedDataCopy();
    return true;
  }
  return false;
}

static void SetDataKeeperToZero(boost::shared_ptr<NFmiQueryDataKeeper> &theDataKeeper)
{
  theDataKeeper = boost::shared_ptr<NFmiQueryDataKeeper>(static_cast<NFmiQueryDataKeeper *>(0));
}

void NFmiInfoOrganizer::UpdateEditedDataCopy()
{
  if (fCreateEditedDataCopy)
  {
    if (itsEditedDataKeeper)  // Testaa toimiiko tällöinen sharep-ptr nolla tarkastus!!!!
    {
      boost::shared_ptr<NFmiOwnerInfo> tmpInfo(
          itsEditedDataKeeper->OriginalData()->NFmiOwnerInfo::Clone());
      itsCopyOfEditedDataKeeper =
          boost::shared_ptr<NFmiQueryDataKeeper>(new NFmiQueryDataKeeper(tmpInfo));
      itsCopyOfEditedDataKeeper->OriginalData()->DataType(NFmiInfoData::kCopyOfEdited);
      return;  // onnistuneen operaation jälkeen paluu, ettei mene datan nollaukseen
    }
  }
  // muissa tapauksissa varmuuden vuoksi nollataan copy-data
  ::SetDataKeeperToZero(itsCopyOfEditedDataKeeper);
}

bool NFmiInfoOrganizer::Add(NFmiOwnerInfo *theInfo,
                            int theMaxLatestDataCount,
                            int theModelRunTimeGap,
                            int theFakeProducerId,
                            bool &fDataWasDeletedOut,
                            bool reloadCaseStudyData)
{
  fDataWasDeletedOut = false;
  if (theInfo)
  {
    boost::shared_ptr<NFmiOwnerInfo> dataPtr(theInfo);
    MapType::iterator pos = itsDataMap.find(theInfo->DataFilePattern());
    if (pos != itsDataMap.end())
    {  // kyseinen data patterni löytyi, lisätään annettu data sen keeper-settiin
      if (pos->second->MaxLatestDataCount() != theMaxLatestDataCount)
        pos->second->MaxLatestDataCount(theMaxLatestDataCount);
      if (pos->second->ModelRunTimeGap() != theModelRunTimeGap)
        pos->second->ModelRunTimeGap(theModelRunTimeGap);
      pos->second->AddData(dataPtr, false, fDataWasDeletedOut);
    }
    else
    {
      // lisätään kyseinen data keeper-set listaan
      itsDataMap.insert(
          std::make_pair(theInfo->DataFilePattern(),
                         boost::shared_ptr<NFmiQueryDataSetKeeper>(
                             new NFmiQueryDataSetKeeper(dataPtr,
                                                        theMaxLatestDataCount,
                                                        theModelRunTimeGap,
                                                        theFakeProducerId,
                                                        reloadCaseStudyData,
                                                        kQueryDataKeepInMemoryTimeInMinutes))));
    }
    return true;
  }
  else
    return false;
}

// ***** Datan kyselyyn liittyviä metodeja/funktioita ****************

static bool UseParIdOnly(NFmiInfoData::Type theDataType)
{
  if (theDataType == NFmiInfoData::kEditable || theDataType == NFmiInfoData::kCopyOfEdited ||
      theDataType == NFmiInfoData::kAnyData)  // jos editoitava data, ei tuottajalla väliä
    return true;
  return false;
}

static bool CheckDataType(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                          NFmiInfoData::Type theType)
{
  bool anyDataOk = (theType == NFmiInfoData::kAnyData);
  if (theInfo && (theInfo->DataType() == theType || anyDataOk))
    return true;
  return false;
}

static bool CheckMetaWindParamCases(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                    const NFmiDataIdent &theDataIdent,
                                    bool fUseParIdOnly)
{
  if (fUseParIdOnly || (*theInfo->Producer() == *theDataIdent.GetProducer()))
  {
    // Tutki onko haluttu parametri joku mahdollisista tuulen meta parametreista
    switch (theDataIdent.GetParamIdent())
    {
      case kFmiWindDirection:
      case kFmiWindSpeedMS:
      case kFmiWindVectorMS:
      case kFmiWindUMS:
      case kFmiWindVMS:
      {
        NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage =
            NFmiFastInfoUtils::CheckMetaWindParamUsage(theInfo);
        if (metaWindParamUsage.HasWsAndWd() || metaWindParamUsage.HasWindComponents())
        {
          // Jos löytyy jompi kumpi parametri pareista, voidaan mitä tahansa laskea meta
          // parametreina
          return true;
        }
      }
      default:
        break;
    }
  }
  return false;
}

static bool CheckNormalDataIdentCase(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                     const NFmiDataIdent &theDataIdent,
                                     bool fUseParIdOnly)
{
  if (fUseParIdOnly ? theInfo->Param(static_cast<FmiParameterName>(theDataIdent.GetParamIdent()))
                    : theInfo->Param(theDataIdent))
    return true;
  else
    return false;
}

static bool CheckStreamlineCase(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                const NFmiDataIdent &theDataIdent,
                                bool fUseParIdOnly)
{
  if (theDataIdent.GetParamIdent() == NFmiInfoData::kFmiSpStreamline && theInfo->IsGrid())
  {
    // streamline parametri on ns. meta-parametri ja se pitää käsitellä erikseen
    // Tapaus-1: löytyykö tuuli u-komponenttia (oletus että silloin datasta löytyy myös
    // v-komponentti)
    NFmiDataIdent metaParamReplacer(theDataIdent);
    metaParamReplacer.GetParam()->SetIdent(kFmiWindUMS);
    if (::CheckNormalDataIdentCase(theInfo, metaParamReplacer, fUseParIdOnly))
      return true;
    // Tapaus-2: löytyykö tuulen nopeutta (oletus että silloin datasta löytyy myös tuulen suunta)
    metaParamReplacer.GetParam()->SetIdent(kFmiWindSpeedMS);
    if (::CheckNormalDataIdentCase(theInfo, metaParamReplacer, fUseParIdOnly))
      return true;
  }
  return false;
}

bool NFmiInfoOrganizer::CheckForDataIdent(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                          const NFmiDataIdent &theDataIdent,
                                          bool fUseParIdOnly)
{
  if (theInfo)
  {
    if (::CheckNormalDataIdentCase(theInfo, theDataIdent, fUseParIdOnly))
      return true;
    else if (::CheckStreamlineCase(theInfo, theDataIdent, fUseParIdOnly))
      return true;
    else if (::CheckMetaWindParamCases(theInfo, theDataIdent, fUseParIdOnly))
      return true;
  }
  return false;
}

void NFmiInfoOrganizer::MarkLoadedDataAsOld(bool newState)
{
  gMarkLoadedDataAsOld = newState;
}

bool NFmiInfoOrganizer::IsLoadedDataTreatedAsOld()
{
  return gMarkLoadedDataAsOld;
}

static bool CheckLevel(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                       const NFmiLevel *theLevel)
{
  if (theInfo && (!theLevel || (theLevel && theInfo->Level(*theLevel))))
    return true;
  return false;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::Info(
    boost::shared_ptr<NFmiDrawParam> &theDrawParam,
    bool fCrossSectionInfoWanted,
    bool fGetLatestIfArchiveNotFound,
    bool &fGetDataFromServer)
{
  fGetDataFromServer = false;
  boost::shared_ptr<NFmiFastQueryInfo> aInfo = Info(theDrawParam, fCrossSectionInfoWanted);
  if (aInfo == 0 && fGetLatestIfArchiveNotFound && theDrawParam->ModelRunIndex() < 0)
  {
    boost::shared_ptr<NFmiDrawParam> tmpDrawParam(new NFmiDrawParam(*theDrawParam));
    tmpDrawParam->ModelRunIndex(0);
    aInfo =
        Info(tmpDrawParam, fCrossSectionInfoWanted);  // koetetaan sitten hakea viimeisintä dataa
    if (aInfo)
      fGetDataFromServer = true;
  }
  return aInfo;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::Info(
    boost::shared_ptr<NFmiDrawParam> &theDrawParam,
    bool fCrossSectionInfoWanted,
    bool fGetLatestIfArchiveNotFound)
{
  bool getDataFromServer = false;
  return Info(
      theDrawParam, fCrossSectionInfoWanted, fGetLatestIfArchiveNotFound, getDataFromServer);
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::Info(
    boost::shared_ptr<NFmiDrawParam> &theDrawParam, bool fCrossSectionInfoWanted)
{
  NFmiInfoData::Type dataType = theDrawParam->DataType();
  if (fCrossSectionInfoWanted)
    return CrossSectionInfo(theDrawParam->Param(), dataType, theDrawParam->ModelRunIndex());
  else
    return GetInfo(theDrawParam);
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::GetInfo(
    boost::shared_ptr<NFmiDrawParam> &theDrawParam)
{
  NFmiLevel *level = &theDrawParam->Level();
  NFmiInfoData::Type dataType = theDrawParam->DataType();
  if (level && level->GetIdent() == 0)  // jos tämä on ns. default-level otus (GetIdent() == 0),
                                        // annetaan 0-pointteri Info-metodiin
    level = 0;
  return GetInfo(theDrawParam->Param(),
                 level,
                 dataType,
                 ::UseParIdOnly(dataType),
                 theDrawParam->ModelRunIndex());
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::Info(
    const NFmiDataIdent &theIdent,
    const NFmiLevel *theLevel,
    NFmiInfoData::Type theType,
    bool fUseParIdOnly,
    bool fLevelData,
    int theModelRunIndex,
    const std::vector<FmiParameterName> *possibleComparisonParameters)
{
  if (fLevelData)
    return CrossSectionInfo(theIdent, theType, theModelRunIndex);
  else
  {
    auto useParameterIdOnly = (fUseParIdOnly || ::UseParIdOnly(theType));
    auto info = GetInfo(theIdent, theLevel, theType, useParameterIdOnly, theModelRunIndex);
    if (info)
      return info;
    else if (possibleComparisonParameters)
    {
      // Yritetään josko datasta löytyisi haluttuja vertailu parametreja originaali parametrin
      // sijasta
      for (auto comparisonParameterId : *possibleComparisonParameters)
      {
        auto usedDataIdent = theIdent;
        usedDataIdent.GetParam()->SetIdent(comparisonParameterId);
        auto info = GetInfo(usedDataIdent, theLevel, theType, useParameterIdOnly, theModelRunIndex);
        if (info)
          return info;
      }
    }
  }
  return nullptr;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::GetWantedProducerInfo(
    NFmiInfoData::Type theType, FmiProducerName theProducerName)
{
  if (itsEditedDataKeeper && theType == NFmiInfoData::kEditable)
    return itsEditedDataKeeper->GetIter();
  else
  {
    for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    {
      auto aInfo = iter->second->GetFastInfoIter();
      if(aInfo && aInfo->DataType() == theType &&
          static_cast<FmiProducerName>(aInfo->Producer()->GetIdent()) == theProducerName)
      {
        return aInfo;
      }
    }
  }
  // Jos ei löytynyt sopivaa dataa, palautetaan tyhjä.
  return boost::shared_ptr<NFmiFastQueryInfo>();
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::GetSynopPlotParamInfo(
    NFmiInfoData::Type theType)
{
  return GetWantedProducerInfo(theType, kFmiSYNOP);
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::GetSoundingPlotParamInfo(
    NFmiInfoData::Type theType)
{
  // En uskalla rikkoa käytettyä logiikkaa ja käyttää suotaan
  // NFmiInfoOrganizer::GetPrioritizedSoundingInfo -metodia, koska
  // siinä ei oteta huomioon parametrina annettua theType: ollenkaan, siksi teen tähän uuden
  // priorisointi listan.
  // Prioriteetti haku järjestys: 1. Bufr-luotaus, 2. Temp-luotaus

  boost::shared_ptr<NFmiFastQueryInfo> soundingInfo = GetWantedProducerInfo(theType, kFmiBufrTEMP);
  if (!soundingInfo)
    soundingInfo = GetWantedProducerInfo(theType, kFmiTEMP);
  return soundingInfo;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::GetMetarPlotParamInfo(
    NFmiInfoData::Type theType)
{
  return GetWantedProducerInfo(theType, kFmiMETAR);
}

// HUOM! Nämä makroParamData jutut pitää miettiä uusiksi, jos niitä aletaan käsittelemään eri
// säikeissä. Tällöin
// Niistä pitää luoda aina ilmeisesti paikalliset kopiot?!?!
boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::MacroParamData()
{
  return itsMacroParamData;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::CrossSectionMacroParamData()
{
  return itsCrossSectionMacroParamData;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::TimeSerialMacroParamData()
{
  return itsTimeSerialMacroParamData;
}

static bool MatchData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                      NFmiInfoData::Type theType,
                      const NFmiDataIdent &theDataIdent,
                      bool fUseParIdOnly,
                      const NFmiLevel *theLevel)
{
  if (::CheckDataType(theInfo, theType) &&
      NFmiInfoOrganizer::CheckForDataIdent(theInfo, theDataIdent, fUseParIdOnly) &&
      ::CheckLevel(theInfo, theLevel))
    return true;

  return false;
}

static bool MatchCrossSectionData(const boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                  NFmiInfoData::Type theType,
                                  const NFmiDataIdent &theDataIdent,
                                  bool fUseParIdOnly)
{
  if (::CheckDataType(theInfo, theType) &&
      NFmiInfoOrganizer::CheckForDataIdent(theInfo, theDataIdent, fUseParIdOnly) &&
      theInfo->SizeLevels() > 1)
    return true;

  return false;
}

// Palauttaa annetun datan, paitsi jos kyse on arkisto datasta, tarkistetaan että sellainen löytyy
// ja palautetaan se (parametri asetettuna oikein).
// Jos ei löydy oikeaa arkisto dataa, palautetaan 0-pointteri.
static boost::shared_ptr<NFmiFastQueryInfo> DoArchiveCheck(
    boost::shared_ptr<NFmiFastQueryInfo> &theData,
    const NFmiDataIdent &theDataIdent,
    bool fUseParIdOnly,
    const NFmiLevel *theLevel,
    int theModelRunIndex,
    NFmiInfoOrganizer::MapType::iterator &theDataKeeperIter)
{
  boost::shared_ptr<NFmiFastQueryInfo> aInfo = theData;
  if (aInfo && theModelRunIndex < 0)
  {
    boost::shared_ptr<NFmiQueryDataKeeper> qDataKeeper =
        theDataKeeperIter->second->GetDataKeeper(theModelRunIndex);
    if (qDataKeeper)
      aInfo = qDataKeeper->GetIter();  // tässä katsotaan löytyykö vielä haluttu arkisto data
    else
      aInfo = boost::shared_ptr<NFmiFastQueryInfo>();  // ei löytynyt arkisto dataa, nollataan
    // info-pointteri, että data koetetaan sitten
    // hakea q2serveriltä
    NFmiInfoOrganizer::CheckForDataIdent(
        aInfo, theDataIdent, fUseParIdOnly);  // pitää asettaa arkisto datakin oikeaan parametriin
    ::CheckLevel(aInfo, theLevel);
  }
  return aInfo;
}

//--------------------------------------------------------
// GetInfo
// Yritin aluksi tehdä metodin käyttämällä parametria
// bool fIgnoreProducerName = false
// siksi että voisi olla samalta tuottajalta useita samantyyppisiä
// datoja käytössä yhtäaikaa, mutta tämä osoittautui liian
// haavoittuvaiseksi koska eri tilanteissa datoilla voi olla erilaisia nimiä
// ja tuottajien nimet pitää pystyä vaihtamaan ilman ongelmia
// querydatojen tuotanto ketjuissa.
// Kun törmäsin nyt kahteen eri ongelmaan:
// 1. querydatan tuottaja nimi vaihdetaan, mutta se on laitettu view-makroon.
// 2. EC:n 3 vrk pinta datassa on 12 utc ajossa eri nimi kuin 00 ajossa.
// SIKSI nyt metodi toimii siten että se yrittää etsiä dataa oikealla tuottaja nimellä.
// Mutta jos oikealla nimellä ei löytynyt, otetaan talteen 1. muilta kriteereiltä
// oikea data ja palautetaan se.
//
//--------------------------------------------------------
boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::GetInfo(const NFmiDataIdent &theDataIdent,
                                                                const NFmiLevel *theLevel,
                                                                NFmiInfoData::Type theType,
                                                                bool fUseParIdOnly,
                                                                int theModelRunIndex)
{
  // etsitää tähän 1. data joka muuten sopii kriteereihin, mutta jonka tuottaja nimi
  // on eri kuin haluttu. Jos oikealla nimellä ei löydy dataa, käytetään tätä.
  boost::shared_ptr<NFmiFastQueryInfo> backupData;
  if (theDataIdent.GetParamIdent() == NFmiInfoData::kFmiSpSynoPlot ||
      theDataIdent.GetParamIdent() == NFmiInfoData::kFmiSpMinMaxPlot)
  {
    // synop plot paramille pitää tehdä kikka (ja min/max plot 9996)
    return GetSynopPlotParamInfo(theType);
  }

  if (theDataIdent.GetParamIdent() == NFmiInfoData::kFmiSpMetarPlot)
  {
    // metar plot paramille pitää tehdä kikka (9995)
    return GetMetarPlotParamInfo(theType);
  }

  if (theLevel && theLevel->LevelType() == kFmiSoundingLevel &&
      (theDataIdent.GetParamIdent() == NFmiInfoData::kFmiSpSoundingPlot ||
       theDataIdent.GetProducer()->GetIdent() == kFmiTEMP))
  {
    // sounding plot paramille pitää tehdä kikka
    return GetSoundingPlotParamInfo(theType);
  }

  if (theType == NFmiInfoData::kMacroParam || theType == NFmiInfoData::kQ3MacroParam)
  {
    // macro- parametrit lasketaan tällä.
    // Tässä ei parametreja ja leveleitä ihmetellä, koska laskut tehdään aina ainoaan parametriin
    return MacroParamData();
  }

  if (theType == NFmiInfoData::kCrossSectionMacroParam)
  {
    // Tässä ei parametreja ja leveleitä ihmetellä, koska laskut tehdään aina ainoaan parametriin
    return CrossSectionMacroParamData();
  }

  if (theType == NFmiInfoData::kTimeSerialMacroParam)
  {
    // Tässä ei parametreja ja leveleitä ihmetellä, koska laskut tehdään aina ainoaan parametriin
    return TimeSerialMacroParamData();
  }

  if (theDataIdent.GetParamIdent() == NFmiInfoData::kFmiSpSelectedGridPoints)
  {
    // editoitu data on tässä haluttu data
    return itsEditedDataKeeper->GetIter();
  }

  boost::shared_ptr<NFmiFastQueryInfo> foundData;
  if (itsEditedDataKeeper &&
      ::MatchData(itsEditedDataKeeper->GetIter(), theType, theDataIdent, fUseParIdOnly, theLevel))
  {
    foundData = itsEditedDataKeeper->GetIter();
  }
  else if (itsCopyOfEditedDataKeeper && ::MatchData(itsCopyOfEditedDataKeeper->GetIter(),
                                                    theType,
                                                    theDataIdent,
                                                    fUseParIdOnly,
                                                    theLevel))
  {
    foundData = itsCopyOfEditedDataKeeper->GetIter();
  }
  else
  {
    // tutkitaan ensin löytyykö theParam suoraan joltain listassa olevalta
    // NFmiSmartInfo-pointterilta
    for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    {
      // tässä haetaan ensin viimeisin data!!
      auto aInfo = iter->second->GetFastInfoIter(0);
      if (::MatchData(aInfo, theType, theDataIdent, fUseParIdOnly, theLevel))
      {
        if (!(theLevel == 0 && aInfo->SizeLevels() > 1))
        {
          if (theDataIdent.GetProducer()->GetName() == aInfo->Param().GetProducer()->GetName())
          {
            foundData =
                ::DoArchiveCheck(aInfo,
                                 theDataIdent,
                                 fUseParIdOnly,
                                 theLevel,
                                 theModelRunIndex,
                                 iter);  // tämä saa olla 0-pointteri, jos kyse oli arkistodatasta
            if (foundData)
              break;
          }
          else if (backupData == 0)
            backupData = ::DoArchiveCheck(
                aInfo, theDataIdent, fUseParIdOnly, theLevel, theModelRunIndex, iter);
        }
      }
    }
  }
  if (foundData == 0 && backupData != 0)
    foundData = backupData;

  if (foundData)
  {
    if (foundData->SizeLevels() == 1)
      foundData->FirstLevel();
  }
  return foundData;
}

// Etsi haluttu crossSection-data. Eli pitää olla yli 1 leveliä
// eikä etsitä tiettyä leveliä.
// HUOM! Tein tähän CrossSectionInfo-metodiin saman tuottaja nimi ohitus virityksen kuin
// Info-metodiin. Ks. kommenttia sieltä.
boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::CrossSectionInfo(
    const NFmiDataIdent &theDataIdent, NFmiInfoData::Type theType, int theModelRunIndex)
{
  if (theType == NFmiInfoData::kCrossSectionMacroParam || theType == NFmiInfoData::kMacroParam)
    return CrossSectionMacroParamData();
  boost::shared_ptr<NFmiFastQueryInfo>
      backupData;  // etsitää tähän 1. data joka muuten sopii kriteereihin, mutta
                   // jonka tuottaja nimi on eri kuin haluttu. Jos oikealla nimellä ei löydy dataa,
                   // käytetään tätä.
  boost::shared_ptr<NFmiFastQueryInfo> foundData;
  if (itsEditedDataKeeper &&
      ::MatchCrossSectionData(itsEditedDataKeeper->GetIter(), theType, theDataIdent, true))
    foundData = itsEditedDataKeeper->GetIter();
  else
  {
    // tutkitaan ensin löytyykö theParam suoraan joltain listassa olevalta
    // NFmiSmartInfo-pointterilta
    for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    {
      // tässä haetaan ensin viimeisin data!!
      auto aInfo = iter->second->GetFastInfoIter(0);
      if (::MatchCrossSectionData(aInfo, theType, theDataIdent, false))
      {
        if (theDataIdent.GetProducer()->GetName() == aInfo->Param().GetProducer()->GetName())
        {
          foundData =
              ::DoArchiveCheck(aInfo,
                               theDataIdent,
                               false,
                               0,
                               theModelRunIndex,
                               iter);  // tämä saa olla 0-pointteri, jos kyse oli arkistodatasta
          if (foundData)
            break;
        }
        else if (backupData == 0)
          backupData = ::DoArchiveCheck(aInfo,
                                        theDataIdent,
                                        false,
                                        0,
                                        theModelRunIndex,
                                        iter);  // tähän laitetaan siis vain prod-namesta poikkeava
                                                // data (tämä tapahtuu mm. kun käyttäjä tekee
                                                // changeAllProducers-toiminnon)
      }
    }
  }

  if (foundData == 0 && backupData != 0)
    foundData = backupData;

  return foundData;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::FindInfo(
    NFmiInfoData::Type theDataType,
    int theIndex)  // Hakee indeksin mukaisen tietyn datatyypin infon
{
  if (itsEditedDataKeeper && theDataType == NFmiInfoData::kEditable)
    return itsEditedDataKeeper->GetIter();
  else if (itsCopyOfEditedDataKeeper && theDataType == NFmiInfoData::kCopyOfEdited)
    return itsCopyOfEditedDataKeeper->GetIter();
  else
  {
    int ind = 0;
    for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    {
      auto aInfo = iter->second->GetFastInfoIter();
      if (aInfo && aInfo->DataType() == theDataType)
      {
        if (ind == theIndex)
          return aInfo;
        ind++;
      }
    }
  }
  return boost::shared_ptr<NFmiFastQueryInfo>();
}

// Haetaan halutun datatyypin, tuottajan joko pinta tai level dataa (mahd indeksi kertoo sitten
// konfliktin
// yhteydessä, monesko otetaan)
boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::FindInfo(NFmiInfoData::Type theDataType,
                                                                 const NFmiProducer &theProducer,
                                                                 bool fGroundData,
                                                                 int theIndex)
{
  if (itsEditedDataKeeper && theDataType == NFmiInfoData::kEditable)
    return itsEditedDataKeeper->GetIter();
  else if (itsCopyOfEditedDataKeeper && theDataType == NFmiInfoData::kCopyOfEdited)
    return itsCopyOfEditedDataKeeper->GetIter();
  else
  {
    int ind = 0;
    for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    {
      auto aInfo = iter->second->GetFastInfoIter();
      if (aInfo && aInfo->DataType() == theDataType)
      {
        aInfo->FirstParam();  // pitää varmistaa, että producer löytyy
        if (*(aInfo->Producer()) == theProducer)
        {
          int levSize = aInfo->SizeLevels();
          if ((levSize == 1 && fGroundData) || (levSize > 1 && (!fGroundData)))
          {
            if (ind == theIndex)
              return aInfo;
            ind++;
          }
        }
      }
    }
  }
  return boost::shared_ptr<NFmiFastQueryInfo>();
}

static bool CheckForVerticalData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  return theInfo->PressureDataAvailable() || theInfo->HeightDataAvailable();
}

// Luotaus dataksi kelpaa, jos siitä löytyy jotkut tuuliparametrit (WS+WD tai u+v) TAI joku annetun
// listan parametreista.
static bool CheckForSoundingParams(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                   const std::vector<FmiParameterName> &wantedParams)
{
  if (::CheckForVerticalData(theInfo))
  {
    NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage =
        NFmiFastInfoUtils::CheckMetaWindParamUsage(theInfo);
    if (metaWindParamUsage.HasWsAndWd() || metaWindParamUsage.HasWindComponents())
      return true;

    for (size_t i = 0; i < wantedParams.size(); i++)
    {
      // Riittää kun yksikin halutuista parametreista löytyy
      if (theInfo->Param(wantedParams[i]))
        return true;
    }
  }

  return false;
}

// Trajektori dataksi kelpaa, jos siitä löytyy jotkut tuuliparametrit (WS+WD tai u+v) JA joku
// annetun listan parametreista.
static bool CheckForTrajectoryParams(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                     const std::vector<FmiParameterName> &wantedParams)
{
  if (::CheckForVerticalData(theInfo))
  {
    NFmiFastInfoUtils::MetaWindParamUsage metaWindParamUsage =
        NFmiFastInfoUtils::CheckMetaWindParamUsage(theInfo);
    if (metaWindParamUsage.HasWsAndWd() || metaWindParamUsage.HasWindComponents())
    {
      for (size_t i = 0; i < wantedParams.size(); i++)
      {
        // Riittää kun yksikin listatuista parametreista löytyy (= jompi kumpi vertikaaliliike
        // parametreista)
        if (theInfo->Param(wantedParams[i]))
          return true;
      }
    }
  }

  return false;
}

bool NFmiInfoOrganizer::IsTempData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  theInfo->FirstLevel();
  return (theInfo->Level()->LevelType() == kFmiSoundingLevel);
}

// Normaali tarkistus: onko id normaali TEMP tai Bufr-TEMP.
// Halutessa voidaan ottaa tarkasteluun myös ns. RAW-TEMP, joka on suotetty erikseen SmartMetin
// Luotaus-dialogin TEMP-syöttö dialogista.
bool NFmiInfoOrganizer::IsTempData(unsigned long theProducerId, bool includeRawTemp)
{
  if (includeRawTemp && theProducerId == kFmiRAWTEMP)
    return true;
  if (theProducerId == kFmiTEMP || theProducerId == kFmiBufrTEMP)
    return true;
  else
    return false;
}

int NFmiInfoOrganizer::CalcWantedParameterCount(
    boost::shared_ptr<NFmiFastQueryInfo> &info,
    const std::vector<FmiParameterName> &wantedParameters)
{
  int counter = 0;
  if (info && wantedParameters.size())
  {
    FmiParameterName oldParamId = static_cast<FmiParameterName>(info->Param().GetParamIdent());
    for (size_t i = 0; i < wantedParameters.size(); i++)
    {
      if (info->Param(wantedParameters[i]))
        counter++;
    }
    info->Param(oldParamId);
  }
  return counter;
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::GetInfoWithMostWantedParams(
    std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &infos,
    const std::vector<FmiParameterName> &wantedParameters)
{
  boost::shared_ptr<NFmiFastQueryInfo> info;
  if (infos.size())
  {
    int maxWantedParams = 0;
    for (size_t i = 0; i < infos.size(); i++)
    {
      int currentWantedParams =
          NFmiInfoOrganizer::CalcWantedParameterCount(infos[i], wantedParameters);
      if (currentWantedParams > maxWantedParams)
      {
        info = infos[i];
        maxWantedParams = currentWantedParams;
      }
    }
  }
  return info;
}

bool NFmiInfoOrganizer::HasGoodParamsForSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                                     const ParamCheckFlags &paramCheckFlags)
{
  InitializeCheckParams();  // varmistetaan että on alustettu lista tarkistettavista parametreista

  if (paramCheckFlags.fSounding)
    return ::CheckForSoundingParams(theInfo, itsWantedSoundingParams);
  else if (paramCheckFlags.fTrajectory)
    return ::CheckForTrajectoryParams(theInfo, itsWantedTrajectoryParams);

  return true;
}

// vastaus 0 = ei ole
// 1 = on sounding dataa, mutta ei välttämättä paras mahd.
// 2 = on hyvää dataa
// Tämä on malli datojen kanssa  niin että painepinta data on 1 ja hybridi on 2
int NFmiInfoOrganizer::IsGoodSoundingData(boost::shared_ptr<NFmiFastQueryInfo> &theInfo,
                                          const NFmiProducer &theProducer,
                                          bool ignoreProducer,
                                          const ParamCheckFlags &paramCheckFlags)
{
  if (theInfo)
  {
    if (ignoreProducer || (*(theInfo->Producer()) == theProducer))
    {
      if (theInfo->SizeLevels() >
          3)  // pitää olla väh 4 leveliä ennen kuin kelpuutetaan sounding dataksi
      {
        // Datassa pitää olla tiettyjä parametreja, että se kelpaa luotaukseen, ja liikkuvat
        // luotaukset ovat poikkeus, ne pitää päästää läpi myös, koska niilla ei ole muka
        // 'vertikaali' dataa
        if (HasGoodParamsForSoundingData(theInfo, paramCheckFlags) ||
            NFmiFastInfoUtils::IsMovingSoundingData(theInfo))
        {
          if (theInfo->DataType() == NFmiInfoData::kHybridData)
            return 2;
          else
            return 1;
        }
      }
    }
  }
  return 0;
}

static bool IsGivenTimeInDataRange(const boost::shared_ptr<NFmiFastQueryInfo> &info,
                                   const NFmiMetTime &wantedDataTime,
                                   int amdarDataStartOffsetInMinutes)
{
  if (!info)
    return false;
  if (wantedDataTime == NFmiMetTime::gMissingTime)
    return true;  // with missing-time we don't care if time in data's range
  else
  {
    auto isInsideTimeRange = info->TimeDescriptor().IsInside(wantedDataTime);
    if (isInsideTimeRange)
      return true;
    else
    {
      if (amdarDataStartOffsetInMinutes == 0)
        return false;
      else
      {
        auto startTimeOfTimeRange = wantedDataTime;
        startTimeOfTimeRange.ChangeByMinutes(-amdarDataStartOffsetInMinutes);
        return info->TimeDescriptor().IsInside(startTimeOfTimeRange);
      }
    }
  }
}

static bool IsGivenLatlonInDataArea(const boost::shared_ptr<NFmiFastQueryInfo> &info,
                                    const NFmiPoint &wantedLatlon)
{
  if (!info)
    return false;
  if (wantedLatlon == NFmiPoint::gMissingLatlon)
    return true;  // with missing-latlon we don't care if latlon is in area
  else
  {
    if (info->IsGrid() && !info->Area()->IsInside(wantedLatlon))
      return false;
    else
      return true;
  }
}

// Hakee parhaan luotaus infon tuottajalle. Eli jos kyseessä esim hirlam tuottaja, katsotaan
// löytyykö
// hybridi dataa ja sitten tyydytään viewable-dataa (= painepinta)
boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::FindSoundingInfo(
    const NFmiProducer &theProducer, int theIndex, ParamCheckFlags paramCheckFlags)
{
  return FindSoundingInfo(theProducer, NFmiMetTime::gMissingTime, NFmiPoint::gMissingLatlon, theIndex, paramCheckFlags);
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::FindSoundingInfo(
    const NFmiProducer &theProducer,
    const NFmiMetTime &theDataTime,
    const NFmiPoint &theLatlon,
    int theIndex,
    ParamCheckFlags paramCheckFlags,
    int amdarDataStartOffsetInMinutes)
{
  boost::shared_ptr<NFmiFastQueryInfo> exceptableInfo;
  for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
  {
    auto aInfo = iter->second->GetFastInfoIter();
    int result = IsGoodSoundingData(aInfo, theProducer, false, paramCheckFlags);
    if (!::IsGivenTimeInDataRange(aInfo, theDataTime, amdarDataStartOffsetInMinutes))
      result = 0;
    if (!::IsGivenLatlonInDataArea(aInfo, theLatlon))
      result = 0;
    if (result != 0 && theIndex < 0)
    {  // haetaan vanhempaa malliajo dataa
      auto dataKeeper = iter->second->GetDataKeeper(theIndex);
      if (dataKeeper)
        aInfo = dataKeeper->GetIter();
      else
        aInfo = boost::shared_ptr<NFmiFastQueryInfo>();  // ei löytynyt vanhoista malliajoista,
                                                         // pitää nollata pointteri
    }
    if (aInfo)
    {
      if (result == 2)
        return aInfo;
      else if (result == 1)
        exceptableInfo = aInfo;
    }
  }

  if (exceptableInfo)
    return exceptableInfo;

  boost::shared_ptr<NFmiFastQueryInfo> aInfo = FindInfo(NFmiInfoData::kEditable);
  if (aInfo)
  {
    if (theProducer.GetIdent() == kFmiMETEOR ||
        (*aInfo->Producer() ==
         theProducer))  // tässä hanskataan 'editoitu' data, jolloin ignoorataan tuottaja
    {
      int result = IsGoodSoundingData(aInfo, theProducer, true, paramCheckFlags);
      if (result != 0)
        exceptableInfo = aInfo;
    }
  }

  return exceptableInfo;
}

static boost::shared_ptr<NFmiFastQueryInfo> AllowOnlyStationData(
    const boost::shared_ptr<NFmiFastQueryInfo> &info)
{
  if (info)
  {
    if (!info->IsGrid())
      return info;
  }
  return nullptr;
}

// Prioriteetti haku järjestys: 1. editoitu data, 2. Bufr-luotaus, 3. Temp-luotaus
boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::GetPrioritizedSoundingInfo(
    ParamCheckFlags paramCheckFlags)
{
  boost::shared_ptr<NFmiFastQueryInfo> info =
      ::AllowOnlyStationData(FindSoundingInfo(NFmiProducer(kFmiMETEOR), 0, paramCheckFlags));
  if (info == 0)
  {
    info = AllowOnlyStationData(FindSoundingInfo(NFmiProducer(kFmiBufrTEMP), 0, paramCheckFlags));
    if (info == 0)
      info = AllowOnlyStationData(FindSoundingInfo(NFmiProducer(kFmiTEMP), 0, paramCheckFlags));
  }
  return info;
}

// Haetaan infoOrganizerista kaikki ne SmartInfot, joihin annettu fileNameFilter sopii.
// Mielestäni vastauksia pitäisi tulla korkeintaan yksi, mutta ehkä tulevaisuudessa voisi tulla
// lista.
// HUOM! Palauttaa vectorin halutunlaisia infoja, vectori ei omista pointtereita, joten infoja ei
// saa tuhota delete:llä.
// Ei käy läpi kEditable, eikä kCopyOfEdited erikois datoja!
std::vector<boost::shared_ptr<NFmiFastQueryInfo> > NFmiInfoOrganizer::GetInfos(
    const std::string &theFileNameFilter, int theModelRunIndex)
{
  std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;

  if (theFileNameFilter.empty() == false)
  {
    for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    {
      if (iter->second->FilePattern() == theFileNameFilter)
      {
        if (theModelRunIndex < 0)
        {
          auto fastInfo = iter->second->GetFastInfoIter(theModelRunIndex);
          if (fastInfo)
          {
            infoVector.push_back(fastInfo);
          }
        }
        else
        {
          auto fastInfo = iter->second->GetFastInfoIter();
          if (fastInfo)
          {
            infoVector.push_back(fastInfo);
          }
        }
      }
    }
  }
  return infoVector;
}

static bool IsProducerWanted(int theCurrentProdId,
                             int theProducerId1,
                             int theProducerId2,
                             int theProducerId3 = -1,
                             int theProducerId4 = -1)
{
  if (theCurrentProdId == theProducerId1)
    return true;
  else if (theProducerId2 != -1 && theCurrentProdId == theProducerId2)
    return true;
  else if (theProducerId3 != -1 && theCurrentProdId == theProducerId3)
    return true;
  else if (theProducerId4 != -1 && theCurrentProdId == theProducerId4)
    return true;
  return false;
}

// Palauttaa vectorin halutun tuottajan infoja, vectori ei omista pointtereita, joten infoja ei saa
// tuhota.
// Ei katso tuottaja datoja editable infosta eikä sen kopioista!
// voi antaa kaksi eri tuottaja id:tä jos haluaa, jos esim. hirlamia voi olla kahden eri tuottaja
// id:n alla
std::vector<boost::shared_ptr<NFmiFastQueryInfo> > NFmiInfoOrganizer::GetInfos(int theProducerId,
                                                                               int theProducerId2,
                                                                               int theProducerId3,
                                                                               int theProducerId4)
{
  std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;

  int currentProdId = 0;
  if (itsEditedDataKeeper)
  {
    boost::shared_ptr<NFmiFastQueryInfo> editedDataIter = itsEditedDataKeeper->GetIter();
    if (editedDataIter && editedDataIter->IsGrid() == false)  // laitetaan myös mahdollisesti
                                                              // editoitava data, jos kyseessä on
                                                              // asema dataa eli havainto
    {
      currentProdId =
          editedDataIter->FirstParamProducer().GetIdent();  // haetaan aina 1. parametrin tuottaja
                                                            // => ei satunnaisuutta, jos datassa on
      // väärin rakennettu parambagi jossa eri
      // tuottajia
      if (::IsProducerWanted(
              currentProdId, theProducerId, theProducerId2, theProducerId3, theProducerId4))
        infoVector.push_back(editedDataIter);
    }
  }

  for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
  {
    auto aInfo = iter->second->GetFastInfoIter();
    if (aInfo)
    {
      // haetaan aina 1. parametrin tuottaja => ei satunnaisuutta, 
      // jos datassa on väärin rakennettu parambagi jossa eri tuottajia
      currentProdId = static_cast<int>(
          aInfo->FirstParamProducer().GetIdent());
      if (::IsProducerWanted(
              currentProdId, theProducerId, theProducerId2, theProducerId3, theProducerId4))
      {
        infoVector.push_back(aInfo);
      }
    }
  }
  return infoVector;
}

// HUOM! Tästä pitää tehdä multithreaddauksen kestävää koodia, eli
// iteraattorista pitää tehdä lokaali kopio.
std::vector<boost::shared_ptr<NFmiFastQueryInfo> > NFmiInfoOrganizer::GetInfos(
    NFmiInfoData::Type theType, bool fGroundData, int theProducerId, int theProducerId2)
{
  std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;
  if (theType == NFmiInfoData::kEditable)
  {
    if (itsEditedDataKeeper)
    {
      boost::shared_ptr<NFmiFastQueryInfo> info = itsEditedDataKeeper->GetIter();
      if (info)
        infoVector.push_back(itsEditedDataKeeper->GetIter());
    }
  }
  else if (theType == NFmiInfoData::kCopyOfEdited)
  {
    if (itsCopyOfEditedDataKeeper)
    {
      boost::shared_ptr<NFmiFastQueryInfo> info = itsCopyOfEditedDataKeeper->GetIter();
      if (info)
        infoVector.push_back(itsEditedDataKeeper->GetIter());
    }
  }
  else
  {
    for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    {
      auto info = iter->second->GetFastInfoIter();
      if (info && info->DataType() == theType)
      {
        if ((fGroundData == true && info->SizeLevels() == 1) ||
            (fGroundData == false && info->SizeLevels() > 1))
        {
          // HUOM! info->Producer() on potentiaalisti vaarallinen kutsu multi-threaddaavassa
          // tilanteessa.
          int currentProdId = static_cast<int>(info->Producer()->GetIdent());
          if (::IsProducerWanted(currentProdId, theProducerId, theProducerId2))
            infoVector.push_back(info);
        }
      }
    }
  }
  return infoVector;
}

// Palauttaa vectorin viewable infoja, vectori ei omista pointtereita,
// joten infoja ei saa tuhota.
std::vector<boost::shared_ptr<NFmiFastQueryInfo> > NFmiInfoOrganizer::GetInfos(
    NFmiInfoData::Type theDataType)
{
  std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infoVector;

  if (itsEditedDataKeeper && theDataType == NFmiInfoData::kEditable)
  {
    boost::shared_ptr<NFmiFastQueryInfo> editedDataIter = itsEditedDataKeeper->GetIter();
    if (editedDataIter)
      infoVector.push_back(editedDataIter);
  }
  else if (itsCopyOfEditedDataKeeper && theDataType == NFmiInfoData::kCopyOfEdited)
  {
    boost::shared_ptr<NFmiFastQueryInfo> copyOfEditedDataIter =
        itsCopyOfEditedDataKeeper->GetIter();
    if (copyOfEditedDataIter)
      infoVector.push_back(copyOfEditedDataIter);
  }
  else
  {
    for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    {
      auto info = iter->second->GetFastInfoIter();
      if (info && info->DataType() == theDataType)
      {
        infoVector.push_back(info);
      }
    }
  }
  return infoVector;
}

NFmiParamBag NFmiInfoOrganizer::GetParams(int theProducerId1)
{
  NFmiParamBag paramBag;
  std::vector<boost::shared_ptr<NFmiFastQueryInfo> > infos(GetInfos(theProducerId1));
  size_t size = infos.size();
  if (size > 0)
  {
    for (size_t i = 0; i < size; i++)
    {
      paramBag = paramBag.Combine(infos[i]->ParamBag());
    }
  }

  return paramBag;
}

//--------------------------------------------------------
// CreateDrawParam(NFmiDataIdent& theDataIdent)
//--------------------------------------------------------
// Tutkii löytyykö listasta itsList infoa, jossa on theDataIdent - siis
// etsitään info, jonka tuottaja ja parametri saadaan theDataIdent:stä.
// Jos tälläinen info löytyy, pyydetään itsDrawParamFactory luomaan
// drawParam kyseiselle parametrille löydetyn infon avulla.
boost::shared_ptr<NFmiDrawParam> NFmiInfoOrganizer::CreateDrawParam(const NFmiDataIdent &theIdent,
                                                                    const NFmiLevel *theLevel,
                                                                    NFmiInfoData::Type theType)
{
  // Huomaa, että itsDrawParamFactory luo pointterin drawParam new:llä, joten
  // drawParam pitää muistaa tuhota  NFmiInfoOrganizer:n ulkopuolella
  boost::shared_ptr<NFmiDrawParam> drawParam;
  if (theType == NFmiInfoData::kSatelData ||
      theType == NFmiInfoData::kConceptualModelData)  // spesiaali keissi satelliitti kuville,
                                                      // niillä ei ole infoa
  {
    drawParam =
        boost::shared_ptr<NFmiDrawParam>(new NFmiDrawParam(theIdent, NFmiLevel(), 1, theType));
    drawParam->ParameterAbbreviation(static_cast<char *>(theIdent.GetParamName()));
    return drawParam;
  }
  if (theIdent.GetParamIdent() ==
      NFmiInfoData::kFmiSpSynoPlot)  // synop plottia varten taas kikkailua
  {
    return CreateSynopPlotDrawParam(theIdent, theLevel, theType);
  }
  drawParam = itsDrawParamFactory->CreateDrawParam(theIdent, theLevel);
  if (drawParam)
    drawParam->DataType(theType);  // data tyyppi pitää myös asettaa!!
  return drawParam;
}

// hakee poikkileikkausta varten haluttua dataa ja luo siihen sopivan drawparamin
boost::shared_ptr<NFmiDrawParam> NFmiInfoOrganizer::CreateCrossSectionDrawParam(
    const NFmiDataIdent &theDataIdent, NFmiInfoData::Type theType)
{
  boost::shared_ptr<NFmiDrawParam> drawParam =
      itsDrawParamFactory->CreateCrossSectionDrawParam(theDataIdent);
  if (drawParam)
    drawParam->DataType(theType);  // data tyyppi pitää myös asettaa!!
  return drawParam;
}

boost::shared_ptr<NFmiDrawParam> NFmiInfoOrganizer::CreateSynopPlotDrawParam(
    const NFmiDataIdent &theDataIdent, const NFmiLevel *theLevel, NFmiInfoData::Type theType)
{
  NFmiDataIdent usedDataIdent(theDataIdent);
  if (usedDataIdent.GetProducer()->GetIdent() == 0)
  {  // tämä pitää fiksata, että saan q2-serveriltä haetut synopit plottautumaan, jostain syystä
    // originaali systeemissä synop-plottauksen tuottaja on dummy arvoilla täytetty
    usedDataIdent.GetProducer()->SetIdent(kFmiSYNOP);
    usedDataIdent.GetProducer()->SetName("Synop");
  }
  boost::shared_ptr<NFmiDrawParam> drawParam = itsDrawParamFactory->CreateDrawParam(
      usedDataIdent,
      theLevel);  // false merkitsee, että parametria ei taas aseteta tuolla metodissa
  if (drawParam)
    drawParam->DataType(theType);
  return drawParam;
}

//--------------------------------------------------------
// Clear
//--------------------------------------------------------
// tuhoaa aina datan
bool NFmiInfoOrganizer::Clear()
{
  itsDataMap.clear();
  return true;  // tämä paluu arvo on turha
}

// TODO: Tämän käyttöä pitää miettiä, halutaanko siivota pois koko setti vai mitä?
void NFmiInfoOrganizer::ClearData(NFmiInfoData::Type theDataType)
{
  if (theDataType == NFmiInfoData::kEditable)
  {
    ::SetDataKeeperToZero(itsEditedDataKeeper);
    UpdateEditedDataCopy();
  }
  else
  {  // käydään lista läpi ja tuhotaan halutun tyyppiset datat
    MapType::iterator iter = itsDataMap.begin();
    for (;;)
    {
      if (iter == itsDataMap.end())
        break;

      auto fastInfo = iter->second->GetFastInfoIter();
      if (fastInfo && fastInfo->DataType() == theDataType)
      {
#ifdef UNIX
        // RHEL5 and RHEL6 bug??
        MapType::iterator tmp = iter;
        ++iter;
        itsDataMap.erase(tmp);
#else
        iter = itsDataMap.erase(iter);
#endif
      }
      else
        ++iter;  // jos ei poistettu objektia, pitää iteraattoria edistää....
    }
  }
}

// this kind of määritellään tällä hetkellä:
// parametrien, leveleiden ja mahdollisen gridin avulla (ei location bagin avulla)
// TODO: tarvitaanko tälläistä tarkastelua, parametritlistat tai levelit voiva muuttua jonain
// päivänä saman tyyppisessä datassa, pitäisikö tehdä tästä löysempi tarkastelu?!?
bool NFmiInfoOrganizer::IsInfosTwoOfTheKind(NFmiQueryInfo *theInfo1,
                                            NFmiInfoData::Type theType1,
                                            const std::string &theFileNamePattern,
                                            const boost::shared_ptr<NFmiFastQueryInfo> &theInfo2)
{
  // parametrit ja tuottajat samoja
  if (theInfo1 && theInfo2)
  {
    if (theType1 == theInfo2->DataType())
    {
      const std::string &dataFilePatternStr =
          dynamic_cast<NFmiOwnerInfo *>(theInfo2.get())->DataFilePattern();
      if (theFileNamePattern.empty() == false && dataFilePatternStr.empty() == false &&
          theFileNamePattern == dataFilePatternStr)
        return true;  // jos filepatternit eivät olleet tyhjiä ja ne ovat samanlaisia, pidetään
      // datoja samanlaisina (tämä auttaa, jos datat muuttuvat (paramereja lisää,
      // asemia lisää jne.))
      if (theInfo1->ParamBag() == theInfo2->ParamBag())
      {
        // Level tyypit pitääolla samoja ja niiden lukumäärät niin että joko leveleitä on molemmissa
        // tasan yksi tai molemmissa on yli yksi.
        // Ongelmia muuten data päivityksen kanssa jos esim. luotaus datoissa on eri määrä leveleitä
        // tai päivitetään hybridi dataa ja level määrä muuttuu tms.
        FmiLevelType levelType1 = theInfo1->VPlaceDescriptor().Level(0)->LevelType();
        FmiLevelType levelType2 = theInfo2->VPlaceDescriptor().Level(0)->LevelType();
        unsigned long size1 = theInfo1->VPlaceDescriptor().Size();
        unsigned long size2 = theInfo2->VPlaceDescriptor().Size();
        if (levelType1 == levelType2 && (size1 == size2 || (size1 > 1 && size2 > 1)))
        {
          // mahdollinen gridi samoja
          bool status3 = true;
          if (theInfo1->Grid() && theInfo2->Grid())
          {
            status3 = (theInfo1->Grid()->AreGridsIdentical(*(theInfo2->Grid()))) == true;
          }
          if (status3)
          {
            theInfo1->FirstParam();  // varmistaa, että producer löytyy
            theInfo2->FirstParam();
            if (*theInfo1->Producer() == *theInfo2->Producer())
              return true;
          }
        }
      }
    }
  }
  return false;
}

// TODO: Tämän käyttöä pitää miettiä, halutaanko siivota pois koko setti vai mitä? NYT AddData tekee
// siirrot DataSetKeeper:issä jo automaattisesti!!!
// ******
// theRemovedDatasTimesOut -parametri käytetään optimoidaan esim. havaittua hila datan kanssa
// tehtäviä ruudun päivityksiä,
// editoitavasta datasta ei oteta mitään aikoja talteen.
void NFmiInfoOrganizer::ClearThisKindOfData(NFmiQueryInfo *theInfo,
                                            NFmiInfoData::Type theDataType,
                                            const std::string &theFileNamePattern,
                                            NFmiTimeDescriptor &theRemovedDatasTimesOut)
{
  if (theInfo)
  {
    if (itsEditedDataKeeper)
    {
      if (IsInfosTwoOfTheKind(
              theInfo, theDataType, theFileNamePattern, itsEditedDataKeeper->GetIter()))
      {
        ::SetDataKeeperToZero(itsEditedDataKeeper);
        UpdateEditedDataCopy();
        return;
      }
    }

    for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    {
      auto info = iter->second->GetFastInfoIter();
      if (IsInfosTwoOfTheKind(theInfo, theDataType, theFileNamePattern, info))
      {
        theRemovedDatasTimesOut = info->TimeDescriptor();
        // TODO en tiedä mitä pitäisi tehdä?!?!? tuhota data vai mitä? Onko se jo tehty/tehdäänkö se
        // muualla?
        break;
      }
    }
  }
}

static bool DeleteDynamicHelpData(const std::vector<NFmiInfoData::Type> &ignoreTypesVector,
                                  bool caseStudyEvent,
                                  NFmiInfoOrganizer::MapType::iterator &dataIterator)
{
  auto fastInfo = dataIterator->second->GetFastInfoIter();
  if (!fastInfo)
  {
    return false;
  }

  if (std::find(ignoreTypesVector.begin(), ignoreTypesVector.end(), fastInfo->DataType()) !=
      ignoreTypesVector.end())
  {
    return false;
  }

  if (caseStudyEvent && !dataIterator->second->ReloadCaseStudyData())
  {
    return false;
  }

  return true;
}

void NFmiInfoOrganizer::ClearDynamicHelpData(bool caseStudyEvent)
{
  if (itsEditedDataKeeper)
    itsEditedDataKeeper.reset();
  if (itsCopyOfEditedDataKeeper)
    itsCopyOfEditedDataKeeper.reset();

  std::vector<NFmiInfoData::Type> ignoreTypesVector;
  // Stationaariset eli esim. topografia data ei kuulu poistettaviin datoihin
  ignoreTypesVector.push_back(NFmiInfoData::kStationary);

  // käydään lista läpi ja tuhotaan dynaamiset help datat
  for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end();)
  {
    if (::DeleteDynamicHelpData(ignoreTypesVector, caseStudyEvent, iter))
    {
#ifdef UNIX
      // RHEL5 and RHEL6 bug?
      MapType::iterator tmp = iter;
      ++iter;
      itsDataMap.erase(tmp);
#else
      iter = itsDataMap.erase(iter);
#endif
    }
    else
      ++iter;  // jos dataa ei poistettu, pitää sitä siirtää tässä pykälä eteenpäin
  }
}

const std::string NFmiInfoOrganizer::GetDrawParamPath()
{
  std::string retValue;
  if (itsDrawParamFactory)
    retValue = itsDrawParamFactory->LoadDirectory();
  return retValue;
}

// Tätä metodia käytetään ainakin aluksi luomaan dataa infoOrganizerin ulkopuolelle.
// Sitä käytetään NFmiExtraMacroParamData -luokassa luomaan halutun resoluutioista
// macroParam dataa.
boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::CreateNewMacroParamData(
    int x, int y, NFmiInfoData::Type theDataType)
{
  NFmiInfoOrganizer::FixMacroParamDataGridSize(x, y);
  return NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(x, y, theDataType);
}

void NFmiInfoOrganizer::FixMacroParamDataGridSize(int &x, int &y)
{
  x = FmiMin(x, static_cast<int>(itsMacroParamMaxGridSize.X()));
  y = FmiMin(y, static_cast<int>(itsMacroParamMaxGridSize.Y()));
  x = FmiMax(x, static_cast<int>(itsMacroParamMinGridSize.X()));
  y = FmiMax(y, static_cast<int>(itsMacroParamMinGridSize.Y()));
}

void NFmiInfoOrganizer::SetMacroParamDataGridSize(int x, int y)
{
  NFmiInfoOrganizer::FixMacroParamDataGridSize(x, y);
  itsMacroParamGridSize = NFmiPoint(x, y);
  UpdateMacroParamDataSize(x, y);
}

void NFmiInfoOrganizer::SetOptimizedVisualizationMacroParamDataGridSize(int x, int y)
{
  NFmiInfoOrganizer::FixMacroParamDataGridSize(x, y);
  itsOptimizedVisualizationGridSize = NFmiPoint(x, y);
  UpdateOptimizedVisualizationMacroParamDataSize(x, y, nullptr);
}

void NFmiInfoOrganizer::SetMacroParamDataMinGridSize(int x, int y)
{
  itsMacroParamMinGridSize = NFmiPoint(x, y);
}
void NFmiInfoOrganizer::SetMacroParamDataMaxGridSize(int x, int y)
{
  itsMacroParamMaxGridSize = NFmiPoint(x, y);
}

static NFmiQueryData *CreateDefaultMacroParamQueryData(const NFmiArea *theArea,
                                                       int gridSizeX,
                                                       int gridSizeY)
{
  NFmiLevelBag levelBag;
  levelBag.AddLevel(NFmiLevel(kFmiGroundSurface, 0));  // ihan mitä puppua vain, ei väliä
  NFmiVPlaceDescriptor vPlace(levelBag);

  NFmiParamBag parBag;
  parBag.Add(NFmiDataIdent(NFmiParam(
      998, "macroParam", kFloatMissing, kFloatMissing, 1, 0, NFmiString("%.1f"), kLinearly)));
  NFmiParamDescriptor parDesc(parBag);

  NFmiMetTime originTime;
  NFmiTimeBag validTimes(originTime, originTime, 60);  // yhden kokoinen feikki timebagi
  NFmiTimeDescriptor timeDesc(originTime, validTimes);

  NFmiGrid grid(theArea, gridSizeX, gridSizeY);
  // Pitää varmistaa että annetussa alueessa on 0,0 - 1,1 relatiivienen alue
  grid.Area()->SetXYArea(NFmiRect(0, 0, 1, 1));
  NFmiHPlaceDescriptor hPlace(grid);

  NFmiQueryInfo info(parDesc, timeDesc, hPlace, vPlace);
  return NFmiQueryDataUtil::CreateEmptyData(info);
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(
    int x, int y, NFmiInfoData::Type theDataType, const NFmiArea *wantedArea)
{
  static boost::shared_ptr<NFmiArea> dummyArea(
      new NFmiLatLonArea(NFmiPoint(19, 57), NFmiPoint(32, 71)));

  auto usedArea = wantedArea ? wantedArea : dummyArea.get();
  // Luo uusi data jossa on yksi aika,param ja level ja luo hplaceDesc annetusta areasta ja hila
  // koosta
  NFmiQueryData *data = ::CreateDefaultMacroParamQueryData(usedArea, x, y);
  if (data)
  {
    return boost::shared_ptr<NFmiFastQueryInfo>(
        new NFmiOwnerInfo(data, theDataType, "", "", false));
  }
  else
    return boost::shared_ptr<NFmiFastQueryInfo>();
}

void NFmiInfoOrganizer::UpdateMacroParamDataSize(int x, int y)
{
  itsMacroParamData =
      NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(x, y, NFmiInfoData::kMacroParam);
}

void NFmiInfoOrganizer::UpdateOptimizedVisualizationMacroParamDataSize(
    int x, int y, boost::shared_ptr<NFmiArea> wantedArea)
{
  itsOptimizedVisualizationMacroParamData = NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(
      x, y, NFmiInfoData::kMacroParam, wantedArea.get());
}

void NFmiInfoOrganizer::UpdateCrossSectionMacroParamDataSize(int x, int y)
{
  itsCrossSectionMacroParamData = NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(
      x, y, NFmiInfoData::kCrossSectionMacroParam);
}

void NFmiInfoOrganizer::UpdateTimeSerialMacroParamDataSize(int x)
{
  itsTimeSerialMacroParamData = NFmiInfoOrganizer::CreateNewMacroParamData_checkedInput(
      x, 1, NFmiInfoData::kTimeSerialMacroParam);
}

int NFmiInfoOrganizer::CountData()
{
  int count = 0;
  if (itsEditedDataKeeper)
    count++;
  if (itsCopyOfEditedDataKeeper)
    count++;

  for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    count += static_cast<int>(iter->second->DataCount());

  return count;
}

double NFmiInfoOrganizer::CountDataSize()
{
  double dataSize = 0;
  if (itsEditedDataKeeper)
    dataSize += itsEditedDataKeeper->OriginalData()->Size() * sizeof(float);
  if (itsCopyOfEditedDataKeeper)
    dataSize += itsCopyOfEditedDataKeeper->OriginalData()->Size() * sizeof(float);

  for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    dataSize += iter->second->DataByteCount();

  return dataSize;
}

int NFmiInfoOrganizer::CleanUnusedDataFromMemory()
{
  int dataRemovedCounter = 0;
  for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
    dataRemovedCounter += iter->second->CleanUnusedDataFromMemory();
  return dataRemovedCounter;
}

void NFmiInfoOrganizer::SetupVirtualTime(const NFmiMetTime &virtualTime, bool virtualTimeUsed)
{
  for (auto &dataMapPair : itsDataMap)
  {
    dataMapPair.second->SetupVirtualTime(virtualTime, virtualTimeUsed);
  }
}

// Jos kyse ns. editoidusta datasta (esim. kepa-data), joilla on epäsäännöllinen ilmestymis
// aikaväli,
// etsitään qdatakeeperin listoilta lähin origin aika ennen annettua aikaa ja palautetaan sen
// indeksi.
// Jos ei löydy aikaa ennen annettua aikaa, palautetaan viimeinen indeksi (eli vanhimman ajan
// indeksi).
// Jos ei löytynyt sopivaa epsäännöllistä dataKeeperiä, palautetaan arvo 99, joka kertoo että ei
// löydy.
int NFmiInfoOrganizer::GetNearestUnRegularTimeIndex(boost::shared_ptr<NFmiDrawParam> &theDrawParam,
                                                    const NFmiMetTime &theTime)
{
  for (MapType::iterator iter = itsDataMap.begin(); iter != itsDataMap.end(); ++iter)
  {
    if (iter->second->ModelRunTimeGap() == -1)
    {
      boost::shared_ptr<NFmiFastQueryInfo> aInfo = GetInfo(theDrawParam);
      if (aInfo)
      {  // löytyi haluttu dataKeeper, nyt katsotaan minkä indeksin saadaan palautettua
        return iter->second->GetNearestUnRegularTimeIndex(theTime);
      }
    }
  }
  return 0;
}

// Tämä tekee dynaamisesti matalan kopion, joka tarkoittaa että se kokeilee dynaamisesti mistä
// FastInfon lapsiluokasta on
// kyse ja tekee siitä matalan kopion (= dataa ei kopioida). Tätä tarvitaan, jos käyttöön pitää
// saada NFmiOwnerInfo- tai
// NFmiSmartInfo- tason otuksia.
boost::shared_ptr<NFmiFastQueryInfo> NFmiInfoOrganizer::DoDynamicShallowCopy(
    const boost::shared_ptr<NFmiFastQueryInfo> &theInfo)
{
  if (theInfo)
  {
    NFmiSmartInfo *smartInfo = dynamic_cast<NFmiSmartInfo *>(theInfo.get());
    if (smartInfo)
      return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiSmartInfo(*smartInfo));

    NFmiOwnerInfo *ownerInfo = dynamic_cast<NFmiOwnerInfo *>(theInfo.get());
    if (ownerInfo)
      return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiOwnerInfo(*ownerInfo));

    NFmiFastQueryInfo *fastInfo = dynamic_cast<NFmiFastQueryInfo *>(theInfo.get());
    if (fastInfo)
      return boost::shared_ptr<NFmiFastQueryInfo>(new NFmiFastQueryInfo(*fastInfo));
  }

  return boost::shared_ptr<NFmiFastQueryInfo>();
}
