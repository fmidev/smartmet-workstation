
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

// En osaa laittaa tälläistä muuttujaa luokan muuttujaksi, koska siinä on tyhjä taulukko ja kääntäjä tekee erinäisiä valituksia sellaisen käytöstä luokan dataosana.
const TCHAR BASED_CODE g_SaveImageFileFilter[] = _TEXT("PNG (*.png)|*.png|JPEG (*.jpg)|*.jpg|BMP (*.bmp)|*.bmp|TIFF (*.tiff)|*.tiff|GIF (*.gif)|*.gif|");

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
    // Jos lukitus moodi on päällä, ei tehdä muutoksia start/end-point asetuksiin eikä MouseDragOn asetukseen
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
    std::string mSectionName; // tässä on GriddingProperties

    // HKEY_CURRENT_USER -keys
    boost::shared_ptr<CachedRegInt> mFunction;
    boost::shared_ptr<CachedRegDouble> mRangeLimitInKm;
    boost::shared_ptr<CachedRegInt> mLocalFitMethod;
    boost::shared_ptr<CachedRegDouble> mLocalFitDelta;
    boost::shared_ptr<CachedRegInt> mSmoothLevel;
    boost::shared_ptr<CachedRegDouble> mLocalFitFilterRadius;
    boost::shared_ptr<CachedRegDouble> mLocalFitFilterFactor;

    // Tätä yksinkertaistettua kokooma luokkaa käytetään varsinaisesti SmartMetin griddauskoodeissa
    NFmiGriddingProperties mGriddingProperties;
};

using CaseStudyBoolMap = std::map<std::string, boost::shared_ptr<CachedRegBool>>;

// NFmiHelpDataInfoSystem:issä olevien kaikkien dynaamisten datojen enable-asetukset Windows rekisterissä
class NFmiHelpDataEnableWinRegistry
{
public:
    NFmiHelpDataEnableWinRegistry();
    bool Init(const std::string &baseRegistryPath, NFmiHelpDataInfoSystem &theHelpDataInfoSystem);
    bool Update(NFmiHelpDataInfoSystem &theHelpDataInfoSystem);

private:
    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath; // TÄMÄN pitää olla ennen muidat dataosia, että se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // tässä on EnableData

    // HKEY_CURRENT_USER -keys

    CaseStudyBoolMap mHelpDataEnableMap;
};

// Talletetaan jokaista uniikkia datanimeä kohden pari, jossa on datan tyyppi ja lukumäärä int arvona.
using CaseStudyCountMap = std::map<std::string, std::pair<NFmiInfoData::Type, boost::shared_ptr<CachedRegInt>>>;
using ModelDataOffsetRangeInHours = std::pair<int, int>;
const ModelDataOffsetRangeInHours gMissingOffsetRangeInHours = std::make_pair(-1, -1);
const ModelDataOffsetRangeInHours gLatestDataOnlyRangeInHours = std::make_pair(0, 0);
const ModelDataOffsetRangeInHours gDefaultModelDataOffsetRangeInHours = std::make_pair(24, 0);

// CaseStudy dialogiin liittyvien asetuksien asetukset Windows rekisterissä:
// 1) Kuinka monta viimeistä dataa säilytetään lokaali cachessa
//    - Jos kyse mallidatasta, kuin monta viimeistä malliajoa
//    - Jos kyse havainto/analyysi datoista, numero on aina vakio 1
//    - Jos kyse on satel-kuvista, numero on aina vakio 0, koska niitä ei säilytetä lokaalissa cachessa
// 2) Kuinka monta viimeistä dataa talletetaan tehtävään CaseStudy dataan 
//    - Samat perus säädöt kuin 1:ssäkin
//    - Satel kuville tosin on nyt säätö, toisin kuin local-cache-countissa
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
    std::string mSectionNameLocalCacheCount; // tässä on LocalCacheCount
    std::string mSectionNameCaseStudyOffset1; // tässä on CaseStudyOffset1
    std::string mSectionNameCaseStudyOffset2; // tässä on CaseStudyOffset2
    std::string mSectionNameStoreData; // tässä on StoreData

    // Kaikkien queryData konffien EnableData -osio
    NFmiHelpDataEnableWinRegistry mHelpDataEnableWinRegistry;

    // HKEY_CURRENT_USER -keys

    CaseStudyCountMap mHelpDataLocalCacheCountMap;
    // Muutetaan talletettavien datojen lukumäärä kahdeksi indeksiksi, joiden väliin jäävät malliajot talletetaan.
    // CaseStudy-dialogissa molemmat luvut annetaan yhdestä grid-controllin cell:ista.
    // Jos cell:in string arvo on vain yksi luku (esim. "3"), tällöin talletetaan kaikki malliajot 3-1 (3. tuoreimmasta 1. tuoreimpaan).
    // Muuten cell:in string arvossa on kaksi lukua eroteltuna '-' merkillä ,esim. "4-3". Tällöin talletetaan 4. tuorein ja 3. tuorein, mutta ei 2. ja 1. tuoreimpia.
    // Näin voi tallettaa CaseStudyn jälkikäteen vaikka eilisen datoilla niin että uusimpia datoja ei tarvitse sulloa väkisin datapakettiin.

    // Tässä on siis aloitus offset, joka kertoo talletusikkunan alkuhetken offsetin seinäkelloajasta.
    CaseStudyCountMap mHelpDataCaseStudyOffset1Map;
    // Tässä on siis lopetus offset, joka kertoo talletusikkunan loppuhetken offsetin seinäkelloajasta.
    CaseStudyCountMap mHelpDataCaseStudyOffset2Map;
    CaseStudyBoolMap mCaseStudyStoreDataMap;
    bool mInitialized = false; // ei sallita tupla initialisointia
};

// Yksittäisten karttanäyttöjen asetuksia Windows rekisterissä
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
    std::string mBaseRegistryPath; // TÄMÄN pitää olla ennen muidat dataosia, että se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // tässä on  MapView + index, esim. MapView0 (pääikkuna)
    int mMapIndex;

    // HKEY_CURRENT_USER -keys

    boost::shared_ptr<CachedRegBool> mShowMasksOnMap;
    // 0=ei harvennusta, 1=harvenna jonkin verran, 2=harvenna enemmän
    boost::shared_ptr<CachedRegInt> mSpacingOutFactor; 
    // 0 = kartta1 (esim. suomi), 1 = kartta2 (esim. skandinavia), 2 = kartta3 (esim. eurooppa), 3 = kartta4 (esim. maailma)
    boost::shared_ptr<CachedRegInt> mSelectedMapIndex; 
    // näytetäänkö se typerä asema piste vai ei?
	boost::shared_ptr<CachedRegBool> mShowStationPlot; 
    // karttanäyttöruudukon koko (esim. 3 riviä, joissa 5 aikaa == NFmiPoint(5,3))
    boost::shared_ptr<CachedRegString> mViewGridSizeStr; 
    // Lokaali+wms karttojen yhdistelmä moodiin liittyvät valitut taustakarttaindeksit kaikille eri kartta-alueille (suomi,skandi,euro,maailma).
    // Teksti on seuraavaa muotoa (tämä luokka ei tosin parseroi tai tee muuta kuin säilyttää stringin): 
    // mapAreaCount:area1Index,area1Index,area1Index,area1Index     (esim. 4:2,1,4,3)
    boost::shared_ptr<CachedRegString> mCombinedMapModeSelectedBackgroundIndicesStr; 
    // Vastaava teksti overlay kartta-alueiden indekseille
    boost::shared_ptr<CachedRegString> mCombinedMapModeSelectedOverlayIndicesStr;
    // Time-boxin sijainti
    boost::shared_ptr<CachedRegInt> mTimeBoxLocation;
    // Time-boxin tekstien yhteiskokokerroin
    boost::shared_ptr<CachedRegDouble> mTimeBoxTextSizeFactor;
    // Time-boxin pohjaväri alpha-kertoimen kera
    boost::shared_ptr<CachedRegString> mTimeBoxFillColorStr;
};

// Poikkileikkausnäyttöjen asetuksia Windows rekisterissä, SmartMet konffi kohtaisia
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
    std::string mSectionName; // tässä on  CrossSection

    // HKEY_CURRENT_USER -keys

    boost::shared_ptr<CachedRegString> mStartPointStr; // alkupisteen latlon
    boost::shared_ptr<CachedRegString> mMiddlePointStr; // keskipisteen latlon
    boost::shared_ptr<CachedRegString> mEndPointStr; // loppupisteen latlon
    boost::shared_ptr<CachedRegInt> mHorizontalPointCount;
    boost::shared_ptr<CachedRegInt> mVerticalPointCount;
    boost::shared_ptr<CachedRegDouble> mAxisValuesDefaultLowerEndValue; // ns. default paineakselin maanpintaa lähempi akseli arvo
    boost::shared_ptr<CachedRegDouble> mAxisValuesDefaultUpperEndValue; // ns. default paineakselin korkeammassa päässä oleva akseli arvo
    boost::shared_ptr<CachedRegDouble> mAxisValuesSpecialLowerEndValue; // ns. erikois paineakselin maanpintaa lähempi akseli arvo
    boost::shared_ptr<CachedRegDouble> mAxisValuesSpecialUpperEndValue; // ns. erikois paineakselin korkeammassa päässä oleva akseli arvo
};

// Eri näyttöjen sijainnit ja koot Windows rekisterissä
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
    std::string mBaseRegistryPath; // TÄMÄN pitää olla ennen muidat dataosia, että se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // tässä on  ViewPositions

    // HKEY_CURRENT_USER -keys

    WindowRectStringMap mWindowRectStringMap;
};

// Konfiguraatio kohtaiset asetukset Windows rekisterissä
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

    // Karttänäyttöihin liittyvät asetukset
    std::vector<boost::shared_ptr<NFmiMapViewWinRegistry> > mMapViewVector;
    NFmiViewPositionsWinRegistry mMapViewPositionsWinRegistry;

    // Poikkileikkausnäyttöön liittyviä juttuja
    NFmiCrossSectionViewWinRegistry mCrossSectionViewWinRegistry;

    // Konffikohtaiset General-section asetukset
    boost::shared_ptr<CachedRegBool> mLoadDataAtStartUp; // Alkaako Smartmet lukemaan datoja käynnistyksessä automaattisesti
    boost::shared_ptr<CachedRegBool> mAutoLoadNewCacheData; // Lukeeko SmartMet automaattisesti uutta dataa lokaaliin cacheen ja siivoaako samalla vanhoja datoja pois.
    boost::shared_ptr<CachedRegBool> mShowLastSendTimeOnMapView; // Editoinnissa halutaan joskus nähdä milloin on viimeksi lähetetty editoitua dataa ja onko se tullut takaisin käyttöön. Tämä määrää näytetäänkö kyseisiä tietoja karttanäytöllä.
    boost::shared_ptr<CachedRegBool> mAddHelpDataIdAtSendindDataToDatabase; // Lisätäänkö editoituun dataan myös help-datan tunnus
    boost::shared_ptr<CachedRegInt> mMacroParamGridSizeX; // Makroparam laskuissa käytetyn perushilan x-koko
    boost::shared_ptr<CachedRegInt> mMacroParamGridSizeY; // Makroparam laskuissa käytetyn perushilan y-koko
    boost::shared_ptr<CachedRegInt> mLogLevel; // CatLog Severity taso
    boost::shared_ptr<CachedRegInt> mLogViewerLogLevel; // CFmiLogViever dialogissa näytetty Severity taso
    boost::shared_ptr<CachedRegInt> mLogViewerCategory; // CFmiLogViever dialogissa näytetty categoria
    boost::shared_ptr<CachedRegBool> mDroppedDataEditable; // Pääkarttanäytölle pudotettua sqd tiedostoa voidaan editoida, tällöin tiedostot on hidas tiputtaa, koska data luetaan muistiin ja siitä tehdään monia kopioita
    boost::shared_ptr<CachedRegBool> mUseCombinedMapMode; // Käytetäänkö karttojen kanssa lokaaleja bitmappeja ja WMS palveluja yhdessä.
    NFmiMapViewRangeMeterWinRegistry mMapViewRangeMeter;
};


// Tässä luokassa on kaikki SmartMetin Windows rekistereihin tallettamat tiedot
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
    std::string mBaseConfigurationRegistryPath; // Karttanäyttöjen asetukset laitetaan konfiguraatio kohtaisiin asetuksiin
    std::string mConfigurationName; // esim. control_scand_saa2_edit_conf, huom! jos on annettu absoluuttinen polku konffiin, erotellaan siitä vain viimeinen osio talteen

    // HKEY_CURRENT_USER -keys

    // Konfiguraatio kohtaisia asetuksia (esim. control_scand_saa2_edit_conf tai control_latvia)
    NFmiConfigurationRelatedWinRegistry mConfigurationRelatedWinRegistry;

    // Yleiset kaikkia konfiguraatioita koskevat asetukset
    NFmiViewPositionsWinRegistry mOtherViewPositionsWinRegistry;
    // Kaikkien konffien DataNotifications -osio
    NFmiDataNotificationSettingsWinRegistry mDataNotificationSettingsWinRegistry;
    // Eri queryDatoihin liittyvät lukumäärä arvot (local-cache + case-study)
    NFmiCaseStudySettingsWinRegistry mCaseStudySettingsWinRegistry;
    // Kaikkien konffien General -osio
    boost::shared_ptr<CachedRegBool> mUseTimeSerialAxisAutoAdjust; // käytetäänkö 'vihattua' auto-adjust säätöä aikasarjaikkunassa (laskee min ja max arvoja ja päättelee siitä sopivan arvoasteikon automaattisesti)
    boost::shared_ptr<CachedRegBool> mSoundingTextUpward; // Luotausnäytössä olevan tekstiosion voi nyt laittaa menemään yläreunasta alkaen joko alhaalta ylös tai päinvastoin (ennen oli vain alhaalta ylös eli nurinpäin suhteessä luotaus käyriin)
    boost::shared_ptr<CachedRegBool> mSoundingTimeLockWithMapView; // Luotausnäytössä voi olla nyt aikalukko päällä, jolloin luotausten ajat sidotaan pääkarttanäyttöön, eli niitä säädetään jos karttanäytöllä vaihdetaan aikaa
    boost::shared_ptr<CachedRegBool> mShowStabilityIndexSideView; // Onko Luotausnäytössä stabiilisuusindeksi sivunäyttö auki vai ei
    boost::shared_ptr<CachedRegBool> mShowTextualSoundingDataSideView; // Onko Luotausnäytössä tekstimuotoinen luotausdata sivunäyttö auki vai ei
    boost::shared_ptr<CachedRegBool> mKeepMapAspectRatio; // Pitääkö smartmet karttanäytöissä aspect-ratio -lukkoa päälä vai ei

    boost::shared_ptr<CachedRegBool> mUseMultiProcessCpCalc; // käytetäänkö kontrollipistetyökalun yhteydessä multi-process-worker -poolia vai ei
    boost::shared_ptr<CachedRegBool> mAllowRightClickDisplaySelection; // Sallitaanko käyttäjän valita hiiren oikealla napilla asemia kartalta vai ei.
    std::string mFixedDrawParamsPath; // Ns. tehdasasetus piirto-ominaisuus asetuksien polku (oli aiemmin Windows rekisterissä, mutta poistin sieltä, koska asetus pitää saada ehdottomasti konfiguraatioista)
    boost::shared_ptr<CachedRegInt> mLocationFinderThreadTimeOutInMS; // Kuinka kauan maksimissaan odotetaan että Location Finder (Autocomplete toiminto SmartMetissa, x näppäin karttanäytössä) haku valmistuu, ennenkuin lopetetaan (ettei jää jumiin pitkäksi aikaa)

    boost::shared_ptr<CachedRegBool> mShowHakeMessages; // Näytetäänkö Warnings dialogin kautta Hake sanomia? (Hälytys keskus)
    boost::shared_ptr<CachedRegBool> mShowKaHaMessages; // Näytetäänkö Warnings dialogin kautta KaHa sanomia? (Kansalais havaintoja)
    // Tällä voidaan säätää mikä on minimi aikaväli, mitä käytetään karttanäytöillä kun
    // siellä näytetään joko Hake tai KaHa sanomia. Jos tämä on <= 0, käytetään karttanäytön time-steppiä.
    // Jos tämä on > 0, käytetään tämän ja karttanäytön time-stepin maksimia.
    boost::shared_ptr<CachedRegInt> mMinimumTimeRangeForWarningsOnMapViewsInMinutes;

    // Tällä mDrawObjectScaleFactor:illa skaalataan erilaisten (vektori, ei bitmap) piirto-objektien piirto kokoa. Koska ei voi kysyä järjestelmältä 
    // kuinka iso monitori fyysisesti (useat näytön ohjaimet 'valehtelevat' koon karkeasti) on ja niimuodoin ei voi laskea oikeaa pixel/mm suhdetta.
    // Jos arvo 0 (oletus), SmartMet laskee pixel/mm suhteen koneesta saatavilla arvoilla. Lisäksi voidaan tehdä pikä säätö, missä kaiken piirto kokoa 
    // voidaan vaikka väliaikaisesti vaikka suurentaa/pienentää (kuten web-selaimissa) (EI VOIKAAN, SITÄ VARTEN PITÄÄ TEHDÄ OMA KERROIN, koska kaikkea 
    // ei luultavasti haluta isontaa esim. param-boxia ihan lennossa).
    boost::shared_ptr<CachedRegDouble> mDrawObjectScaleFactor; 
    // Tämän avulla skaalataan maksimi fontti kokoa (ei laiteta vielä rekisteriin, koska sitä ei ole mahdollista muokata)
    double itsMaximumFontSizeFactor; 
    // Tällä voidaan säätää piirrettävien yksittäisten isoviivojen minimipituutta, jos ei haluta 
    // piirtää kaikkea pientä söherrystä jollain tiheällä datalla.
    // Tämä on globaali kerroin kaikelle isoviiva piirrolle.
    // Jos arvo on 0, rajoitin on pois päältä.
    // Jos arvo on 1, rajoitetaan n. 4 mm pituisten isoviivojen piirtoa n. 20 cm korkealla näytöllä (tämä elää dynaamisesti käytettyjen näyttöjen mukaan).
    // Jos arvo < 0, minimi pituus on pienempi kuin tuo 4 mm ja jos se on > 1, min pituus on suurempi.
    // Tälle voidaan antaa arvo välillä 0-100.
    boost::shared_ptr<CachedRegDouble> mIsolineMinLengthFactor;
    // Tämän avulla säädetään rakennetaanko, mahdollisesti konffattuja aikakombinaatio datoja. Näihin
    // datoihin kuuluu mm. analyysi- ja tutkadatat, joihin siis tippuu yksi aika-askel dataa kerrallaan ja 
    // joista palasista rakennetaan halutun pituinen aikajakso yhdistelmä.
    // On osoittautunut että näiden datojen rakentaminen ja/tai ottaminen SmartMetin käyttöön voi aiheuttaa 
    // toistaiseksi mystisiä kaatoja. Tällä siis yritetään estää kaadot ja mahdollisesti kartoittaa ongelmaa.
    boost::shared_ptr<CachedRegBool> mGenerateTimeCombinationData;


    // General Printing options
	boost::shared_ptr<CachedRegBool> mFitToPagePrint; // mahdutetaanko kuva aina väkisin koko paperin alueelle, vaiko säilytetäänkö kuvan mitta suhteet
	boost::shared_ptr<CachedRegBool> mSmartOrientationPrint; // lasketaanko ja käännetäänkö paperi kuvan mukaan automaattisesti vai ei
	boost::shared_ptr<CachedRegBool> mLowMemoryPrint; // printataanko valmis bitmap kuva (karkea kuva, mutta jos printterissä vähän muistia, tämä auttaa) vai piirretäänkö kuva käyttäen paperin resoluutio tasoa
	boost::shared_ptr<CachedRegInt> mMaxRangeInPrint; // kuinka monta sivua on mahdollista printata maksimissaan range optiolla

    // Pelkästään editointityökaluihin (CP-työkalu ja Obs-blender) liittyvät (täydet) griddaus säädöt (käyttäjä voi muokata)
    NFmiGriddingPropertiesWinRegistry mEditingToolsGriddingProperties;
    // Visualisointiin (mm. havaintojen hilaus visualisointi) ja macroParam laskuihin littyvät griddaus säädöt (käyttäjä ei voi muokata ainakaan vielä)
    NFmiGriddingProperties mVisualizationGriddingProperties;

    // Kun kuvia talletetaan png/jpg/bmp/tiff/gif formaatissa, laitetaan käytetty filtteri talteen rekisteriin.
    // Indeksi alkaa 1:stä.
    boost::shared_ptr<CachedRegInt> mSaveImageExtensionFilterIndex; 
    // mSaveImageFileFilterExtensions vektorin ja g_SaveImageFileFilter:in sisällöt pitää asettaa kohdilleen, jos meinaa muuttaa filtterien listaa tai niiden järjestystä!
    const std::vector<std::string> mSaveImageFileFilterExtensions = { "png", "jpg", "bmp", "tiff", "gif" };

    // Näytön cachena käytettyjen bitmappien max koko megatavuina, tarkoittaa siis aina yhden näytön
    // cachea kerrallaan, eli kolme näyttöä ottaa mahdollisesti kaikkiaan 3x tämä koon.
    boost::shared_ptr<CachedRegDouble> mMapViewCacheMaxSizeInMB;
    boost::shared_ptr<CachedRegBool> mShowTooltipOnSmarttoolDialog;

    boost::shared_ptr<CachedRegDouble> mHatchingToolmasterEpsilonFactor;

    NFmiVisualizationSpaceoutSettings mVisualizationSpaceoutSettings;
    boost::shared_ptr<CachedRegBool> mUseLedLightStatusSystem;

    // HKEY_LOCAL_MACHINE -keys // HUOM! tämä vaatii ohjelmalta admin oikeuksia!!!!

};
