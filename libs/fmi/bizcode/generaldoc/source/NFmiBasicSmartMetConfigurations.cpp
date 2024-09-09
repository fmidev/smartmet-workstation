
#include "NFmiBasicSmartMetConfigurations.h"
#include "NFmiSettings.h"
#include "NFmiTime.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "NFmiApplicationDataBase.h"
#include "NFmiApplicationWinRegistry.h"
#include "NFmiBetaProductHelperFunctions.h"
#include "NFmiPathUtils.h"

#include <direct.h> // working directory juttuja varten
#include <boost/algorithm/string.hpp>
#include <regex>


static int GetUsedMessageBoxIcon(CatLog::Severity severity)
{
    switch(severity)
    {
    case CatLog::Severity::Info:
        return MB_ICONINFORMATION;
    case CatLog::Severity::Warning:
        return MB_ICONWARNING;
    case CatLog::Severity::Error:
    case CatLog::Severity::Critical:
        return MB_ICONERROR;
    default:
        return MB_ICONQUESTION;
    }
}

static void DoInitializationAbortOrContinueMessageBox(const std::string &errorString, const std::string &titleString, CatLog::Severity severity)
{
    int usedIcon = ::GetUsedMessageBoxIcon(severity);
    std::string finalErrorString = errorString;
    finalErrorString += "\n\n";
    finalErrorString += "Press Cancel to abort, press Ok to continue";
    CatLog::logMessage(finalErrorString, severity, CatLog::Category::Configuration, true);
    if(::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(finalErrorString.c_str()), CA2T(titleString.c_str()), MB_OKCANCEL | usedIcon) == IDCANCEL)
    {
        throw AbortSmartMetInitializationGracefullyException();
    }
}

void NFmiBasicSmartMetConfigurations::DoInitializationAbortMessageBox(const std::string &errorString, const std::string &titleString, bool throwAbortException)
{
    int usedIcon = ::GetUsedMessageBoxIcon(CatLog::Severity::Critical);
    std::string finalErrorString = errorString;
    finalErrorString += "\n\n";
    finalErrorString += "Too severe errors in initialization, press Ok to abort";
    CatLog::logMessage(finalErrorString, CatLog::Severity::Error, CatLog::Category::Configuration, true);
    ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(finalErrorString.c_str()), CA2T(titleString.c_str()), MB_OK | usedIcon);
    if(throwAbortException)
    {
        throw AbortSmartMetInitializationGracefullyException();
    }
}

NFmiBasicSmartMetConfigurations::NFmiBasicSmartMetConfigurations(void)
:itsWorkingDirectory()
,itsControlBasePath()
,itsControlPath("ohjaus")
,itsHelpDataPath()
,itsLogFileDirectory()
,itsDictionaryLanguageString()
,itsLanguage(kEnglish)
,itsEditorVersionStr()
,itsApplicationTitle()
,fDeveloperModePath(false)
,itsApplicationDataBase()
,fVerbose(false)
,itsElapsedRunningTimer()
,fToolMasterAvailable(false)
,fEnableCrashReporter(true)
{
}

NFmiBasicSmartMetConfigurations::~NFmiBasicSmartMetConfigurations(void)
{
}

static FmiLanguage GetLanguageFromString(const std::string & theLanguageAbbreviation)
{
    std::string tmpUpperString = boost::to_upper_copy(theLanguageAbbreviation);
    if(tmpUpperString == "FI")
        return kFinnish;

    // default on englanti, eli ei välitetä vaikka olisi annettu parametrina mitä roskaa
    return kEnglish; 
}


static std::string GetDictionaryFilename(const std::string &languageAbbreviation)
{
    const std::string languagePlacerString = "¤¤";
    const std::string baseName = std::string("Dictionary_") + languagePlacerString + ".conf";
    std::string dictionaryFilename(baseName);
    boost::replace_all(dictionaryFilename, languagePlacerString, languageAbbreviation);
    return dictionaryFilename;
}

std::string NFmiBasicSmartMetConfigurations::MakeDictionaryFilePath() const
{
    std::string dictionaryFilePath = HelpDataPath();
    dictionaryFilePath += kFmiDirectorySeparator;
    dictionaryFilePath += ::GetDictionaryFilename(*itsDictionaryLanguageString);
    return dictionaryFilePath;
}

void NFmiBasicSmartMetConfigurations::LogBasicPaths()
{
    CatLog::logMessage(std::string("WorkingDirectory = ") + itsWorkingDirectory, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("ControlBasePath = ") + itsControlBasePath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("ControlPath = ") + itsControlPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("BaseConfigurationFilePath = ") + itsBaseConfigurationFilePath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
}

void NFmiBasicSmartMetConfigurations::LogOtherPaths()
{
    CatLog::logMessage(std::string("FactorySettingsConfigurationFilePath = ") + itsFactorySettingsConfigurationFilePath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("HelpDataPath = ") + itsHelpDataPath, CatLog::Severity::Info, CatLog::Category::Configuration, true);
    CatLog::logMessage(std::string("LogFileDirectory = ") + itsLogFileDirectory, CatLog::Severity::Info, CatLog::Category::Configuration, true);
}

bool NFmiBasicSmartMetConfigurations::Init(const std::string &avsToolMasterVersion)
{
	GetWorkingDirectory();
	::_chdir(itsWorkingDirectory.c_str()); // kun versiosta 5.4 alkaen exe:t ajetaan 32/64-bit hakemistoistaan, pitää workin directory asettaa tässä oikeaan
    LogBasicPaths();
	// Read all configurations compatible NFmiSettings
	if(!ReadConfigurations())
		return false;

	try
	{
        itsHelpDataPath = NFmiSettings::Optional<std::string>("SmartMet::HelpDataPath", itsControlPath + "\\help_data");
        itsHelpDataPath = PathUtils::makeFixedAbsolutePath(itsHelpDataPath, itsControlPath);

        HKEY usedKey = HKEY_CURRENT_USER;
        std::string sectionName = "\\General";
        std::string baseRegistryPath = NFmiApplicationWinRegistry::MakeBaseRegistryPath();
        // Sanakirja luetaan itsHelpDataPath:ista. 
        // Käytetty kieli talletetaan rekisteriin ja kielen avulla tehdään luettavan sanakirjan tiedostonimi.
        itsDictionaryLanguageString = ::CreateRegValue<CachedRegString>(baseRegistryPath, sectionName, "\\GuiLanguage", usedKey, "EN");
        itsLanguage = ::GetLanguageFromString(*itsDictionaryLanguageString);
        std::string dictionaryFilePath = MakeDictionaryFilePath();

        if(!NFmiSettings::Read(dictionaryFilePath))
        {
            std::string errorStr("Problems while trying to read dictionary file from:\n");
            errorStr += dictionaryFilePath;
            DoInitializationAbortMessageBox(errorStr, "Cannot read dictionary file", false);
            return false;
        }
		NFmiTime::Init(itsLanguage);
	}
	catch(std::exception &e)
	{
        DoInitializationAbortMessageBox(e.what(), "Problems with Settings", false);
		return false;
	}

    // tätä pitää kutsua ennen kuin loggeri alustetaan
	SetEditorVersionStr(); 
    // logger pitää alustaan ennen InitApplicationDataBase -kutsua
    if(!InitLogger())
        return false;
	InitApplicationDataBase(avsToolMasterVersion);
    LogOtherPaths();

	return true;
}

// Palauttaa itsControlBasePath:sta, jossa on täydellinen absoluuttinen polku, sen loppuosion, esim:
// D:\smartmet\MetEditor_5_9\control_scand_saa2_edit_conf -> control_scand_saa2_edit_conf
std::string NFmiBasicSmartMetConfigurations::GetShortConfigurationName(void)
{
    std::string tmpConfName(itsControlBasePath);
    if(!tmpConfName.empty())
    {
        NFmiStringTools::ReplaceAll(tmpConfName, "/", "\\");  // muutetaan mahd. /-merkit \-merkeiksi
        while(tmpConfName.empty() == false && tmpConfName[tmpConfName.size()-1] == '\\')
            tmpConfName.resize(tmpConfName.size()-1); // jos konffinimessä sattui olemaan viimeinen merkki \-merkki, pitää se poistaa
        std::string::size_type pos = tmpConfName.find_last_of('\\');
        if(pos != std::string::npos)
            tmpConfName = std::string(tmpConfName.begin() + pos + 1, tmpConfName.end()); // rakennetaan lopullinen nimi viimeisestä \-merkistä loppuun
    }
    return tmpConfName;
}

static std::string GetLanguageString(FmiLanguage language)
{
    if(language == kFinnish)
        return "FI";
    else if(language == kEnglish)
        return "EN";
    else
        throw std::runtime_error("Error in GetLanguageString: unknown language given.");
}

// Vaihtaa kielen ja sanakirjatiedoston nimen.
void NFmiBasicSmartMetConfigurations::Language(FmiLanguage newValue) 
{
    if(newValue != itsLanguage)
    {
        itsLanguage = newValue;
        *itsDictionaryLanguageString = ::GetLanguageString(itsLanguage);
    }
}


// Funktio tekee caseless testin, että löytyykö kyseinen testisana tiedoston nimestä.
// Jos löytyy, palauta true, jolloin kyseinen konfiguraatiotiedosto ohitetaan.
static bool IsConfigurationFileIgnored(const std::string &theFileName, const std::string &theTestStr)
{
	std::string lcFileName = theFileName;
	NFmiStringTools::LowerCase(lcFileName);
	std::string lcTestStr = theTestStr;
	NFmiStringTools::LowerCase(lcTestStr);
	std::string::size_type pos = lcFileName.find(lcTestStr);
	if(pos == std::string::npos)
		return false; // testi stringiä ei löytynyt, eli ei ohiteta tiedostoa
	return true;
}

void NFmiBasicSmartMetConfigurations::InitFactorySettingFiles()
{
    std::string factorySettingsDefaultPath = itsWorkingDirectory + "\\factory_settings";
    itsFactorySettingsConfigurationFilePath = NFmiSettings::Optional<std::string>("SmartMet::FactorySettingsPath", factorySettingsDefaultPath);
    itsFactorySettingsConfigurationFiles = NFmiFileSystem::DirectoryFiles(itsFactorySettingsConfigurationFilePath);
}

bool NFmiBasicSmartMetConfigurations::IsFactorySettingFile(const std::string &theConfigurationFile, std::string &theFactorySettingFileOut)
{
    for(const auto &fileName : itsFactorySettingsConfigurationFiles)
    {
        auto pos = theConfigurationFile.find(fileName);
        if(pos != std::string::npos)
        {
            theFactorySettingFileOut = fileName;
            return true;
        }
    }

    return false;
}

static bool MakeSureDestinationDirectoryExists(const std::string &theFilePath)
{
    return NFmiFileSystem::CreateDirectory(NFmiFileSystem::PathFromPattern(theFilePath));
}

// Tarkistaa onko annettu theConfigurationFile ns. tehdasasetustiedosto.
// Jos on, tarkista löytyykö sitä annetulta polulta.
// Jos ei löytynyt, kopioi tehdastiedosto haluttuun paikkaan.
void NFmiBasicSmartMetConfigurations::SetupFactorySettingFile(const std::string &theConfigurationFile)
{
    std::string factorySettingFileName;
    if(IsFactorySettingFile(theConfigurationFile, factorySettingFileName))
    {
        std::string factorySettingFilePath = itsFactorySettingsConfigurationFilePath + "\\" + factorySettingFileName;
        if((!NFmiFileSystem::FileExists(theConfigurationFile)) || FileNeedsForcedUpdate(theConfigurationFile, factorySettingFilePath))
        {
            if(::MakeSureDestinationDirectoryExists(theConfigurationFile))
            {
                NFmiFileSystem::CopyFile(factorySettingFilePath, theConfigurationFile);
                LogMessage("" + theConfigurationFile + " was created/updated using FactorySettings", CatLog::Severity::Info, CatLog::Category::Configuration);
            }
        }
    }
}

bool NFmiBasicSmartMetConfigurations::FileNeedsForcedUpdate(const std::string &localConfFilePath, std::string &factorySettingFilePath)
{
    size_t factoryVersion = GetForcedUpdateVersionNumber(factorySettingFilePath);
    size_t localVersion = GetForcedUpdateVersionNumber(localConfFilePath);
    
    if(factoryVersion > localVersion)
    {
        return true;
    }
    return false;
}

size_t NFmiBasicSmartMetConfigurations::GetForcedUpdateVersionNumber(const std::string &filePath)
{
    std::string confString, versionWord, equalSign, versionNumber;
    std::stringstream confStream;

    try
    {
        NFmiFileSystem::ReadFile2String(filePath, confString);
        std::size_t found = confString.find("ForcedUpdateVersion");
        if(found != std::string::npos)
        {
            confStream.str(confString.substr(found));
            confStream >> versionWord >> equalSign >> versionNumber;
        } 
        else
        {
            return 0;
        }

        //Check that version number has 10 digits
        if(std::regex_match(versionNumber, std::regex("^[0-9]{10}$"))) {
            return boost::lexical_cast<size_t>(versionNumber);
        }
        else 
        {
            //Add log info if version numbers were incorrect
            LogMessage("ForcedUpdateVersion numbers were incorrect in " + filePath + ". Check that the format is YYYYMMDDHH", CatLog::Severity::Warning, CatLog::Category::Configuration);
            return 0;
        }    
    }
    catch(std::exception &e)
    {
        throw std::runtime_error(std::string("NFmiBasicSmartMetConfigurations::GetForcedUpdateVersionNumber - failed \n"));
        LogMessage(std::string("Problems with NFmiBasicSmartMetConfigurations::GetForcedUpdateVersionNumber: ") + e.what(), CatLog::Severity::Warning, CatLog::Category::Configuration);
        return 0;
    }
    catch(...)
    {
        throw std::runtime_error(std::string("NFmiBasicSmartMetConfigurations::GetForcedUpdateVersionNumber - failed \n"));
        LogMessage("Problems with NFmiBasicSmartMetConfigurations::GetForcedUpdateVersionNumber!", CatLog::Severity::Warning, CatLog::Category::Configuration);
        return 0;
    }
}

bool NFmiBasicSmartMetConfigurations::ReadConfigurationFileFromAbsolutePath(const std::string &theFilePath)
{
    SetupFactorySettingFile(theFilePath);
    return NFmiSettings::Read(theFilePath);
}

void NFmiBasicSmartMetConfigurations::ReadConfigurationFile(const std::string &theConfigurationFilePath)
{
    bool status = ReadConfigurationFileFromAbsolutePath(PathUtils::getAbsoluteFilePath(theConfigurationFilePath, itsControlPath));
    if(status == false)
        throw std::runtime_error(std::string("Error when reading configuration files: can't open file:\n") + theConfigurationFilePath);
}

bool NFmiBasicSmartMetConfigurations::ReadConfigurations()
{
    std::string fileName;
    try
	{
        // Use an umbrella config file, which contains the names of other config files
        NFmiSettings::Read(itsBaseConfigurationFilePath);

        if(!ReadPreConfigurationSettings())
            return false;
        InitFactorySettingFiles();

		std::string confFileListStr = NFmiSettings::Require<std::string>("SmartMet::ConfigurationFiles");
		std::vector<std::string> rawConfFileNameVector = NFmiStringTools::Split(confFileListStr);

		std::string ignoreFileTestStr; // 32- ja 64-bit winkkareille voidaan tehdä omat konffit, jotka toisessa systeemissä sitten ohitetaan.
										// eli jos win32-sana löytyy konffitiedoston nimestä, se ohitetaan x64-platformilla. Jos 
										// taas tiedoston nimestä löytyy x64, se ohitetaan win32 platformilla.

#ifdef _WIN64  // HUOM! niissä koodeissä missä Windows on mukana, ei saa viitata win32:een pelkästään, koska ATL tuki ilmeisesti definoi WIN32:n aina mukaan oli kyse 32- tai 64-bit koodista
		ignoreFileTestStr = "win32";
#else // WIN32
		ignoreFileTestStr = "x64";
#endif

		for (size_t i=0; i<rawConfFileNameVector.size(); i++)
		{
            fileName = rawConfFileNameVector[i];

			if(::IsConfigurationFileIgnored(fileName, ignoreFileTestStr))
				continue; // skipataan tämä conffi-tiedosto

            ReadConfigurationFile(fileName);
		}
	}
	catch(std::exception &e)
	{
        std::string errorWithFilePath = e.what();
        errorWithFilePath += "\nIn file: ";
        errorWithFilePath += fileName;
        DoInitializationAbortMessageBox(errorWithFilePath, "Problems with Settings", false);
		return false;
	}

	return true;
}

// smartmet.conf tiedostossa voidaan määritellä tietyt konffi tiedostot, jotka on tarkoitus lukea
// ennen kuin normaalit konfiguraatiot luetaan. Näissä konffeissa voidaan tehdä mm. seuraavia asioita:
// 1. Määritellä yhteisiä polku asetuksia, joita käytetään kun varsinaista "SmartMet::ConfigurationFiles" asetuksia käsitellään.
// 2. Täällä voidaan määritellä FactorySettingFiles-polku.
bool NFmiBasicSmartMetConfigurations::ReadPreConfigurationSettings()
{
    try
    {
        std::string preConfigurationFileListStr = NFmiSettings::Optional<std::string>("SmartMet::PreConfigurationFiles", "");
        std::vector<std::string> rawConfFileNameVector = NFmiStringTools::Split(preConfigurationFileListStr);

        for(size_t i = 0; i<rawConfFileNameVector.size(); i++)
        {
            ReadConfigurationFile(rawConfFileNameVector[i]);
        }
    }
    catch(std::exception &e)
    {
        DoInitializationAbortMessageBox(e.what(), "Problems with PreConfigurationSettings", false);
        return false;
    }

    return true;
}

static std::string RemoveLastDirectory(const std::string &theDirectory)
{
	std::string tmpDir = theDirectory;
	tmpDir = NFmiStringTools::ReplaceChars(tmpDir, '/', kFmiDirectorySeparator);
	std::string usedSeparator;
	usedSeparator += kFmiDirectorySeparator;
	std::vector<std::string> dirParts = NFmiStringTools::Split(tmpDir, usedSeparator);
	std::string newDir;
	for(size_t i=0; i<dirParts.size()-1; i++)
	{
		if(i>0)
			newDir += kFmiDirectorySeparator;
		newDir += dirParts[i];
	}
	return newDir;
}

void NFmiBasicSmartMetConfigurations::GetWorkingDirectory(bool fDontInitializeAgain)
{
	if(fDontInitializeAgain && itsWorkingDirectory != std::string(""))
		return ;

	// Otetaan talteen editorin työ-'asema' ja työhakemisto
	static char path[_MAX_PATH];
	int workingDirectoryDrive = ::_getdrive();
	::_getdcwd(workingDirectoryDrive , path, _MAX_PATH );
	itsWorkingDirectory = std::string(path);

	if(fDeveloperModePath == false)
	{ // jos ei olla ns. developer moodissa, pitääkin tulla yksi askel hakemistossa ylöspäin!!
		itsWorkingDirectory = ::RemoveLastDirectory(itsWorkingDirectory);
		if(itsWorkingDirectory.empty() == false)
		{
			if(itsWorkingDirectory[itsWorkingDirectory.size()-1] == kFmiDirectorySeparator)
			{ // jos viimeinen merkki oli hakemisto-viiva, poistetaan se, koska itsWorkingDirectory annetaan ilman sitä
				itsWorkingDirectory.resize(itsWorkingDirectory.size()-1);
			}
		}
	}
}

void NFmiBasicSmartMetConfigurations::DoStartupLogging(const std::string &theAction)
{

    std::string asteriskMarkerLine(itsEditorVersionStr.size() + 1, '*');
    LogMessage(asteriskMarkerLine, CatLog::Severity::Info, CatLog::Category::Configuration);
    std::string actionMessage = std::string(CT2A(AfxGetApp()->m_pszExeName)) + std::string(" ");
    actionMessage += theAction;
    LogMessage(actionMessage, CatLog::Severity::Info, CatLog::Category::Configuration);
	LogMessage(itsEditorVersionStr, CatLog::Severity::Info, CatLog::Category::Configuration);
    LogMessage(asteriskMarkerLine, CatLog::Severity::Info, CatLog::Category::Configuration);
}

#ifdef CreateDirectory
#undef CreateDirectory // pitää poistaa winkkarin tekemiä definejä
#endif

bool NFmiBasicSmartMetConfigurations::InitLogger(void)
{
	try
	{
        itsLogFileDirectory = PathUtils::getFixedAbsolutePathFromSettings("SmartMet::Logger::LogFilePath", itsControlPath);

        std::string logFilePath = itsLogFileDirectory;
        PathUtils::addDirectorySeparatorAtEnd(logFilePath);
        logFilePath += NFmiSettings::Require<std::string>("SmartMet::Logger::LogFileBaseName");
        // How many log messages are stored in memory for logging viewer
        size_t maximumMessagesKeptInMemory = 10000; 
        // ApplicationWinRegistry (where used logging level is) is initialized later on, so we have to put some log level at the start here.
        CatLog::initLogger(logFilePath, maximumMessagesKeptInMemory, CatLog::Severity::Debug);

        DoStartupLogging("starts up.");
        return true;
	}
	catch(std::exception &e)
	{
		std::string errStr("Error while initializing SmartMet logger:\n");
		errStr += e.what();
        DoInitializationAbortMessageBox(errStr, "Error while initializing logger", false);
        return false;
	}
}

void NFmiBasicSmartMetConfigurations::SetEditorVersionStr(void)
{
	std::string appNameStr = CT2A(AfxGetApp()->m_pszExeName);
	std::string appFullNameStr = NFmiApplicationDataBase::GetFullApplicationName();
	std::string appversionStr = NFmiApplicationDataBase::GetFileVersionOfApplication(appFullNameStr);
	std::time_t exeModTime = NFmiFileSystem::FileModificationTime(appFullNameStr);
	NFmiTime exeModTimeLocal(exeModTime);

	NFmiString modTimeStr = exeModTimeLocal.ToStr(" (Build DD Nnn YYYY)", kEnglish);

	itsEditorVersionStr = appNameStr + " version " + appversionStr + modTimeStr; // esim. -> "SmartMet version 5.10.6.0 (Build 22 Feb 2016)")
    std::string shortVersionString = "Version " + appversionStr + modTimeStr; // esim. -> "Version 5.10.6.0 (Build 22 Feb 2016)")

    MakeSplashScreenTextDataVector(exeModTimeLocal, shortVersionString); // tätä pitää kutsua vasta kun itsEditorVersionStr -dataosa on rakennettu
}

void NFmiBasicSmartMetConfigurations::MakeSplashScreenTextDataVector(const NFmiTime &theExeModTimeLocal, const std::string &theVersionString)
{
    // Tehdään dynaamiset tekstit bitmapin päälle
    itsSplashScreenTextDataVector.push_back(DrawStringData(_TEXT("SmartMet Workstation"), _TEXT("Arial"), 37, RGB(0, 0, 0), CPoint(22, 177), true));
    CString versionStringU_ = CA2T(theVersionString.c_str());
    itsSplashScreenTextDataVector.push_back(DrawStringData(versionStringU_, _TEXT("Arial"), 25, RGB(0, 0, 0), CPoint(22, 298), true));
    CString copyrightStringU_ = _TEXT("Copyright 1999-");
    copyrightStringU_ += CA2T(theExeModTimeLocal.ToStr("YYYY", kEnglish));
    copyrightStringU_ += _TEXT(" Finnish Meteorological Institute");
    itsSplashScreenTextDataVector.push_back(DrawStringData(copyrightStringU_, _TEXT("Arial"), 18, RGB(0, 0, 0), CPoint(22, 325), true));

    bool betaVersion = false;
    if(betaVersion)
        itsSplashScreenTextDataVector.push_back(DrawStringData(_TEXT("Beta"), _TEXT("Arial"), 25, RGB(255, 0, 0), CPoint(122, 267), true));

    CString possibleWarningTextU_;// = _TEXT("Mahdollisia ongelmia johtuen kehittimen (VS 2019) päivityksestä");
    if(possibleWarningTextU_.GetLength())
        itsSplashScreenTextDataVector.push_back(DrawStringData(possibleWarningTextU_, _TEXT("Arial"), 25, RGB(255, 0, 0), CPoint(15, 370), true));

    CString possibleWarningText2U_;// = _TEXT("Isoviiva/contour piirtoihin turvallisuus ja optimointi muutoksia");
    if(possibleWarningText2U_.GetLength())
        itsSplashScreenTextDataVector.push_back(DrawStringData(possibleWarningText2U_, _TEXT("Arial"), 25, RGB(255, 0, 0), CPoint(15, 400), true));
}

// Oletus, theControlPath on absoluuttinen polku joko tiedostoon tai hakemistoon.
bool NFmiBasicSmartMetConfigurations::DoControlPathChecks(std::string theControlPath)
{
    theControlPath = PathUtils::simplifyWindowsPath(theControlPath);
    if(NFmiFileSystem::DirectoryExists(theControlPath))
    {
        itsControlPath = theControlPath;
        PathUtils::addDirectorySeparatorAtEnd(itsControlPath);
        itsBaseConfigurationFilePath = itsControlPath + "smartmet.conf";
        return true;
    }
    else if(NFmiFileSystem::FileExists(theControlPath))
    {
        itsBaseConfigurationFilePath = theControlPath;
        NFmiFileString fString(theControlPath);
        itsControlPath = fString.Device() + fString.Path();
        PathUtils::addDirectorySeparatorAtEnd(itsControlPath);
        return true;
    }
    else
    { 
        // else haarassa asetetaan attribuutteja vain virheilmoituksia varten
        itsControlPath = theControlPath;
    }

    return false;
}

bool NFmiBasicSmartMetConfigurations::SetControlPath(const std::string &theControlPath)
{
	itsControlBasePath = theControlPath;
	GetWorkingDirectory();
    if(DoControlPathChecks(PathUtils::getAbsoluteFilePath(theControlPath, itsWorkingDirectory)))
        return true;

    std::string errStr("Given SmartMet control-path nor base-configuration-file:\n'");
    errStr += theControlPath;
    errStr += "'\nAbsolut path:\n'";
    errStr += itsControlPath;
    errStr += "'\n\ndoesn't exist. Is the -p option correct?";
    DoInitializationAbortMessageBox(errStr, "SmartMet control path or base control file doesn't exist", false);
    return false;
}

void NFmiBasicSmartMetConfigurations::LogMessage(const std::string& message, CatLog::Severity severity, CatLog::Category category, bool flushLogger)
{
    CatLog::logMessage(message, severity, category, flushLogger);
}

void NFmiBasicSmartMetConfigurations::LogAndWarnUser(const std::string &theMessageStr, const std::string &theDialogTitleStr, CatLog::Severity severity, CatLog::Category category, bool justLog, bool addAbortOption, bool flushLogger)
{
	LogMessage(theMessageStr, severity, category, flushLogger);
    if(justLog == false)
    {
        if(addAbortOption)
            ::DoInitializationAbortOrContinueMessageBox(theMessageStr, theDialogTitleStr, severity);
        else
        {
            int usedIcon = ::GetUsedMessageBoxIcon(severity);
            ::MessageBox(AfxGetMainWnd()->GetDesktopWindow()->GetSafeHwnd(), CA2T(theMessageStr.c_str()), CA2T(theDialogTitleStr.c_str()), MB_OK | usedIcon);
        }
    }
}

void NFmiBasicSmartMetConfigurations::InitApplicationDataBase(const std::string &avsToolMasterVersion)
{
	try
	{
		itsApplicationDataBase.InitFromSettings("SmartMet::ApplicationDataBase");
		itsApplicationDataBase.ControlBasePath(itsControlBasePath);
		itsApplicationDataBase.CollectSmartMetData(NFmiApplicationDataBase::kStart, itsLanguage, boost::math::iround(RunningTimeInSeconds()), fToolMasterAvailable, nullptr);

        // AVS Toolmaster version is set once outside of NFmiApplicationDataBase class to remove dependency
        itsApplicationDataBase.avstmversion = avsToolMasterVersion;
    }
	catch(std::exception &e)
	{
		std::string errStr("InitApplicationDataBase - Initialization error in configurations: \n");
		errStr += e.what();
		LogAndWarnUser(errStr, "Problems in InitApplicationDataBase.", CatLog::Severity::Error, CatLog::Category::Configuration, false);
	}
}

std::string NFmiBasicSmartMetConfigurations::MakeRunningTimeString(void)
{
    return NFmiMilliSecondTimer::EasyTimeDiffStr(boost::math::iround(itsElapsedRunningTimer.elapsedTimeInSeconds() * 1000), true);
}
