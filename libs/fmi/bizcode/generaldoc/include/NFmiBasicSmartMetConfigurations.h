#pragma once

// T�m� luokka pit�� sis�ll��n tiettyj� SmartMet -ty�aseman perus konfiguraatioita,
// jotka on tarkoitus alustaa ennen kuin SmartMetin NFmiEditMapGeneralDataDoc -otus 
// alustetaan. 
// N�it� tiettyj� arvoja tarvitaan ennen tuon doc-otuksen alustusta kun alustetaan 
// SmartMetissa k�ytetty� CrashRpt -systeemi� (kaato raportointi systeemi).
// T�m�n luokan pit�� alustaa seuraavat asiat:
// 1. K�ytetyt SmartMet konfiguraatiot, jotka luetaan sitten NFmiSetting luokan avulla. 
// 2. NFmiApplicationDataBase-luokka. 
// 3. CatLog systeemi.

#include "catlog/catlog.h"
#include "NFmiApplicationDataBase.h"
#ifndef UNIX
#include "NFmiCachedRegistryValue.h"
#else
#include "linux_compat.h"
#endif
#include "NFmiMilliSecondTimer.h"
#ifndef UNIX
#include "FmiWin32Helpers.h"
#include "Splasher.h"
#endif

#include "boost/shared_ptr.hpp"

#include <list>

// Kun t�m� poikkeus lent�� initialisoinnin yhteydess�, SmartMet lopettaa toimintansa 'hallitusti'
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
#ifndef UNIX
    const std::vector<DrawStringData>& SplashScreenTextDataVector() const { return itsSplashScreenTextDataVector; }
#endif
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
#ifndef UNIX
    void MakeSplashScreenTextDataVector(const NFmiTime &theExeModTimeLocal, const std::string &theVersionString);
#endif
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


    // SmartMetin ty�hakemisto
	std::string itsWorkingDirectory; 
    // T�m� on -p option arvo kun SmartMet k�ynnistet��n. Siin� voi olla hakemisto (mist� smartmet.conf 
    // luetaan) tai suora tiedoston nimi, jota k�sitell��n kuten smartmet.conf -tiedostoa. T�ll�in itsControlPath
    // on se hakemisto, mik� on polkuna annetulle tiedostolle.
	std::string itsControlBasePath; 
    // T�ss� on haluttu ohjaus-hakemisto, mista kaikki ohjaus tiedostot luetaan, polku on absoluuttinen, 
    // joten ohjaus hakemisto voi olla riippumaton editori hakemistosta
    std::string itsControlPath;
    // T�ss� on smartmet.conf tiedoston koko polku. Se voi olla eri nimell�, jos -p optiossa on annettu
    // suoraan halutun tiedoston nimi.
    std::string itsBaseConfigurationFilePath;
    // T�ss� hakemistossa on ns. tehdasasetus GUI konffeja, eli tiedostoja, joihin SmartMet tekee
    // K�ytt�liittym�n kautta muutoksia. Ne voidaan tuottaa oletus tiedostoista, jos ne puuttuvat, 
    // t�ll�in niihin tehdyt muutokset ovat konekohtaisia.
    // Se on workingDirectory\FactorySettings -hakemisto.
    std::string itsFactorySettingsConfigurationFilePath;
    // T�h�n luetaan tehdasasetus tiedotojen nimet em. hakemistosta.
    std::list<std::string> itsFactorySettingsConfigurationFiles;
    // SmartMet lukee tietyt tiedostot ns. help_data -hakemistosta. T�m� on optionaalinen asetus
    // jonka oletusarvo on itsControlDirectory + "\\help_data".
    // Pilvipalvelu asennuksien mukana on tullut tarve ett� hakemsito voi olla muuallakin kuin workinDirectoryn alla.
    std::string itsHelpDataPath;
    // Otetaan talteen lokitiedostojen hakemisto
    std::string itsLogFileDirectory;

    // T�ss� on k�ytetyn kielen lyhenne stringin�, joka on tallessa Windows rekisteriss�
    boost::shared_ptr<CachedRegString> itsDictionaryLanguageString;
    // Editorin k�ytt�liittym�n kieli, joka saadaan itsDictionaryLanguageString:in avulla.
	FmiLanguage itsLanguage; 
	std::string itsEditorVersionStr; // t�h�n talletetaan versio teksti, mik� logataan editoria k�ynnistett�ess� ja suljettaessa, muotoa: SmartMet version 5.8.7.1 (06 Aug 2012)
	std::string itsApplicationTitle; // T�m� teksti n�kyy mainframessa ohjelman nimen�, saadaan winappin -t -argumenttina
	bool fDeveloperModePath; // oletus arvoisesti false, jolloin itsWorkingPath kiipe�� yhden askeleen yl�sp�in hakemistopuussa.
							// Normaalisti kun SmartMet (5.4:sta alkaen) k�ynnistet��n, lukee se halutut konfiguraatiot workingPath + "\..\" + controlPath -hakemistosta.
							// T�m� siksi ett� 5.4:sta alkaen 32 ja 64-bit ohjelmat ovat samassa SmartMet rakenteessa, mutta omissa bin-hakemistoissaan.
							// N�m� bin-hakikset ovat my�s alun perin workingPath:eja. Mutta niist� pit�� siis tulla yksi taso yl�sp�in ett� p��st��n oikeaan paikkaan.
							// Kun taas SmartMet k�ynnistet��n MS developer Studiosta, ollaankin jo oikeassa hakemistossa ja ei tarvitse tulla yl�sp�in. Eli
							// kun SmartMet:i� (5.4 ->) k�ynnistet��n MSDev:ill�, pit�� laittaa optio -d!!!!
	NFmiApplicationDataBase itsApplicationDataBase; // t�ss� on tallessa systeemi ja muita tietoja, joita SmartMet l�hett�� SmartMet-k�ytt�tietokantaan 
													// aina k�ynnistyess�, kerran p�iv�ss� ja sulkeutuessa. T�m� pit�� ottaa talteen, koska siin� oleva GUID
													// luodaan aina instanssi kohtaisesti, ett� tiedet��n mik� open kuuluu mihinkin update ja close tietoihin.
    bool fVerbose; // Onko lokituksen verbose moodi p��ll� vai ei 
#ifndef UNIX
    std::vector<DrawStringData> itsSplashScreenTextDataVector;
#endif
    // Smartmetin k�ynniss�oloajan laskuri
    NFmiNanoSecondTimer itsElapsedRunningTimer;
    // Onko agx Uniras ToolMaster lisenssi kunnossa, jos on, k�yt� toolmasteria, muuten �l�
    bool fToolMasterAvailable;
    // CrashReporter on systeemi, joka tekee SmartMetin kaatuessa kaatumisraportin, joka voidaan l�hett�� kehitt�jille s�hk�postilla.
    // Jossain tapauksissa (kuten Beta tuote automaatio k�yt�ss�) t�t� ominaisuutta ei haluta p��lle. Jos t�m�n arvo on false, ei ominaisuutta laiteta p��lle.
    bool fEnableCrashReporter;
    // Jos SmartMetille halutaan joskus antaa k�ytetyn betaAutomaatioListan polku komentoriviargumenttina
    std::string itsBetaAutomationListPath;
};
