// NFmiHelpDataInfo.cpp

#ifdef _MSC_VER
#pragma warning(disable : 4996)  // 4996 poistaa epäturvallisten string manipulaatio funktioiden
                                 // käytöstä tulevat varoitukset. En aio käyttää ehdotettuja
// turvallisia _s -funktioita (esim. sprintf_s), koska ne eivät ole
// linux yhteensopivia.
#endif

#include "NFmiHelpDataInfo.h"
#include "NFmiPathUtils.h"
#include <newbase/NFmiProducerName.h>
#include <newbase/NFmiArea.h>
#include <newbase/NFmiAreaFactory.h>
#include <newbase/NFmiStereographicArea.h>
#include <newbase/NFmiSettings.h>
#include <newbase/NFmiFileString.h>
#include <boost/algorithm/string/replace.hpp>

using namespace std;

static void FixPathEndWithSeparator(std::string &theFixedPathStr)
{
  if (theFixedPathStr.empty() == false)
  {
    NFmiFileString tmpFileStr(theFixedPathStr);
    tmpFileStr.NormalizeDelimiter();  // varmistetaan myös että polun merkit ovat oikein päin
    theFixedPathStr = static_cast<char *>(tmpFileStr);

    std::string::value_type lastLetter = theFixedPathStr[theFixedPathStr.size() - 1];
    if (lastLetter != kFmiDirectorySeparator) theFixedPathStr.push_back(kFmiDirectorySeparator);
  }
}

static void FixPatternSeparators(std::string &theFixedPatternStr)
{
  if (theFixedPatternStr.empty() == false)
  {
    NFmiFileString tmpFileStr(theFixedPatternStr);
    tmpFileStr.NormalizeDelimiter();  // varmistetaan että polun merkit ovat oikein päin
    theFixedPatternStr = static_cast<char *>(tmpFileStr);
  }
}

// Must fix incomplete absolute path with possible drive letter and ':' character.
// So if cachePath = "/path/xxx" and  absoluteControlBasePath = "C:/yyy/zzz"  => cachePath =
// "c:/path/xxx" Must also possibly add directory slash at the end of cachePath string. If cachePath
// = "c:/path/xxx" => "c:/path/xxx/"
static void FixCachePath(std::string &cachePath, const std::string &absoluteControlBasePath, bool directoryCase)
{
  cachePath = PathUtils::fixMissingDriveLetterToAbsolutePath(cachePath, absoluteControlBasePath);
  cachePath = PathUtils::simplifyWindowsPath(cachePath);
  if (directoryCase)
      ::FixPathEndWithSeparator(cachePath);
}


// ***********************************************************
// ******** NFmiHelpDataInfo alkaa ***************************
// ***********************************************************

NFmiHelpDataInfo::NFmiHelpDataInfo(const NFmiHelpDataInfo &theOther) = default;

NFmiHelpDataInfo &NFmiHelpDataInfo::operator=(const NFmiHelpDataInfo &theOther) = default;

// Esim. 
// fileFilter = P:\data\partial_data\iasi\*_iasi.sqd
// newBaseDirectory = D:\smartmet\wrk\data\cache\
// Palautettava arvo on D:\smartmet\wrk\data\cache\iasi\*_iasi.sqd
static std::string CopyLatestDirectoryLevelFromFileFilterToNewBaseDirectory(
    const std::string &fileFilter, const std::string &newBaseDirectory)
{
  if (!fileFilter.empty())
  {
    int slashesFound = 0;
    size_t i = fileFilter.size() - 1;
    for (; i > 0; i--)
    {
      if (fileFilter[i] == '\\') 
          slashesFound++;
      if (slashesFound > 1) 
          break;
    }

    if (slashesFound > 1)
    {
      std::string newFileFilter = newBaseDirectory;
      newFileFilter += std::string(fileFilter.begin() + i + 1, fileFilter.end());
      return newFileFilter;
    }
  }
  return "";
}

static bool MakeCombinedDataFilePattern(NFmiHelpDataInfo &theDataInfo,
                                        const NFmiHelpDataInfoSystem &theHelpDataSystem)
{
  auto partialDataCacheFileFilter = ::CopyLatestDirectoryLevelFromFileFilterToNewBaseDirectory(
      theDataInfo.FileNameFilter(), theHelpDataSystem.LocalDataPartialDirectory());
  theDataInfo.PartialDataCacheFileNameFilter(partialDataCacheFileFilter);
  return !partialDataCacheFileFilter.empty();
}

static bool MakeCombinedDataResultFileFilter(NFmiHelpDataInfo &theDataInfo,
                                        const NFmiHelpDataInfoSystem &theHelpDataSystem)
{
  auto resultDataCacheFileFilter = ::CopyLatestDirectoryLevelFromFileFilterToNewBaseDirectory(
      theDataInfo.FileNameFilter(), theHelpDataSystem.LocalDataCacheDirectory());
  theDataInfo.CombinedResultDataFileFilter(resultDataCacheFileFilter);
  return !resultDataCacheFileFilter.empty();
}

static long GetDefaultTimeInterpolationRangeInMinutes(NFmiInfoData::Type dataType)
{
    // Nämä aikainterpolaatio jutut koskevat siis vain hilamuotoisia datoja, ei asemadataa.
    // Esim. erilaiset tutkadatat saavat lyhyemmän interpolaatio rajan.
    if(dataType == NFmiInfoData::kObservations || dataType == NFmiInfoData::kSingleStationRadarData)
        return 0;
    else
        return kTimeInterpolationRangeDefaultValueInMinutes;
}

void NFmiHelpDataInfo::InitFromSettings(const std::string &theBaseKey,
                                        const std::string &theName,
                                        const NFmiHelpDataInfoSystem &theHelpDataSystem)
{
  fForceFileFilterName = false;
  itsName = theName;
  itsBaseNameSpace = theBaseKey + "::" + theName;

  std::string fileNameFilterKey = itsBaseNameSpace + "::FilenameFilter";
  if (NFmiSettings::IsSet(fileNameFilterKey))
  {
    // Read configuration
    itsFileNameFilter = NFmiSettings::Require<std::string>(fileNameFilterKey);
    ::FixPatternSeparators(itsFileNameFilter);
    itsDataType = static_cast<NFmiInfoData::Type>(
        NFmiSettings::Require<int>(itsBaseNameSpace + "::DataType"));
    itsFakeProducerId = NFmiSettings::Optional<int>(itsBaseNameSpace + "::ProducerId", 0);
    fNotifyOnLoad = NFmiSettings::Optional<bool>(itsBaseNameSpace + "::NotifyOnLoad", false);
    itsNotificationLabel =
        NFmiSettings::Optional<string>(itsBaseNameSpace + "::NotificationLabel", "");
    itsCustomMenuFolder =
        NFmiSettings::Optional<string>(itsBaseNameSpace + "::CustomMenuFolder", "");
    itsReportNewDataTimeStepInMinutes =
        NFmiSettings::Optional<int>(itsBaseNameSpace + "::ReportNewDataTimeStepInMinutes", 0);
    itsReportNewDataLabel =
        NFmiSettings::Optional<string>(itsBaseNameSpace + "::ReportNewDataLabel", "");
    SetCombineDataMaxTimeSteps(
        NFmiSettings::Optional<int>(itsBaseNameSpace + "::CombineDataMaxTimeSteps", 0), theHelpDataSystem);
    fMakeSoundingIndexData =
        NFmiSettings::Optional<bool>(itsBaseNameSpace + "::MakeSoundingIndexData", false);
    itsRequiredGroundDataFileFilterForSoundingIndexCalculations = NFmiSettings::Optional<string>(
        itsBaseNameSpace + "::RequiredGroundDataFileFilterForSoundingIndexCalculations", "");
    itsAdditionalArchiveFileCount =
        NFmiSettings::Optional<int>(itsBaseNameSpace + "::AdditionalArchiveFileCount", 0);
    fEnable = NFmiSettings::Require<bool>(itsBaseNameSpace + "::Enable");
    fNonFixedTimeGab = NFmiSettings::Optional<bool>(itsBaseNameSpace + "::NonFixedTimeGab", false);
    itsModelRunTimeGapInHours =
        NFmiSettings::Optional<float>(itsBaseNameSpace + "::ModelRunTimeGapInHours", 0);
    itsTimeInterpolationRangeInMinutes =
        NFmiSettings::Optional<long>(itsBaseNameSpace + "::TimeInterpolationRangeInMinutes",
                                     ::GetDefaultTimeInterpolationRangeInMinutes(itsDataType));
    fReloadCaseStudyData =
        NFmiSettings::Optional<bool>(itsBaseNameSpace + "::ReloadCaseStudyData", true);
    fAllowCombiningToSurfaceDataInSoundingView = NFmiSettings::Optional<bool>(
        itsBaseNameSpace + "::AllowCombiningToSurfaceDataInSoundingView", false);
    fCaseStudyLegacyOnly = NFmiSettings::Optional<bool>(
        itsBaseNameSpace + "::CaseStudyLegacyOnly", false);
    itsAgingTimeLimitInMinutes =
        NFmiSettings::Optional<int>(itsBaseNameSpace + "::AgingTimeLimitInMinutes", -1);

    std::string imageProjectionKey(itsBaseNameSpace + "::ImageProjection");
    if (NFmiSettings::IsSet(imageProjectionKey))
    {
      boost::shared_ptr<NFmiArea> area =
          NFmiAreaFactory::Create(NFmiSettings::Require<std::string>(imageProjectionKey));
      if (area)
      {
        if (area->XYArea().Width() != 1 || area->XYArea().Height() != 1)
        {
          area->SetXYArea(NFmiRect(0, 0, 1, 1));
        }
        itsImageArea = area;
      }
      NFmiParam param(NFmiSettings::Require<int>(itsBaseNameSpace + "::ParameterId"),
                      NFmiSettings::Require<std::string>(itsBaseNameSpace + "::ParameterName"));
      itsImageDataIdent = NFmiDataIdent(param, NFmiProducer(itsFakeProducerId));
    }
  }
}

void NFmiHelpDataInfo::SetCombineDataMaxTimeSteps(int newValue,
                                                  const NFmiHelpDataInfoSystem &theHelpDataSystem)
{
  itsCombineDataMaxTimeSteps = newValue;
  if (itsCombineDataMaxTimeSteps > 0)
  {
    auto status1 = ::MakeCombinedDataFilePattern(*this, theHelpDataSystem);
    auto status2 = ::MakeCombinedDataResultFileFilter(*this, theHelpDataSystem);
    if (!status1 || !status2)
    {
      // Ei voi tehda combineddata polkua, laitetaan yhdistely pois
      itsCombineDataMaxTimeSteps = 0;
      // Luodaan lokitussanoma ongelmista
      itsCombineDataErrorMessage = "Problem when doing CombineData settings with data '";
      itsCombineDataErrorMessage += itsName;
      itsCombineDataErrorMessage +=
          "'. Couldn't create properly a partial-data file pattern or result data file pattern.";
      itsCombineDataErrorMessage += " Combined data production for this data is now disabled...";
    }
  }
}

void NFmiHelpDataInfo::ImageArea(boost::shared_ptr<NFmiArea> &newValue)
{
  itsImageArea = newValue;
}

static std::string MakeCacheFilePattern(const NFmiHelpDataInfo &theDataInfo,
                                        const NFmiHelpDataInfoSystem &theHelpDataSystem)
{
  NFmiFileString fileStr(theDataInfo.FileNameFilter());
  std::string cachePattern(theHelpDataSystem.LocalDataLocalDirectory());
  cachePattern += static_cast<char *>(fileStr.FileName());
  return cachePattern;
}

void NFmiHelpDataInfo::FileNameFilter(const std::string &newValue, bool forceFileNameFilter)
{
  itsFileNameFilter = newValue;
  fForceFileFilterName = forceFileNameFilter;
}

// tämä on viritys, että olisi funktio, jolla voidaan pyytää käytetty fileFilter, riippuen siitä
// onko cache käytössä vai ei
const std::string NFmiHelpDataInfo::UsedFileNameFilter(
    const NFmiHelpDataInfoSystem &theHelpDataInfoSystem) const
{
  if (fForceFileFilterName || theHelpDataInfoSystem.UseQueryDataCache() == false ||
      itsDataType == NFmiInfoData::kStationary)
    return FileNameFilter();
  else
  {
    if (!IsCombineData())
      return ::MakeCacheFilePattern(*this, theHelpDataInfoSystem);
    else
      return PartialDataCacheFileNameFilter();
  }
}

// Datojen nimissä on joskus ylimääräinen kirjain nimen edessä, että se menisi akkosjärjestyksessä alku tai loppu päähän.
// Poistetaan tälläinen kirjain nimestä selkeyden takia.
// Lisäksi korvataan kaikki alaviivat space:illa.
// esim. A_ecmwf_eurooppa_pinta  =>  ecmwf eurooppa pinta
std::string NFmiHelpDataInfo::GetCleanedName() const
{
    std::string newName(itsName);

    if(newName.size() > 2 && newName[1] == '_')
        newName = std::string(itsName.begin() + 2, itsName.end());

    NFmiStringTools::ReplaceChars(newName, '_', ' '); // muutetaan myös ala-viivat spaceiksi

    return newName;
}

bool NFmiHelpDataInfo::IsDataUsedCaseStudyChecks(bool caseStudyModeOn) const
{
  if (!CaseStudyLegacyOnly())
  {
    return true;
  }
  return caseStudyModeOn;
}

// ***********************************************************
// ******** NFmiHelpDataInfoSystem alkaa *********************
// ***********************************************************

NFmiHelpDataInfo &NFmiHelpDataInfoSystem::DynamicHelpDataInfo(int theIndex)
{
  static NFmiHelpDataInfo dummy;
  if (!itsDynamicHelpDataInfos.empty() && theIndex >= 0 &&
      theIndex < static_cast<int>(itsDynamicHelpDataInfos.size()))
    return itsDynamicHelpDataInfos[theIndex];
  return dummy;
}
NFmiHelpDataInfo &NFmiHelpDataInfoSystem::StaticHelpDataInfo(int theIndex)
{
  static NFmiHelpDataInfo dummy;
  if (!itsStaticHelpDataInfos.empty() && theIndex >= 0 &&
      theIndex < static_cast<int>(itsStaticHelpDataInfos.size()))
    return itsStaticHelpDataInfos[theIndex];
  return dummy;
}

// Etsii seuraavan satel-kuvan kanavan. Pitää olla sama tuottaja.
// Palauttaa uuden dataidentin, missä uusi parametri.
NFmiDataIdent NFmiHelpDataInfoSystem::GetNextSatelChannel(const NFmiDataIdent &theDataIdent,
                                                          FmiDirection theDir)
{
  NFmiDataIdent returnDataIdent(theDataIdent);
  FmiProducerName prodId = static_cast<FmiProducerName>(theDataIdent.GetProducer()->GetIdent());
  size_t count = itsDynamicHelpDataInfos.size();
  std::vector<NFmiDataIdent> dataIdentVec;
  int counter = 0;
  int currentIndex = -1;
  for (size_t i = 0; i < count; i++)
  {
    if (itsDynamicHelpDataInfos[i].DataType() == NFmiInfoData::kSatelData)
    {
      if (prodId == itsDynamicHelpDataInfos[i].ImageDataIdent().GetProducer()->GetIdent())
      {
        dataIdentVec.push_back(itsDynamicHelpDataInfos[i].ImageDataIdent());
        if (theDataIdent.GetParamIdent() ==
            itsDynamicHelpDataInfos[i].ImageDataIdent().GetParamIdent())
          currentIndex = counter;  // laitetaan currentti param index talteen
        counter++;
      }
    }
  }
  if (counter > 1)
  {
    if (theDir == kForward)
      currentIndex++;
    else
      currentIndex--;
    if (currentIndex < 0)
      currentIndex = counter - 1;
    if (currentIndex >= counter)
      currentIndex = 0;
    returnDataIdent = dataIdentVec[currentIndex];
  }
  return returnDataIdent;
}

void NFmiHelpDataInfoSystem::AddDynamic(const NFmiHelpDataInfo &theInfo)
{
  itsDynamicHelpDataInfos.push_back(theInfo);
}

void NFmiHelpDataInfoSystem::AddStatic(const NFmiHelpDataInfo &theInfo)
{
  itsStaticHelpDataInfos.push_back(theInfo);
}

void NFmiHelpDataInfoSystem::InitDataType(const std::string &theBaseKey,
                                          std::vector<NFmiHelpDataInfo> &theHelpDataInfos,
                                          bool fStaticData)
{
  std::vector<std::string> dataKeys = NFmiSettings::ListChildren(theBaseKey);
  std::vector<std::string>::iterator iter = dataKeys.begin();
  for (; iter != dataKeys.end(); ++iter)
  {
    NFmiHelpDataInfo hdi;
    hdi.InitFromSettings(theBaseKey, *iter, *this);
    if (fStaticData)
      hdi.ForceFileFilterName(
          true);  // varmistetaan että staattisia datoja ei yritetä lukea lokaali cachesta

    // HelpDataInfolla pitää olla tyyppi, muuten sitä ei lisätä listaan. Kun vuoden 2013 lopussa
    // tehtiin erillinen lista
    // eri datojen enable-ominaisuudesta yhteen konffitiedostoon (mm.
    // helpdatainfo_enable_data_fmi_heavy.conf),
    // tuli mahdolliseksi, että tässä tuli ns. haamu dataInfoja, jotka nyt pitää karsia.
    if (hdi.DataType() != NFmiInfoData::kNoDataType)
      theHelpDataInfos.push_back(hdi);
  }
}

void NFmiHelpDataInfoSystem::SetLocalDataBaseDirectory(const std::string &newBaseDirectory)
{
  itsLocalDataBaseDirectory = newBaseDirectory;
  ::FixCachePath(itsLocalDataBaseDirectory, itsAbsoluteControlBasePath, true);
  itsLocalDataLocalDirectory = itsLocalDataBaseDirectory + "local\\";
  itsLocalDataTmpDirectory = itsLocalDataBaseDirectory + "tmp\\";
  itsLocalDataPartialDirectory = itsLocalDataBaseDirectory + "partial_data\\";
  itsLocalDataCacheDirectory = itsLocalDataBaseDirectory + "cache\\";
}

void NFmiHelpDataInfoSystem::InitFromSettings(const std::string &theBaseNameSpaceStr,
                                              const std::string &absoluteControlBasePath,
                                              std::string theHelpEditorFileNameFilter,
                                              std::string theHelpDataName)
{
  itsBaseNameSpace = theBaseNameSpaceStr;
  itsAbsoluteControlBasePath = absoluteControlBasePath;
  auto localDataBaseDirectory = NFmiSettings::Require<std::string>(itsBaseNameSpace + "::LocalDataBaseDirectory");
  SetLocalDataBaseDirectory(localDataBaseDirectory);

  itsCacheTmpFileNameFix =
      NFmiSettings::Require<std::string>(itsBaseNameSpace + "::CacheTmpFileNameFix");
  fUseQueryDataCache = NFmiSettings::Require<bool>(itsBaseNameSpace + "::UseQueryDataCache");

  fDoCleanCache = NFmiSettings::Require<bool>(itsBaseNameSpace + "::DoCleanCache");
  itsCacheFileKeepMaxDays =
      NFmiSettings::Require<float>(itsBaseNameSpace + "::CacheFileKeepMaxDays");
  itsCacheMaxFilesPerPattern =
      NFmiSettings::Require<int>(itsBaseNameSpace + "::CacheMaxFilesPerPattern");

  itsCacheMediumFileSizeMB =
      NFmiSettings::Require<double>(itsBaseNameSpace + "::CacheMediumFileSizeMB");
  itsCacheLargeFileSizeMB =
      NFmiSettings::Require<double>(itsBaseNameSpace + "::CacheLargeFileSizeMB");
  itsCacheMaximumFileSizeMB =
      NFmiSettings::Require<double>(itsBaseNameSpace + "::CacheMaximumFileSizeMB");

  // Read static helpdata configurations
  InitDataType(itsBaseNameSpace + "::Static", itsStaticHelpDataInfos, true);

  // Read dynamic helpdata configurations
  InitDataType(itsBaseNameSpace + "::Dynamic", itsDynamicHelpDataInfos, false);

  // Lisätään help editor mode datan luku jos niin on haluttu
  if (theHelpEditorFileNameFilter.empty() == false)
  {
    NFmiHelpDataInfo helpDataInfo;
    helpDataInfo.Name(theHelpDataName);
    helpDataInfo.FileNameFilter(theHelpEditorFileNameFilter);
    helpDataInfo.DataType(NFmiInfoData::kEditingHelpData);
    helpDataInfo.NonFixedTimeGab(true);
    AddDynamic(helpDataInfo);
  }
}

void NFmiHelpDataInfoSystem::StoreToSettings()
{
  if (itsBaseNameSpace.empty() == false)
  {
    // HUOM! tässä on toistaiseksi vain cacheen liittyvien muutosten talletukset
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::LocalDataBaseDirectory"), itsLocalDataBaseDirectory, true);
    NFmiSettings::Set(
        std::string(itsBaseNameSpace + "::CacheTmpFileNameFix"), itsCacheTmpFileNameFix, true);
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::UseQueryDataCache"),
                      NFmiStringTools::Convert(fUseQueryDataCache),
                      true);
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::DoCleanCache"),
                      NFmiStringTools::Convert(fDoCleanCache),
                      true);
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::CacheFileKeepMaxDays"),
                      NFmiStringTools::Convert(itsCacheFileKeepMaxDays),
                      true);
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::CacheMaxFilesPerPattern"),
                      NFmiStringTools::Convert(itsCacheMaxFilesPerPattern),
                      true);
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::CacheMediumFileSizeMB"),
                      NFmiStringTools::Convert(itsCacheMediumFileSizeMB),
                      true);
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::CacheLargeFileSizeMB"),
                      NFmiStringTools::Convert(itsCacheLargeFileSizeMB),
                      true);
    NFmiSettings::Set(std::string(itsBaseNameSpace + "::CacheMaximumFileSizeMB"),
                      NFmiStringTools::Convert(itsCacheMaximumFileSizeMB),
                      true);
  }
  else
    throw std::runtime_error(
        "Error in NFmiHelpDataInfoSystem::StoreToSettings, unable to store setting.");
}

void NFmiHelpDataInfoSystem::InitSettings(const NFmiHelpDataInfoSystem &theOther,
                                          bool fDoHelpDataInfo)
{
  SetLocalDataBaseDirectory(theOther.itsLocalDataBaseDirectory);
  this->itsCacheTmpFileNameFix = theOther.itsCacheTmpFileNameFix;
  this->fUseQueryDataCache = theOther.fUseQueryDataCache;
  this->fDoCleanCache = theOther.fDoCleanCache;
  this->itsCacheFileKeepMaxDays = theOther.itsCacheFileKeepMaxDays;
  this->itsCacheMaxFilesPerPattern = theOther.itsCacheMaxFilesPerPattern;
  this->itsBaseNameSpace = theOther.itsBaseNameSpace;

  if (fDoHelpDataInfo)
  {
    this->itsDynamicHelpDataInfos = theOther.itsDynamicHelpDataInfos;
    this->itsStaticHelpDataInfos = theOther.itsStaticHelpDataInfos;
  }
}

void NFmiHelpDataInfoSystem::ResetAllDynamicDataTimeStamps()
{
  size_t ssize = itsDynamicHelpDataInfos.size();
  for (size_t i = 0; i < ssize; i++)
    itsDynamicHelpDataInfos[i].LatestFileTimeStamp(-1);
}

static NFmiHelpDataInfo *FindHelpDataInfo(std::vector<NFmiHelpDataInfo> &theHelpInfos,
                                          const std::string &theFileNameFilter,
                                          const NFmiHelpDataInfoSystem &theHelpDataInfoSystem)
{
  for (auto &helpDataInfo : theHelpInfos)
  {
    // Siis jos joko FileNameFilter (server path), UsedFileNameFilter (local path) tai CombineDataPathAndFileName (yhdistelmä datoissa tämä
    // on se data joka luetaan sisään SmartMetiin) on etsitty, palautetaan helpInfo.
    if (helpDataInfo.UsedFileNameFilter(theHelpDataInfoSystem) == theFileNameFilter ||
        helpDataInfo.FileNameFilter() == theFileNameFilter ||
        helpDataInfo.CombinedResultDataFileFilter() == theFileNameFilter)
      return &helpDataInfo;
  }
  return 0;
}

// Etsii annetun fileNameFilterin avulla HelpDataInfon ja palauttaa sen, jos löytyi.
// Jos ei löytynyt vastaavaa filePatternia, palauttaa 0-pointterin.
// Käy ensin läpi dynaamiset helpDataInfot ja sitten staattiset.
NFmiHelpDataInfo *NFmiHelpDataInfoSystem::FindHelpDataInfo(const std::string &theFileNameFilter)
{
  if (theFileNameFilter.empty())
    return 0;

  NFmiHelpDataInfo *helpInfo =
      ::FindHelpDataInfo(itsDynamicHelpDataInfos, theFileNameFilter, *this);
  if (helpInfo == 0)
    helpInfo = ::FindHelpDataInfo(itsStaticHelpDataInfos, theFileNameFilter, *this);

  return helpInfo;
}

static void CollectCustomMenuItems(const std::vector<NFmiHelpDataInfo> &theHelpInfos,
                                   std::set<std::string> &theMenuSet)
{
  size_t ssize = theHelpInfos.size();
  for (size_t i = 0; i < ssize; i++)
  {
    if (theHelpInfos[i].CustomMenuFolder().empty() == false)
      theMenuSet.insert(theHelpInfos[i].CustomMenuFolder());
  }
}

// kerää uniikki lista mahdollisista custom Menu folder asetuksista
std::vector<std::string> NFmiHelpDataInfoSystem::GetUniqueCustomMenuList()
{
  std::set<std::string> menuSet;
  ::CollectCustomMenuItems(itsDynamicHelpDataInfos, menuSet);
  ::CollectCustomMenuItems(itsStaticHelpDataInfos, menuSet);

  std::vector<std::string> menuList(menuSet.begin(), menuSet.end());
  return menuList;
}

static void CollectCustomMenuHelpDatas(const std::vector<NFmiHelpDataInfo> &theHelpInfos,
                                       const std::string &theCustomFolder,
                                       std::vector<NFmiHelpDataInfo> &theCustomHelpDatas)
{
  size_t ssize = theHelpInfos.size();
  for (size_t i = 0; i < ssize; i++)
  {
    if (theHelpInfos[i].CustomMenuFolder().empty() == false)
      if (theHelpInfos[i].CustomMenuFolder() == theCustomFolder)
        theCustomHelpDatas.push_back(theHelpInfos[i]);
  }
}

// kerätään listä niista helpDataInfoissta, joissa on asetettu kyseinen customFolder
std::vector<NFmiHelpDataInfo> NFmiHelpDataInfoSystem::GetCustomMenuHelpDataList(
    const std::string &theCustomFolder)
{
  std::vector<NFmiHelpDataInfo> helpDataList;
  if (theCustomFolder.empty() == false)
  {
    ::CollectCustomMenuHelpDatas(itsDynamicHelpDataInfos, theCustomFolder, helpDataList);
    ::CollectCustomMenuHelpDatas(itsStaticHelpDataInfos, theCustomFolder, helpDataList);
  }
  return helpDataList;
}
