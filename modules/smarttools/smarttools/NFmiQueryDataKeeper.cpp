
#include "NFmiQueryDataKeeper.h"
#include "NFmiSmartInfo.h"
#include "NFmiDrawParam.h"
#include <newbase/NFmiFileString.h>
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiQueryData.h>
#include <fstream>
#include <thread>

namespace
{
TraceLogMessageCallback g_TraceLogMessageCallback;
IsTraceLoggingInUseCallback g_IsTraceLoggingInUseCallback;
// Runsaasti loggausta tekevän koodin saa pois päältä, kun laittaa tähän false arvon.
bool g_AllowTraceLogging = true;

bool IsTraceLoggingUsed()
{
  if (g_AllowTraceLogging)
  {
    if (g_TraceLogMessageCallback && g_IsTraceLoggingInUseCallback)
    {
      return g_IsTraceLoggingInUseCallback();
    }
  }

  return false;
}

void TraceLogMessage(const std::string &message)
{
  if (IsTraceLoggingUsed())
  {
    g_TraceLogMessageCallback(message);
  }
}

}  // namespace

// ************* NFmiQueryDataKeeper-class **********************

void NFmiQueryDataSetKeeper::SetTraceLogMessageCallback(
    TraceLogMessageCallback &traceLogMessageCallback)
{
  g_TraceLogMessageCallback = traceLogMessageCallback;
}

void NFmiQueryDataSetKeeper::SetIsTraceLoggingInUseCallback(
    IsTraceLoggingInUseCallback &isTraceLoggingInUseCallback)
{
  g_IsTraceLoggingInUseCallback = isTraceLoggingInUseCallback;
}

NFmiQueryDataKeeper::NFmiQueryDataKeeper()
    : itsData(),
      itsLastTimeUsedTimer(),
      itsIndex(0),
      itsIteratorList(),
      itsOriginTime(NFmiMetTime::gMissingTime),
      itsDataFileName()
{
}

NFmiQueryDataKeeper::NFmiQueryDataKeeper(boost::shared_ptr<NFmiOwnerInfo> &theOriginalData)
    : itsData(theOriginalData),
      itsLastTimeUsedTimer(),
      itsIndex(0),
      itsIteratorList(),
      itsOriginTime(theOriginalData->OriginTime()),
      itsDataFileName(theOriginalData->DataFileName())
{
}

NFmiQueryDataKeeper::~NFmiQueryDataKeeper() {}

boost::shared_ptr<NFmiOwnerInfo> NFmiQueryDataKeeper::OriginalData()
{
  itsLastTimeUsedTimer.StartTimer();
  return itsData;
}

// Tämä palauttaa vapaana olevan Info-iteraattori kopion dataan.
boost::shared_ptr<NFmiFastQueryInfo> NFmiQueryDataKeeper::GetIter()
{
  WriteLock lock(itsMutex);  // tämä funktio pitää suorittaa aina max yhdestä säikeestä (ainakin kun
                             // tehdään moni-säie laskuja smarttool-kielellä, missä on mukana
                             // asemadataa!!)

  itsLastTimeUsedTimer.StartTimer();
  // Katsotaan onko listassa yhtään Info-iteraattoria, joka ei ole käytössä (ref-count = 1)
  for (size_t i = 0; i < itsIteratorList.size(); i++)
  {
    if (itsIteratorList[i].use_count() <= 1)
      return itsIteratorList[i];
  }

  // Ei löytynyt vapaata (tai ollenkaan) Info-iteraattoria, pitää luoda sellainen ja lisätä listaan
  // ja paluttaa se
  boost::shared_ptr<NFmiFastQueryInfo> infoIter;
  if (OriginalData()->DataType() == NFmiInfoData::kEditable)
    infoIter = boost::shared_ptr<NFmiFastQueryInfo>(new NFmiSmartInfo(
        *(dynamic_cast<NFmiSmartInfo *>(OriginalData().get()))));  // HUOM! Vain editoitu data on
                                                                   // smartInfo2 -tyyppiä, ja clone
                                                                   // ei sovi tässä koska nyt
  // tehdään 'matala' kopio. Että
  // saataisiin kaunis ratkaisu,
  // pitäisi tehdä joku
  // shallowClone virtuaali metodi
  else
    infoIter = boost::shared_ptr<NFmiFastQueryInfo>(new NFmiOwnerInfo(*(OriginalData().get())));

  itsIteratorList.push_back(infoIter);
  return infoIter;
}

int NFmiQueryDataKeeper::LastUsedInMS() const
{
  return itsLastTimeUsedTimer.CurrentTimeDiffInMSeconds();
}

// ************* NFmiQueryDataSetKeeper-class **********************

NFmiQueryDataSetKeeper::NFmiQueryDataSetKeeper(boost::shared_ptr<NFmiOwnerInfo> &theData,
                                               int theMaxLatestDataCount,
                                               int theModelRunTimeGap,
                                               int theFakeProducerId,
                                               bool reloadCaseStudyData,
                                               int theKeepInMemoryTime)
    : itsQueryDatas(),
      itsMaxLatestDataCount(theMaxLatestDataCount),
      itsModelRunTimeGap(theModelRunTimeGap),
      itsFakeProducerId(theFakeProducerId),
      itsFilePattern(),
      itsLatestOriginTime(),
      itsKeepInMemoryTime(theKeepInMemoryTime),
      fReloadCaseStudyData(reloadCaseStudyData)
{
  // Kutsutaan vielä erikseen tämä setter, joka tekee tarpeellisia säätöjä annettuun arvoon
  MaxLatestDataCount(theMaxLatestDataCount);
  bool dataWasDeleted = false;
  AddData(theData, true, dataWasDeleted);  // true tarkoittaa että kyse on 1. lisättävästä datasta
  int x = 0;
}

static void DestroyDataList(NFmiQueryDataSetKeeper::ListType &&queryDatas)
{
  // Varsinkin salama kaudella tämä operaatio voi olla todella hidas salamadatoille,
  // koska niissä voi olla satojatuhansia dynaamisesti luotuja NFmiMetTime olioita.
  queryDatas.clear();
}

// Halusin siirtää tämän datojen tuhoamisen omaan threadiin, koska se voi olla hidasta.
static void DoAsynchronousDataDestroing(NFmiQueryDataSetKeeper::ListType &queryDatas)
{
  // Tehdään väliaikainen lista ja swap, jotta originaali queryDatas lista on paluussa tyhjänä ja valmiina käyttöön.
  NFmiQueryDataSetKeeper::ListType destroyedList;
  destroyedList.swap(queryDatas);
  // Tehdään lopullinen tuhoaminen erillisessä threadissa.
  std::thread t(::DestroyDataList, std::move(destroyedList));
  // Tehdään irroitus, niin ei jäädä odottelemaan loppumista.
  t.detach();
}

// Lisätätään annettu data keeper-settiin.
// Jos	itsMaxLatestDataCount on 0, tyhjennnetään olemassa olevat listat ja datat ja laitetaan
// annettu data käyttöön.
// Jos	itsMaxLatestDataCount on > 0, katsotaan mihin kohtaan (mille indeksille) data sijoittuu,
// mahdollisesti vanhimman datan joutuu siivoamaan pois.
void NFmiQueryDataSetKeeper::AddData(boost::shared_ptr<NFmiOwnerInfo> &theData,
                                     bool fFirstData,
                                     bool &fDataWasDeletedOut)
{
  if (IsDataAcceptedByVirtualTime(theData))
  {
    if (fFirstData || itsMaxLatestDataCount == 0)
    {
      ::DoAsynchronousDataDestroing(itsQueryDatas);
      itsQueryDatas.push_back(
          boost::shared_ptr<NFmiQueryDataKeeper>(new NFmiQueryDataKeeper(theData)));
      itsFilePattern = theData->DataFilePattern();
      itsLatestOriginTime = theData->OriginTime();
    }
    else
      AddDataToSet(theData, fDataWasDeletedOut);
  }
  else
  {
    theData.reset();
    fDataWasDeletedOut = true;
  }
}

bool NFmiQueryDataSetKeeper::IsDataAcceptedByVirtualTime(
    boost::shared_ptr<NFmiOwnerInfo> &theData)
{
  if (!theData)
  {
    return false;
  }

  itsDataType = theData->DataType();

  // Jos virtual-time ei ole käytössä TAI kyse havainto datasta, data kelpaa
  if(!fVirtualTimeUsed || !NFmiDrawParam::IsModelRunDataType(itsDataType))
  {
    return true;
  }

  // Jos vitualli-time jutusta ja mallidatasta, katsotaan onko datan origTime <= VT
  return theData->OriginTime() <= itsVirtualTime;
}

// Oletus: annettu data lisätään tähän data settiin niin että etsitään sen paikka (indeksi).
// Siirretään kaikkia datoja tarpeen mukaan indekseissä.
// Jos datoja on liikaa setissä, poistetaan ylimääräiset (yli max indeksiset).
// Jos sama data löytyy jo setistä (= sama origin-aika), korvaa listassa oleva tällä (esim. on tehty
// uusi korjattu malliajo datahaku).
void NFmiQueryDataSetKeeper::AddDataToSet(boost::shared_ptr<NFmiOwnerInfo> &theData,
                                          bool &fDataWasDeletedOut)
{
  // etsi ja korvaa, jos setistä löytyy jo saman origin-timen data
  NFmiMetTime origTime = theData->OriginTime();
  NFmiMetTime latestOrigTime = origTime;
  bool wasReplace = false;
  for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end(); ++it)
  {
    const NFmiMetTime &currentOrigTime = (*it)->OriginalData()->OriginTime();
    if (latestOrigTime < currentOrigTime)
      latestOrigTime = currentOrigTime;  // etsitään samalla viimeisintä origin aikaa listasta

    if (currentOrigTime == origTime)
    {
      *it = boost::shared_ptr<NFmiQueryDataKeeper>(
          new NFmiQueryDataKeeper(theData));  // korvataan löydetty dataKeeper uudella
      wasReplace = true;
    }
  }

  // Tämä on data uudella origin ajalla.
  // 1. Lisää se listaan.
  itsLatestOriginTime = latestOrigTime;
  if (wasReplace == false)
    itsQueryDatas.push_back(
        boost::shared_ptr<NFmiQueryDataKeeper>(new NFmiQueryDataKeeper(theData)));
  // 2. Laske kaikille setin datoille indeksit uudestaan.
  RecalculateIndexies(itsLatestOriginTime);
  // 3. Ota talteen lisätyn datan origTime
  NFmiMetTime addedDataOrigTime = theData->OriginTime();
  // 4. Poista listasta kaikki datat joiden indeksi on suurempi kuin itsMaxLatestDataCount:in arvo
  // sallii.
  DeleteTooOldDatas();
  // 5. Tutki löytyykö lisätty data vielä listalta
  fDataWasDeletedOut = OrigTimeDataExist(addedDataOrigTime) == false;
}

// Etsi annettua origin-aikaa vastaava dataa listalta, jos ei löydy, palauta false, muuten true.
bool NFmiQueryDataSetKeeper::OrigTimeDataExist(const NFmiMetTime &theOrigTime)
{
  for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end(); ++it)
  {
    if ((*it)->OriginTime() == theOrigTime)
      return true;
  }
  return false;
}

static int CalcIndex(const NFmiMetTime &theLatestOrigTime,
                     const NFmiMetTime &theOrigCurrentTime,
                     int theModelRunTimeGap)
{
  if (theModelRunTimeGap == 0)
    return 0;
  float diffInMinutes = float(theLatestOrigTime.DifferenceInMinutes(theOrigCurrentTime));
  auto floatIndex = -diffInMinutes / theModelRunTimeGap;
  auto roundedFloatIndex = std::roundf(floatIndex);
  if (std::fabs(floatIndex - roundedFloatIndex) < 0.01)
    return int(roundedFloatIndex);
  else
    return -99999; // Tämä on virhe indeksi, voi johtua siitä että datalle on annettu väärä model-runtime-gab konffeissa
}

static bool IsNewer(const boost::shared_ptr<NFmiQueryDataKeeper> &theDataKeeper1,
                    const boost::shared_ptr<NFmiQueryDataKeeper> &theDataKeeper2)
{
  return theDataKeeper1->OriginTime() > theDataKeeper2->OriginTime();
}

void NFmiQueryDataSetKeeper::RecalculateIndexies(const NFmiMetTime &theLatestOrigTime)
{
  if (itsModelRunTimeGap < 0)
  {  // kyse on editoidusta datasta, jolla ei ole vakio malliajo väliä. Laitetaan lista vain
     // aikajärjestykseen ja indeksoidaan numerojärjestyksessä
    itsQueryDatas.sort(::IsNewer);
    int index = 0;
    for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end(); ++it)
    {
      (*it)->Index(index);
      index--;
    }
  }
  else
  {
    for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end(); ++it)
      (*it)->Index(
          ::CalcIndex(theLatestOrigTime, (*it)->OriginalData()->OriginTime(), itsModelRunTimeGap));
  }
}

struct OldDataRemover
{
  OldDataRemover(int theMaxLatestDataCount) : itsMaxLatestDataCount(theMaxLatestDataCount) {}
  bool operator()(boost::shared_ptr<NFmiQueryDataKeeper> &theDataKeeper)
  {
    if (::abs(theDataKeeper->Index()) > itsMaxLatestDataCount)
      return true;
    return false;
  }

  int itsMaxLatestDataCount;
};

void NFmiQueryDataSetKeeper::DeleteTooOldDatas()
{
  itsQueryDatas.remove_if(OldDataRemover(itsMaxLatestDataCount));
}

static boost::shared_ptr<NFmiQueryDataKeeper> FindQDataKeeper(
    NFmiQueryDataSetKeeper::ListType &theQueryDatas, int theIndex)
{
  for (NFmiQueryDataSetKeeper::ListType::iterator it = theQueryDatas.begin();
       it != theQueryDatas.end();
       ++it)
  {
    if ((*it)->Index() == theIndex)
      return (*it);
  }
  return boost::shared_ptr<NFmiQueryDataKeeper>();
}

// TODO: Tulevaisuudessa pitää vielä hanskata tilanne että halutaan uusimman ajon dataa,
// jota ei välttämättä ole kyseiselle datatyypille kyseisestä mallista. Voisi olla siis arvo 0, joka
// tarkoittaa
// että hae viimeisimman malliajon data, siis Hirlam RCR:sta pintadata on jo 06, mutta jos
// mallipinta olisi 00 ajosta,
// pitäisi tällöin palauttaa 0-data. Jos indeksi olisi 1 (tai suurempi), palautettaisiin viimeisin
// data, ed. mainitun
// esimerkin mukaisesti 00 mallipinta data.
boost::shared_ptr<NFmiQueryDataKeeper> NFmiQueryDataSetKeeper::GetDataKeeper(int theIndex)
{
  if (theIndex > 0)
    theIndex = 0;

  boost::shared_ptr<NFmiQueryDataKeeper> qDataKeeperPtr =
      ::FindQDataKeeper(itsQueryDatas, theIndex);
  if (qDataKeeperPtr)
    return qDataKeeperPtr;

  if (theIndex < 0 && DoOnDemandOldDataLoad(theIndex))
  {
    return ::FindQDataKeeper(
        itsQueryDatas, theIndex);  // kokeillaan, löytyykö on-demand pyynnön jälkeen haluttua dataa
  }

  // jos ei löytynyt, palautetaan tyhjä
  return boost::shared_ptr<NFmiQueryDataKeeper>();
}

boost::shared_ptr<NFmiFastQueryInfo> NFmiQueryDataSetKeeper::GetFastInfoIter(int theIndex)
{
  auto dataKeeper = GetDataKeeper(theIndex);
  if (!dataKeeper)
    return nullptr;

  return dataKeeper->GetIter();
}

static NFmiMetTime CalcWantedOrigTime(const NFmiMetTime &theLatestOrigTime,
                                      int theIndex,
                                      int theModelRunTimeGap)
{
  NFmiMetTime wantedOrigTime = theLatestOrigTime;
  long diffInMinutes = theModelRunTimeGap * theIndex;
  wantedOrigTime.ChangeByMinutes(diffInMinutes);
  return wantedOrigTime;
}

static std::string GetFullFileName(const std::string &theFileFilter, const std::string &theFileName)
{
  NFmiFileString fileName(theFileFilter);
  fileName.FileName(theFileName);
  return static_cast<char *>(fileName);
}

bool NFmiQueryDataSetKeeper::DoOnDemandOldDataLoad(int theIndex)
{
  std::string traceLogMessage;
  auto doTraceLogging = ::IsTraceLoggingUsed();
  if (::abs(theIndex) > itsMaxLatestDataCount)  // ei yritetä hakea liian vanhoja datoja
  {
    if (doTraceLogging)
    {
      traceLogMessage +=
          "QueryDataSetKeeper: Too old data requested to be loaded from local querydata file, "
          "fileFilter was ";
      traceLogMessage += itsFilePattern;
      traceLogMessage += ", requested index ";
      traceLogMessage += std::to_string(theIndex);
      traceLogMessage += " is bigger than locally stored file count ";
      traceLogMessage += std::to_string(itsMaxLatestDataCount);
    }
  }
  else
  {
    if (itsModelRunTimeGap > 0)
    {
      if (doTraceLogging)
      {
        traceLogMessage +=
            "QueryDataSetKeeper: Trying to load older model-run data from local querydata file, "
            "fileFilter was ";
        traceLogMessage += itsFilePattern;
        traceLogMessage += ", requested index ";
        traceLogMessage += std::to_string(theIndex);
        traceLogMessage += ", ModelRuntimeGab ";
        traceLogMessage += std::to_string(itsModelRunTimeGap);
      }

      NFmiMetTime wantedOrigTime =
          ::CalcWantedOrigTime(itsLatestOriginTime, theIndex, itsModelRunTimeGap);
      if (doTraceLogging)
      {
        traceLogMessage += ", wanted model-runtime ";
        traceLogMessage += wantedOrigTime.ToStr("YYYY.MM.DD HH:mm", kEnglish);
      }
      std::list<std::string> files = NFmiFileSystem::PatternFiles(itsFilePattern);
      for (std::list<std::string>::iterator it = files.begin(); it != files.end(); ++it)
      {
        try
        {
          std::string usedFileName = ::GetFullFileName(itsFilePattern, *it);
          if (doTraceLogging)
          {
            traceLogMessage += "\nchecking file ";
            traceLogMessage += usedFileName;
          }
          NFmiQueryInfo info;
          std::ifstream in(usedFileName.c_str(), std::ios::binary);
          if (in)
          {
            in >> info;
            if (in.good())
            {
              const auto &originTime = info.OriginTime();
              if (originTime == wantedOrigTime)
              {
                if (doTraceLogging)
                {
                  traceLogMessage += ", origin time in file was the wanted one, using this file";
                  ::TraceLogMessage(traceLogMessage);
                }
                in.close();
                return ReadDataFileInUse(usedFileName);
              }
              else
              {
                if (doTraceLogging)
                {
                  traceLogMessage += ", origin time (";
                  traceLogMessage += originTime.ToStr("YYYY.MM.DD HH:mm", kEnglish);
                  traceLogMessage += ") in file was not the wanted ";
                }
              }
            }
            else
            {
              if (doTraceLogging)
              {
                traceLogMessage += ", unable to read meta info part from file";
              }
            }
          }
          else
          {
            if (doTraceLogging)
            {
              traceLogMessage += ", unable to open the file for unknown reason";
            }
          }
        }
        catch (std::exception &e)
        {
          // pitää vain varmistaa että jos tiedosto on viallinen, poikkeukset napataan kiinni tässä
          if (doTraceLogging)
          {
            traceLogMessage += ", file handling caused exception to be thrown: ";
            traceLogMessage += e.what();
          }
        }
        catch (...)
        {
          // pitää vain varmistaa että jos tiedosto on viallinen, poikkeukset napataan kiinni tässä
          if (doTraceLogging)
          {
            traceLogMessage += ", file handling caused unknown exception to be thrown";
          }
        }
      }

      if (doTraceLogging)
      {
        traceLogMessage += "\n, couldn't find the wanted origin-time from any of the datafiles";
      }
    }
    else if (itsModelRunTimeGap < 0)
    {
      if (doTraceLogging)
      {
        traceLogMessage +=
            ", ModelRunTimeGap was less than 0, means that producer has data generated at random "
            "times (like edited data)";
      }
      // editoidut datat (tai vastaavat, joilla ei ole siis säännöllisiä
      // tekoaikoja) pitää lukea kaikki muistiin, muuten ei voida laskea
      // niiden indeksejä
      ReadAllOldDatasInMemory();
    }
  }

  if (doTraceLogging)
  {
    ::TraceLogMessage(traceLogMessage);
  }
  return false;
}

void NFmiQueryDataSetKeeper::FixLocallyReadDataProducer(NFmiQueryData *locallyReadData)
{
  if (locallyReadData && (itsFakeProducerId != 0))
  {
    auto info = locallyReadData->Info();
    info->FirstParam();
    auto finalProducerPtr = info->Producer();
    if (finalProducerPtr)
    {
      auto finalProducer = *finalProducerPtr;
      finalProducer.SetIdent(itsFakeProducerId);
      info->SetProducer(finalProducer);
    }
  }
}

// Huom! Jos vanhempia lokaaliversioita luetaan erikseen käyttöön (käytetään edellisiä malliajoja),
// pitää tällä lailla luetuille datoille laittaa oikea tuottaja id, koska SmartMetin konffeissä
// voidaan vaihtaa kullekin datalle haluttu producer-id (esim. virallinen data, jonka id pitää
// muuttaa että se ei menisi sekaisin editoidun datan kanssa).
bool NFmiQueryDataSetKeeper::ReadDataFileInUse(const std::string &theFileName)
{
  try
  {
    std::unique_ptr<NFmiQueryData> dataPtr(new NFmiQueryData(theFileName));
    FixLocallyReadDataProducer(dataPtr.get());
    boost::shared_ptr<NFmiOwnerInfo> ownerInfoPtr(
        new NFmiOwnerInfo(dataPtr.release(), itsDataType, theFileName, itsFilePattern, true));
    if (IsDataAcceptedByVirtualTime(ownerInfoPtr))
    {
      bool dataWasDeleted = false;
      AddDataToSet(ownerInfoPtr, dataWasDeleted);
      return (dataWasDeleted == false);
    }
  }
  catch (...)
  {  // pitää vain varmistaa että jos tiedosto on viallinen, poikkeukset napataan kiinni tässä
  }
  return false;
}

std::set<std::string> NFmiQueryDataSetKeeper::GetAllFileNames()
{
  std::set<std::string> fileNames;
  for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end(); ++it)
    fileNames.insert((*it)->DataFileName());
  return fileNames;
}

void NFmiQueryDataSetKeeper::ReadAllOldDatasInMemory()
{
  std::set<std::string> filesInMemory = GetAllFileNames();
  std::list<std::string> filesOnDrive = NFmiFileSystem::PatternFiles(itsFilePattern);
  for (std::list<std::string>::iterator it = filesOnDrive.begin(); it != filesOnDrive.end(); ++it)
  {
    std::set<std::string>::iterator it2 = filesInMemory.find(*it);
    if (it2 == filesInMemory.end())
    {  // kyseistä tiedostoa ei löytynyt muistista, koitetaan lukea se nyt
      std::string usedFileName = ::GetFullFileName(itsFilePattern, *it);
      ReadDataFileInUse(usedFileName);
    }
  }
}

size_t NFmiQueryDataSetKeeper::DataCount()
{
  return itsQueryDatas.size();
}

size_t NFmiQueryDataSetKeeper::DataByteCount()
{
  size_t sizeInBytes = 0;
  for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end(); ++it)
    sizeInBytes += (*it)->OriginalData()->Size() * sizeof(float);

  return sizeInBytes;
}

// palauttaa true, jos data on jo 'vanhaa' eli sen voi poistaa muistista.
bool NFmiQueryDataSetKeeper::CheckKeepTime(ListType::iterator &it)
{
  if ((*it)->Index() != 0)
  {  // vain viimeisin data jää tutkimatta, koska sitä ei ole tarkoitus poistaa muistista koskaan
    if ((*it)->LastUsedInMS() > itsKeepInMemoryTime * 60 * 1000)
      return true;
  }
  return false;
}

// 1. Jos jotain arkisto-dataa ei ole käytetty tarpeeksi pitkään aikaan, poistetaan ne muistista.
// 2. Viimeisintä dataa ei poisteta koskaan muistista.
// 3. Jos kyse editoidusta datasta (esim. kepa-data, ei säännöllistä ilmestymis tiheyttä, vaan niitä
// syntyy satunnaisina aikoina),
// pitää tutkia kaikki arkisto ajat ennen niiden siivoamista. Jos yhtäkään arkisto datoista on
// käytetty aikarajan sisällä,
// ei mitään heitetä pois muistista.
// 4. Palauttaa kuinka monta dataa poistettiin muistista operaation aikana
int NFmiQueryDataSetKeeper::CleanUnusedDataFromMemory()
{
  int dataRemovedCounter = 0;
  if (itsModelRunTimeGap > 0)
  {  // normaalit mallidatat
    for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end();)
    {
      if (CheckKeepTime(it))
      {
        it = itsQueryDatas.erase(it);
        dataRemovedCounter++;
      }
      else
        ++it;
    }
  }
  else if (itsModelRunTimeGap < 0)
  {  // editoitu data
    bool oneUsedTimeNewer = false;
    for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end(); ++it)
    {
      if ((*it)->Index() != 0 && CheckKeepTime(it) == false)
      {
        oneUsedTimeNewer = true;
        break;
      }
    }
    if (oneUsedTimeNewer == false)
    {
      for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end();)
      {
        if ((*it)->Index() != 0)
        {
          it = itsQueryDatas.erase(it);
          dataRemovedCounter++;
        }
        else
          ++it;
      }
    }
  }

  return dataRemovedCounter;
}

int NFmiQueryDataSetKeeper::GetNearestUnRegularTimeIndex(const NFmiMetTime &theTime)
{
  if (ModelRunTimeGap() == -1)
  {
    ReadAllOldDatasInMemory();
    std::map<long, int> timeDiffsWithIndexies;
    for (ListType::iterator it = itsQueryDatas.begin(); it != itsQueryDatas.end(); ++it)
    {
      // HUOM! etsitään pienintä negatiivista lukua, sitten nollaa, jos kumpaakaan ei löydy,
      // palautetaan pienimmän positiivisen luvun indeksi
      long diffInMinutes = (*it)->OriginTime().DifferenceInMinutes(theTime);
      int index = (*it)->Index();
      timeDiffsWithIndexies.insert(std::make_pair(diffInMinutes, index));
    }

    if (timeDiffsWithIndexies.size())
    {
      if (timeDiffsWithIndexies.size() <= 1)
        return timeDiffsWithIndexies.begin()->second;

      std::map<long, int>::iterator it2 = timeDiffsWithIndexies.begin();
      long diffValue1 = it2->first;
      int indexValue1 = it2->second;
      long diffValue2 = diffValue1;
      for (; it2 != timeDiffsWithIndexies.end();)
      {
        if (indexValue1 >= 0)  // jos ollaan viimeisessä malliajo ajassa, se on otettava käyttöön
          return indexValue1;
        ++it2;
        diffValue2 = it2->first;
        if (diffValue1 < 0 && diffValue2 >= 0)
          return indexValue1;
        else if (diffValue1 == 0 && diffValue2 > 0)
          return indexValue1;
        else if (diffValue1 > 0)
          return indexValue1;

        diffValue1 = it2->first;
        indexValue1 = it2->second;
      }
    }
  }
  return 0;
}

void NFmiQueryDataSetKeeper::MaxLatestDataCount(int newValue)
{
  itsMaxLatestDataCount = newValue;
  // "Pitää olla minimissään" 0 -tarkastus
  if (itsMaxLatestDataCount < 0)
    itsMaxLatestDataCount = 0;
}

static bool VirtualTimeChangesRequireReloading(const NFmiMetTime &origVirtualTime,
                                               bool origVirtualTimeUsed,
                                               const NFmiMetTime &newVirtualTime,
                                               bool newVirtualTimeUsed)
{
  // Jos virtuaaliaika tila muuttuu, pitää aina tehdä reload
  if (origVirtualTimeUsed != newVirtualTimeUsed)
    return true;

  // Jos virtual-time moodi on päällä ja Virtual-Time on muuttunut, pitää tehdä reload
  if (newVirtualTimeUsed)
  {
    return origVirtualTime != newVirtualTime;
  }

  return false;
}

void NFmiQueryDataSetKeeper::SetupVirtualTime(const NFmiMetTime& virtualTime, bool virtualTimeUsed)
{
  auto reloadRequired = ::VirtualTimeChangesRequireReloading(
      itsVirtualTime, fVirtualTimeUsed, virtualTime, virtualTimeUsed);
  itsVirtualTime = virtualTime;
  fVirtualTimeUsed = virtualTimeUsed;
  if(reloadRequired && NFmiDrawParam::IsModelRunDataType(itsDataType))
  {
    ReloadModelDataAfterVirtualTimeChange();
  }
}

void NFmiQueryDataSetKeeper::ReloadModelDataAfterVirtualTimeChange()
{
  itsQueryDatas.clear();
  std::list<std::string> filesOnLocalCacheDirectory = NFmiFileSystem::PatternFiles(itsFilePattern);
  for (const auto &fileName : filesOnLocalCacheDirectory)
  {
    std::string usedFileName = ::GetFullFileName(itsFilePattern, fileName);
    ReadDataFileInUse(usedFileName);
  }
}
