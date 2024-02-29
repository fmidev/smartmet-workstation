// ======================================================================
/*!
 * \file NFmiFileCleanerSystem.cpp
 * \brief System that handles different kind of MEtEditor data directory cleanings.
 */
// ======================================================================

#include "NFmiFileCleanerSystem.h"
#include "NFmiFileSystem.h"
#include "NFmiSettings.h"

#include <algorithm>
#include <functional>

using namespace std;

void NFmiDirectorCleanerInfo::CleanDirectory(void)
{
	try
	{
		NFmiFileSystem::CleanDirectory(this->itsDirectoryPath, this->itsKeepDataDays*24);
	}
	catch(...)
	{
		// ei tehd‰ toistaiseksi mit‰‰n...
	}
}

void NFmiFilePatternCleanerInfo::CleanFilePattern(void)
{
	try
	{
		NFmiFileSystem::CleanFilePattern(this->itsFilePattern, this->itsKeepMaxFiles);
	}
	catch(...)
	{
		// ei tehd‰ toistaiseksi mit‰‰n...
	}
}

// theInitNameSpace on esim. "FileCleanerSystem", t‰m‰n avulla rakennetaan halutut
// asetukset jotka puretaan konffitiedostosta. Huom per‰ss‰ ei saa olla "::" namespace
// merkkej‰, koska ne lis‰t‰‰n. esim.
// FileCleanerSystem::CleaningTimeStepInHours = 3
// FileCleanerSystem::DirectoryInfoCount = 2
// FileCleanerSystem::DirectoryInfoPath_1 = c:\dir1
// FileCleanerSystem::DirectoryInfoFileMaxAgeInDays_1 = 2
// FileCleanerSystem::DirectoryInfoPath_2 = c:\dir2
// FileCleanerSystem::DirectoryInfoFileMaxAgeInDays_2 = 0.5 (= 12 h)
// FileCleanerSystem::PatternInfoCount = 3
// FileCleanerSystem::PatternInfo_1 = c:\dir\myfile1_*.txt
// FileCleanerSystem::PatternInfoFileCount_1 = 4
// FileCleanerSystem::PatternInfo_2 = c:\dir\myfile2_*.txt
// FileCleanerSystem::PatternInfoFileCount_2 = 6
// FileCleanerSystem::PatternInfo_3 = c:\dir\myfile3_*.txt
// FileCleanerSystem::PatternInfoFileCount_3 = 2
// jne.
// throws if there is major problems. There has to certain setting found
// or exception is thrown. But you can tell that there might be 50 as DirectoryInfoCount
// but if only 4 is found that is ok, because now you don't have change counter in
// settings file every time you add or remove directories or patterns.
// Also if there is DirectoryInfoPath_1 but not DirectoryInfoFileMaxAgeInDays_1 then 
// exception is thrown. So there has to be matching pairs.
void NFmiFileCleanerSystem::InitFromSettings(const std::string &theInitNameSpace)
{
	itsInitializeLogStr = "";
	itsCleaningTimeStepInHours = -1;
	itsDirectoryInfos.clear();
	itsPatternInfos.clear();

	try
	{
		itsBaseNameSpace = theInitNameSpace;

		itsCleaningTimeStepInHours = NFmiSettings::Optional<double>(string(itsBaseNameSpace+"::CleaningTimeStepInHours"), 0.16);
		InitDirectoriesFromSettings();
		InitPatternsFromSettings();
		itsInitializeLogStr = "NFmiFileCleanerSystem::InitFromSettings was ok. There will be ";
		itsInitializeLogStr += itsCleaningTimeStepInHours > 0 ? std::string("cleaning every ") + NFmiStringTools::Convert<double>(itsCleaningTimeStepInHours) + " hours." : "no cleanings (CleaningTimeStepInHours had negative or 0 value).";
		itsInitializeLogStr += "\nFound ";
		itsInitializeLogStr += NFmiStringTools::Convert<size_t>(itsDirectoryInfos.size());
		itsInitializeLogStr += " paths and ";
		itsInitializeLogStr += NFmiStringTools::Convert<size_t>(itsPatternInfos.size());
		itsInitializeLogStr += " file-patterns to be cleaned.";
	}
	catch(std::exception &e)
	{
		itsInitializeLogStr = "##ERROR## in NFmiFileCleanerSystem::InitFromSettings\n";
		itsInitializeLogStr += e.what();
		throw std::runtime_error(itsInitializeLogStr);
	}
}

void NFmiFileCleanerSystem::InitDirectoriesFromSettings(void)
{
	int maxDirectoryCount = NFmiSettings::Optional<int>(string(itsBaseNameSpace+"::DirectoryInfoCount"), 0);
	for(int i=1; i <= maxDirectoryCount; i++)
	{
		std::string str1 = itsBaseNameSpace + "::DirectoryInfoPath_";
		str1 += NFmiStringTools::Convert(i);
		std::string str2 = itsBaseNameSpace + "::DirectoryInfoFileMaxAgeInDays_";
		str2 += NFmiStringTools::Convert(i);
		std::string path;
		path = NFmiSettings::Optional(str1, path);
		const double missingAge = -9898.;
		double maxAgeInDays = NFmiSettings::Optional(str2, missingAge);

		// kaksi seuraavaa tarkistavat ett‰ jos vain toinen asetus pareista onnistuttiin initialisoimaan, sitten lent‰‰ poikkeus.
		if(!path.empty() && maxAgeInDays == missingAge)
			throw std::runtime_error("NFmiFileCleanerSystem::InitDirectoriesFromSettings - can't initialize setting: " + str2);
		if(path.empty() && maxAgeInDays != missingAge)
			throw std::runtime_error("NFmiFileCleanerSystem::InitDirectoriesFromSettings - can't initialize setting: " + str1);

		if(!path.empty() && maxAgeInDays != missingAge)
			Add(NFmiDirectorCleanerInfo(path, maxAgeInDays));
	}
}

void NFmiFileCleanerSystem::InitPatternsFromSettings(void)
{
	int maxPatternCount = NFmiSettings::Optional<int>(string(itsBaseNameSpace+"::PatternInfoCount"), 0);
	for(int i=1; i <= maxPatternCount; i++)
	{
		std::string str1 = itsBaseNameSpace + "::PatternInfo_";
		str1 += NFmiStringTools::Convert(i);
		std::string str2 = itsBaseNameSpace + "::PatternInfoFileCount_";
		str2 += NFmiStringTools::Convert(i);
		std::string pattern;
		pattern = NFmiSettings::Optional(str1, pattern);
		const int missingFileCount = -9898;
		int maxFileCount = NFmiSettings::Optional(str2, missingFileCount);

		// kaksi seuraavaa tarkistavat ett‰ jos vain toinen asetus pareista onnistuttiin initialisoimaan, sitten lent‰‰ poikkeus.
		if(!pattern.empty() && maxFileCount == missingFileCount)
			throw std::runtime_error("NFmiFileCleanerSystem::InitPatternsFromSettings - can't initialize setting: " + str2);
		if(pattern.empty() && maxFileCount != missingFileCount)
			throw std::runtime_error("NFmiFileCleanerSystem::InitPatternsFromSettings - can't initialize setting: " + str1);

		if(!pattern.empty() && maxFileCount != missingFileCount)
			Add(NFmiFilePatternCleanerInfo(pattern, maxFileCount));
	}
}

void NFmiFileCleanerSystem::Add(const NFmiDirectorCleanerInfo &theDirInfo)
{
	itsDirectoryInfos.push_back(theDirInfo);
}

void NFmiFileCleanerSystem::Add(const NFmiFilePatternCleanerInfo &theFilePattern)
{
	itsPatternInfos.push_back(theFilePattern);
}

bool NFmiFileCleanerSystem::DoCleaning(void)
{
	if(this->itsCleaningTimeStepInHours < 0)
		return true;
	bool status = CleanDirectories();
	status &= CleanFilePatterns();
	return status;
}

bool NFmiFileCleanerSystem::CleanDirectories(void)
{
	std::for_each(itsDirectoryInfos.begin(), itsDirectoryInfos.end(), [&](auto& directorCleanerInfo) {directorCleanerInfo.CleanDirectory(); });
	return true;
}

bool NFmiFileCleanerSystem::CleanFilePatterns(void)
{
	std::for_each(itsPatternInfos.begin(), itsPatternInfos.end(), [&](auto& filePatternCleanerInfo) {filePatternCleanerInfo.CleanFilePattern(); });
	return true;
}

