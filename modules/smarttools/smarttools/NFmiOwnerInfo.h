#pragma once

// Luokan tehtävä on toimia ns. smart-pointer luokkana, joka on queryData
// iteraattori ja omistaa smart-pointterin avulla sen queryDatan, mitä se iteroi.
// Eli SmartMetissa voi olla useita info-iteraattoreita, jotka osoittavat samaan
// queryDataan. Niitä voidaan käyttää eri säikeissä ja nämä talletetaan
// ns. InfoOrganizer-luokkaan talteen, mistä eri datojen infoja pyydetään.
// TODO: Keksi luokalle parempi nimi.

#include <boost/shared_ptr.hpp>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiMilliSecondTimer.h>

class NFmiOwnerInfo : public NFmiFastQueryInfo
{
 public:
  NFmiOwnerInfo();
  NFmiOwnerInfo(const NFmiOwnerInfo &theInfo);  // matala kopio, eli jaettu data
  NFmiOwnerInfo(NFmiQueryData *theOwnedData,
                NFmiInfoData::Type theDataType,
                const std::string &theDataFileName,
                const std::string &theDataFilePattern,
                bool IsConsideredOldData);  // ottaa datan omistukseensa
  ~NFmiOwnerInfo();

  NFmiOwnerInfo &operator=(const NFmiOwnerInfo &theInfo);  // matala kopio, eli jaettu data
  NFmiOwnerInfo *Clone(
      void) const;  // syvä kopio, eli kloonille luodaan oma queryData sen omistukseen
                    // TODO: katso pitääkö metodin nimi muuttaa, koska emoissa Clone on
                    // virtuaali funktio, jossa eri paluu-luokka.
  const std::string &DataFileName() const { return itsDataFileName; }
  void DataFileName(const std::string &theDataFileName) { itsDataFileName = theDataFileName; }
  const std::string &DataFilePattern() const { return itsDataFilePattern; }
  void DataFilePattern(const std::string &theDataFilePattern)
  {
    itsDataFilePattern = theDataFilePattern;
  }

  boost::shared_ptr<NFmiQueryData> DataReference() { return itsDataPtr; }
  double ElapsedTimeFromLoadInSeconds() const override;

 protected:
  void SetupDataLoadedTimer(bool IsConsideredOldData);

  boost::shared_ptr<NFmiQueryData> itsDataPtr;
  std::string itsDataFileName;
  // Tätä käytetään tunnistamaan mm. info-organizerissa, että onko
  // data samanlaista, eli pyyhitäänkö vanha tälläinen data pois alta
  std::string itsDataFilePattern;
  // SmartMetin käyttäjille kerrotaan tietyillä korostuksilla, että onko
  // data luettu vasta käyttöön, vai onko se ollut käytössä jo pidempään.
  // Tämä timer osaa kertoa kuinka kauan data on ollut käytössä.
  NFmiNanoSecondTimer itsDataLoadedTimer;
};
