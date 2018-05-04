// ======================================================================
/*!
 * \file NFmiFileCleanerSystem.h
 * \brief System that handles different kind of MEtEditor data directory cleanings.
 */
// ======================================================================

#pragma once

#include <vector>

// Luokka pit‰‰ tietoa mit‰ hakemistoa on tarkoitus siivota **kaikista** tiedostoista 
// (ei rekursiivinen). Pit‰‰ sis‰ll‰‰n hakemiston polkua ja vanhuus p‰iv‰‰ eli kuinka monta p‰iv‰‰
// vanhemmat tiedostot on tarkoitus siivota pois.
class NFmiDirectorCleanerInfo
{
public:
	NFmiDirectorCleanerInfo(void)
	:itsDirectoryPath()
	,itsKeepDataDays(-1)
	{
	}

	NFmiDirectorCleanerInfo(const std::string &theDirectoryPath, double theKeepDataDays)
	:itsDirectoryPath(theDirectoryPath)
	,itsKeepDataDays(theKeepDataDays)
	{
	}

	const std::string& DirectoryPath(void) const {return itsDirectoryPath;}
	void DirectoryPath(const std::string &theDirectoryPath) {itsDirectoryPath = theDirectoryPath;}
	void CleanDirectory(void);

private:
	// Mit‰ hakemistoa t‰ss‰ siivotaan.
	std::string itsDirectoryPath;

	// Kuinka monta p‰iv‰‰ vanhemmat tiedostot on tarkoitus siivota pois.
	// Jos luku on 0 tai negatiivinen, ei tehd‰ siivousta lainkaan.
	// esim. luku 0.25 on kuusi tuntia ja 2 on 48 tuntia (eli sit‰ vanhemmat siivotaan)
	double itsKeepDataDays;

};

class NFmiFilePatternCleanerInfo
{
public:
	NFmiFilePatternCleanerInfo(void)
	:itsFilePattern()
	,itsKeepMaxFiles(-1)
	{
	}

	NFmiFilePatternCleanerInfo(const std::string &theFilePattern, int theKeepMaxFiles)
	:itsFilePattern(theFilePattern)
	,itsKeepMaxFiles(theKeepMaxFiles)
	{
	}

	const std::string& FilePattern(void) const {return itsFilePattern;}
	void FilePattern(const std::string &theFilePattern) {itsFilePattern = theFilePattern;}
	void CleanFilePattern(void);

private:
	// Mit‰ tiedostonimi-patternia on tarkoitus siivota (koko polku mukana).
	std::string itsFilePattern;

	// Kuinka monta uusinta tiedostoa kyseisest‰ filePatternista on aina tarkoitus s‰ilytt‰‰ oli 
	// kuinka vanhoja tiedostoja hyv‰ns‰. Negatiivisilla ja 0 arvoilla ei siivota ollenkaan, 
	// vaan pidet‰‰n kaikki tiedostot tallessa.
	int itsKeepMaxFiles;

};


class NFmiFileCleanerSystem
{
public:
	NFmiFileCleanerSystem(void)
	:itsCleaningTimeStepInHours(-1)
	,itsDirectoryInfos()
	,itsPatternInfos()
	,itsInitializeLogStr()
	{
	}

	double CleaningTimeStepInHours(void) const {return itsCleaningTimeStepInHours;}
	void CleaningTimeStepInHours(double newValue) {itsCleaningTimeStepInHours = newValue;}

	// HUOM! t‰m‰n voi initialisoida NFmiSettings-luokan asetuksien avulla.
	void InitFromSettings(const std::string &theInitNameSpace);
	void StoreToSettings(void);
	void Add(const NFmiDirectorCleanerInfo &theDirInfo);
	void Add(const NFmiFilePatternCleanerInfo &theFilePattern);
	bool DoCleaning(void);
	const std::string& GetInitializeLogStr(void) const {return itsInitializeLogStr;}

private:
	void InitDirectoriesFromSettings(void);
	void InitPatternsFromSettings(void);
	bool CleanDirectories(void);
	bool CleanFilePatterns(void);

	// Kuinka pitk‰ tauko pidet‰‰n siivous operaatioiden v‰lill‰ (esim. 3 tunnin v‰lein tms.). 
	// Jos negatiivinen/0, ei tehd‰ koskaan mit‰‰n.
	double itsCleaningTimeStepInHours;
	std::vector<NFmiDirectorCleanerInfo> itsDirectoryInfos;
	std::vector<NFmiFilePatternCleanerInfo> itsPatternInfos;
	std::string itsInitializeLogStr;

	std::string itsBaseNameSpace;
};

