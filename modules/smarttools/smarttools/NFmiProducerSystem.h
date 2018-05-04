//© Ilmatieteenlaitos/Marko.
// Original 20.6.2006
//
// Luokka pitää huolta Editoriin konffatuista malli tuottajista.
// Voidaan konffata esim. 10 tuottajaa. Mutta editorin eri valikot ja systeemit
// itse päättävät kuinka montaa ensimmäistä voidaan käyttää. Eli Esim. Luotausnäytön
// tuottaja valinta radio buttoneihin saa mukaan vain kolme mallia (pitää kai tehdä
// joskus dropdown lista). Mutta esim. erilaisiin dynaamisiin parametrin valinta
// popup-valikoihin voidaan laittaa useampia tuottajia.
//---------------------------------------------------------- NFmiProducerSystem.h

#pragma once

#include <newbase/NFmiProducer.h>
#include <newbase/NFmiProducerName.h>
#include <newbase/NFmiInfoData.h>
#include <string>
#include <vector>

class NFmiLevel;
class NFmiMetTime;

class NFmiProducerInfo
{
 public:
  NFmiProducerInfo(void)
      : itsName(),
        itsShortNameVector(),
        itsUltraShortName(),
        itsProducerId(kFmiNoProducer),
        itsDescription(),
        fHasRealVerticalData(false),
        fHasQ2ArchiveData(false)
  {
  }

  const std::string &Name(void) const { return itsName; }
  void Name(const std::string &newValue) { itsName = newValue; }
  const std::string &ShortName(size_t index = 0) const;
  void ShortName(const std::string &newValue, size_t index = 0);
  size_t ShortNameCount(void) const { return itsShortNameVector.size(); }
  void SetShortNames(const std::string &newShortNames);

  const std::string &UltraShortName(void) const { return itsUltraShortName; }
  void UltraShortName(const std::string &newValue) { itsUltraShortName = newValue; }
  unsigned long ProducerId(void) const { return itsProducerId; }
  void ProducerId(unsigned long newValue) { itsProducerId = newValue; }
  const std::string &Description(void) const { return itsDescription; }
  void Description(const std::string &newValue) { itsDescription = newValue; }
  NFmiProducer GetProducer(void);
  bool HasRealVerticalData(void) const { return fHasRealVerticalData; }
  void HasRealVerticalData(bool newValue) { fHasRealVerticalData = newValue; }
  bool HasQ2ArchiveData(void) const { return fHasQ2ArchiveData; }
  void HasQ2ArchiveData(bool newValue) { fHasQ2ArchiveData = newValue; }
 private:
  std::string itsName;  // Pitempi nimi esim. Hirlam tai Ecmwf (voidaan käyttää esim.
                        // popup-valikoissa, missä on tilaa)
  std::vector<std::string> itsShortNameVector;  // Lista lyhyemmistä nimistä esim. Hir tai Ec
                                                // (käytetään nyt ainoastaan smarttool-kielen
                                                // tuottaja nimenä skripteissä)
  std::string itsUltraShortName;  // Lyhyempi nimi esim. Hir tai Ec (käytetään mm. jossain
                                  // pikavalinnoissa nimenä)
  unsigned long itsProducerId;
  std::string itsDescription;
  bool fHasRealVerticalData;  // esim. UK,DWDjaUSa datoissa ei ole nyt oikeasti vertikaalidataa,
                              // vaikka siinä on painepintoja, näistä malleista ei haluta
  // tehdä luotauksia, poikkileikkausia tai trajektori laskuja, joten nämä mallit saavat arvon
  // false.
  bool fHasQ2ArchiveData;  // tämä määrää, voidaanko katsoa arkisto dataa Q2-serveriltä
};

class NFmiProducerSystem
{
 public:
  static const unsigned int gHelpEditorDataProdId =
      996;  // pistin tämän tuottaja id:n tanne, koska en viitsi laittaa tätä
            // metkun editorin sisäistä help-data tuottaja id juttua
            // newbasen ProducerName listaan. Tätä käytetään ainakin
            // smarttool-kielen alustamisessa ja dokumentin HelpDataSystemin alustuksessa

  NFmiProducerSystem(void) : itsProducers() {}
  void Add(const NFmiProducerInfo &newValue) { itsProducers.push_back(newValue); }
  void Clear(void) { itsProducers.clear(); }
  std::vector<NFmiProducerInfo> &Producers(void) { return itsProducers; }
  void InitFromSettings(const std::string &theInitNameSpace);
  const std::string &GetInitializeLogStr(void) const { return itsInitializeLogStr; }
  bool ExistProducer(unsigned int index1Based) const;
  NFmiProducerInfo &Producer(unsigned int index1Based);
  unsigned int FindProducerInfo(const NFmiProducer &theProducer);
  NFmiString GetProducerAndLevelTypeString(const NFmiProducer &theProducer,
                                           const NFmiLevel &theLevel,
                                           const NFmiMetTime &theOriginTime,
                                           bool fEncloseInBracers);

 private:
  NFmiProducerInfo GetProducerInfoFromSettings(const std::string &theUsedNameSpaceBase);

  std::vector<NFmiProducerInfo>
      itsProducers;  // Tähän voi siis laittaa tuottajia kuinka paljon tahansa,
                     // mutta kannattaa sijoittaa alkuun tärkeimmät, koska
                     // Editori päättää eri ikkunoissa/työkaluissa, kuinka monta
                     // tuottajaa (alkupäästä) se ottaa käyttöön. Esim. Luotaus
  // näytön malli valinta radio-buttoneita on esim. vain kolme (tällä hetkellä)
  std::string itsInitializeLogStr;  // jos luokka initialisoidaan settingeistä, tähän talletetaan
                                    // loki stringi
};

struct NFmiProducerHelperInfo
{
  NFmiProducerHelperInfo(void)
      : itsName(),
        itsProducerId(kFmiNoProducer),
        itsDataType(NFmiInfoData::kNoDataType),
        fGroundData(false)
  {
  }

  NFmiProducerHelperInfo(const std::string theName,
                         int theProducerId,
                         NFmiInfoData::Type theDataType,
                         bool groundData = true)
      : itsName(theName),
        itsProducerId(theProducerId),
        itsDataType(theDataType),
        fGroundData(groundData)
  {
  }

  static NFmiProducerHelperInfo MakeProducerHelperInfo(NFmiProducerInfo &theProducerInfo,
                                                       NFmiInfoData::Type theType,
                                                       bool groundData = true)
  {
    return NFmiProducerHelperInfo(
        theProducerInfo.Name(), theProducerInfo.ProducerId(), theType, groundData);
  }

  std::string itsName;
  unsigned long itsProducerId;
  NFmiInfoData::Type itsDataType;
  bool fGroundData;
};

