#pragma once

// Luokan tehtävä on toimia ns. smart-pointer luokkana, joka on queryData
// iteraattori ja omistaa smart-pointterin avulla sen queryDatan, mitä se iteroi.
// Eli SmartMetissa voi olla useita info-iteraattoreita, jotka osoittavat samaan
// queryDataan. Niitä voidaan käyttää eri säikeissä ja nämä talletetaan
// ns. InfoOrganizer-luokkaan talteen, mistä eri datojen infoja pyydetään.
// TODO: Keksi luokalle parempi nimi.

#include <newbase/NFmiFastQueryInfo.h>
#include <boost/shared_ptr.hpp>

class NFmiOwnerInfo : public NFmiFastQueryInfo
{
 public:
  NFmiOwnerInfo(void);
  NFmiOwnerInfo(const NFmiOwnerInfo &theInfo);  // matala kopio, eli jaettu data
  NFmiOwnerInfo(NFmiQueryData *theOwnedData,
                NFmiInfoData::Type theDataType,
                const std::string &theDataFileName,
                const std::string &theDataFilePattern);  // ottaa datan omistukseensa
  ~NFmiOwnerInfo(void);

  NFmiOwnerInfo &operator=(const NFmiOwnerInfo &theInfo);  // matala kopio, eli jaettu data
  NFmiOwnerInfo *Clone(
      void) const;  // syvä kopio, eli kloonille luodaan oma queryData sen omistukseen
                    // TODO: katso pitääkö metodin nimi muuttaa, koska emoissa Clone on
                    // virtuaali funktio, jossa eri paluu-luokka.
  const std::string &DataFileName(void) const { return itsDataFileName; }
  void DataFileName(const std::string &theDataFileName) { itsDataFileName = theDataFileName; }
  const std::string &DataFilePattern(void) const { return itsDataFilePattern; }
  void DataFilePattern(const std::string &theDataFilePattern)
  {
    itsDataFilePattern = theDataFilePattern;
  }

  boost::shared_ptr<NFmiQueryData> DataReference(void) { return itsDataPtr; }
 protected:
  boost::shared_ptr<NFmiQueryData> itsDataPtr;
  std::string itsDataFileName;
  std::string itsDataFilePattern;  // tätä käytetään tunnistamaan mm. info-organizerissa, että onko
  // data samanlaista, eli pyyhitäänkö vanha tälläinen data pois
  // alta
};
