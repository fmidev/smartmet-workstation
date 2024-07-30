#pragma once

#include "FmiCopyingStatus.h"
#include <memory>
#include <string>
#include <vector>

class NFmiHelpDataInfoSystem;
class NFmiHelpDataInfo;
class NFmiStopFunctor;
class NFmiMissingDataOnServerReporter;
struct NFmiCachedDataFileInfo;

namespace LocalCacheSingleFileLoaderThread
{
    void InitSingleFileLoader(std::shared_ptr<NFmiStopFunctor>& stopFunctorPtr, std::shared_ptr<NFmiMissingDataOnServerReporter>& missingDataOnServerReporterPtr, double maxDataFileSizeInMB, const std::string& smartMetBinDirectory, const std::string& smartMetWorkingDirectory);
    CFmiCopyingStatus CopyQueryDataToCache(const NFmiHelpDataInfo& theDataInfo, std::shared_ptr<NFmiHelpDataInfoSystem> helpDataInfoSystemPtr);
    void CloseNow();
    std::string MakeDailyUnpackLogFilePath();
    const std::vector<std::string>& GetZippedFileExtensions();
    void MakeRestOfTheFileNames(NFmiCachedDataFileInfo& theCachedDataFileInfoInOut, const NFmiHelpDataInfo& theDataInfo, const NFmiHelpDataInfoSystem& theHelpDataSystem);
    CFmiCopyingStatus CopyFileToLocalCache(NFmiCachedDataFileInfo& theCachedDataFileInfo, const NFmiHelpDataInfo& theDataInfo, std::string* possibleThreadName = nullptr);
    bool DoesThisThreadCopyFile(NFmiCachedDataFileInfo& theCachedDataFileInfo);
}
