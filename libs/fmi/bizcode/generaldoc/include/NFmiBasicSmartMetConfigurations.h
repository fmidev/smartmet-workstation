#pragma once

// Tämä luokka pitää sisällään tiettyjä SmartMet -työaseman perus konfiguraatioita,
// jotka on tarkoitus alustaa ennen kuin SmartMetin NFmiEditMapGeneralDataDoc -otus 
// alustetaan. 
// Näitä tiettyjä arvoja tarvitaan ennen tuon doc-otuksen alustusta kun alustetaan 
// SmartMetissa käytettyä CrashRpt -systeemiä (kaato raportointi systeemi).
// Tämän luokan pitää alustaa seuraavat asiat:
// 1. Käytetyt SmartMet konfiguraatiot, jotka luetaan sitten NFmiSetting luokan avulla. 
// 2. NFmiApplicationDataBase-luokka. 
// 3. CatLog systeemi.

#include "catlog/catlog.h"
#include "NFmiApplicationDataBase.h"
#include "NFmiCachedRegistryValue.h"
#include "NFmiMilliSecondTimer.h"
#include "FmiWin32Helpers.h"
#include "Splasher.h"

#include "boost/shared_ptr.hpp"

#include <list>

// Kun tämä poikkeus lentää initialisoinnin yhteydessä, SmartMet lopettaa toimintansa 'hallitusti'
class AbortSmartMetInitializationGracefullyException
{
};

class NFmiBasicSmartMetConfigurations
{
public:
	NFmiBasicSmartMetConfigurations(void);
	~NFmiBasicSmartMetConfigurations(void);
	bool Init(const std::string &avsToolMasterVersion);

	const std::string& WorkingDirectory(void) const {return itsWorkingDirectory;}
	const std::string& ControlBasePath(void) const {return itsControlBasePath;}
    std::string GetShortConfigurationName(void);
	bool DeveloperModePath(void) const {return fDeveloperModePath;}
	void DeveloperModePath(bool newValue) {fDeveloperModePath = newValue;}
	void GetWorkingDirectory(bool fDontInitializeAgain = true);
	bool SetControlPath(const std::string &theControlPath);
	const std::string& ControlPath(void) const {return itsControlPath;}
    const std::string& HelpDataPath() const { return itsHelpDataPath; }
    const std::string& LogFileDirectory() const { return itsLogFileDirectory; }
    const std::string& BaseConfigurationFilePath() const { return itsBaseConfigurationFilePath; }

    void LogMessage(const std::string& message, CatLog::Severity severity, CatLog::Category category, bool flushLogger = false);
    void LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption = false, bool flushLogger = false);
	const std::string& GetApplicationTitle(void) {return itsApplicationTitle;}
	void SetApplicationTitle(const std::string &theTitle) {itsApplicationTitle = theTitle;}
	NFmiApplicationDataBase& ApplicationDataBase(void) {return itsApplicationDataBase;}
	const std::string& EditorVersionStr(void) const {return itsEditorVersionStr;}
	FmiLanguage Language(void) const {return itsLanguage;}
	void Language(FmiLanguage newValue);
    bool Verbose() const {return fVerbose;}
    void Verbose(bool newValue) {fVerbose = newValue;}
    void DoStartupLogging(const std::string &theAction);
    const std::vector<DrawStringData>& SplashScreenTextDataVector() const { return itsSplashScreenTextDataVector; }
    double RunningTimeInSeconds() { return itsElapsedRunningTimer.elapsedTimeInSeconds(); }
    std::string MakeRunningTimeString();
    bool IsToolMasterAvailable() const { return fToolMasterAvailable; }
    void ToolMasterAvailable(bool newValue) { fToolMasterAvailable = newValue; }
    bool EnableCrashReporter() const { return fEnableCrashReporter; }
    void EnableCrashReporter(bool newValue) { fEnableCrashReporter = newValue; }
    const std::string& BetaAutomationListPath() const { return itsBetaAutomationListPath; }
    void BetaAutomationListPath(const std::string &newPath) { itsBetaAutomationListPath = newPath; }
    static void DoInitializationAbortMessageBox(const std::string& errorString, const std::string& titleString, bool throwAbortException);

private:
	bool ReadConfigurations();
    bool ReadPreConfigurationSettings();
    bool InitLogger(void);
	void SetEditorVersionStr(void);
    void MakeSplashScreenTextDataVector(const NFmiTime &theExeModTimeLocal, const std::string &theVersionString);
	void InitApplicationDataBase(const std::string &avsToolMasterVersion);
    bool DoControlPathChecks(std::string theControlPath);
    void SetupFactorySettingFile(const std::string &theConfigurationFile);
    void InitFactorySettingFiles();
    void ReadConfigurationFile(const std::string &theConfigurationFilePath);
    bool ReadConfigurationFileFromAbsolutePath(const std::string &theFilePath);
    bool IsFactorySettingFile(const std::string &theConfigurationFile, std::string &theFactorySettingFileOut);
    std::string MakeDictionaryFilePath() const;
    bool FileNeedsForcedUpdate(const std::string &localConfFilePath, std::string &factorySettingFilePath);
    size_t GetForcedUpdateVersionNumber(const std::string &filePath);
    void LogBasicPaths();
    void LogOtherPaths();


    // SmartMetin työhakemisto
	std::string itsWorkingDirectory; 
    // Tämä on -p option arvo kun SmartMet käynnistetään. Siinä voi olla hakemisto (mistä smartmet.conf 
    // luetaan) tai suora tiedoston nimi, jota käsitellään kuten smartmet.conf -tiedostoa. Tällöin itsControlPath
    // on se hakemisto, mikä on polkuna annetulle tiedostolle.
	std::string itsControlBasePath; 
    // Tässä on haluttu ohjaus-hakemisto, mista kaikki ohjaus tiedostot luetaan, polku on absoluuttinen, 
    // joten ohjaus hakemisto voi olla riippumaton editori hakemistosta
    std::string itsControlPath;
    // Tässä on smartmet.conf tiedoston koko polku. Se voi olla eri nimellä, jos -p optiossa on annettu
    // suoraan halutun tiedoston nimi.
    std::string itsBaseConfigurationFilePath;
    // Tässä hakemistossa on ns. tehdasasetus GUI konffeja, eli tiedostoja, joihin SmartMet tekee
    // Käyttöliittymän kautta muutoksia. Ne voidaan tuottaa oletus tiedostoista, jos ne puuttuvat, 
    // tällöin niihin tehdyt muutokset ovat konekohtaisia.
    // Se on workingDirectory\FactorySettings -hakemisto.
    std::string itsFactorySettingsConfigurationFilePath;
    // Tähän luetaan tehdasasetus tiedotojen nimet em. hakemistosta.
    std::list<std::string> itsFactorySettingsConfigurationFiles;
    // SmartMet lukee tietyt tiedostot ns. help_data -hakemistosta. Tämä on optionaalinen asetus
    // jonka oletusarvo on itsControlDirectory + "\\help_data".
    // Pilvipalvelu asennuksien mukana on tullut tarve että hakemsito voi olla muuallakin kuin workinDirectoryn alla.
    std::string itsHelpDataPath;
    // Otetaan talteen lokitiedostojen hakemisto
    std::string itsLogFileDirectory;

    // Tässä on käytetyn kielen lyhenne stringinä, joka on tallessa Windows rekisterissä
    boost::shared_ptr<CachedRegString> itsDictionaryLanguageString;
    // Editorin käyttöliittymän kieli, joka saadaan itsDictionaryLanguageString:in avulla.
	FmiLanguage itsLanguage; 
	std::string itsEditorVersionStr; // tähän talletetaan versio teksti, mikä logataan editoria käynnistettäessä ja suljettaessa, muotoa: SmartMet version 5.8.7.1 (06 Aug 2012)
	std::string itsApplicationTitle; // Tämä teksti näkyy mainframessa ohjelman nimenä, saadaan winappin -t -argumenttina
	bool fDeveloperModePath; // oletus arvoisesti false, jolloin itsWorkingPath kiipeää yhden askeleen ylöspäin hakemistopuussa.
							// Normaalisti kun SmartMet (5.4:sta alkaen) käynnistetään, lukee se halutut konfiguraatiot workingPath + "\..\" + controlPath -hakemistosta.
							// Tämä siksi että 5.4:sta alkaen 32 ja 64-bit ohjelmat ovat samassa SmartMet rakenteessa, mutta omissa bin-hakemistoissaan.
							// Nämä bin-hakikset ovat myös alun perin workingPath:eja. Mutta niistä pitää siis tulla yksi taso ylöspäin että päästään oikeaan paikkaan.
							// Kun taas SmartMet käynnistetään MS developer Studiosta, ollaankin jo oikeassa hakemistossa ja ei tarvitse tulla ylöspäin. Eli
							// kun SmartMet:iä (5.4 ->) käynnistetään MSDev:illä, pitää laittaa optio -d!!!!
	NFmiApplicationDataBase itsApplicationDataBase; // tässä on tallessa systeemi ja muita tietoja, joita SmartMet lähettää SmartMet-käyttötietokantaan 
													// aina käynnistyessä, kerran päivässä ja sulkeutuessa. Tämä pitää ottaa talteen, koska siinä oleva GUID
													// luodaan aina instanssi kohtaisesti, että tiedetään mikä open kuuluu mihinkin update ja close tietoihin.
    bool fVerbose; // Onko lokituksen verbose moodi päällä vai ei 
    std::vector<DrawStringData> itsSplashScreenTextDataVector;
    // Smartmetin käynnissäoloajan laskuri
    NFmiNanoSecondTimer itsElapsedRunningTimer;
    // Onko agx Uniras ToolMaster lisenssi kunnossa, jos on, käytä toolmasteria, muuten älä
    bool fToolMasterAvailable;
    // CrashReporter on systeemi, joka tekee SmartMetin kaatuessa kaatumisraportin, joka voidaan lähettää kehittäjille sähköpostilla.
    // Jossain tapauksissa (kuten Beta tuote automaatio käytössä) tätä ominaisuutta ei haluta päälle. Jos tämän arvo on false, ei ominaisuutta laiteta päälle.
    bool fEnableCrashReporter;
    // Jos SmartMetille halutaan joskus antaa käytetyn betaAutomaatioListan polku komentoriviargumenttina
    std::string itsBetaAutomationListPath;
};
