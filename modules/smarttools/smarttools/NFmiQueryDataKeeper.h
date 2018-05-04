#pragma once

#include <newbase/NFmiMilliSecondTimer.h>
#include <newbase/NFmiDataMatrix.h>
#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiInfoData.h>
#include <boost/shared_ptr.hpp>
#include <list>
#include <set>

#ifdef _MSC_VER
#pragma warning( \
    disable : 4244 4267 4512)  // boost:in thread kirjastosta tulee ikävästi 4244 varoituksia
#endif
#include <boost/thread.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4244 4267 4512)  // laitetaan 4244 takaisin päälle, koska se on tärkeä
                                           // (esim. double -> int auto castaus varoitus)
#endif

class NFmiOwnerInfo;
class NFmiFastQueryInfo;

// NFmiQueryDataKeeper on luokka joka pitää kirjanpitoa NFmiInfoOrganizer-luokassa
// säilytettävistä queryDatoista.
// HUOM! Tätä luokkaa ei ole tarkoitettu käytettäväksi kuin NFmiInfoOrganizerin sisäisesti!!!
class NFmiQueryDataKeeper
{
 public:
  typedef boost::shared_mutex MutexType;
  typedef boost::shared_lock<MutexType>
      ReadLock;  // Read-lockia ei oikeasti tarvita, mutta laitan sen tähän, jos joskus tarvitaankin
  typedef boost::unique_lock<MutexType> WriteLock;

  NFmiQueryDataKeeper(void);
  NFmiQueryDataKeeper(boost::shared_ptr<NFmiOwnerInfo> &theOriginalData);
  ~NFmiQueryDataKeeper(void);

  boost::shared_ptr<NFmiOwnerInfo> OriginalData(void);  // Tätä saa käyttää vain
                                                        // NFmiInfoOrganizer-luokka sisäisesti,
                                                        // koska tätä ei ole tarkoitus palauttaa,
  // kun tarvitaan moni-säie turvallinen info-iteraattori kopio, käytetään mieluummin
  // GetIter-metodia.
  boost::shared_ptr<NFmiFastQueryInfo> GetIter(
      void);  // Tämä palauttaa vapaana olevan Info-iteraattori kopion dataan.
  int Index(void) const { return itsIndex; }
  void Index(int newValue) { itsIndex = newValue; }
  const NFmiMetTime &OriginTime(void) const { return itsOriginTime; }
  const std::string &DataFileName(void) { return itsDataFileName; }
  int LastUsedInMS(void) const;

 private:
  boost::shared_ptr<NFmiOwnerInfo> itsData;   // tämä on originaali data
  NFmiMilliSecondTimer itsLastTimeUsedTimer;  // aina kun kyseistä dataa käytetään, käytetään
                                              // StartTimer-metodia, jotta myöhemmin voidaan
  // laskea, voidaanko kyseinen data siivota pois muistista (jos dataa ei ole käytetty tarpeeksi
  // pitkään aikaan)
  int itsIndex;  // malliajo datoissa 0 arvo tarkoittaa viimeisintä ja -1 sitä edellistä jne.
  checkedVector<boost::shared_ptr<NFmiFastQueryInfo> > itsIteratorList;  // originaali datasta
                                                                         // tehnään tarvittaessa n
                                                                         // kpl iteraattori
                                                                         // kopioita, ulkopuoliset
                                                                         // rutiinit/säikeet
  // käyttävät aina vain iteraattori-kopioita alkuperäisestä, jolloin niitä voidaan käyttää eri
  // säikeissä yht'aikaa.
  // nämä luodaan on demandina, eli jos InfoOrganizerilta pyydetään dataa, ja listassa ei ole
  // vapaata iteraattoria
  // luodaan tällöin uusi kopio joka palautetaan.
  // TODO: Miten tiedän että joku rutiini/säie on lopettanut iteraattorin käytön? Ehkä shared_ptr:n
  // use_count:in avulla?
  NFmiMetTime itsOriginTime;    // tähän talletetaan datan origin-time vertailuja helpottamaan
  std::string itsDataFileName;  // tähän talletetaan datan tiedosto nimi
  MutexType itsMutex;
};

// NFmiQueryDataSetKeeper-luokka pitää kirjaa n kpl viimeisitä malliajoista/datasta
class NFmiQueryDataSetKeeper
{
 public:
  typedef std::list<boost::shared_ptr<NFmiQueryDataKeeper> > ListType;

  NFmiQueryDataSetKeeper(void);
  NFmiQueryDataSetKeeper(boost::shared_ptr<NFmiOwnerInfo> &theData,
                         int theMaxLatestDataCount = 0,
                         int theModelRunTimeGap = 0,
                         int theKeepInMemoryTime = 5);
  ~NFmiQueryDataSetKeeper(void);

  void AddData(boost::shared_ptr<NFmiOwnerInfo> &theData,
               bool fFirstData,
               bool &fDataWasDeletedOut);
  boost::shared_ptr<NFmiQueryDataKeeper> GetDataKeeper(int theIndex = 0);
  const std::string &FilePattern(void) const { return itsFilePattern; }
  void FilePattern(const std::string &newValue) { itsFilePattern = newValue; }
  int MaxLatestDataCount(void) const { return itsMaxLatestDataCount; }
  void MaxLatestDataCount(int newValue);
  int ModelRunTimeGap(void) const { return itsModelRunTimeGap; }
  void ModelRunTimeGap(int newValue) { itsModelRunTimeGap = newValue; }
  std::set<std::string> GetAllFileNames(void);
  int CleanUnusedDataFromMemory(void);
  int KeepInMemoryTime(void) const { return itsKeepInMemoryTime; }
  void KeepInMemoryTime(int newValue) { itsKeepInMemoryTime = newValue; }
  void ReadAllOldDatasInMemory(void);
  int GetNearestUnRegularTimeIndex(const NFmiMetTime &theTime);

  size_t DataCount(void);
  size_t DataByteCount(void);

 private:
  void AddDataToSet(boost::shared_ptr<NFmiOwnerInfo> &theData, bool &fDataWasDeletedOut);
  void RecalculateIndexies(const NFmiMetTime &theLatestOrigTime);
  void DeleteTooOldDatas(void);
  bool DoOnDemandOldDataLoad(int theIndex);
  bool ReadDataFileInUse(const std::string &theFileName);
  bool CheckKeepTime(ListType::iterator &it);
  bool OrigTimeDataExist(const NFmiMetTime &theOrigTime);

  ListType itsQueryDatas;  // tässä on n kpl viimeisintä malliajoa tallessa (tai esim. havaintojen
                           // tapauksessa vain viimeisin data)
  int itsMaxLatestDataCount;  // kuinka monta viimeisintä malliajoa/dataa maksimissään kullekin
                              // datalle on, 0 jos kyse esim. havainnoista, joille ei ole kuin
                              // viimeisin data.
  int itsModelRunTimeGap;  // millä ajoväleillä kyseisen datan mallia ajetaan (yksikkö minuutteja),
                           // jos kyse havainnosta, eli ei ole kuin viimeinen data, arvo 0 ja jos
  // kyse esim. editoidusta datasta (epämääräinen ilmestymisväli) on arvo
  // -1.
  std::string itsFilePattern;  // erilaiset datat erotellaan fileFilterin avulla (esim.
                               // "D:\smartmet\wrk\data\local\*_hirlam_skandinavia_mallipinta.sqd")
  NFmiMetTime itsLatestOriginTime;  // tähän talletetaan aina viimeisimmän datan origin-time
                                    // vertailuja helpottamaan
  NFmiInfoData::Type itsDataType;  // tähän laitetaan 1. datan datattyyppi (pitäisi olla yhtenäinen
                                   // kaikille setissä oleville datoille)
  int itsKeepInMemoryTime;  // kuinka kauan pidetään data muistissa, jos sitä ei ole käytetty.
                            // yksikkö on minuutteja
};

