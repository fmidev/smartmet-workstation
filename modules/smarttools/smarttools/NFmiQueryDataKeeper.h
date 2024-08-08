#pragma once

#include <boost/shared_ptr.hpp>
#include <newbase/NFmiInfoData.h>
#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiMilliSecondTimer.h>
#include <list>
#include <mutex>
#include <set>

class NFmiOwnerInfo;
class NFmiQueryData;
class NFmiFastQueryInfo;
class NFmiProducer;

const int kQueryDataKeepInMemoryTimeInMinutes = 5;

// NFmiQueryDataKeeper on luokka joka pitää kirjanpitoa NFmiInfoOrganizer-luokassa
// säilytettävistä queryDatoista.
// HUOM! Tätä luokkaa ei ole tarkoitettu käytettäväksi kuin NFmiInfoOrganizerin sisäisesti!!!
class NFmiQueryDataKeeper
{
 public:
  typedef std::mutex MutexType;
  typedef std::lock_guard<MutexType> WriteLock;

  NFmiQueryDataKeeper();
  NFmiQueryDataKeeper(boost::shared_ptr<NFmiOwnerInfo> &theOriginalData);
  ~NFmiQueryDataKeeper();

  // Tätä saa käyttää vain NFmiInfoOrganizer-luokka sisäisesti,
  // koska tätä ei ole tarkoitus palauttaa, kun tarvitaan 
  // moni-säie turvallinen info-iteraattori kopio, käytetään 
  // mieluummin GetIter-metodia.
  boost::shared_ptr<NFmiOwnerInfo> OriginalData();
  // Tämä palauttaa vapaana olevan Info-iteraattori kopion dataan.
  boost::shared_ptr<NFmiFastQueryInfo> GetIter();
  int Index() const { return itsIndex; }
  void Index(int newValue) { itsIndex = newValue; }
  const NFmiMetTime &OriginTime() const { return itsOriginTime; }
  const std::string &DataFileName() { return itsDataFileName; }
  int LastUsedInMS() const;

 private:
  // tämä on originaali data
  boost::shared_ptr<NFmiOwnerInfo> itsData;
  // aina kun kyseistä dataa käytetään, käytetään StartTimer-metodia, 
  // jotta myöhemmin voidaan laskea, voidaanko kyseinen data siivota 
  // pois muistista (jos dataa ei ole käytetty tarpeeksi pitkään aikaan)
  NFmiMilliSecondTimer itsLastTimeUsedTimer;  
  // malliajo datoissa 0 arvo tarkoittaa viimeisintä ja -1 sitä edellistä jne.
  int itsIndex;  
  // originaali datasta tehnään tarvittaessa n kpl iteraattori
  // kopioita, ulkopuoliset rutiinit/säikeet käyttävät aina vain 
  // iteraattori-kopioita alkuperäisestä, jolloin niitä voidaan 
  // käyttää eri säikeissä yht'aikaa. Nämä luodaan on demandina, 
  // eli jos InfoOrganizerilta pyydetään dataa, ja listassa ei ole
  // vapaata iteraattoria luodaan tällöin uusi kopio joka palautetaan.
  // TODO: Miten tiedän että joku rutiini/säie on lopettanut 
  // iteraattorin käytön? Ehkä shared_ptr:n use_count:in avulla?
  std::vector<boost::shared_ptr<NFmiFastQueryInfo>> itsIteratorList;
  // Tähän talletetaan datan origin-time vertailuja helpottamaan
  NFmiMetTime itsOriginTime;
  // Tähän talletetaan datan tiedosto nimi
  std::string itsDataFileName;
  MutexType itsMutex;
};

using TraceLogMessageCallback = std::function<void(const std::string &)>;
using IsTraceLoggingInUseCallback = std::function<bool()>;

// NFmiQueryDataSetKeeper-luokka pitää kirjaa n kpl viimeisitä malliajoista/datasta
class NFmiQueryDataSetKeeper
{
 public:
  using ListType = std::list<boost::shared_ptr<NFmiQueryDataKeeper>>;

  NFmiQueryDataSetKeeper() = default;
  NFmiQueryDataSetKeeper(boost::shared_ptr<NFmiOwnerInfo> &theData,
                         int theMaxLatestDataCount,
                         int theModelRunTimeGap,
                         int theFakeProducerId,
                         bool reloadCaseStudyData,
                         int theKeepInMemoryTime);
  ~NFmiQueryDataSetKeeper() = default;

  void AddData(boost::shared_ptr<NFmiOwnerInfo> &theData,
               bool fFirstData,
               bool &fDataWasDeletedOut);
  boost::shared_ptr<NFmiQueryDataKeeper> GetDataKeeper(int theIndex = 0);
  boost::shared_ptr<NFmiFastQueryInfo> GetFastInfoIter(int theIndex = 0);
  const std::string &FilePattern() const { return itsFilePattern; }
  void FilePattern(const std::string &newValue) { itsFilePattern = newValue; }
  int MaxLatestDataCount() const { return itsMaxLatestDataCount; }
  void MaxLatestDataCount(int newValue);
  int ModelRunTimeGap() const { return itsModelRunTimeGap; }
  void ModelRunTimeGap(int newValue) { itsModelRunTimeGap = newValue; }
  std::set<std::string> GetAllFileNames();
  int CleanUnusedDataFromMemory();
  int KeepInMemoryTime() const { return itsKeepInMemoryTime; }
  void KeepInMemoryTime(int newValue) { itsKeepInMemoryTime = newValue; }
  void ReadAllOldDatasInMemory();
  int GetNearestUnRegularTimeIndex(const NFmiMetTime &theTime);
  bool ReloadCaseStudyData() const { return fReloadCaseStudyData; }
  void ReloadCaseStudyData(bool newValue) { fReloadCaseStudyData = newValue; }

  size_t DataCount();
  size_t DataByteCount();

  static void SetTraceLogMessageCallback(TraceLogMessageCallback &traceLogMessageCallback);
  static void SetIsTraceLoggingInUseCallback(
      IsTraceLoggingInUseCallback &isTraceLoggingInUseCallback);

  void SetupVirtualTime(const NFmiMetTime &virtualTime, bool virtualTimeUsed);
 private:
  void AddDataToSet(boost::shared_ptr<NFmiOwnerInfo> &theData, bool &fDataWasDeletedOut);
  void RecalculateIndexies(const NFmiMetTime &theLatestOrigTime);
  void DeleteTooOldDatas();
  bool DoOnDemandOldDataLoad(int theIndex);
  bool ReadDataFileInUse(const std::string &theFileName);
  bool CheckKeepTime(ListType::iterator &it);
  bool OrigTimeDataExist(const NFmiMetTime &theOrigTime);
  void FixLocallyReadDataProducer(NFmiQueryData *locallyReadData);
  void ReloadModelDataAfterVirtualTimeChange();
  bool IsDataAcceptedByVirtualTime(boost::shared_ptr<NFmiOwnerInfo> &theData);

  // Tässä on n kpl viimeisintä malliajoa tallessa (tai esim. havaintojen 
  // tapauksessa vain viimeisin data)
  ListType itsQueryDatas;
  // Kuinka monta viimeisintä malliajoa/dataa maksimissään kullekin
  // datalle on, 0 jos kyse esim. havainnoista, joille ei ole kuin viimeisin data.
  int itsMaxLatestDataCount = 0;
  // Millä ajoväleillä kyseisen datan mallia ajetaan (yksikkö minuutteja),
  // jos kyse havainnosta, eli ei ole kuin viimeinen data, arvo 0 ja jos
  // kyse esim. editoidusta datasta (epämääräinen ilmestymisväli) on arvo -1.
  int itsModelRunTimeGap = 0;
  // Esim. kepa datan tuottaja id halutaan muuttaa siitä mikä se on tiedostossa.
  // Jos tämän arvo ei ole 0, muutetaan lokaali cachesta luetun tiedoston prod-id tähän. 
  int itsFakeProducerId = 0;
  // Erilaiset datat erotellaan fileFilterin avulla (esim.
  // "D:\smartmet\wrk\data\local\*_hirlam_skandinavia_mallipinta.sqd").
  std::string itsFilePattern;
  // Tähän talletetaan aina viimeisimmän datan origin-time vertailuja helpottamaan
  NFmiMetTime itsLatestOriginTime;
  // Tähän laitetaan 1. datan datatyyppi (pitäisi olla yhtenäinen kaikille 
  // setissä oleville datoille)
  NFmiInfoData::Type itsDataType = NFmiInfoData::kNoDataType;
  // Kuinka kauan pidetään data muistissa, jos sitä ei ole käytetty, yksikkö on minuutteja.
  int itsKeepInMemoryTime = kQueryDataKeepInMemoryTimeInMinutes;
  // Jotkin datat halutaan pitää tallessa tietyissä tilanteissa, vaikka tehtäisiin 
  // datojen reload-operaatio (Case-study tapahtumien yhteydessä).
  bool fReloadCaseStudyData = true;
  // Virtual-Time ominaisuuteen liittyvät tiedot.
  // Jos käytössä Virtual-Time systeemi, käytetään vain sellaisia mallidatoja, joiden 
  // origin-timet ovat <= kyseistä VT aikaa.
  NFmiMetTime itsVirtualTime;
  bool fVirtualTimeUsed = false;
};
