//© Ilmatieteenlaitos/Marko.
// Original 20.6.2006
//
// Luokka pitää huolta Editoriin konffatuista malli tuottajista.
// Voidaan konffata editorin kolme päätuottajat ja 10 sivu tuottajaa.
// Pää tuottajat näkyvät esim. luotaus valikoissa (toistaiseksi vain 3 radio buttonia käytössä).
// Pää ja Sivu tuottajat näkyvät esim. popup-valikoissa kun erilaisille näytöille valitaan dataa.
//---------------------------------------------------------- NFmiWarningCenterSystem.h

#include "NFmiProducerSystem.h"
#include <newbase/NFmiSettings.h>
#include <newbase/NFmiMetTime.h>
#include <newbase/NFmiLevel.h>

const std::string &NFmiProducerInfo::ShortName(size_t index) const
{
  if (index < itsShortNameVector.size())
    return itsShortNameVector[index];
  else
    throw std::runtime_error("Out of bounds error in NFmiProducerInfo::ShortName");
}

void NFmiProducerInfo::ShortName(const std::string &newValue, size_t index)
{
  if (index < itsShortNameVector.size())
    itsShortNameVector[index] = newValue;
  else
    throw std::runtime_error("Out of bounds error in NFmiProducerInfo::ShortName");
}

// newShortNames pitää sisällään yhden tai useamman lyhyen nimen pilkuilla erotellussa listassa.
void NFmiProducerInfo::SetShortNames(const std::string &newShortNames)
{
  itsShortNameVector.clear();
  itsShortNameVector = NFmiStringTools::Split(newShortNames, ",");
  // poistetaan mhdolliset tyhjät stringit listasta
  itsShortNameVector.erase(std::remove(itsShortNameVector.begin(), itsShortNameVector.end(), ""),
                           itsShortNameVector.end());
}

NFmiProducer NFmiProducerInfo::GetProducer(void)
{
  return NFmiProducer(itsProducerId, itsName);
}

NFmiProducerInfo NFmiProducerSystem::GetProducerInfoFromSettings(
    const std::string &theUsedNameSpaceBase)
{
  NFmiProducerInfo prod;
  prod.Name(NFmiSettings::Require<std::string>(theUsedNameSpaceBase + "::Name"));
  prod.SetShortNames(NFmiSettings::Require<std::string>(theUsedNameSpaceBase + "::ShortName"));
  prod.UltraShortName(
      NFmiSettings::Require<std::string>(theUsedNameSpaceBase + "::UltraShortName"));
  prod.Description(NFmiSettings::Require<std::string>(theUsedNameSpaceBase + "::Description"));
  prod.ProducerId(NFmiSettings::Require<unsigned long>(theUsedNameSpaceBase + "::ProducerIds"));
  prod.HasRealVerticalData(
      NFmiSettings::Require<bool>(theUsedNameSpaceBase + "::HasRealVerticalData"));
  prod.HasQ2ArchiveData(
      NFmiSettings::Optional<bool>(theUsedNameSpaceBase + "::HasQ2ArchiveData", false));

  return prod;
}

// Settings should be following format
// ProducerSystem::Producer1
// {
//   Name = Hirlam
//   ShortName = Hir
//   Description = Hirlam RCR model
//   ProducerIds = 1,230
//   HasRealVerticalData = 1
// }
// ProducerSystem::Producer2
// {
//   Name = Ecmwf
//   ShortName = Ec
//   Description = Ecmwf global model
//   ProducerIds = 240
//   HasRealVerticalData = 1
// }
// and so on. System will try to find as many Producer# as it can.
// You should put them in rising order in file to make it clearer.
void NFmiProducerSystem::InitFromSettings(const std::string &theInitNameSpace)
{
  itsProducers.clear();
  itsInitializeLogStr = "";

  int maxDirectoryCount = 1000;  // lets try to read 20 diffrent models producer infos
  int prodCounter = 0;
  int consecutiveMissingCount = 0;  // kuinka monta peräkkäin on ollut puuttuvaa asetusta
  const int maxConsecutiveMissingCount = 20;  // ei kuitenkaan käydä läpi kaikkia 1000
                                              // mahdollisuutta, vaan jos on ollut näin monta
                                              // puuttuvaa peräkkäin, niin lopetetaan
  for (int i = 1; i <= maxDirectoryCount; i++)
  {
    try
    {
      std::string usedNameSpaceBase = theInitNameSpace + "::Producer";
      usedNameSpaceBase += NFmiStringTools::Convert(i);
      NFmiProducerInfo prodInfo(GetProducerInfoFromSettings(usedNameSpaceBase));
      Add(prodInfo);
      prodCounter++;
      consecutiveMissingCount = 0;
    }
    catch (std::exception & /* e */)
    {
      consecutiveMissingCount++;
    }
    catch (...)
    {
    }
    if (consecutiveMissingCount > maxConsecutiveMissingCount)
      break;  // tein tämän skippauksen, koska debug versiossa 1000 läpikäynti on aika hidasta,
              // release versiossa ei juuri huomaa
  }
  if (prodCounter > 0)
  {
    itsInitializeLogStr += "Producer initialization ok. There were ";
    itsInitializeLogStr += NFmiStringTools::Convert(prodCounter);
    itsInitializeLogStr += " model producer settings found in initialization.";
  }
  else
  {
    itsInitializeLogStr +=
        "Error in NFmiProducerSystem::InitFromSettings: Producer initialization failed, no "
        "producers were found, check editor.conf -file.";
    throw std::runtime_error(itsInitializeLogStr);
  }
}

// kuten parametrin nimestäkin voi päätellä, on indeksi alkaa ykkösestä eteenpäin
bool NFmiProducerSystem::ExistProducer(unsigned int index1Based) const
{
  // jos annettu indeksi on 0, o-1 -> 4 miljardia unsigned maailmassa, joten riittää yksi testi
  if (index1Based - 1 < itsProducers.size())
    return true;
  return false;
}

NFmiProducerInfo &NFmiProducerSystem::Producer(unsigned int index1Based)
{
  // jos annettu indeksi on 0, o-1 -> 4 miljardia unsigned maailmassa, joten riittää yksi testi
  if (index1Based - 1 < itsProducers.size())
    return itsProducers[index1Based - 1];
  else
  {
    static NFmiProducerInfo dummy;
    return dummy;
  }
}

// Etsi halutun tuottajan ProducerInfo indeksi, eli indeksi millä NFmiProducerInfo-otuksen saa ulos
// Producer-matodilla. Palautuva indeksi on siis 1:stä lähtevä ja 0 tarkoittaa että kyseitä
// tuottajaa ei löytynyt.
// Etsintä tehdään tuottaja id:tä vertailemalla.
unsigned int NFmiProducerSystem::FindProducerInfo(const NFmiProducer &theProducer)
{
  for (unsigned int index = 0; index < itsProducers.size(); index++)
  {
    NFmiProducerInfo &prodInfo = itsProducers[index];
    if (prodInfo.ProducerId() == static_cast<unsigned long>(theProducer.GetIdent()))
      return index + 1;  // palautetaan siis 1-pohjainen indeksi
  }
  return 0;  // ei löytynyt, palautetaan 0;
}

NFmiString NFmiProducerSystem::GetProducerAndLevelTypeString(const NFmiProducer &theProducer,
                                                             const NFmiLevel &theLevel,
                                                             const NFmiMetTime &theOriginTime,
                                                             bool fEncloseInBracers)
{
  NFmiString txt;
  if (fEncloseInBracers)
    txt += "(";

  // etsi mallin nimi
  unsigned int modelIndex = FindProducerInfo(theProducer);
  if (modelIndex > 0)
    txt += Producer(modelIndex).UltraShortName();
  else
    txt += "X?";

  txt += theOriginTime.ToStr("HH ");

  if (theLevel.LevelType() == kFmiHybridLevel)
    txt += "hyb";
  else if (theLevel.LevelType() == kFmiPressureLevel)
    txt += "pre";
  else
    txt += "sfc";

  if (fEncloseInBracers)
    txt += ")";

  return txt;
}
