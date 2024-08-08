#include "LocalCacheHistoryDataThread.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiFileSystem.h"
#include "LocalCacheSingleFileLoaderThread.h"
#include "NFmiCachedDataFileInfo.h"
#include "catlog/catlog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace
{
	bool gThreadHasRun = false;
	std::string gThreadName = "HistoryData-1";
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

	void CollectHistoryDataToCache(const NFmiHelpDataInfo& theDataInfo, const NFmiHelpDataInfoSystem& theHelpDataSystem, bool loadOldDataCase, std::string callingThreadName)
	{
		bool doCombineDataCase = !loadOldDataCase && theDataInfo.IsCombineData();
		bool doOldDataCase = loadOldDataCase && NFmiInfoData::IsModelRunBasedData(theDataInfo.DataType());
		// aluksi tehd‰‰n vain combine-datojen historiat
		if(doCombineDataCase || doOldDataCase)
		{
			std::string debugStr = callingThreadName;
			if(doOldDataCase)
				debugStr += ": starting to load older model run data for: ";
			else
				debugStr += ": starting to load combined history data for: ";

			debugStr += theDataInfo.UsedFileNameFilter(theHelpDataSystem);
			CatLog::logMessage(debugStr, CatLog::Severity::Debug, CatLog::Category::Data);

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
				if(LocalCacheSingleFileLoaderThread::DoesThisThreadCopyFile(cachedDataFileInfo))
				{
					LocalCacheSingleFileLoaderThread::CopyFileToLocalCache(cachedDataFileInfo, theDataInfo, callingThreadName);
				}
				counter++;
				if(!doOldDataCase && counter > theDataInfo.CombineDataMaxTimeSteps())
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
				::CollectHistoryDataToCache(helpDataInfo, theHelpDataSystem, false, gThreadName);
			}
		}
	}

} // nameless namespace ends

namespace LocalCacheHistoryDataThread
{
	// HUOM! helpDataInfoSystemPtr parametri on tarkoituksella shared_ptr kopio, ‰l‰ muuta referenssiksi!
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

	// HUOM! helpDataInfoSystemPtr parametri on tarkoituksella shared_ptr kopio, ‰l‰ laita referenssiksi!
	CFmiCopyingStatus CollectOldModelRunDataToCache(const NFmiHelpDataInfo& theDataInfo, std::shared_ptr<NFmiHelpDataInfoSystem> helpDataInfoSystemPtr, std::string callingThreadName)
	{
		::CollectHistoryDataToCache(theDataInfo, *helpDataInfoSystemPtr, true, callingThreadName);
		// T‰m‰n pit‰‰ palauttaa status vain future:a varten, muuta v‰li‰ sill‰ ei ole
		return kFmiCopyWentOk;
	}

} // LocalCacheHistoryDataThread namespace ends
