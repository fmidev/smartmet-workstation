
#ifdef _MSC_VER
#pragma warning( disable : 4800 ) // tämä estää pitkän varoituksen joka tulee kun käytetään CachedRegBool -> registry_int<bool> -tyyppistä dataa, siellä operator T -metodissa DWORD muuttuu bool:iksi (Huom! static_cast ei auta)
#endif

#include "NFmiApplicationWinRegistry.h"
#include "NFmiStringTools.h"
#include "NFmiSettings.h"
#include "NFmiHelpDataInfo.h"
#include "SoundingViewSettingsFromWindowsRegisty.h"
#include "catlog/catlog.h"

#include <unordered_map>

// ************************************************
// ****   NFmiGriddingPropertiesWinRegistry *******
// ************************************************

NFmiGriddingPropertiesWinRegistry::NFmiGriddingPropertiesWinRegistry()
:mInitialized(false)
,mBaseRegistryPath()
,mSectionName("\\GriddingProperties")
,mFunction()
,mRangeLimitInKm()
,mLocalFitMethod()
,mLocalFitDelta()
,mSmoothLevel()
,mLocalFitFilterRadius()
,mLocalFitFilterFactor()
,mGriddingProperties()
{}

bool NFmiGriddingPropertiesWinRegistry::Init(const std::string &baseRegistryPath, bool isToolMasterAvailable)
{
    if(mInitialized)
        throw std::runtime_error("NFmiGriddingPropertiesWinRegistry::Init: all ready initialized.");

    mInitialized = true;
    mBaseRegistryPath = baseRegistryPath;

    mGriddingProperties.toolMasterAvailable(isToolMasterAvailable);

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    // Otetaan kaikki oletusarvot pelkillä oletusarvoilla ensin rakennetusta mGriddingProperties dataosasta
    mFunction = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionName, "\\Function", usedKey, mGriddingProperties.function());
    mRangeLimitInKm = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, mSectionName, "\\RangeLimitInKm", usedKey, mGriddingProperties.rangeLimitInKm());
    mLocalFitMethod = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionName, "\\LocalFitMethod", usedKey, mGriddingProperties.localFitMethod());
    mLocalFitDelta = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, mSectionName, "\\LocalFitDelta", usedKey, mGriddingProperties.localFitDelta());
    mSmoothLevel = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionName, "\\SmoothLevel", usedKey, mGriddingProperties.smoothLevel());
    mLocalFitFilterRadius = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, mSectionName, "\\LocalFitFilterRadius", usedKey, mGriddingProperties.localFitFilterRadius());
    mLocalFitFilterFactor = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, mSectionName, "\\LocalFitFilterFactor", usedKey, mGriddingProperties.localFitFilterFactor());

    // Nyt kun rekisteri arvot on haettu, pitää päivittää tämä POD tyyppinen kokoomadata niiden mukaan
    UpdateGriddingPropertiesFromRegistry();

    return true;
}

void NFmiGriddingPropertiesWinRegistry::Update(const NFmiGriddingProperties &griddingProperties)
{
    // Ensin muutokset asetetaan rekiteri muutujiin
    *mFunction = griddingProperties.function();
    *mRangeLimitInKm = griddingProperties.rangeLimitInKm();
    *mLocalFitMethod = griddingProperties.localFitMethod();
    *mLocalFitDelta = griddingProperties.localFitDelta();
    *mSmoothLevel = griddingProperties.smoothLevel();
    *mLocalFitFilterRadius = griddingProperties.localFitFilterRadius();
    *mLocalFitFilterFactor = griddingProperties.localFitFilterFactor();
    // Lopuksi luokan oma data-member päivitetään vastaavasti
    mGriddingProperties = griddingProperties;
}

const NFmiGriddingProperties& NFmiGriddingPropertiesWinRegistry::GetGriddingProperties() const
{
    return mGriddingProperties;
}

void NFmiGriddingPropertiesWinRegistry::UpdateGriddingPropertiesFromRegistry()
{
    mGriddingProperties.function(static_cast<FmiGriddingFunction>(static_cast<int>(*mFunction)));
    mGriddingProperties.rangeLimitInKm(*mRangeLimitInKm);
    mGriddingProperties.localFitMethod(*mLocalFitMethod);
    mGriddingProperties.localFitDelta(*mLocalFitDelta);
    mGriddingProperties.smoothLevel(*mSmoothLevel);
    mGriddingProperties.localFitFilterRadius(*mLocalFitFilterRadius);
    mGriddingProperties.localFitFilterFactor(*mLocalFitFilterFactor);
}


// ************************************************
// ******   NFmiHelpDataEnableWinRegistry *********
// ************************************************

NFmiHelpDataEnableWinRegistry::NFmiHelpDataEnableWinRegistry()
:mInitialized(false)
,mBaseRegistryPath()
,mSectionName()
,mHelpDataEnableMap()
{
}

// Alustus menee niin että haetaan rekistereistä eri datojen enable -tiedot. 
// Jos sellainen löytyy datalle, asetetaan se arvoksi täällä ja myös annettuun theHelpDataInfoSystem:in helpdatan arvoksi.
// Jos ei löydy, asetetaan theHelpDataInfoSystem:ista saatu arvo (konffeista) myös rekisterin arvoksi.
bool NFmiHelpDataEnableWinRegistry::Init(const std::string &baseRegistryPath, NFmiHelpDataInfoSystem &theHelpDataInfoSystem)
{
    if(mInitialized)
        throw std::runtime_error("NFmiHelpDataEnableWinRegistry::Init: all ready initialized.");

    mInitialized = true;
    mBaseRegistryPath = baseRegistryPath;
    mSectionName = "\\EnableData";

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    int dynamicDataCount = theHelpDataInfoSystem.DynamicCount();
    for(int i = 0; i < dynamicDataCount; i++)
    {
        NFmiHelpDataInfo &dataInfo = theHelpDataInfoSystem.DynamicHelpDataInfo(i);
        std::string regName("\\");
        regName += dataInfo.Name();
        boost::shared_ptr<CachedRegBool> tmpValue = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, mSectionName, regName.c_str(), usedKey, dataInfo.IsEnabled()); // oletus arvoksi annetaan konffeista saatu arvo
        dataInfo.Enable(*tmpValue); // tässä annetaan enable arvo takaisin, jos se oli eri rekisterissä kuin oli konffeissa, tässä on nyt eri arvo
        mHelpDataEnableMap.insert(std::make_pair(dataInfo.FileNameFilter(), tmpValue));
    }
    return true;
}

// Update päivittää käyttäjän tekemät muutokset theHelpDataInfoSystem:istä rekisteriin.
bool NFmiHelpDataEnableWinRegistry::Update(NFmiHelpDataInfoSystem &theHelpDataInfoSystem)
{
    int dynamicDataCount = theHelpDataInfoSystem.DynamicCount();
    for(int i = 0; i < dynamicDataCount; i++)
    {
        NFmiHelpDataInfo &dataInfo = theHelpDataInfoSystem.DynamicHelpDataInfo(i);
        std::map<std::string, boost::shared_ptr<CachedRegBool> >::iterator it = mHelpDataEnableMap.find(dataInfo.FileNameFilter());
        if(it != mHelpDataEnableMap.end())
        {
            *(it->second) = dataInfo.IsEnabled();
        }
    }
    return true;
}

// **********************************************************
// *********   NFmiCaseStudySettingsWinRegistry *************
// **********************************************************

NFmiCaseStudySettingsWinRegistry::NFmiCaseStudySettingsWinRegistry() = default;

int NFmiCaseStudySettingsWinRegistry::GetDefaultLocalCacheCountValue(NFmiInfoData::Type dataType)
{
    switch(dataType)
    {
    case NFmiInfoData::kEditable:
    case NFmiInfoData::kViewable:
    case NFmiInfoData::kCopyOfEdited:
    case NFmiInfoData::kKepaData:
    case NFmiInfoData::kClimatologyData:
    case NFmiInfoData::kHybridData:
    case NFmiInfoData::kFuzzyData:
    case NFmiInfoData::kVerificationData:
    case NFmiInfoData::kModelHelpData:
    case NFmiInfoData::kEditingHelpData:
        return 3;
    case NFmiInfoData::kStationary:
    case NFmiInfoData::kObservations:
    case NFmiInfoData::kAnalyzeData:
    case NFmiInfoData::kFlashData:
    case NFmiInfoData::kTrajectoryHistoryData:
    case NFmiInfoData::kSingleStationRadarData:
        return 1;
    case NFmiInfoData::kSatelData:
        return 0;
    default:
        return 0;
    }
}

int NFmiCaseStudySettingsWinRegistry::GetDefaultCaseStudyCountValue(NFmiInfoData::Type dataType)
{
    switch(dataType)
    {
    case NFmiInfoData::kEditable:
    case NFmiInfoData::kViewable:
    case NFmiInfoData::kCopyOfEdited:
    case NFmiInfoData::kKepaData:
    case NFmiInfoData::kClimatologyData:
    case NFmiInfoData::kHybridData:
    case NFmiInfoData::kFuzzyData:
    case NFmiInfoData::kVerificationData:
    case NFmiInfoData::kModelHelpData:
    case NFmiInfoData::kEditingHelpData:
        return 3;
    case NFmiInfoData::kObservations:
    case NFmiInfoData::kAnalyzeData:
    case NFmiInfoData::kFlashData:
    case NFmiInfoData::kTrajectoryHistoryData:
    case NFmiInfoData::kSingleStationRadarData:
        return 1;
    case NFmiInfoData::kSatelData:
        return 30;
    default:
        return 0;
    }
}

static void CheckAndFixDataCountValue(NFmiInfoData::Type dataType, boost::shared_ptr<CachedRegInt>& countValue, bool doLocalCacheCount)
{
    auto defaultCountValueByType = doLocalCacheCount ? NFmiCaseStudySettingsWinRegistry::GetDefaultLocalCacheCountValue(dataType) : NFmiCaseStudySettingsWinRegistry::GetDefaultCaseStudyCountValue(dataType);
    auto value = (int)(*countValue);
    if(value < 0)
    {
        // Mikään luku ei saa olla negatiivinen!
        *countValue = defaultCountValueByType;
    }
    else if(defaultCountValueByType == 0 && value != 0)
    {
        // Näillä "0 lkm" tyypeillä (ainakin satel-image) ei saa olla 0:sta poikkeavaa arvoa
        *countValue = defaultCountValueByType;
    }
    else if(defaultCountValueByType == 1 && value != 1)
    {
        // Näillä "1 lkm" tyypeillä ei saa olla muita arvoja kuin se 1!
        *countValue = defaultCountValueByType;
    }
}

bool NFmiCaseStudySettingsWinRegistry::Init(const std::string& baseRegistryPath, NFmiHelpDataInfoSystem& theHelpDataInfoSystem)
{
    if(mInitialized)
        throw std::runtime_error("NFmiHelpDataDataCountWinRegistry::Init: all ready initialized.");

    mInitialized = true;

    // EnableData section käyttää vanhaa base-polkua
    mHelpDataEnableWinRegistry.Init(baseRegistryPath, theHelpDataInfoSystem);

    mBaseRegistryPath = baseRegistryPath + "\\CaseStudyDlgSettings";
    mSectionNameLocalCacheCount = "\\LocalCache";
    mSectionNameCaseStudyCount = "\\CaseStudy";
    mSectionNameStoreData = "\\StoreData";

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    for(const auto &helpDataInfo : theHelpDataInfoSystem.DynamicHelpDataInfos())
    {
        auto uniqueDataName = helpDataInfo.Name();
        std::string regName("\\");
        regName += uniqueDataName;
        auto dataType = helpDataInfo.DataType();

        // Local cache lukemien asetus
        auto defaultLocalCacheCountValueByType = GetDefaultLocalCacheCountValue(dataType);
        auto tmpLocalCacheCountValue = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionNameLocalCacheCount, regName.c_str(), usedKey, defaultLocalCacheCountValueByType);
        ::CheckAndFixDataCountValue(dataType, tmpLocalCacheCountValue, true);
        mHelpDataLocalCacheCountMap.insert(std::make_pair(uniqueDataName, std::make_pair(dataType, tmpLocalCacheCountValue)));

        // CaseStudy lukemien asetus
        auto defaultCaseStudyCountValueByType = GetDefaultCaseStudyCountValue(dataType);
        auto tmpCaseStudyCountValue = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionNameCaseStudyCount, regName.c_str(), usedKey, defaultCaseStudyCountValueByType);
        ::CheckAndFixDataCountValue(dataType, tmpCaseStudyCountValue, false);
        mHelpDataCaseStudyCountMap.insert(std::make_pair(uniqueDataName, std::make_pair(dataType, tmpCaseStudyCountValue)));

        // Store data option asetus
        auto tmpCaseStudyStoreDataState = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, mSectionNameStoreData, regName.c_str(), usedKey, true);
        mCaseStudyStoreDataMap.insert(std::make_pair(uniqueDataName, tmpCaseStudyStoreDataState));
    }
    return true;
}

int NFmiCaseStudySettingsWinRegistry::GetHelpDataLocalCacheCount(const std::string& uniqueDataName) const
{
    auto iter = mHelpDataLocalCacheCountMap.find(uniqueDataName);
    if(iter != mHelpDataLocalCacheCountMap.end())
        return *(iter->second.second);
    else
        return -1; // negatiivinen arvo on virheen merkki
}

void NFmiCaseStudySettingsWinRegistry::SetHelpDataLocalCacheCount(const std::string& uniqueDataName, int newValue)
{
    auto iter = mHelpDataLocalCacheCountMap.find(uniqueDataName);
    if(iter != mHelpDataLocalCacheCountMap.end())
    {
        auto& countValue = iter->second.second;
        *countValue = newValue;
        ::CheckAndFixDataCountValue(iter->second.first, countValue, true);
    }
}

int NFmiCaseStudySettingsWinRegistry::GetHelpDataCaseStudyCount(const std::string& uniqueDataName) const
{
    auto iter = mHelpDataCaseStudyCountMap.find(uniqueDataName);
    if(iter != mHelpDataCaseStudyCountMap.end())
        return *(iter->second.second);
    else
        return -1; // negatiivinen arvo on virheen merkki
}

void NFmiCaseStudySettingsWinRegistry::SetHelpDataCaseStudyCount(const std::string& uniqueDataName, int newValue)
{
    auto iter = mHelpDataCaseStudyCountMap.find(uniqueDataName);
    if(iter != mHelpDataCaseStudyCountMap.end())
    {
        auto& countValue = iter->second.second;
        *countValue = newValue;
        ::CheckAndFixDataCountValue(iter->second.first, countValue, false);
    }
}

bool NFmiCaseStudySettingsWinRegistry::GetStoreDataState(const std::string& uniqueDataName) const
{
    auto iter = mCaseStudyStoreDataMap.find(uniqueDataName);
    if(iter != mCaseStudyStoreDataMap.end())
        return *iter->second;
    else
        return false; // Palautetaan false virhetiilanteissa

}

void NFmiCaseStudySettingsWinRegistry::SetStoreDataState(const std::string& uniqueDataName, bool newState)
{
    auto iter = mCaseStudyStoreDataMap.find(uniqueDataName);
    if(iter != mCaseStudyStoreDataMap.end())
        *iter->second = newState;
}

// ************************************************
// *********   NFmiMapViewWinRegistry *************
// ************************************************

NFmiMapViewWinRegistry::NFmiMapViewWinRegistry()
:mInitialized(false)
,mBaseRegistryPath()
,mSectionName()
,mMapIndex(-1)
{
}

bool NFmiMapViewWinRegistry::Init(const std::string &baseRegistryPath, int mapIndex)
{
    if(mInitialized)
        throw std::runtime_error("NFmiMapViewWinRegistry::Init: all ready initialized.");

    mInitialized = true;
    mBaseRegistryPath = baseRegistryPath;
    mMapIndex = mapIndex;
    // HUom! mMapIndex on 0-pohjainen indeksi, mutta siitä tehdään mMapIndex+1 -pohjainen, kun tehdään mMapSectionName:a, eli rekisterissä MapView-nimet alkavat 1:stä
    mSectionName = std::string("\\MapView") + NFmiStringTools::Convert(mMapIndex+1);

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    mShowMasksOnMap = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, mSectionName, "\\ShowMasksOnMap", usedKey, true); // tälle ei ole ollut koskaan NFmiSettings:eissä asetusta
    std::string mapViewBaseSettingsKey = std::string("MetEditor::MapView::") + NFmiStringTools::Convert(mMapIndex+1) + "::";
    mSpacingOutFactor = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionName, "\\SpacingOutFactor", usedKey, 1, std::string(mapViewBaseSettingsKey + "SpacingOutFactor").c_str());
    mSelectedMapIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionName, "\\SelectedMapIndex", usedKey, 1, std::string(mapViewBaseSettingsKey + "SelectedMap").c_str());
    mShowStationPlot = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, mSectionName, "\\ShowStationPlot", usedKey, false, std::string(mapViewBaseSettingsKey + "ShowStationPlot").c_str());
    mViewGridSizeStr = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\ViewGridSize", usedKey, "1,1", std::string(mapViewBaseSettingsKey + "ViewGridSize").c_str());
    mCombinedMapModeSelectedBackgroundIndicesStr = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\CombinedMapModeSelectedBackgroundIndices", usedKey, "4:0,0,0,0");
    mCombinedMapModeSelectedOverlayIndicesStr = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\CombinedMapModeSelectedOverlayIndices", usedKey, "4:0,0,0,0");

    return true;
}

bool NFmiMapViewWinRegistry::ShowMasksOnMap() const
{
    return *mShowMasksOnMap;
}

void NFmiMapViewWinRegistry::ShowMasksOnMap(bool newValue)
{
    *mShowMasksOnMap = newValue;
}

int NFmiMapViewWinRegistry::SpacingOutFactor() const
{
    return *mSpacingOutFactor;
}

void NFmiMapViewWinRegistry::SpacingOutFactor(int newValue)
{
    *mSpacingOutFactor = newValue;
}

void NFmiMapViewWinRegistry::ToggleSpacingOutFactor()
{
    int spacingOutFactor = SpacingOutFactor();
    spacingOutFactor++;
	if(spacingOutFactor > 2)
		spacingOutFactor = 0;
    SpacingOutFactor(spacingOutFactor);
}

unsigned int NFmiMapViewWinRegistry::SelectedMapIndex() const
{
    return *mSelectedMapIndex;
}

void NFmiMapViewWinRegistry::SelectedMapIndex(unsigned int newValue)
{
    *mSelectedMapIndex = newValue;
}

bool NFmiMapViewWinRegistry::ShowStationPlot() const
{
    return *mShowStationPlot;
}

void NFmiMapViewWinRegistry::ShowStationPlot(bool newValue)
{
    *mShowStationPlot = newValue;
}

std::string NFmiMapViewWinRegistry::ViewGridSizeStr() const
{
    return *mViewGridSizeStr;
}

void NFmiMapViewWinRegistry::ViewGridSizeStr(const std::string &newValue)
{
    *mViewGridSizeStr = newValue;
}

std::string NFmiMapViewWinRegistry::CombinedMapModeSelectedBackgroundIndices() const
{
    return *mCombinedMapModeSelectedBackgroundIndicesStr;
}

void NFmiMapViewWinRegistry::CombinedMapModeSelectedBackgroundIndices(const std::string& newValue)
{
    *mCombinedMapModeSelectedBackgroundIndicesStr = newValue;
}

std::string NFmiMapViewWinRegistry::CombinedMapModeSelectedOverlayIndices() const
{
    return *mCombinedMapModeSelectedOverlayIndicesStr;
}

void NFmiMapViewWinRegistry::CombinedMapModeSelectedOverlayIndices(const std::string& newValue)
{
    *mCombinedMapModeSelectedOverlayIndicesStr = newValue;
}

// ******************************************************
// ********   NFmiCrossSectionViewWinRegistry ***********
// ******************************************************

NFmiCrossSectionViewWinRegistry::NFmiCrossSectionViewWinRegistry()
:mInitialized(false)
,mBaseRegistryPath()
,mSectionName("\\CrossSection")
,mStartPointStr()
,mMiddlePointStr()
,mEndPointStr()
,mHorizontalPointCount()
,mVerticalPointCount()
,mAxisValuesDefaultLowerEndValue()
,mAxisValuesDefaultUpperEndValue()
,mAxisValuesSpecialLowerEndValue()
,mAxisValuesSpecialUpperEndValue()
{
}

bool NFmiCrossSectionViewWinRegistry::Init(const std::string &baseRegistryPath)
{
    if(mInitialized)
        throw std::runtime_error("NFmiCrossSectionViewWinRegistry::Init: all ready initialized.");

    mInitialized = true;
    mBaseRegistryPath = baseRegistryPath;

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    mStartPointStr = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\StartPoint", usedKey, "20.0 60.0", "MetEditor::CrossSection::StartPoint");
    mMiddlePointStr = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\MiddlePoint", usedKey, "21.0 63.0", "MetEditor::CrossSection::MiddlePoint");
    mEndPointStr = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, "\\EndPoint", usedKey, "22.0 65.0", "MetEditor::CrossSection::EndPoint");
    mHorizontalPointCount = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionName, "\\HorizontalPointCount", usedKey, 50); // tälle ei ole ollut koskaan NFmiSettings:eissä asetusta
    mVerticalPointCount = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, mSectionName, "\\VerticalPointCount", usedKey, 50); // tälle ei ole ollut koskaan NFmiSettings:eissä asetusta
    mAxisValuesDefaultLowerEndValue = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, mSectionName, "\\DefaultLowerEnd", usedKey, 1015, "MetEditor::CrossSection::DefaultAxis::LowerEndValue");
    mAxisValuesDefaultUpperEndValue = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, mSectionName, "\\DefaultUpperEnd", usedKey, 150, "MetEditor::CrossSection::DefaultAxis::UpperEndValue");
    mAxisValuesSpecialLowerEndValue = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, mSectionName, "\\SpecialLowerEnd", usedKey, 1020, "MetEditor::CrossSection::SpecialAxis::LowerEndValue");
    mAxisValuesSpecialUpperEndValue = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, mSectionName, "\\SpecialUpperEnd", usedKey, 925, "MetEditor::CrossSection::SpecialAxis::UpperEndValue");

    return true;
}

std::string NFmiCrossSectionViewWinRegistry::StartPointStr()
{
    return *mStartPointStr;
}

void NFmiCrossSectionViewWinRegistry::StartPointStr(const std::string &newValue)
{
    *mStartPointStr = newValue;
}

std::string NFmiCrossSectionViewWinRegistry::MiddlePointStr()
{
    return *mMiddlePointStr;
}

void NFmiCrossSectionViewWinRegistry::MiddlePointStr(const std::string &newValue)
{
    *mMiddlePointStr = newValue;
}

std::string NFmiCrossSectionViewWinRegistry::EndPointStr()
{
    return *mEndPointStr;
}

void NFmiCrossSectionViewWinRegistry::EndPointStr(const std::string &newValue)
{
    *mEndPointStr = newValue;
}

int NFmiCrossSectionViewWinRegistry::HorizontalPointCount()
{
    return *mHorizontalPointCount;
}

void NFmiCrossSectionViewWinRegistry::HorizontalPointCount(int newValue)
{
    *mHorizontalPointCount = newValue;
}

int NFmiCrossSectionViewWinRegistry::VerticalPointCount()
{
    return *mVerticalPointCount;
}

void NFmiCrossSectionViewWinRegistry::VerticalPointCount(int newValue)
{
    *mVerticalPointCount = newValue;
}

double NFmiCrossSectionViewWinRegistry::AxisValuesDefaultLowerEndValue()
{
    return *mAxisValuesDefaultLowerEndValue;
}
void NFmiCrossSectionViewWinRegistry::AxisValuesDefaultLowerEndValue(double newValue)
{
    *mAxisValuesDefaultLowerEndValue = newValue;
}
double NFmiCrossSectionViewWinRegistry::AxisValuesDefaultUpperEndValue()
{
    return *mAxisValuesDefaultUpperEndValue;
}
void NFmiCrossSectionViewWinRegistry::AxisValuesDefaultUpperEndValue(double newValue)
{
    *mAxisValuesDefaultUpperEndValue = newValue;
}
double NFmiCrossSectionViewWinRegistry::AxisValuesSpecialLowerEndValue()
{
    return *mAxisValuesSpecialLowerEndValue;
}
void NFmiCrossSectionViewWinRegistry::AxisValuesSpecialLowerEndValue(double newValue)
{
    *mAxisValuesSpecialLowerEndValue = newValue;
}
double NFmiCrossSectionViewWinRegistry::AxisValuesSpecialUpperEndValue()
{
    return *mAxisValuesSpecialUpperEndValue;
}
void NFmiCrossSectionViewWinRegistry::AxisValuesSpecialUpperEndValue(double newValue)
{
    *mAxisValuesSpecialUpperEndValue = newValue;
}


// ******************************************************
// *********   NFmiViewPositionsWinRegistry *************
// ******************************************************

NFmiViewPositionsWinRegistry::NFmiViewPositionsWinRegistry()
:mInitialized(false)
,mBaseRegistryPath()
,mSectionName()
,mWindowRectStringMap()
{
}

bool NFmiViewPositionsWinRegistry::Init(const std::string &baseRegistryPath, std::map<std::string, std::string> &windowPosMap)
{
    if(mInitialized)
        throw std::runtime_error("NFmiViewPositionsWinRegistry::Init: all ready initialized.");

    mInitialized = true;
    mBaseRegistryPath = baseRegistryPath;
    mSectionName = "\\ViewPositions";

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    for(std::map<std::string, std::string>::iterator it = windowPosMap.begin(); it != windowPosMap.end(); ++it)
    {
        boost::shared_ptr<CachedRegString> windowRectStr = ::CreateRegValue<CachedRegString>(mBaseRegistryPath, mSectionName, it->first, usedKey, it->second);
        if(windowRectStr)
            mWindowRectStringMap.insert(std::make_pair(it->first, windowRectStr));
    }

    return true;
}

std::string NFmiViewPositionsWinRegistry::WindowRectStr(const std::string &keyString)
{
    std::string str;
    WindowRectStringMap::iterator it = mWindowRectStringMap.find(keyString);
    if(it != mWindowRectStringMap.end())
        str = *(it->second);
    return str;
}

void NFmiViewPositionsWinRegistry::WindowRectStr(const std::string &keyString, const std::string &value)
{
    WindowRectStringMap::iterator it = mWindowRectStringMap.find(keyString);
    if(it != mWindowRectStringMap.end())
        *(it->second) = value;
}

boost::shared_ptr<CachedRegString> NFmiViewPositionsWinRegistry::CachedWindowRectStr(const std::string &keyString)
{
    WindowRectStringMap::iterator it = mWindowRectStringMap.find(keyString);
    if(it != mWindowRectStringMap.end())
        return it->second;
    else
        return boost::shared_ptr<CachedRegString>();
}

// *********************************************************
// *******   NFmiConfigurationRelatedWinRegistry ***********
// *********************************************************

NFmiConfigurationRelatedWinRegistry::NFmiConfigurationRelatedWinRegistry()
:mInitialized(false)
,mBaseConfigurationRegistryPath()
,mMapViewVector()
,mMapViewPositionsWinRegistry()
,mLoadDataAtStartUp()
,mAutoLoadNewCacheData()
,mShowLastSendTimeOnMapView()
,mAddHelpDataIdAtSendindDataToDatabase()
,mMacroParamGridSizeX()
,mMacroParamGridSizeY()
,mLogLevel()
,mLogViewerLogLevel()
,mLogViewerCategory()
,mDroppedDataEditable()
,mUseCombinedMapMode()
{
}

bool NFmiConfigurationRelatedWinRegistry::Init(const std::string &baseConfigurationRegistryPath, int mapViewCount, std::map<std::string, std::string> &mapWindowPosMap)
{
    mBaseConfigurationRegistryPath = baseConfigurationRegistryPath;
    if(mapViewCount < 3 || mapViewCount > 10)
        throw std::runtime_error(std::string("NFmiConfigurationRelatedWinRegistry::Init: odd mapViewCount given ") + NFmiStringTools::Convert(mapViewCount) + ", stopping Windows registry init...");

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    // MapView section
    for(int i = 0; i < mapViewCount; i++)
    {
        boost::shared_ptr<NFmiMapViewWinRegistry> mapViewRegistry(new NFmiMapViewWinRegistry());
        if(mapViewRegistry && mapViewRegistry->Init(mBaseConfigurationRegistryPath, i))
            mMapViewVector.push_back(mapViewRegistry);
    }

    // MapViewPosition section
    mMapViewPositionsWinRegistry.Init(mBaseConfigurationRegistryPath, mapWindowPosMap);

    // CrossSection section
    mCrossSectionViewWinRegistry.Init(mBaseConfigurationRegistryPath);

    // configuration related General section
    std::string sectionName = "\\General";

    mLoadDataAtStartUp = ::CreateRegValue<CachedRegBool>(mBaseConfigurationRegistryPath, sectionName, "\\LoadDataAtStartUp", usedKey, true, "MetEditor::DoAutoLoadDataAtStartUp");
    mAutoLoadNewCacheData = ::CreateRegValue<CachedRegBool>(mBaseConfigurationRegistryPath, sectionName, "\\AutoLoadNewCacheData", usedKey, true, "SmartMet::GeneralOptions::AutoLoadNewCacheData");
    mShowLastSendTimeOnMapView = ::CreateRegValue<CachedRegBool>(mBaseConfigurationRegistryPath, sectionName, "\\ShowLastSendTimeOnMapView", usedKey, false);
    mAddHelpDataIdAtSendindDataToDatabase = ::CreateRegValue<CachedRegBool>(mBaseConfigurationRegistryPath, sectionName, "\\AddHelpDataIdAtSendindDataToDatabase", usedKey, false, "SmartMet::AddHelpDataIdAtSendindDataToDatabase");
    mMacroParamGridSizeX = ::CreateRegValue<CachedRegInt>(mBaseConfigurationRegistryPath, sectionName, "\\MacroParamGridSizeX", usedKey, 70);
    mMacroParamGridSizeY = ::CreateRegValue<CachedRegInt>(mBaseConfigurationRegistryPath, sectionName, "\\MacroParamGridSizeY", usedKey, 70);
    mLogLevel = ::CreateRegValue<CachedRegInt>(mBaseConfigurationRegistryPath, sectionName, "\\LogLevel", usedKey, static_cast<int>(CatLog::Severity::Debug));
    mLogViewerLogLevel = ::CreateRegValue<CachedRegInt>(mBaseConfigurationRegistryPath, sectionName, "\\LogViewerLogLevel", usedKey, static_cast<int>(CatLog::Severity::Debug));
    mLogViewerCategory = ::CreateRegValue<CachedRegInt>(mBaseConfigurationRegistryPath, sectionName, "\\LogViewerCategory", usedKey, static_cast<int>(CatLog::Category::NoCategory));
    mDroppedDataEditable = ::CreateRegValue<CachedRegBool>(mBaseConfigurationRegistryPath, sectionName, "\\DroppedDataEditable", usedKey, false);
    mUseCombinedMapMode = ::CreateRegValue<CachedRegBool>(mBaseConfigurationRegistryPath, sectionName, "\\UseCombinedMapMode", usedKey, false);

    return true;
}

static boost::shared_ptr<NFmiMapViewWinRegistry> MakeNonMapViewWinRegistryObject()
{
    boost::shared_ptr<NFmiMapViewWinRegistry> dummyMapViewWinRegistry(new NFmiMapViewWinRegistry());
    // 998:sta tulee oikeasti 999 Init metodin sisällä (999 on tässä selkeämmin spesiaali indeksi, joka tarkoittaa muita kuin oikeiden karttanäyttöjen indeksiä)
    dummyMapViewWinRegistry->Init("Software\\Fmi\\SmartMet\\General", 998); 
    return dummyMapViewWinRegistry;
}

boost::shared_ptr<NFmiMapViewWinRegistry> NFmiConfigurationRelatedWinRegistry::MapView(int mapIndex)
{
    if(mapIndex >= 0 && mapIndex < static_cast<int>(mMapViewVector.size()))
        return mMapViewVector[mapIndex];
    else
    {
        // Palautetaan ei-oikeille karttanäytöille (aika-sarja, poikkileikkaus, jne.) dummy-olion asetukset, jotta ei tarvitse testailla kaikkialla nullptr juttuja
        static boost::shared_ptr<NFmiMapViewWinRegistry> dummyMapViewWinRegistry = ::MakeNonMapViewWinRegistryObject();
        return dummyMapViewWinRegistry;
    }
}

std::string NFmiConfigurationRelatedWinRegistry::WindowRectStr(const std::string &keyString)
{
    return mMapViewPositionsWinRegistry.WindowRectStr(keyString);
}

void NFmiConfigurationRelatedWinRegistry::WindowRectStr(const std::string &keyString, const std::string &value)
{
    boost::shared_ptr<CachedRegString> cachedRectStr = mMapViewPositionsWinRegistry.CachedWindowRectStr(keyString);
    if(cachedRectStr)
        *cachedRectStr = value;
}

bool NFmiConfigurationRelatedWinRegistry::LoadDataAtStartUp() const
{
    return *mLoadDataAtStartUp;
}

void NFmiConfigurationRelatedWinRegistry::LoadDataAtStartUp(bool newValue)
{
    *mLoadDataAtStartUp = newValue;
}

bool NFmiConfigurationRelatedWinRegistry::AutoLoadNewCacheData() const
{
    return *mAutoLoadNewCacheData;
}

void NFmiConfigurationRelatedWinRegistry::AutoLoadNewCacheData(bool newValue)
{
    *mAutoLoadNewCacheData = newValue;
}

bool NFmiConfigurationRelatedWinRegistry::ShowLastSendTimeOnMapView() const
{
    return *mShowLastSendTimeOnMapView;
}

void NFmiConfigurationRelatedWinRegistry::ShowLastSendTimeOnMapView(bool newValue)
{
    *mShowLastSendTimeOnMapView = newValue;
}

bool NFmiConfigurationRelatedWinRegistry::AddHelpDataIdAtSendindDataToDatabase()
{
    return *mAddHelpDataIdAtSendindDataToDatabase;
}

void NFmiConfigurationRelatedWinRegistry::AddHelpDataIdAtSendindDataToDatabase(bool newValue)
{
    *mAddHelpDataIdAtSendindDataToDatabase = newValue;
}

int NFmiConfigurationRelatedWinRegistry::MacroParamGridSizeX()
{
    return *mMacroParamGridSizeX;
}

void NFmiConfigurationRelatedWinRegistry::MacroParamGridSizeX(int newValue)
{
    *mMacroParamGridSizeX = newValue;
}

int NFmiConfigurationRelatedWinRegistry::MacroParamGridSizeY()
{
    return *mMacroParamGridSizeY;
}

void NFmiConfigurationRelatedWinRegistry::MacroParamGridSizeY(int newValue)
{
    *mMacroParamGridSizeY = newValue;
}

int NFmiConfigurationRelatedWinRegistry::LogLevel()
{
    return *mLogLevel;
}

void NFmiConfigurationRelatedWinRegistry::LogLevel(int logLevel)
{
    *mLogLevel = logLevel;
}

int NFmiConfigurationRelatedWinRegistry::LogViewerLogLevel()
{
    return *mLogViewerLogLevel;
}

void NFmiConfigurationRelatedWinRegistry::LogViewerLogLevel(int newValue)
{
    *mLogViewerLogLevel = newValue;
}

int NFmiConfigurationRelatedWinRegistry::LogViewerCategory()
{
    return *mLogViewerCategory;
}

void NFmiConfigurationRelatedWinRegistry::LogViewerCategory(int newValue)
{
    *mLogViewerCategory = newValue;
}

bool NFmiConfigurationRelatedWinRegistry::DroppedDataEditable()
{
    return *mDroppedDataEditable;
}

void NFmiConfigurationRelatedWinRegistry::DroppedDataEditable(bool newValue)
{
    *mDroppedDataEditable = newValue;
}

bool NFmiConfigurationRelatedWinRegistry::UseCombinedMapMode()
{
    return *mUseCombinedMapMode;
}

void NFmiConfigurationRelatedWinRegistry::UseCombinedMapMode(bool newValue)
{
    *mUseCombinedMapMode = newValue;
}

// ************************************************
// *******   NFmiApplicationWinRegistry ***********
// ************************************************

std::string NFmiApplicationWinRegistry::MakeBaseRegistryPath()
{
    return std::string("Software\\Fmi\\SmartMet");
}

std::string NFmiApplicationWinRegistry::MakeGeneralSectionName()
{
    return std::string("\\General");
}

NFmiApplicationWinRegistry::NFmiApplicationWinRegistry()
:mInitialized(false)
,mBaseRegistryPath()
,mBaseRegistryWithVersionPath()
,mBaseConfigurationRegistryPath()
,mConfigurationName()
,mConfigurationRelatedWinRegistry()
,mOtherViewPositionsWinRegistry()
,mDataNotificationSettingsWinRegistry()
,mUseTimeSerialAxisAutoAdjust()
,mSoundingTextUpward()
,mSoundingTimeLockWithMapView()
,mShowStabilityIndexSideView()
,mShowTextualSoundingDataSideView()
,mKeepMapAspectRatio()
,mFitToPagePrint()
,mSmartOrientationPrint()
,mLowMemoryPrint()
,mMaxRangeInPrint()
,mUseMultiProcessCpCalc()
,mAllowRightClickDisplaySelection()
,mFixedDrawParamsPath()
,mLocationFinderThreadTimeOutInMS()
,mShowHakeMessages()
,mShowKaHaMessages()
,mMinimumTimeRangeForWarningsOnMapViewsInMinutes()
,mDrawObjectScaleFactor()
,itsMaximumFontSizeFactor(3.)
,mIsolineMinLengthFactor()
,mGenerateTimeCombinationData()
,mEditingToolsGriddingProperties()
,mVisualizationGriddingProperties()
,mSaveImageExtensionFilterIndex()
,mMapViewCacheMaxSizeInMB()
,mShowTooltipOnSmarttoolDialog()
,mHatchingToolmasterEpsilonFactor()
{
}

// fullAppVer esim. 5.9.3.0
// shortAppVer esim. 5.9
// configurationName se loppu osa konffista ilman mitään alku polkuja esim. control_scand_saa2_edit_conf
bool NFmiApplicationWinRegistry::Init(const std::string &fullAppVer, const std::string &shortAppVer, const std::string &configurationName, int mapViewCount, std::map<std::string, std::string> &mapWindowPosMap, std::map<std::string, std::string> &otherWindowPosMap, NFmiHelpDataInfoSystem &theHelpDataInfoSystem)
{
    if(mInitialized)
        throw std::runtime_error("NFmiApplicationWinRegistry::Init: already initialized.");

    mInitialized = true;
    mConfigurationName = configurationName;
    mBaseRegistryPath = NFmiApplicationWinRegistry::MakeBaseRegistryPath();
    mBaseRegistryWithVersionPath = mBaseRegistryPath + "\\" + shortAppVer;
    mBaseConfigurationRegistryPath = mBaseRegistryPath + "\\" + mConfigurationName;

    // HKEY_CURRENT_USER -keys
    HKEY usedKey = HKEY_CURRENT_USER;

    // Configuration related section
    mConfigurationRelatedWinRegistry.Init(mBaseConfigurationRegistryPath, mapViewCount, mapWindowPosMap);

    // OtherViewPosition section
    mOtherViewPositionsWinRegistry.Init(mBaseRegistryPath, otherWindowPosMap);

    // DataNotifications section
    mDataNotificationSettingsWinRegistry.Init(mBaseRegistryPath, fullAppVer);

    mCaseStudySettingsWinRegistry.Init(mBaseRegistryPath, theHelpDataInfoSystem);

    // General section
    std::string sectionName = NFmiApplicationWinRegistry::MakeGeneralSectionName();

    mUseTimeSerialAxisAutoAdjust = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\UseTimeSerialAxisAutoAdjust", usedKey, true, "SmartMet::UseTimeSerialAxisAutoAdjust");
    mSoundingTextUpward = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\SoundingTextUpward", usedKey, true);
    mSoundingTimeLockWithMapView = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\SoundingTimeLockWithMapView", usedKey, false);
    mShowStabilityIndexSideView = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\ShowStabilityIndexSideView", usedKey, false);
    mShowTextualSoundingDataSideView = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\ShowTextualSoundingDataSideView", usedKey, false);

    mKeepMapAspectRatio = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\KeepMapAspectRatio", usedKey, false, "SmartMet::GeneralOptions::KeepMapAspectRatio");

    mFitToPagePrint = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\FitToPagePrint", usedKey, true, "MetEditor::FitToPagePrint");
    mSmartOrientationPrint = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\SmartOrientationPrint", usedKey, true, "MetEditor::SmartOrientationPrint");
    mLowMemoryPrint = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\LowMemoryPrint", usedKey, true, "MetEditor::LowMemoryPrint");
    mMaxRangeInPrint = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, sectionName, "\\MaxRangeInPrint", usedKey, 5, "MetEditor::MaxRangeInPrint");
    mUseMultiProcessCpCalc = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\UseMultiProcessCpCalc", usedKey, false);
    mAllowRightClickDisplaySelection = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\AllowRightClickDisplaySelection", usedKey, false);
    mFixedDrawParamsPath = NFmiSettings::Optional<std::string>("SmartMet::FixedDrawParamsPath", "FixedDrawParams");
    mLocationFinderThreadTimeOutInMS = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, sectionName, "\\LocationFinderThreadTimeOutInMS", usedKey, 1500, "SmartMet::LocationFinderThreadTimeOutInMS");
    mShowHakeMessages = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\ShowHakeMessages", usedKey, true);
    mShowKaHaMessages = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\ShowKaHaMessages", usedKey, false);
    mMinimumTimeRangeForWarningsOnMapViewsInMinutes = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, sectionName, "\\MinimumTimeRangeForWarningsOnMapViewsInMinutes", usedKey, 0);
    mDrawObjectScaleFactor = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, sectionName, "\\DrawObjectScaleFactor", usedKey, 0.9, "MetEditor::DrawObjectScaleFactor");
    //mMaximumFontSizeFactor = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, sectionName, "\\MaximumFontSizeFactor", usedKey, 2);
    mIsolineMinLengthFactor = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, sectionName, "\\IsolineMinLengthFactor", usedKey, 1.);
    mGenerateTimeCombinationData = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\GenerateTimeCombinationData", usedKey, true);
    mSaveImageExtensionFilterIndex = ::CreateRegValue<CachedRegInt>(mBaseRegistryPath, sectionName, "\\SaveImageExtensionFilterIndex", usedKey, 1);
    // Pitää vielä varmistaa että rekisteristä luettu indeksi menee rajojen sisälle oikein, siksi kutsutaan vielä sen Set-metodia, joka tekee tarkasteluja.
    SetSaveImageExtensionFilterIndex(*mSaveImageExtensionFilterIndex);
    mMapViewCacheMaxSizeInMB = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, sectionName, "\\MapViewCacheMaxSizeInMB", usedKey, 2000);
    // En tiedä onko oikeasti väliä, jos luku olisi vahingossa esim. negatiivinen, joten laitetaan alustettu arvo varmuuden vuoksi setterin läpi, joka tekee tarkistuksia.
    MapViewCacheMaxSizeInMB(*mMapViewCacheMaxSizeInMB);
    mShowTooltipOnSmarttoolDialog = ::CreateRegValue<CachedRegBool>(mBaseRegistryPath, sectionName, "\\ShowTooltipOnSmarttoolDialog", usedKey, true);
    mHatchingToolmasterEpsilonFactor = ::CreateRegValue<CachedRegDouble>(mBaseRegistryPath, sectionName, "\\HatchingToolmasterEpsilonFactor", usedKey, 1.);

    // HKEY_LOCAL_MACHINE -keys (HUOM! nämä vaatii Admin oikeuksia Vista/Win7)
    usedKey = HKEY_LOCAL_MACHINE;

    return true;
}

void NFmiApplicationWinRegistry::InitGriddingProperties(bool isToolMasterAvailable)
{
    std::string sectionName = NFmiApplicationWinRegistry::MakeGeneralSectionName();
    // EditingTools section in general
    mEditingToolsGriddingProperties.Init(mBaseRegistryPath + "\\" + sectionName + "\\EditingTools", isToolMasterAvailable);
    // This Visualization section is not yet in Windows registry
    mVisualizationGriddingProperties.toolMasterAvailable(isToolMasterAvailable);
}

std::string NFmiApplicationWinRegistry::WindowRectStr(const std::string &keyString)
{
//    DebugHelper_TestIfAnyViewsHaveSameRectValues();
    std::string windowRectStr = mOtherViewPositionsWinRegistry.WindowRectStr(keyString);
    if(!windowRectStr.empty())
        return windowRectStr;
    else
        return mConfigurationRelatedWinRegistry.WindowRectStr(keyString);
}

//static bool CheckIfSameValuesExists(std::unordered_map<std::string, std::string> &viewRectStringMapInOut, const NFmiViewPositionsWinRegistry::WindowRectStringMap &stringMap)
//{
//    for(const auto &mapItem : stringMap)
//    {
//        auto rectString = static_cast<std::string>((*(mapItem.second)));
//        if(viewRectStringMapInOut.find(rectString) != viewRectStringMapInOut.end())
//            return true;
//        viewRectStringMapInOut.insert(std::pair<std::string, std::string>(rectString, mapItem.first));
//    }
//    return false;
//}
//
//void NFmiApplicationWinRegistry::DebugHelper_TestIfAnyViewsHaveSameRectValues()
//{
//    // View's Rect value is key and it's registry key is value (so I can see which of the Views have same values)
//    std::unordered_map<std::string, std::string> viewRectStringmap;
//    bool status1 = CheckIfSameValuesExists(viewRectStringmap, mOtherViewPositionsWinRegistry.GetWindowRectStringMap());
//    bool status2 = CheckIfSameValuesExists(viewRectStringmap, mConfigurationRelatedWinRegistry.GetWindowRectStringMap());
//}

void NFmiApplicationWinRegistry::WindowRectStr(const std::string &keyString, const std::string &value)
{
//    DebugHelper_TestIfAnyViewsHaveSameRectValues();
    boost::shared_ptr<CachedRegString> cachedRectStr = mOtherViewPositionsWinRegistry.CachedWindowRectStr(keyString);
    if(cachedRectStr)
        *cachedRectStr = value;
    else
        mConfigurationRelatedWinRegistry.WindowRectStr(keyString, value);
//    DebugHelper_TestIfAnyViewsHaveSameRectValues();
}

bool NFmiApplicationWinRegistry::UseTimeSerialAxisAutoAdjust()
{
    return *mUseTimeSerialAxisAutoAdjust;
}

void NFmiApplicationWinRegistry::UseTimeSerialAxisAutoAdjust(bool newValue)
{
    *mUseTimeSerialAxisAutoAdjust = newValue;
}

bool NFmiApplicationWinRegistry::SoundingTextUpward()
{
    return *mSoundingTextUpward;
}

void NFmiApplicationWinRegistry::SoundingTextUpward(bool newValue)
{
    *mSoundingTextUpward = newValue;
}

bool NFmiApplicationWinRegistry::SoundingTimeLockWithMapView()
{
    return *mSoundingTimeLockWithMapView;
}

void NFmiApplicationWinRegistry::SoundingTimeLockWithMapView(bool newValue)
{
    *mSoundingTimeLockWithMapView = newValue;
}

bool NFmiApplicationWinRegistry::ShowStabilityIndexSideView()
{
    return *mShowStabilityIndexSideView;
}

void NFmiApplicationWinRegistry::ShowStabilityIndexSideView(bool newValue)
{
    *mShowStabilityIndexSideView = newValue;
}

bool NFmiApplicationWinRegistry::ShowTextualSoundingDataSideView()
{
    return *mShowTextualSoundingDataSideView;
}

void NFmiApplicationWinRegistry::ShowTextualSoundingDataSideView(bool newValue)
{
    *mShowTextualSoundingDataSideView = newValue;
}

SoundingViewSettingsFromWindowsRegisty NFmiApplicationWinRegistry::GetSoundingViewSettings() const
{
    return SoundingViewSettingsFromWindowsRegisty(*mSoundingTextUpward, *mSoundingTimeLockWithMapView, *mShowStabilityIndexSideView, *mShowTextualSoundingDataSideView);
}

void NFmiApplicationWinRegistry::SetSoundingViewSettings(const SoundingViewSettingsFromWindowsRegisty& soundingViewSettings)
{
    SoundingTextUpward(soundingViewSettings.SoundingTextUpward());
    SoundingTimeLockWithMapView(soundingViewSettings.SoundingTimeLockWithMapView());
    ShowStabilityIndexSideView(soundingViewSettings.ShowStabilityIndexSideView());
    ShowTextualSoundingDataSideView(soundingViewSettings.ShowTextualSoundingDataSideView());
}

bool NFmiApplicationWinRegistry::KeepMapAspectRatio()
{
    return *mKeepMapAspectRatio;
}

void NFmiApplicationWinRegistry::KeepMapAspectRatio(bool newValue)
{
    *mKeepMapAspectRatio = newValue;
}

bool NFmiApplicationWinRegistry::FitToPagePrint()
{
    return *mFitToPagePrint;
}

void NFmiApplicationWinRegistry::FitToPagePrint(bool newValue)
{
    *mFitToPagePrint = newValue;
}

bool NFmiApplicationWinRegistry::SmartOrientationPrint()
{
    return *mSmartOrientationPrint;
}

void NFmiApplicationWinRegistry::SmartOrientationPrint(bool newValue)
{
    *mSmartOrientationPrint = newValue;
}

bool NFmiApplicationWinRegistry::LowMemoryPrint()
{
    return *mLowMemoryPrint;
}

void NFmiApplicationWinRegistry::LowMemoryPrint(bool newValue)
{
    *mLowMemoryPrint = newValue;
}

int NFmiApplicationWinRegistry::MaxRangeInPrint()
{
    return *mMaxRangeInPrint;
}

void NFmiApplicationWinRegistry::MaxRangeInPrint(int newValue)
{
    *mMaxRangeInPrint = newValue;
}

bool NFmiApplicationWinRegistry::UseMultiProcessCpCalc()
{
    return *mUseMultiProcessCpCalc;
}

void NFmiApplicationWinRegistry::UseMultiProcessCpCalc(bool newValue)
{
    *mUseMultiProcessCpCalc = newValue;
}

bool NFmiApplicationWinRegistry::AllowRightClickDisplaySelection()
{
    return *mAllowRightClickDisplaySelection;
}

void NFmiApplicationWinRegistry::AllowRightClickDisplaySelection(bool newValue)
{
    *mAllowRightClickDisplaySelection = newValue;
}

std::string NFmiApplicationWinRegistry::FixedDrawParamsPath()
{
    return mFixedDrawParamsPath;
}

void NFmiApplicationWinRegistry::FixedDrawParamsPath(const std::string &newValue)
{
    mFixedDrawParamsPath = newValue;
}

int NFmiApplicationWinRegistry::LocationFinderThreadTimeOutInMS()
{
    return *mLocationFinderThreadTimeOutInMS;
}

void NFmiApplicationWinRegistry::LocationFinderThreadTimeOutInMS(int newValue)
{
    *mLocationFinderThreadTimeOutInMS = newValue;
}

bool NFmiApplicationWinRegistry::ShowHakeMessages()
{
    return *mShowHakeMessages;
}

void NFmiApplicationWinRegistry::ShowHakeMessages(bool newValue)
{
    *mShowHakeMessages = newValue;
}

bool NFmiApplicationWinRegistry::ShowKaHaMessages()
{
    return *mShowKaHaMessages;
}

void NFmiApplicationWinRegistry::ShowKaHaMessages(bool newValue)
{
    *mShowKaHaMessages = newValue;
}

int NFmiApplicationWinRegistry::MinimumTimeRangeForWarningsOnMapViewsInMinutes()
{
    return *mMinimumTimeRangeForWarningsOnMapViewsInMinutes;
}

void NFmiApplicationWinRegistry::MinimumTimeRangeForWarningsOnMapViewsInMinutes(int newValue)
{
    *mMinimumTimeRangeForWarningsOnMapViewsInMinutes = newValue;
}

double NFmiApplicationWinRegistry::DrawObjectScaleFactor()
{
    return *mDrawObjectScaleFactor;
}

void NFmiApplicationWinRegistry::DrawObjectScaleFactor(double newValue)
{
    *mDrawObjectScaleFactor = newValue;
}

double NFmiApplicationWinRegistry::MaximumFontSizeFactor()
{
    return itsMaximumFontSizeFactor;
}

void NFmiApplicationWinRegistry::MaximumFontSizeFactor(double newValue)
{
    itsMaximumFontSizeFactor = newValue;
}

double NFmiApplicationWinRegistry::IsolineMinLengthFactor()
{
    return *mIsolineMinLengthFactor;
}

#ifdef min
#undef min
#undef max
#endif

void NFmiApplicationWinRegistry::IsolineMinLengthFactor(double newValue)
{
    // Rajataan käytetty kerroin arvovälille 0-100
    auto usedValue = std::min(100., std::max(0., newValue));
    *mIsolineMinLengthFactor = usedValue;
}

bool NFmiApplicationWinRegistry::GenerateTimeCombinationData()
{
    return *mGenerateTimeCombinationData;
}

void NFmiApplicationWinRegistry::GenerateTimeCombinationData(bool newValue)
{
    *mGenerateTimeCombinationData = newValue;
}

const NFmiGriddingProperties& NFmiApplicationWinRegistry::GriddingProperties(bool getEditingRelatedProperties) const
{
    if(getEditingRelatedProperties)
        return mEditingToolsGriddingProperties.GetGriddingProperties();
    else
        return mVisualizationGriddingProperties;
}

void NFmiApplicationWinRegistry::SetGriddingProperties(bool setEditingRelatedProperties, const NFmiGriddingProperties &griddingProperties)
{
    if(setEditingRelatedProperties)
        mEditingToolsGriddingProperties.Update(griddingProperties);
    else
        mVisualizationGriddingProperties = griddingProperties;
}

int NFmiApplicationWinRegistry::SaveImageExtensionFilterIndex() const
{
    return *mSaveImageExtensionFilterIndex;
}

void NFmiApplicationWinRegistry::SetSaveImageExtensionFilterIndex(int newValue)
{
    if(newValue < 1 || newValue >= mSaveImageFileFilterExtensions.size())
        newValue = 1;
    *mSaveImageExtensionFilterIndex = newValue;
}

const std::vector<std::string>& NFmiApplicationWinRegistry::SaveImageFileFilterExtensions() const
{
    return mSaveImageFileFilterExtensions;
}

const std::string& NFmiApplicationWinRegistry::GetCurrentSaveImageFileFilterExtension() const
{
    size_t usedVectorIndex = *mSaveImageExtensionFilterIndex - 1;
    if(usedVectorIndex < mSaveImageFileFilterExtensions.size())
        return mSaveImageFileFilterExtensions[usedVectorIndex];
    else
        return mSaveImageFileFilterExtensions[0];
}

double NFmiApplicationWinRegistry::MapViewCacheMaxSizeInMB()
{
    return *mMapViewCacheMaxSizeInMB;
}

void NFmiApplicationWinRegistry::MapViewCacheMaxSizeInMB(double newValue)
{
    if(newValue < 0)
    {
        newValue = 0;
    }

    *mMapViewCacheMaxSizeInMB = newValue;
}

bool NFmiApplicationWinRegistry::ShowTooltipOnSmarttoolDialog()
{
    return *mShowTooltipOnSmarttoolDialog;
}

void NFmiApplicationWinRegistry::ShowTooltipOnSmarttoolDialog(bool newValue)
{
    *mShowTooltipOnSmarttoolDialog = newValue;
}

void NFmiApplicationWinRegistry::HatchingToolmasterEpsilonFactor(float newEpsilon)
{
    *mHatchingToolmasterEpsilonFactor = newEpsilon;
}

float NFmiApplicationWinRegistry::HatchingToolmasterEpsilonFactor() const
{
    return static_cast<float>(*mHatchingToolmasterEpsilonFactor);
}
