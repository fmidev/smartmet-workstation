#include "CombineDataFileHelper.h"
#include "NFmiFileSystem.h"
#include "NFmiStreamQueryData.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiQueryDataUtil.h"

#include "boost\algorithm\string\replace.hpp"

namespace
{
	void cleanCombinedDataDirectory(const std::string& combinedDataFileFilter)
	{
		auto combinedDataDirectory = NFmiFileSystem::PathFromPattern(combinedDataFileFilter);
		// Poistetaan yli 5 tuntia vanhat tiedostot pois
		NFmiFileSystem::CleanDirectory(combinedDataDirectory, 5);
		// Poistetaan viel‰ muut paitsi 3 viimeist‰ tiedostoa, jotka osuvat filefilteriin
		NFmiFileSystem::CleanFilePattern(combinedDataFileFilter, 1);
	}

	void createCombinedDataDirectory(const std::string& combinedDataFileFilter)
	{
		// varmistetaan ett‰ kohde hakemisto on olemassa
		auto combinedDataDirectory = NFmiFileSystem::PathFromPattern(combinedDataFileFilter);
		NFmiFileSystem::CreateDirectory(combinedDataDirectory);
	}
}

CombineDataFileHelper::CombineDataFileHelper(const std::string& partialDataFileFilter, const std::string& combinedDataFileFilter, int maxTimeSteps, bool doRebuildCheck, LoggingFunction& loggingFunction)
	:partialDataFileFilter_(partialDataFileFilter)
	,combinedDataFileFilter_(combinedDataFileFilter)
	,maxTimeSteps_(maxTimeSteps)
	,doRebuildCheck_(doRebuildCheck)
	,loggingFunction_(loggingFunction)
{
	initTemporaryDataFileFilter();
	fileNameTimeStamp_ = makeFileTimeStamp();
}

void CombineDataFileHelper::doDataCombinationWork()
{
	// Mitataan datanyhdistelyyn menev‰ aika
	NFmiNanoSecondTimer timer;

	// 1. Yritet‰‰n ensin vanhojen tmp-tiedostojen siivousta varmuuden vuoksi
	tryCleaningUpTemporaryFiles();
	if(isDataCombinationNeeded())
	{
		// 4. parametri on true => tehd‰‰n aika-askel yhdistelty‰ dataa
		std::unique_ptr<NFmiQueryData> data(NFmiQueryDataUtil::CombineQueryDatas(doRebuildCheck_, combinedDataFileFilter_, partialDataFileFilter_, true, maxTimeSteps_, nullptr, &loggingFunction_));
		if(data)
		{
			::createCombinedDataDirectory(combinedDataFileFilter_);
			if(tryToStoreCombinedDataToFile(*data))
			{
				std::string timerMessage = "Building ";
				timerMessage += combinedDataFileFilter_;
				timerMessage += " lasted ";
				timerMessage += timer.elapsedTimeInSecondsString();
				logMessage(timerMessage);
			}
		}
		else
		{
			std::string dataCreationFailedMessage = "Unable to create combined data for ";
			dataCreationFailedMessage += combinedDataFileFilter_;
			logMessage(dataCreationFailedMessage);

		}
	}
	else
	{
		if(!temporaryFileIsBlocking_)
		{
			std::string noNeedToCombineDataMessage = "No need to create combined data for ";
			noNeedToCombineDataMessage += combinedDataFileFilter_;
			noNeedToCombineDataMessage += ", some other process may have done this job already?";
			logMessage(noNeedToCombineDataMessage);
		}
	}
}

// T‰t‰ on tarkoitus kutsua ennnen kuin tehd‰‰n isDataCombinationNeeded metodin testit.
void CombineDataFileHelper::tryCleaningUpTemporaryFiles() const
{
	std::string basePath = NFmiFileSystem::PathFromPattern(temporaryDataFileFilter_);
	doFinalTemporaryFileCleaning(temporaryDataFileFilter_, basePath);
}

void CombineDataFileHelper::doFinalTemporaryFileCleaning(const std::string& temporaryFileFilter, const std::string& basePath) const
{
	auto files = NFmiFileSystem::PatternFiles(temporaryFileFilter);
	for(const auto& fileName : files)
	{
		auto finalFilePath = basePath + fileName;
		if(NFmiFileSystem::RemoveFile(finalFilePath))
			logMessage(std::string("Deleted temporary file: ") + finalFilePath);
		else
			logMessage(std::string("Cannot delete temporary file: ") + finalFilePath);
	}
}

bool CombineDataFileHelper::checkIfDataCombinationIsNeeded(const std::string& partialDataFileFilter, const std::string& combinedDataFileFilter, std::time_t* lastCheckedPartialDataFileTimeInOut)
{
	NFmiQueryDataUtil::LoggingFunction loggingFunction;
	// Esitarkasteluvaiheessa ei tarvitse antaa oikeita arvoja time-steps (15) ja doRebuild (false) parametreille.
	CombineDataFileHelper combinationChecker(partialDataFileFilter, combinedDataFileFilter, 15, false, loggingFunction);
	combinationChecker.tryCleaningUpTemporaryFiles();
	return combinationChecker.isDataCombinationNeeded(lastCheckedPartialDataFileTimeInOut);
}

static bool doPartialDataFileTimeChecks(std::time_t newestPartialDataFileTime, std::time_t* lastCheckedPartialDataFileTimeInOut)
{
	if(lastCheckedPartialDataFileTimeInOut)
	{
		if(newestPartialDataFileTime > *lastCheckedPartialDataFileTimeInOut)
		{
			*lastCheckedPartialDataFileTimeInOut = newestPartialDataFileTime;
			return true;
		}
		else
			return false;
	}

	return true;
}

// T‰t‰ on tarkoitus kutsua vasta kun tryCleaningUpTemporaryFiles metodia on kutsuttu.
bool CombineDataFileHelper::isDataCombinationNeeded(std::time_t* lastCheckedPartialDataFileTimeInOut) const
{
	// Katsotaan lˆytyykˆ rakenteilla olevaa temporary tiedostoa ensin
	if(NFmiFileSystem::NewestPatternFileTime(temporaryDataFileFilter_) == 0)
	{
		auto newestPartialDataFileTime = NFmiFileSystem::NewestPatternFileTime(partialDataFileFilter_);
		if(newestPartialDataFileTime) // lˆytyikˆ tiedostoja ollenkaan?
		{
			if(::doPartialDataFileTimeChecks(newestPartialDataFileTime, lastCheckedPartialDataFileTimeInOut))
			{
				if(newestPartialDataFileTime > NFmiFileSystem::NewestPatternFileTime(combinedDataFileFilter_))
					return true;
			}
		}
	}
	else
	{
		auto fileName = NFmiFileSystem::NewestPatternFileName(temporaryDataFileFilter_);
		logMessage(std::string("There was temporary file blocking combined data creation: ") + fileName);
		temporaryFileIsBlocking_ = true;
		return false;
	}

	return false;
}

bool CombineDataFileHelper::tryToStoreCombinedDataToFile(NFmiQueryData& queryData)
{
	std::string finalCombinedDataFilePath = combinedDataFileFilter_;
	boost::replace_all(finalCombinedDataFilePath, "*", fileNameTimeStamp_);
	std::string finalTemporaryDataFilePath = temporaryDataFileFilter_;
	boost::replace_all(finalTemporaryDataFilePath, "*", fileNameTimeStamp_);
	NFmiStreamQueryData sQueryData;
	if(sQueryData.WriteData(finalTemporaryDataFilePath, &queryData, static_cast<long>(queryData.InfoVersion())))
	{
		if(NFmiFileSystem::RenameFile(finalTemporaryDataFilePath, finalCombinedDataFilePath))
		{
			std::string message = "Stored combined data to file: ";
			message += finalCombinedDataFilePath;
			logMessage(message);
			::cleanCombinedDataDirectory(combinedDataFileFilter_);
			return true;
		}
		else
		{
			std::string message = "Unable to rename temporary data file from ";
			message += finalTemporaryDataFilePath;
			message += " to ";
			message += finalCombinedDataFilePath;
			logMessage(message);
		}
	}
	else
	{
		std::string message = "Unable to create/write temporary data file ";
		message += finalTemporaryDataFilePath;
		logMessage(message);
	}

	return false;
}

void CombineDataFileHelper::initTemporaryDataFileFilter()
{
	temporaryDataFileFilter_ = combinedDataFileFilter_ + temporaryDataFileNameExtension_;
}

// Esim. 202102121114 (YYYYMMDDHHmm lokaaliajasta)
std::string CombineDataFileHelper::makeFileTimeStamp()
{
	NFmiStaticTime currentTime;
	// Tehd‰‰n minuutin tarkkuudella aikaleima
	return std::string(currentTime.ToStr(kYYYYMMDDHHMM));
}

void CombineDataFileHelper::logMessage(const std::string& message) const
{
	if(loggingFunction_)
		loggingFunction_(message);
}

