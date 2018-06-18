
#pragma once

#include <vector>
#include <map>
#include "NFmiCachedRegistryValue.h"
#include "NFmiDataNotificationSettingsWinRegistry.h"

class NFmiHelpDataInfoSystem;

// NFmiHelpDataInfoSystem:iss� olevien kaikkien dynaamisten datojen enable-asetukset Windows rekisteriss�
class NFmiHelpDataEnableWinRegistry
{
public:
    NFmiHelpDataEnableWinRegistry(void);
    bool Init(const std::string &baseRegistryPath, NFmiHelpDataInfoSystem &theHelpDataInfoSystem);
    bool Update(NFmiHelpDataInfoSystem &theHelpDataInfoSystem);

private:
    bool mInitialized; // ei sallita tupla initialisointia
    std::string mBaseRegistryPath; // T�M�N pit�� olla ennen muidat dataosia, ett� se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // t�ss� on EnableData

    // HKEY_CURRENT_USER -keys

	std::map<std::string, boost::shared_ptr<CachedRegBool> > mHelpDataEnableMap;
};

// Yksitt�isten karttan�ytt�jen asetuksia Windows rekisteriss�
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
    std::string mBaseRegistryPath; // T�M�N pit�� olla ennen muidat dataosia, ett� se alustetaan ensin konstruktorissa!!!!
    std::string mSectionName; // t�ss� on  MapView + index, esim. MapView0 (p��ikkuna)
    int mMapIndex;

    // HKEY_CURRENT_USER -keys

    boost::shared_ptr<CachedRegBool> mShowMasksOnMap;
    boost::shared_ptr<CachedRegInt> mSpacingOutFactor; // 0=ei harvennusta, 1=harvenna jonkin verran, 2=harvenna enemm�n
    boost::shared_ptr<CachedRegInt> mSelectedMapIndex; // 0 = kartta1 (esim. suomi), 1 = kartta2 (esim. skandinavia), 2 = kartta3 (esim. eurooppa), 3 = kartta4 (esim. maailma)
	boost::shared_ptr<CachedRegBool> mShowStationPlot; // n�ytet��nk� se typer� asema piste vai ei?
	boost::shared_ptr<CachedRegString> mViewGridSizeStr; // karttan�ytt�ruudukon koko (esim. 3 rivi�, joissa 5 aikaa == NFmiPoint(5,3))
};

// Poikkileikkausn�ytt�jen asetuksia Windows rekisteriss�, SmartMet konffi kohtaisia
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

    NFmiViewPositionsWinRegistry(void);
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
    const NFmiViewPositionsWinRegistry::WindowRectStringMap& GetWindowRectStringMap() const { return mMapViewPositionsWinRegistry.GetWindowRectStringMap(); }
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
};


// T�ss� luokassa on kaikki SmartMetin Windows rekistereihin tallettamat tiedot
class NFmiApplicationWinRegistry
{
public:
    NFmiApplicationWinRegistry(void);
    bool Init(const std::string &fullAppVer, const std::string &shortAppVer, const std::string &configurationName, int mapViewCount, std::map<std::string, std::string> &mapWindowPosMap, std::map<std::string, std::string> &otherWindowPosMap, NFmiHelpDataInfoSystem &theHelpDataInfoSystem);

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
    bool UseWmsMaps();
    void UseWmsMaps(bool newValue);

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
    bool UseLocalFixedDrawParams();
    void UseLocalFixedDrawParams(bool newValue);
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

    static std::string MakeBaseRegistryPath(void);
    static std::string MakeGeneralSectionName(void);
    
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
    // Kaikkien konffien EnableData -osio
    NFmiHelpDataEnableWinRegistry mHelpDataEnableWinRegistry;
    // Kaikkien konffien General -osio
    boost::shared_ptr<CachedRegBool> mUseTimeSerialAxisAutoAdjust; // k�ytet��nk� 'vihattua' auto-adjust s��t�� aikasarjaikkunassa (laskee min ja max arvoja ja p��ttelee siit� sopivan arvoasteikon automaattisesti)
    boost::shared_ptr<CachedRegBool> mSoundingTextUpward; // Luotausn�yt�ss� olevan tekstiosion voi nyt laittaa menem��n yl�reunasta alkaen joko alhaalta yl�s tai p�invastoin (ennen oli vain alhaalta yl�s eli nurinp�in suhteess� luotaus k�yriin)
    boost::shared_ptr<CachedRegBool> mSoundingTimeLockWithMapView; // Luotausn�yt�ss� voi olla nyt aikalukko p��ll�, jolloin luotausten ajat sidotaan p��karttan�ytt��n, eli niit� s��det��n jos karttan�yt�ll� vaihdetaan aikaa
    boost::shared_ptr<CachedRegBool> mKeepMapAspectRatio; // Pit��k� smartmet karttan�yt�iss� aspect-ratio -lukkoa p��l� vai ei
    boost::shared_ptr<CachedRegBool> mUseWmsMaps; // k�ytet��nk� WMS palveluja hakemaan karttakuvia (jos niit� on k�yt�ss�)

    boost::shared_ptr<CachedRegBool> mUseMultiProcessCpCalc; // k�ytet��nk� kontrollipistety�kalun yhteydess� multi-process-worker -poolia vai ei
    boost::shared_ptr<CachedRegBool> mAllowRightClickDisplaySelection; // Sallitaanko k�ytt�j�n valita hiiren oikealla napilla asemia kartalta vai ei.
    std::string mFixedDrawParamsPath; // Ns. tehdasasetus piirto-ominaisuus asetuksien polku (oli aiemmin Windows rekisteriss�, mutta poistin sielt�, koska asetus pit�� saada ehdottomasti konfiguraatioista)
    boost::shared_ptr<CachedRegBool> mUseLocalFixedDrawParams; // Jos normaalisti fixedDrawParamit ovat esim. serverill�, mutta ei ole verkkoyhteytt�, t�ll�in voidaan siirty� k�ytt�m��n lokaaleja tehdasasetuksia, jotka tulevat SmartMet asennuspaketin mukana.
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
    double itsMaximumFontSizeFactor; // t�m� avulla skaalataan maksimi fontti kokoa (ei laiteta viel� rekisteriin, koska sit� ei ole mahdollista muokata)


    // General Printing options
	boost::shared_ptr<CachedRegBool> mFitToPagePrint; // mahdutetaanko kuva aina v�kisin koko paperin alueelle, vaiko s�ilytet��nk� kuvan mitta suhteet
	boost::shared_ptr<CachedRegBool> mSmartOrientationPrint; // lasketaanko ja k��nnet��nk� paperi kuvan mukaan automaattisesti vai ei
	boost::shared_ptr<CachedRegBool> mLowMemoryPrint; // printataanko valmis bitmap kuva (karkea kuva, mutta jos printteriss� v�h�n muistia, t�m� auttaa) vai piirret��nk� kuva k�ytt�en paperin resoluutio tasoa
	boost::shared_ptr<CachedRegInt> mMaxRangeInPrint; // kuinka monta sivua on mahdollista printata maksimissaan range optiolla

    // HKEY_LOCAL_MACHINE -keys // HUOM! t�m� vaatii ohjelmalta admin oikeuksia!!!!

};
