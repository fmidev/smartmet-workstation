#include "LocalCacheHistoryDataThread.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFileSystem.h"
#include "LocalCacheSingleFileLoaderThread.h"
#include "NFmiCachedDataFileInfo.h"
#include "catlog/catlog.h"

namespace
{
	bool gThreadHasRun = false;
	std::string gThreadName = "History-thread";
	// Kun ohjelma halutaan sulkea (k‰ytt‰j‰), t‰h‰n tulee tieto asiasta
	std::shared_ptr<NFmiStopFunctor> gStopFunctorPtr;

	// T‰m‰ heitt‰‰ erikois poikkeuksen, jos k‰ytt‰j‰ on halunnut sulkea ohjelman.
	void CheckIfProgramWantsToStop()
	{
		NFmiQueryDataUtil::CheckIfStopped(gStopFunctorPtr.get());
	}

	void LogGeneralMessage(const std::string& theThreadNameStr, const std::string& theStartMessage, const std::string& theEndMessage, CatLog::Severity logLevel)
	{
		std::string logStr(theStartMessage);
		logStr += " ";
		logStr += theThreadNameStr;
		logStr += " ";
		logStr += theEndMessage;
		CatLog::logMessage(logStr, logLevel, CatLog::Category::Data);
	}

	// Ei miss‰‰n tilanteessa haluta ladata dataa serverilta lokaali cacheen, 
	// jos se on disabloitu tai se on merkitty CaseStudy legacy dataksi.
	bool IsDataUsed(const NFmiHelpDataInfo& helpDataInfo)
	{
		return helpDataInfo.IsEnabled() && !helpDataInfo.CaseStudyLegacyOnly();
	}

	// Kokeillaan eri pakkaus p‰‰tteit‰ prioriteetti j‰rjestyksess‰, heti kun lˆytyy jotain jollain p‰‰tteell‰, etsint‰ loppuu.
	std::list<std::string> TryToFindPackedFileNameListWithFileFilter(const std::string& theFileFilter)
	{
		const auto& zippedFileExtensions = LocalCacheSingleFileLoaderThread::GetZippedFileExtensions();
		for(auto& zipExtension : zippedFileExtensions)
		{
			auto packedFileNameList = NFmiFileSystem::PatternFiles(theFileFilter + zipExtension);
			if(!packedFileNameList.empty())
				return packedFileNameList;
		}
		return std::list<std::string>();
	}

	// Haetaan joko pakattujen tiedostojen listaa tai ei pakattujen tiedostojen listaa.
	std::pair<std::list<std::string>, bool> GetNewestFileInfoList(const std::string& theFileFilter)
	{
		std::list<std::string> packedFileList = ::TryToFindPackedFileNameListWithFileFilter(theFileFilter);
		if(packedFileList.empty() == false)
			return std::make_pair(packedFileList, true);
		else
		{
			std::list<std::string> fileList = NFmiFileSystem::PatternFiles(theFileFilter);
			return std::make_pair(fileList, false);
		}
	}

	void CollectHistoryDataToCache(const NFmiHelpDataInfo& theDataInfo, const NFmiHelpDataInfoSystem& theHelpDataSystem)
	{
		// aluksi tehd‰‰n vain combine-datojen historiat
		if(theDataInfo.IsCombineData())
		{
			if(CatLog::doTraceLevelLogging())
			{
				std::string debugStr = gThreadName;
				debugStr += ": starting to load history data for: ";
				debugStr += theDataInfo.UsedFileNameFilter(theHelpDataSystem);
				CatLog::logMessage(debugStr, CatLog::Severity::Trace, CatLog::Category::Data);
			}

			std::string usedPattern = theDataInfo.FileNameFilter();
			std::string usedPath = NFmiFileSystem::PathFromPattern(usedPattern);
			std::pair<std::list<std::string>, bool> fileInfoList = ::GetNewestFileInfoList(usedPattern);
			std::list<std::string>& fileList = fileInfoList.first;
			fileList.sort(); // sortataan
			fileList.reverse(); // k‰‰nnet‰‰n j‰rjestys, jolloin jos k‰ytetty YYYYMMDDHHmmss aikaleimaa tiedoston alussa, tulee uusimmat tiedostot alkuun
			size_t counter = 0;
			for(std::list<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
			{
				CheckIfProgramWantsToStop();
				std::string totalFileName = usedPath;
				totalFileName += *it;
				NFmiCachedDataFileInfo cachedDataFileInfo;
				cachedDataFileInfo.itsTotalServerFileName = totalFileName;
				cachedDataFileInfo.fFilePacked = fileInfoList.second;
				LocalCacheSingleFileLoaderThread::MakeRestOfTheFileNames(cachedDataFileInfo, theDataInfo, theHelpDataSystem);
				LocalCacheSingleFileLoaderThread::CopyFileToLocalCache(cachedDataFileInfo, theDataInfo, &gThreadName);
				counter++;
				if(counter > theDataInfo.CombineDataMaxTimeSteps())
					break;
			}
		}
	}

	void CollectAllHistoryDatas(const NFmiHelpDataInfoSystem& theHelpDataSystem)
	{
		const auto& helpDataInfos = theHelpDataSystem.DynamicHelpDataInfos();
		for(const auto& helpDataInfo : helpDataInfos)
		{
			CheckIfProgramWantsToStop();
			if(::IsDataUsed(helpDataInfo))
			{
				::CollectHistoryDataToCache(helpDataInfo, theHelpDataSystem);
			}
		}
	}

} // nameless namespace ends

namespace LocalCacheHistoryDataThread
{
	void DoHistoryThread(std::shared_ptr<NFmiStopFunctor>& stopFunctorPtr, std::shared_ptr<NFmiHelpDataInfoSystem> helpDataInfoSystemPtr)
	{
		if(gThreadHasRun)
		{
			CatLog::logMessage(std::string("DoHistoryThread error - ") + gThreadName + " has been run once already, stopping...", CatLog::Severity::Error, CatLog::Category::Data);
			return;
		}

		gThreadHasRun = true;

		try
		{
			gStopFunctorPtr = stopFunctorPtr;
			::LogGeneralMessage(gThreadName, "DoHistoryThread with", "was started...", CatLog::Severity::Debug);

			// T‰m‰ historia-threadi k‰yd‰‰n siis vain kerran l‰pi
			::CollectAllHistoryDatas(*helpDataInfoSystemPtr);
		}
		catch(...)
		{
		}

		::LogGeneralMessage(gThreadName, "DoHistoryThread with", "is now stopped as requested...", CatLog::Severity::Debug);
	}

} // LocalCacheHistoryDataThread namespace ends
