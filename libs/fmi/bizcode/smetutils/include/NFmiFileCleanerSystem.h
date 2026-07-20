// ======================================================================
/*!
 * \file NFmiFileCleanerSystem.h
 * \brief System that handles different kind of MEtEditor data directory cleanings.
 */
// ======================================================================

#pragma once

#include <vector>
#include <string>

// This class keeps track of which directory is meant to be cleaned of **all** files
// (not recursive). It holds the directory path and the age in days, i.e. how many days
// older files are meant to be cleaned away.
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
	// Which directory is cleaned here.
	std::string itsDirectoryPath;

	// How many days older files are meant to be cleaned away.
	// If the number is 0 or negative, no cleaning is done at all.
	// e.g. the value 0.25 is six hours and 2 is 48 hours (i.e. files older than that are cleaned)
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
	// Which file-name pattern is meant to be cleaned (full path included).
	std::string itsFilePattern;

	// How many of the newest files from the given filePattern are always meant to be kept, no
	// matter how old the files are. With negative and 0 values nothing is cleaned at all,
	// and all files are kept.
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

	// NOTE! this can be initialized using the settings of the NFmiSettings class.
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

	// How long a pause is kept between cleaning operations (e.g. every 3 hours or so).
	// If negative/0, nothing is ever done.
	double itsCleaningTimeStepInHours;
	std::vector<NFmiDirectorCleanerInfo> itsDirectoryInfos;
	std::vector<NFmiFilePatternCleanerInfo> itsPatternInfos;
	std::string itsInitializeLogStr;

	std::string itsBaseNameSpace;
};

