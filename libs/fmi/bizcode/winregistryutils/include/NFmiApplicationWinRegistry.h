
#pragma once

#include <vector>
#include <map>
#include "NFmiCachedRegistryValue.h"
#include "NFmiDataNotificationSettingsWinRegistry.h"
#include "NFmiGriddingProperties.h"

class NFmiHelpDataInfoSystem;

// En osaa laittaa tälläistä muuttujaa luokan muuttujaksi, koska siinä on tyhjä taulukko ja kääntäjä tekee erinäisiä valituksia sellaisen käytöstä luokan dataosana.
const TCHAR BASED_CODE g_SaveImageFileFilter[] = _TEXT("PNG (*.png)|*.png|JPEG (*.jpg)|*.jpg|BMP (*.bmp)|*.bmp|TIFF (*.tiff)|*.tiff|GIF (*.gif)|*.gif|");

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

// NFmiHelpDataInfoSystem:issä olevien kaikkien dynaamisten datojen enable-asetukset Windows rekisterissä
class NFmiHelpDataEnableWinRegistry
{
public:
    NFmiHelpDataEnableWinRegistry(void);
    bool Init(const std::string &baseRegistryPath, NFmiHelpDataInfoSystem &theHelpDataInfoSystem);
    bool Update(NFmiHelpDataInfoSystem &theHelpDataInfoSystem);

private:
    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath; // TÄMÄN pitää olla ennen muidat dataosia, että se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // tässä on EnableData

    // HKEY_CURRENT_USER -keys

	std::map<std::string, boost::shared_ptr<CachedRegBool> > mHelpDataEnableMap;
};

// Yksittäisten karttanäyttöjen asetuksia Windows rekisterissä
class NFmiMapViewWinRegistry
{
public:
    NFmiMapViewWinRegistry(void);
    bool Init(const std::string &baseRegistryPath, int mapIndex);

    bool ShowMasksOnMap() const;
    void ShowMasksOnMap(bool newValue);
    int SpacingOutFactor() const;
    void SpacingOutFactor(int newValue);
    void ToggleSpacingOutFactor(void);
    unsigned int SelectedMapIndex(void) const;
    void SelectedMapIndex(unsigned int newValue);
	bool ShowStationPlot() const;
	void ShowStationPlot(bool newValue);
	std::string ViewGridSizeStr() const;
	void ViewGridSizeStr(const std::string &newValue);

private:
    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath; // TÄMÄN pitää olla ennen muidat dataosia, että se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // tässä on  MapView + index, esim. MapView0 (pääikkuna)
    int mMapIndex;

    // HKEY_CURRENT_USER -keys

    boost::shared_ptr<CachedRegBool> mShowMasksOnMap;
    boost::shared_ptr<CachedRegInt> mSpacingOutFactor; // 0=ei harvennusta, 1=harvenna jonkin verran, 2=harvenna enemmän
    boost::shared_ptr<CachedRegInt> mSelectedMapIndex; // 0 = kartta1 (esim. suomi), 1 = kartta2 (esim. skandinavia), 2 = kartta3 (esim. eurooppa), 3 = kartta4 (esim. maailma)
	boost::shared_ptr<CachedRegBool> mShowStationPlot; // näytetäänkö se typerä asema piste vai ei?
	boost::shared_ptr<CachedRegString> mViewGridSizeStr; // karttanäyttöruudukon koko (esim. 3 riviä, joissa 5 aikaa == NFmiPoint(5,3))
};

// Poikkileikkausnäyttöjen asetuksia Windows rekisterissä, SmartMet konffi kohtaisia
class NFmiCrossSectionViewWinRegistry
{
public:
    NFmiCrossSectionViewWinRegistry(void);
    bool Init(const std::string &baseRegistryPath);

    std::string StartPointStr(void);
    void StartPointStr(const std::string &newValue);
    std::string MiddlePointStr(void);
    void MiddlePointStr(const std::string &newValue);
    std::string EndPointStr(void);
    void EndPointStr(const std::string &newValue);
    int HorizontalPointCount(void);
    void HorizontalPointCount(int newValue);
    int VerticalPointCount(void);
    void VerticalPointCount(int newValue);
    double AxisValuesDefaultLowerEndValue(void);
    void AxisValuesDefaultLowerEndValue(double newValue);
    double AxisValuesDefaultUpperEndValue(void);
    void AxisValuesDefaultUpperEndValue(double newValue);
    double AxisValuesSpecialLowerEndValue(void);
    void AxisValuesSpecialLowerEndValue(double newValue);
    double AxisValuesSpecialUpperEndValue(void);
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

    NFmiViewPositionsWinRegistry(void);
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
    NFmiConfigurationRelatedWinRegistry(void);
    bool Init(const std::string &baseConfigurationRegistryPath, int mapViewCount, std::map<std::string, std::string> &mapWindowPosMap);

    boost::shared_ptr<NFmiMapViewWinRegistry> MapView(int mapIndex);
    int MapViewCount(void) const {return static_cast<int>(mMapViewVector.size());}
    NFmiViewPositionsWinRegistry& MapViewPositionsWinRegistry(void) {return mMapViewPositionsWinRegistry;}
    std::string WindowRectStr(const std::string &keyString);
    void WindowRectStr(const std::string &keyString, const std::string &value);
    NFmiCrossSectionViewWinRegistry& CrossSectionViewWinRegistry(void) {return mCrossSectionViewWinRegistry;}

    bool LoadDataAtStartUp(void) const;
    void LoadDataAtStartUp(bool newValue);

    bool AutoLoadNewCacheData(void) const;
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
};


// Tässä luokassa on kaikki SmartMetin Windows rekistereihin tallettamat tiedot
class NFmiApplicationWinRegistry
{
public:
    NFmiApplicationWinRegistry(void);
    bool Init(const std::string &fullAppVer, const std::string &shortAppVer, const std::string &configurationName, int mapViewCount, std::map<std::string, std::string> &mapWindowPosMap, std::map<std::string, std::string> &otherWindowPosMap, NFmiHelpDataInfoSystem &theHelpDataInfoSystem);
    void InitGriddingProperties(bool isToolMasterAvailable);
    NFmiViewPositionsWinRegistry& OtherViewPositionsWinRegistry(void) {return mOtherViewPositionsWinRegistry;}
    NFmiDataNotificationSettingsWinRegistry& DataNotificationSettingsWinRegistry(void) {return mDataNotificationSettingsWinRegistry;}
    NFmiHelpDataEnableWinRegistry& HelpDataEnableWinRegistry(void) {return mHelpDataEnableWinRegistry;}

    std::string WindowRectStr(const std::string &keyString);
    void WindowRectStr(const std::string &keyString, const std::string &value);
    NFmiConfigurationRelatedWinRegistry& ConfigurationRelatedWinRegistry(void) {return mConfigurationRelatedWinRegistry;}
    const NFmiConfigurationRelatedWinRegistry& ConfigurationRelatedWinRegistry(void) const {return mConfigurationRelatedWinRegistry;}
    const std::string& BaseConfigurationRegistryPath() const { return mBaseConfigurationRegistryPath; }

    bool UseTimeSerialAxisAutoAdjust();
    void UseTimeSerialAxisAutoAdjust(bool newValue);
    bool SoundingTextUpward();
    void SoundingTextUpward(bool newValue);
    bool SoundingTimeLockWithMapView();
    void SoundingTimeLockWithMapView(bool newValue);
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
    bool ForceWdParameterToLinearInterpolation();
    void ForceWdParameterToLinearInterpolation(bool newValue);
    bool ShowTooltipOnSmarttoolDialog();
    void ShowTooltipOnSmarttoolDialog(bool newValue);

    static std::string MakeBaseRegistryPath(void);
    static std::string MakeGeneralSectionName(void);
    
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
    // Kaikkien konffien EnableData -osio
    NFmiHelpDataEnableWinRegistry mHelpDataEnableWinRegistry;
    // Kaikkien konffien General -osio
    boost::shared_ptr<CachedRegBool> mUseTimeSerialAxisAutoAdjust; // käytetäänkö 'vihattua' auto-adjust säätöä aikasarjaikkunassa (laskee min ja max arvoja ja päättelee siitä sopivan arvoasteikon automaattisesti)
    boost::shared_ptr<CachedRegBool> mSoundingTextUpward; // Luotausnäytössä olevan tekstiosion voi nyt laittaa menemään yläreunasta alkaen joko alhaalta ylös tai päinvastoin (ennen oli vain alhaalta ylös eli nurinpäin suhteessä luotaus käyriin)
    boost::shared_ptr<CachedRegBool> mSoundingTimeLockWithMapView; // Luotausnäytössä voi olla nyt aikalukko päällä, jolloin luotausten ajat sidotaan pääkarttanäyttöön, eli niitä säädetään jos karttanäytöllä vaihdetaan aikaa
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
    boost::shared_ptr<CachedRegBool> mForceWdParameterToLinearInterpolation;
    boost::shared_ptr<CachedRegBool> mShowTooltipOnSmarttoolDialog;

    // HKEY_LOCAL_MACHINE -keys // HUOM! tämä vaatii ohjelmalta admin oikeuksia!!!!

};
