#pragma once

#include <memory>
#include <string>

class NFmiHelpDataInfoSystem;
class NFmiHelpDataInfo;
class NFmiStopFunctor;
class NFmiMissingDataOnServerReporter;

enum CFmiCopyingStatus
{
    kFmiNoCopyNeeded = 0,
    kFmiCopyWentOk = 1,
    kFmiCopyNotSuccessfull = 2,
    kFmiGoOnWithCopying = 3,
    kFmiUnpackIsDoneInSeparateProcess = 4
};

namespace LocalCacheSingleFileLoaderThread
{
    void InitSingleFileLoader(std::shared_ptr<NFmiStopFunctor>& stopFunctorPtr, std::shared_ptr<NFmiMissingDataOnServerReporter>& missingDataOnServerReporterPtr, double maxDataFileSizeInMB, const std::string& smartMetBinDirectory, const std::string& smartMetWorkingDirectory);
    CFmiCopyingStatus CopyQueryDataToCache(const NFmiHelpDataInfo& theDataInfo, const NFmiHelpDataInfoSystem& theHelpDataSystem);
    void CloseNow();
    std::string MakeDailyUnpackLogFilePath();
}
