
#pragma once

#include <vector>
#include <map>
#include "NFmiCachedRegistryValue.h"
#include "NFmiDataNotificationSettingsWinRegistry.h"
#include "NFmiGriddingProperties.h"
#include "NFmiInfoData.h"
#include "NFmiVisualizationSpaceoutSettings.h"
#include "NFmiColor.h"

class NFmiHelpDataInfoSystem;
class SoundingViewSettingsFromWindowsRegisty;
class NFmiCategoryHeaderInitData;

// En osaa laittaa t�ll�ist� muuttujaa luokan muuttujaksi, koska siin� on tyhj� taulukko ja k��nt�j� tekee erin�isi� valituksia sellaisen k�yt�st� luokan dataosana.
#ifndef UNIX
const TCHAR BASED_CODE g_SaveImageFileFilter[] = _TEXT("PNG (*.png)|*.png|JPEG (*.jpg)|*.jpg|BMP (*.bmp)|*.bmp|TIFF (*.tiff)|*.tiff|GIF (*.gif)|*.gif|");
#else
const char g_SaveImageFileFilter[] = "PNG (*.png)|*.png|JPEG (*.jpg)|*.jpg|BMP (*.bmp)|*.bmp|TIFF (*.tiff)|*.tiff|GIF (*.gif)|*.gif|";
#endif

class NFmiMapViewRangeMeterWinRegistry
{
    bool mInitialized = false; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath;
    std::string mSectionName = "\\MapViewRangeMeter";

    bool mModeOn = false;
    boost::shared_ptr<CachedRegInt> mColorIndex;
    static const std::vector<NFmiColor> mColors;
    NFmiPoint mDragStartLatlonPoint = NFmiPoint::gMissingLatlon;
    NFmiPoint mDragEndLatlonPoint = NFmiPoint::gMissingLatlon;
    bool mMouseDragOn = false;
    // Jos lukitus moodi on p��ll�, ei tehd� muutoksia start/end-point asetuksiin eik� MouseDragOn asetukseen
    bool mLockModeOn = false;
public:
    static const int TooltipDelayInMS = 1500;
    NFmiMapViewRangeMeterWinRegistry();
    bool Init(const std::string& baseRegistryPath);

    bool ModeOn() const;
    void ModeOn(bool newValue);
    void ModeOnToggle();
    int ColorIndex() const;
    void ColorIndex(int newValue);
    void ToggleColor();
    const NFmiColor& GetSelectedColor() const;
    static const std::vector<NFmiColor>& Colors() { return mColors; }
    const NFmiPoint& DragStartLatlonPoint() const { return mDragStartLatlonPoint; }
    void DragStartLatlonPoint(const NFmiPoint& newValue) { mDragStartLatlonPoint = newValue; }
    const NFmiPoint& DragEndLatlonPoint() const { return mDragEndLatlonPoint; }
    void DragEndLatlonPoint(const NFmiPoint& newValue) { mDragEndLatlonPoint = newValue; }
    bool MouseDragOn() const { return mMouseDragOn; }
    void MouseDragOn(bool newValue) { mMouseDragOn = newValue; }
    double CalculateStartEndDistanceInMeters() const;
    bool LockModeOn() const { return mLockModeOn; }
    void LockModeOn(bool newValue) { mLockModeOn = newValue; }
    void LockModeOnToggle();
    void MoveStartLatlonPoint(const NFmiPoint& newValue);
};

class NFmiGriddingPropertiesWinRegistry
{
public:
    NFmiGriddingPropertiesWinRegistry();
    bool Init(const std::string &baseRegistryPath, bool isToolMasterAvailable);
    void Update(const NFmiGriddingProperties &griddingProperties);
    const NFmiGriddingProperties& GetGriddingProperties() const;

private:
    void UpdateGriddingPropertiesFromRegistry();

    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath;
    std::string mSectionName; // t�ss� on GriddingProperties

    // HKEY_CURRENT_USER -keys
    boost::shared_ptr<CachedRegInt> mFunction;
    boost::shared_ptr<CachedRegDouble> mRangeLimitInKm;
    boost::shared_ptr<CachedRegInt> mLocalFitMethod;
    boost::shared_ptr<CachedRegDouble> mLocalFitDelta;
    boost::shared_ptr<CachedRegInt> mSmoothLevel;
    boost::shared_ptr<CachedRegDouble> mLocalFitFilterRadius;
    boost::shared_ptr<CachedRegDouble> mLocalFitFilterFactor;

    // T�t� yksinkertaistettua kokooma luokkaa k�ytet��n varsinaisesti SmartMetin griddauskoodeissa
    NFmiGriddingProperties mGriddingProperties;
};

using CaseStudyBoolMap = std::map<std::string, boost::shared_ptr<CachedRegBool>>;

// NFmiHelpDataInfoSystem:iss� olevien kaikkien dynaamisten datojen enable-asetukset Windows rekisteriss�
class NFmiHelpDataEnableWinRegistry
{
public:
    NFmiHelpDataEnableWinRegistry();
    bool Init(const std::string &baseRegistryPath, NFmiHelpDataInfoSystem &theHelpDataInfoSystem);
    bool Update(NFmiHelpDataInfoSystem &theHelpDataInfoSystem);

private:
    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath; // T�M�N pit�� olla ennen muidat dataosia, ett� se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // t�ss� on EnableData

    // HKEY_CURRENT_USER -keys

    CaseStudyBoolMap mHelpDataEnableMap;
};

// Talletetaan jokaista uniikkia datanime� kohden pari, jossa on datan tyyppi ja lukum��r� int arvona.
using CaseStudyCountMap = std::map<std::string, std::pair<NFmiInfoData::Type, boost::shared_ptr<CachedRegInt>>>;
using ModelDataOffsetRangeInHours = std::pair<int, int>;
const ModelDataOffsetRangeInHours gMissingOffsetRangeInHours = std::make_pair(-1, -1);
const ModelDataOffsetRangeInHours gLatestDataOnlyRangeInHours = std::make_pair(0, 0);
const ModelDataOffsetRangeInHours gDefaultModelDataOffsetRangeInHours = std::make_pair(24, 0);

// CaseStudy dialogiin liittyvien asetuksien asetukset Windows rekisteriss�:
// 1) Kuinka monta viimeist� dataa s�ilytet��n lokaali cachessa
//    - Jos kyse mallidatasta, kuin monta viimeist� malliajoa
//    - Jos kyse havainto/analyysi datoista, numero on aina vakio 1
//    - Jos kyse on satel-kuvista, numero on aina vakio 0, koska niit� ei s�ilytet� lokaalissa cachessa
// 2) Kuinka monta viimeist� dataa talletetaan teht�v��n CaseStudy dataan 
//    - Samat perus s��d�t kuin 1:ss�kin
//    - Satel kuville tosin on nyt s��t�, toisin kuin local-cache-countissa
// 3) Store data optio jokaiseen dataan liittyen (talletetaanko data talletettavaan case-study dataan vai ei)
class NFmiCaseStudySettingsWinRegistry
{
public:
    NFmiCaseStudySettingsWinRegistry();
    bool Init(const std::string& baseRegistryPath, NFmiHelpDataInfoSystem& theHelpDataInfoSystem, const std::vector<NFmiCategoryHeaderInitData> &categoryHeaders);

    int GetHelpDataLocalCacheCount(const std::string& uniqueDataName) const;
    void SetHelpDataLocalCacheCount(const std::string& uniqueDataName, int newValue);
    ModelDataOffsetRangeInHours GetHelpDataCaseStudyOffsetRangeInHours(const std::string& uniqueDataName) const;
    void SetHelpDataCaseStudyOffsetRangeInHours(const std::string& uniqueDataName, const ModelDataOffsetRangeInHours &offsetRangeInHours);
    bool GetStoreDataState(const std::string& uniqueDataName) const;
    void SetStoreDataState(const std::string& uniqueDataName, bool newState);
    static int GetDefaultCaseStudyCountValue(NFmiInfoData::Type dataType);
    static int GetDefaultLocalCacheCountValue(NFmiInfoData::Type dataType);
    NFmiHelpDataEnableWinRegistry& HelpDataEnableWinRegistry() { return mHelpDataEnableWinRegistry; }
    CaseStudyCountMap& GetHelpDataLocalCacheCountMap() { return mHelpDataLocalCacheCountMap; }
    CaseStudyCountMap& GetHelpDataCaseStudyOffset1Map() { return mHelpDataCaseStudyOffset1Map; }
    CaseStudyCountMap& GetHelpDataCaseStudyOffset2Map() { return mHelpDataCaseStudyOffset2Map; }
    CaseStudyBoolMap& GetCaseStudyStoreDataMap() { return  mCaseStudyStoreDataMap; }

private:
    void InitHelpDataRelatedWinRegValues(const std::string& uniqueDataName, NFmiInfoData::Type dataType);

    std::string mBaseRegistryPath;
    std::string mSectionNameLocalCacheCount; // t�ss� on LocalCacheCount
    std::string mSectionNameCaseStudyOffset1; // t�ss� on CaseStudyOffset1
    std::string mSectionNameCaseStudyOffset2; // t�ss� on CaseStudyOffset2
    std::string mSectionNameStoreData; // t�ss� on StoreData

    // Kaikkien queryData konffien EnableData -osio
    NFmiHelpDataEnableWinRegistry mHelpDataEnableWinRegistry;

    // HKEY_CURRENT_USER -keys

    CaseStudyCountMap mHelpDataLocalCacheCountMap;
    // Muutetaan talletettavien datojen lukum��r� kahdeksi indeksiksi, joiden v�liin j��v�t malliajot talletetaan.
    // CaseStudy-dialogissa molemmat luvut annetaan yhdest� grid-controllin cell:ista.
    // Jos cell:in string arvo on vain yksi luku (esim. "3"), t�ll�in talletetaan kaikki malliajot 3-1 (3. tuoreimmasta 1. tuoreimpaan).
    // Muuten cell:in string arvossa on kaksi lukua eroteltuna '-' merkill� ,esim. "4-3". T�ll�in talletetaan 4. tuorein ja 3. tuorein, mutta ei 2. ja 1. tuoreimpia.
    // N�in voi tallettaa CaseStudyn j�lkik�teen vaikka eilisen datoilla niin ett� uusimpia datoja ei tarvitse sulloa v�kisin datapakettiin.

    // T�ss� on siis aloitus offset, joka kertoo talletusikkunan alkuhetken offsetin sein�kelloajasta.
    CaseStudyCountMap mHelpDataCaseStudyOffset1Map;
    // T�ss� on siis lopetus offset, joka kertoo talletusikkunan loppuhetken offsetin sein�kelloajasta.
    CaseStudyCountMap mHelpDataCaseStudyOffset2Map;
    CaseStudyBoolMap mCaseStudyStoreDataMap;
    bool mInitialized = false; // ei sallita tupla initialisointia
};

// Yksitt�isten karttan�ytt�jen asetuksia Windows rekisteriss�
class NFmiMapViewWinRegistry
{
public:
    NFmiMapViewWinRegistry();
    bool Init(const std::string &baseRegistryPath, int mapIndex);

    bool ShowMasksOnMap() const;
    void ShowMasksOnMap(bool newValue);
    int SpacingOutFactor() const;
    void SpacingOutFactor(int newValue);
    void ToggleSpacingOutFactor();
    unsigned int SelectedMapIndex() const;
    void SelectedMapIndex(unsigned int newValue);
	bool ShowStationPlot() const;
	void ShowStationPlot(bool newValue);
	std::string ViewGridSizeStr() const;
	void ViewGridSizeStr(const std::string &newValue);
    std::string CombinedMapModeSelectedBackgroundIndices() const;
    void CombinedMapModeSelectedBackgroundIndices(const std::string &newValue);
    std::string CombinedMapModeSelectedOverlayIndices() const;
    void CombinedMapModeSelectedOverlayIndices(const std::string& newValue);
    FmiDirection TimeBoxLocation();
    void TimeBoxLocation(FmiDirection newValue);
    float TimeBoxTextSizeFactor();
    void TimeBoxTextSizeFactor(float newValue);
    NFmiColor TimeBoxFillColor();
    void TimeBoxFillColor(const NFmiColor &newColor);

private:
    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath; // T�M�N pit�� olla ennen muidat dataosia, ett� se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // t�ss� on  MapView + index, esim. MapView0 (p��ikkuna)
    int mMapIndex;

    // HKEY_CURRENT_USER -keys

    boost::shared_ptr<CachedRegBool> mShowMasksOnMap;
    // 0=ei harvennusta, 1=harvenna jonkin verran, 2=harvenna enemm�n
    boost::shared_ptr<CachedRegInt> mSpacingOutFactor; 
    // 0 = kartta1 (esim. suomi), 1 = kartta2 (esim. skandinavia), 2 = kartta3 (esim. eurooppa), 3 = kartta4 (esim. maailma)
    boost::shared_ptr<CachedRegInt> mSelectedMapIndex; 
    // n�ytet��nk� se typer� asema piste vai ei?
	boost::shared_ptr<CachedRegBool> mShowStationPlot; 
    // karttan�ytt�ruudukon koko (esim. 3 rivi�, joissa 5 aikaa == NFmiPoint(5,3))
    boost::shared_ptr<CachedRegString> mViewGridSizeStr; 
    // Lokaali+wms karttojen yhdistelm� moodiin liittyv�t valitut taustakarttaindeksit kaikille eri kartta-alueille (suomi,skandi,euro,maailma).
    // Teksti on seuraavaa muotoa (t�m� luokka ei tosin parseroi tai tee muuta kuin s�ilytt�� stringin): 
    // mapAreaCount:area1Index,area1Index,area1Index,area1Index     (esim. 4:2,1,4,3)
    boost::shared_ptr<CachedRegString> mCombinedMapModeSelectedBackgroundIndicesStr; 
    // Vastaava teksti overlay kartta-alueiden indekseille
    boost::shared_ptr<CachedRegString> mCombinedMapModeSelectedOverlayIndicesStr;
    // Time-boxin sijainti
    boost::shared_ptr<CachedRegInt> mTimeBoxLocation;
    // Time-boxin tekstien yhteiskokokerroin
    boost::shared_ptr<CachedRegDouble> mTimeBoxTextSizeFactor;
    // Time-boxin pohjav�ri alpha-kertoimen kera
    boost::shared_ptr<CachedRegString> mTimeBoxFillColorStr;
};

// Poikkileikkausn�ytt�jen asetuksia Windows rekisteriss�, SmartMet konffi kohtaisia
class NFmiCrossSectionViewWinRegistry
{
public:
    NFmiCrossSectionViewWinRegistry();
    bool Init(const std::string &baseRegistryPath);

    std::string StartPointStr();
    void StartPointStr(const std::string &newValue);
    std::string MiddlePointStr();
    void MiddlePointStr(const std::string &newValue);
    std::string EndPointStr();
    void EndPointStr(const std::string &newValue);
    int HorizontalPointCount();
    void HorizontalPointCount(int newValue);
    int VerticalPointCount();
    void VerticalPointCount(int newValue);
    double AxisValuesDefaultLowerEndValue();
    void AxisValuesDefaultLowerEndValue(double newValue);
    double AxisValuesDefaultUpperEndValue();
    void AxisValuesDefaultUpperEndValue(double newValue);
    double AxisValuesSpecialLowerEndValue();
    void AxisValuesSpecialLowerEndValue(double newValue);
    double AxisValuesSpecialUpperEndValue();
    void AxisValuesSpecialUpperEndValue(double newValue);

private:
    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath;
    std::string mSectionName; // t�ss� on  CrossSection

    // HKEY_CURRENT_USER -keys

    boost::shared_ptr<CachedRegString> mStartPointStr; // alkupisteen latlon
    boost::shared_ptr<CachedRegString> mMiddlePointStr; // keskipisteen latlon
    boost::shared_ptr<CachedRegString> mEndPointStr; // loppupisteen latlon
    boost::shared_ptr<CachedRegInt> mHorizontalPointCount;
    boost::shared_ptr<CachedRegInt> mVerticalPointCount;
    boost::shared_ptr<CachedRegDouble> mAxisValuesDefaultLowerEndValue; // ns. default paineakselin maanpintaa l�hempi akseli arvo
    boost::shared_ptr<CachedRegDouble> mAxisValuesDefaultUpperEndValue; // ns. default paineakselin korkeammassa p��ss� oleva akseli arvo
    boost::shared_ptr<CachedRegDouble> mAxisValuesSpecialLowerEndValue; // ns. erikois paineakselin maanpintaa l�hempi akseli arvo
    boost::shared_ptr<CachedRegDouble> mAxisValuesSpecialUpperEndValue; // ns. erikois paineakselin korkeammassa p��ss� oleva akseli arvo
};

// Eri n�ytt�jen sijainnit ja koot Windows rekisteriss�
class NFmiViewPositionsWinRegistry
{
public:
    using WindowRectStringMap = std::map<std::string, boost::shared_ptr<CachedRegString>>;

    NFmiViewPositionsWinRegistry();
    bool Init(const std::string &baseRegistryPath, std::map<std::string, std::string> &windowPosMap);

    std::string WindowRectStr(const std::string &keyString);
    void WindowRectStr(const std::string &keyString, const std::string &value);
    boost::shared_ptr<CachedRegString> CachedWindowRectStr(const std::string &keyString);
    const WindowRectStringMap& GetWindowRectStringMap() const { return mWindowRectStringMap; }

private:
    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath; // T�M�N pit�� olla ennen muidat dataosia, ett� se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // t�ss� on  ViewPositions

    // HKEY_CURRENT_USER -keys

    WindowRectStringMap mWindowRectStringMap;
};

// Konfiguraatio kohtaiset asetukset Windows rekisteriss�
class NFmiConfigurationRelatedWinRegistry
{
public:
    NFmiConfigurationRelatedWinRegistry();
    bool Init(const std::string &baseConfigurationRegistryPath, int mapViewCount, std::map<std::string, std::string> &mapWindowPosMap);

    boost::shared_ptr<NFmiMapViewWinRegistry> MapView(int mapIndex);
    int MapViewCount() const {return static_cast<int>(mMapViewVector.size());}
    NFmiViewPositionsWinRegistry& MapViewPositionsWinRegistry() {return mMapViewPositionsWinRegistry;}
    std::string WindowRectStr(const std::string &keyString);
    void WindowRectStr(const std::string &keyString, const std::string &value);
    NFmiCrossSectionViewWinRegistry& CrossSectionViewWinRegistry() {return mCrossSectionViewWinRegistry;}

    bool LoadDataAtStartUp() const;
    void LoadDataAtStartUp(bool newValue);

    bool AutoLoadNewCacheData() const;
    void AutoLoadNewCacheData(bool newValue);
    bool ShowLastSendTimeOnMapView() const;
    void ShowLastSendTimeOnMapView(bool newValue);
    bool AddHelpDataIdAtSendindDataToDatabase();
    void AddHelpDataIdAtSendindDataToDatabase(bool newValue);
    int MacroParamGridSizeX();
    void MacroParamGridSizeX(int newValue);
    int MacroParamGridSizeY();
    void MacroParamGridSizeY(int newValue);
    int LogLevel();
    void LogLevel(int logLevel);
    int LogViewerLogLevel();
    void LogViewerLogLevel(int newValue);
    int LogViewerCategory();
    void LogViewerCategory(int newValue);
    bool DroppedDataEditable();
    void DroppedDataEditable(bool newValue);
    bool UseCombinedMapMode();
    void UseCombinedMapMode(bool newValue);
    const NFmiViewPositionsWinRegistry::WindowRectStringMap& GetWindowRectStringMap() const { return mMapViewPositionsWinRegistry.GetWindowRectStringMap(); }
    NFmiMapViewRangeMeterWinRegistry& MapViewRangeMeter() { return mMapViewRangeMeter; }
private:
    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseConfigurationRegistryPath;

    // HKEY_CURRENT_USER -keys

    // Kartt�n�ytt�ihin liittyv�t asetukset
    std::vector<boost::shared_ptr<NFmiMapViewWinRegistry> > mMapViewVector;
    NFmiViewPositionsWinRegistry mMapViewPositionsWinRegistry;

    // Poikkileikkausn�ytt��n liittyvi� juttuja
    NFmiCrossSectionViewWinRegistry mCrossSectionViewWinRegistry;

    // Konffikohtaiset General-section asetukset
    boost::shared_ptr<CachedRegBool> mLoadDataAtStartUp; // Alkaako Smartmet lukemaan datoja k�ynnistyksess� automaattisesti
    boost::shared_ptr<CachedRegBool> mAutoLoadNewCacheData; // Lukeeko SmartMet automaattisesti uutta dataa lokaaliin cacheen ja siivoaako samalla vanhoja datoja pois.
    boost::shared_ptr<CachedRegBool> mShowLastSendTimeOnMapView; // Editoinnissa halutaan joskus n�hd� milloin on viimeksi l�hetetty editoitua dataa ja onko se tullut takaisin k�ytt��n. T�m� m��r�� n�ytet��nk� kyseisi� tietoja karttan�yt�ll�.
    boost::shared_ptr<CachedRegBool> mAddHelpDataIdAtSendindDataToDatabase; // Lis�t��nk� editoituun dataan my�s help-datan tunnus
    boost::shared_ptr<CachedRegInt> mMacroParamGridSizeX; // Makroparam laskuissa k�ytetyn perushilan x-koko
    boost::shared_ptr<CachedRegInt> mMacroParamGridSizeY; // Makroparam laskuissa k�ytetyn perushilan y-koko
    boost::shared_ptr<CachedRegInt> mLogLevel; // CatLog Severity taso
    boost::shared_ptr<CachedRegInt> mLogViewerLogLevel; // CFmiLogViever dialogissa n�ytetty Severity taso
    boost::shared_ptr<CachedRegInt> mLogViewerCategory; // CFmiLogViever dialogissa n�ytetty categoria
    boost::shared_ptr<CachedRegBool> mDroppedDataEditable; // P��karttan�yt�lle pudotettua sqd tiedostoa voidaan editoida, t�ll�in tiedostot on hidas tiputtaa, koska data luetaan muistiin ja siit� tehd��n monia kopioita
    boost::shared_ptr<CachedRegBool> mUseCombinedMapMode; // K�ytet��nk� karttojen kanssa lokaaleja bitmappeja ja WMS palveluja yhdess�.
    NFmiMapViewRangeMeterWinRegistry mMapViewRangeMeter;
};


// T�ss� luokassa on kaikki SmartMetin Windows rekistereihin tallettamat tiedot
class NFmiApplicationWinRegistry
{
public:
    NFmiApplicationWinRegistry();
    bool Init(const std::string &fullAppVer, const std::string &shortAppVer, const std::string &configurationName, int mapViewCount, std::map<std::string, std::string> &mapWindowPosMap, std::map<std::string, std::string> &otherWindowPosMap, NFmiHelpDataInfoSystem &theHelpDataInfoSystem, const std::vector<NFmiCategoryHeaderInitData>& categoryHeaders);
    void InitGriddingProperties(bool isToolMasterAvailable);
    NFmiViewPositionsWinRegistry& OtherViewPositionsWinRegistry() {return mOtherViewPositionsWinRegistry;}
    NFmiDataNotificationSettingsWinRegistry& DataNotificationSettingsWinRegistry() {return mDataNotificationSettingsWinRegistry;}
    NFmiCaseStudySettingsWinRegistry& CaseStudySettingsWinRegistry() { return mCaseStudySettingsWinRegistry; }

    std::string WindowRectStr(const std::string &keyString);
    void WindowRectStr(const std::string &keyString, const std::string &value);
    NFmiConfigurationRelatedWinRegistry& ConfigurationRelatedWinRegistry() {return mConfigurationRelatedWinRegistry;}
    const NFmiConfigurationRelatedWinRegistry& ConfigurationRelatedWinRegistry() const {return mConfigurationRelatedWinRegistry;}
    const std::string& BaseConfigurationRegistryPath() const { return mBaseConfigurationRegistryPath; }

    bool UseTimeSerialAxisAutoAdjust();
    void UseTimeSerialAxisAutoAdjust(bool newValue);
    bool SoundingTextUpward();
    void SoundingTextUpward(bool newValue);
    bool SoundingTimeLockWithMapView();
    void SoundingTimeLockWithMapView(bool newValue);
    bool ShowStabilityIndexSideView();
    void ShowStabilityIndexSideView(bool newValue);
    bool ShowTextualSoundingDataSideView();
    void ShowTextualSoundingDataSideView(bool newValue);
    SoundingViewSettingsFromWindowsRegisty GetSoundingViewSettings() const;
    void SetSoundingViewSettings(const SoundingViewSettingsFromWindowsRegisty& soundingViewSettings);
    bool KeepMapAspectRatio();
    void KeepMapAspectRatio(bool newValue);

	bool FitToPagePrint();
	void FitToPagePrint(bool newValue);
	bool SmartOrientationPrint();
	void SmartOrientationPrint(bool newValue);
	bool LowMemoryPrint();
	void LowMemoryPrint(bool newValue);
	int MaxRangeInPrint();
	void MaxRangeInPrint(int newValue);
	bool UseMultiProcessCpCalc();
	void UseMultiProcessCpCalc(bool newValue);
    bool AllowRightClickDisplaySelection();
    void AllowRightClickDisplaySelection(bool newValue);
    std::string FixedDrawParamsPath();
    void FixedDrawParamsPath(const std::string &newValue);
    int LocationFinderThreadTimeOutInMS();
    void LocationFinderThreadTimeOutInMS(int newValue);

    bool ShowHakeMessages();
    void ShowHakeMessages(bool newValue);
    bool ShowKaHaMessages();
    void ShowKaHaMessages(bool newValue);
    int MinimumTimeRangeForWarningsOnMapViewsInMinutes();
    void MinimumTimeRangeForWarningsOnMapViewsInMinutes(int newValue);

    double DrawObjectScaleFactor();
    void DrawObjectScaleFactor(double newValue);
    double MaximumFontSizeFactor();
    void MaximumFontSizeFactor(double newValue);
    double IsolineMinLengthFactor();
    void IsolineMinLengthFactor(double newValue);
    bool GenerateTimeCombinationData();
    void GenerateTimeCombinationData(bool newValue);

    const NFmiGriddingProperties& GriddingProperties(bool getEditingRelatedProperties) const;
    void SetGriddingProperties(bool setEditingRelatedProperties, const NFmiGriddingProperties &griddingProperties);

    int SaveImageExtensionFilterIndex() const;
    void SetSaveImageExtensionFilterIndex(int newValue);
    const std::vector<std::string>& SaveImageFileFilterExtensions() const;
    const std::string& GetCurrentSaveImageFileFilterExtension() const;
    double MapViewCacheMaxSizeInMB();
    void MapViewCacheMaxSizeInMB(double newValue);
    bool ShowTooltipOnSmarttoolDialog();
    void ShowTooltipOnSmarttoolDialog(bool newValue);
    void HatchingToolmasterEpsilonFactor(float newEpsilon);
    float HatchingToolmasterEpsilonFactor() const;

    NFmiVisualizationSpaceoutSettings& VisualizationSpaceoutSettings() { return mVisualizationSpaceoutSettings; }
    bool UseLedLightStatusSystem();
    void UseLedLightStatusSystem(bool newValue);

    static std::string MakeBaseRegistryPath();
    static std::string MakeGeneralSectionName();
    
private:
    //void DebugHelper_TestIfAnyViewsHaveSameRectValues();

    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath;
    std::string mBaseRegistryWithVersionPath;
    std::string mBaseConfigurationRegistryPath; // Karttan�ytt�jen asetukset laitetaan konfiguraatio kohtaisiin asetuksiin
    std::string mConfigurationName; // esim. control_scand_saa2_edit_conf, huom! jos on annettu absoluuttinen polku konffiin, erotellaan siit� vain viimeinen osio talteen

    // HKEY_CURRENT_USER -keys

    // Konfiguraatio kohtaisia asetuksia (esim. control_scand_saa2_edit_conf tai control_latvia)
    NFmiConfigurationRelatedWinRegistry mConfigurationRelatedWinRegistry;

    // Yleiset kaikkia konfiguraatioita koskevat asetukset
    NFmiViewPositionsWinRegistry mOtherViewPositionsWinRegistry;
    // Kaikkien konffien DataNotifications -osio
    NFmiDataNotificationSettingsWinRegistry mDataNotificationSettingsWinRegistry;
    // Eri queryDatoihin liittyv�t lukum��r� arvot (local-cache + case-study)
    NFmiCaseStudySettingsWinRegistry mCaseStudySettingsWinRegistry;
    // Kaikkien konffien General -osio
    boost::shared_ptr<CachedRegBool> mUseTimeSerialAxisAutoAdjust; // k�ytet��nk� 'vihattua' auto-adjust s��t�� aikasarjaikkunassa (laskee min ja max arvoja ja p��ttelee siit� sopivan arvoasteikon automaattisesti)
    boost::shared_ptr<CachedRegBool> mSoundingTextUpward; // Luotausn�yt�ss� olevan tekstiosion voi nyt laittaa menem��n yl�reunasta alkaen joko alhaalta yl�s tai p�invastoin (ennen oli vain alhaalta yl�s eli nurinp�in suhteess� luotaus k�yriin)
    boost::shared_ptr<CachedRegBool> mSoundingTimeLockWithMapView; // Luotausn�yt�ss� voi olla nyt aikalukko p��ll�, jolloin luotausten ajat sidotaan p��karttan�ytt��n, eli niit� s��det��n jos karttan�yt�ll� vaihdetaan aikaa
    boost::shared_ptr<CachedRegBool> mShowStabilityIndexSideView; // Onko Luotausn�yt�ss� stabiilisuusindeksi sivun�ytt� auki vai ei
    boost::shared_ptr<CachedRegBool> mShowTextualSoundingDataSideView; // Onko Luotausn�yt�ss� tekstimuotoinen luotausdata sivun�ytt� auki vai ei
    boost::shared_ptr<CachedRegBool> mKeepMapAspectRatio; // Pit��k� smartmet karttan�yt�iss� aspect-ratio -lukkoa p��l� vai ei

    boost::shared_ptr<CachedRegBool> mUseMultiProcessCpCalc; // k�ytet��nk� kontrollipistety�kalun yhteydess� multi-process-worker -poolia vai ei
    boost::shared_ptr<CachedRegBool> mAllowRightClickDisplaySelection; // Sallitaanko k�ytt�j�n valita hiiren oikealla napilla asemia kartalta vai ei.
    std::string mFixedDrawParamsPath; // Ns. tehdasasetus piirto-ominaisuus asetuksien polku (oli aiemmin Windows rekisteriss�, mutta poistin sielt�, koska asetus pit�� saada ehdottomasti konfiguraatioista)
    boost::shared_ptr<CachedRegInt> mLocationFinderThreadTimeOutInMS; // Kuinka kauan maksimissaan odotetaan ett� Location Finder (Autocomplete toiminto SmartMetissa, x n�pp�in karttan�yt�ss�) haku valmistuu, ennenkuin lopetetaan (ettei j�� jumiin pitk�ksi aikaa)

    boost::shared_ptr<CachedRegBool> mShowHakeMessages; // N�ytet��nk� Warnings dialogin kautta Hake sanomia? (H�lytys keskus)
    boost::shared_ptr<CachedRegBool> mShowKaHaMessages; // N�ytet��nk� Warnings dialogin kautta KaHa sanomia? (Kansalais havaintoja)
    // T�ll� voidaan s��t�� mik� on minimi aikav�li, mit� k�ytet��n karttan�yt�ill� kun
    // siell� n�ytet��n joko Hake tai KaHa sanomia. Jos t�m� on <= 0, k�ytet��n karttan�yt�n time-steppi�.
    // Jos t�m� on > 0, k�ytet��n t�m�n ja karttan�yt�n time-stepin maksimia.
    boost::shared_ptr<CachedRegInt> mMinimumTimeRangeForWarningsOnMapViewsInMinutes;

    // T�ll� mDrawObjectScaleFactor:illa skaalataan erilaisten (vektori, ei bitmap) piirto-objektien piirto kokoa. Koska ei voi kysy� j�rjestelm�lt� 
    // kuinka iso monitori fyysisesti (useat n�yt�n ohjaimet 'valehtelevat' koon karkeasti) on ja niimuodoin ei voi laskea oikeaa pixel/mm suhdetta.
    // Jos arvo 0 (oletus), SmartMet laskee pixel/mm suhteen koneesta saatavilla arvoilla. Lis�ksi voidaan tehd� pik� s��t�, miss� kaiken piirto kokoa 
    // voidaan vaikka v�liaikaisesti vaikka suurentaa/pienent�� (kuten web-selaimissa) (EI VOIKAAN, SIT� VARTEN PIT�� TEHD� OMA KERROIN, koska kaikkea 
    // ei luultavasti haluta isontaa esim. param-boxia ihan lennossa).
    boost::shared_ptr<CachedRegDouble> mDrawObjectScaleFactor; 
    // T�m�n avulla skaalataan maksimi fontti kokoa (ei laiteta viel� rekisteriin, koska sit� ei ole mahdollista muokata)
    double itsMaximumFontSizeFactor; 
    // T�ll� voidaan s��t�� piirrett�vien yksitt�isten isoviivojen minimipituutta, jos ei haluta 
    // piirt�� kaikkea pient� s�herryst� jollain tihe�ll� datalla.
    // T�m� on globaali kerroin kaikelle isoviiva piirrolle.
    // Jos arvo on 0, rajoitin on pois p��lt�.
    // Jos arvo on 1, rajoitetaan n. 4 mm pituisten isoviivojen piirtoa n. 20 cm korkealla n�yt�ll� (t�m� el�� dynaamisesti k�ytettyjen n�ytt�jen mukaan).
    // Jos arvo < 0, minimi pituus on pienempi kuin tuo 4 mm ja jos se on > 1, min pituus on suurempi.
    // T�lle voidaan antaa arvo v�lill� 0-100.
    boost::shared_ptr<CachedRegDouble> mIsolineMinLengthFactor;
    // T�m�n avulla s��det��n rakennetaanko, mahdollisesti konffattuja aikakombinaatio datoja. N�ihin
    // datoihin kuuluu mm. analyysi- ja tutkadatat, joihin siis tippuu yksi aika-askel dataa kerrallaan ja 
    // joista palasista rakennetaan halutun pituinen aikajakso yhdistelm�.
    // On osoittautunut ett� n�iden datojen rakentaminen ja/tai ottaminen SmartMetin k�ytt��n voi aiheuttaa 
    // toistaiseksi mystisi� kaatoja. T�ll� siis yritet��n est�� kaadot ja mahdollisesti kartoittaa ongelmaa.
    boost::shared_ptr<CachedRegBool> mGenerateTimeCombinationData;


    // General Printing options
	boost::shared_ptr<CachedRegBool> mFitToPagePrint; // mahdutetaanko kuva aina v�kisin koko paperin alueelle, vaiko s�ilytet��nk� kuvan mitta suhteet
	boost::shared_ptr<CachedRegBool> mSmartOrientationPrint; // lasketaanko ja k��nnet��nk� paperi kuvan mukaan automaattisesti vai ei
	boost::shared_ptr<CachedRegBool> mLowMemoryPrint; // printataanko valmis bitmap kuva (karkea kuva, mutta jos printteriss� v�h�n muistia, t�m� auttaa) vai piirret��nk� kuva k�ytt�en paperin resoluutio tasoa
	boost::shared_ptr<CachedRegInt> mMaxRangeInPrint; // kuinka monta sivua on mahdollista printata maksimissaan range optiolla

    // Pelk�st��n editointity�kaluihin (CP-ty�kalu ja Obs-blender) liittyv�t (t�ydet) griddaus s��d�t (k�ytt�j� voi muokata)
    NFmiGriddingPropertiesWinRegistry mEditingToolsGriddingProperties;
    // Visualisointiin (mm. havaintojen hilaus visualisointi) ja macroParam laskuihin littyv�t griddaus s��d�t (k�ytt�j� ei voi muokata ainakaan viel�)
    NFmiGriddingProperties mVisualizationGriddingProperties;

    // Kun kuvia talletetaan png/jpg/bmp/tiff/gif formaatissa, laitetaan k�ytetty filtteri talteen rekisteriin.
    // Indeksi alkaa 1:st�.
    boost::shared_ptr<CachedRegInt> mSaveImageExtensionFilterIndex; 
    // mSaveImageFileFilterExtensions vektorin ja g_SaveImageFileFilter:in sis�ll�t pit�� asettaa kohdilleen, jos meinaa muuttaa filtterien listaa tai niiden j�rjestyst�!
    const std::vector<std::string> mSaveImageFileFilterExtensions = { "png", "jpg", "bmp", "tiff", "gif" };

    // N�yt�n cachena k�ytettyjen bitmappien max koko megatavuina, tarkoittaa siis aina yhden n�yt�n
    // cachea kerrallaan, eli kolme n�ytt�� ottaa mahdollisesti kaikkiaan 3x t�m� koon.
    boost::shared_ptr<CachedRegDouble> mMapViewCacheMaxSizeInMB;
    boost::shared_ptr<CachedRegBool> mShowTooltipOnSmarttoolDialog;

    boost::shared_ptr<CachedRegDouble> mHatchingToolmasterEpsilonFactor;

    NFmiVisualizationSpaceoutSettings mVisualizationSpaceoutSettings;
    boost::shared_ptr<CachedRegBool> mUseLedLightStatusSystem;

    // HKEY_LOCAL_MACHINE -keys // HUOM! t�m� vaatii ohjelmalta admin oikeuksia!!!!

};
