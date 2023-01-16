#include "stdafx.h"
#include "NFmiMissingDataOnServerReporter.h"
#include "NFmiHelpDataInfo.h"
#include "SmartMetThreads_resource.h"
#include "NFmiLedLightStatus.h"
#include "NFmiValueString.h"
#include "NFmiCachedDataFileInfo.h"

namespace
{
    bool isOriginalDataOnLocalCaheDirectory(const NFmiHelpDataInfo& helpDataInfo, const std::string& localCacheBaseDirectory)
    {
        // Jotkut datat generoidaan lokaaliin cacheen (combined- ja generoidut soundingIndex-datat) ja sitten ne ladataan 
        // samaa rataa kuin muutkin datat, t‰ss‰ tarkistetaan osoittaako 'server' polku oikeasti lokaaliin hakemistoon.
        auto searchLocalCacheDirectoryIter = helpDataInfo.FileNameFilter().find(localCacheBaseDirectory);
        return searchLocalCacheDirectoryIter != std::string::npos;
    }

    int calcDataOnServerCount(NFmiHelpDataInfoSystem& helpDataSystem)
    {
        int queryDataOnServerCount = 0;
        const auto& helpDataInfoVector = helpDataSystem.DynamicHelpDataInfos();
        for(size_t index = 0; index < helpDataInfoVector.size(); index++)
        {
            const auto& helpDataInfo = helpDataInfoVector[index];
            if(helpDataInfo.IsEnabled() && NFmiCachedDataFileInfo::IsDataCached(helpDataInfo))
            {
                if(!::isOriginalDataOnLocalCaheDirectory(helpDataInfo, helpDataSystem.LocalDataBaseDirectory()))
                {
                    queryDataOnServerCount++;
                }
            }
        }
        return queryDataOnServerCount;
    }

}

NFmiMissingDataOnServerReporter::NFmiMissingDataOnServerReporter() = default;

bool NFmiMissingDataOnServerReporter::initialize(NFmiHelpDataInfoSystem & helpDataSystem, int workerThreadCount)
{
    if(!initialized_)
    {
        workerThreadCount_ = workerThreadCount;
        queryDataOnServerCount_ = ::calcDataOnServerCount(helpDataSystem);
        if(queryDataOnServerCount_ > 0)
        {
            initialized_ = true;
            return true;
        }
    }
    return false;
}

// T‰m‰ tekee tai valmistelee puuttuvien datojen raportointia kahdella tasolla:
// 1. Puuttuvan datan lokitus, mutta vain 1. syklin aikana
// 2. Puuttuvan datan nimen talletus, jotta tehd‰‰n sykli kohtaisia led-light status raportteja
void NFmiMissingDataOnServerReporter::doReportIfFileFilterHasNoRelatedDataOnServer(const NFmiCachedDataFileInfo& cachedDataFileInfo, const std::string& fileFilter)
{
    if(cachedDataFileInfo.itsTotalServerFileName.empty())
    {
        // Raportoidaan ne tapaukset, kun dataa ei lˆydy serverilt‰. T‰m‰ saattaa auttaa lˆyt‰m‰‰n konffi ongelmia helpommin
        // T‰t‰ funktiota kutsutaan useasta eri threadeista, joten pakko k‰ytt‰‰ lukkoa ennen kuin laitetaan yhteiseen containeriin tavaraa
        bool logThisMissingData = insertMissingDataFileFilter(fileFilter);
        if(logThisMissingData)
        {
            CatLog::logMessage(std::string("Found no files from queryData server with filefilter: ") + fileFilter, CatLog::Severity::Debug, CatLog::Category::Data);
        }
    }
}

void NFmiMissingDataOnServerReporter::workerThreadCompletesCycle(const std::string& workerThreadName)
{
    {
        // Tehd‰‰n omassa blokissa, jotta lukko aukeaa heti kuin mahdollista
        std::lock_guard<std::mutex> lock(loaderThreadsThatHaveRunThroughCycleMutex_);
        loaderThreadsThatHaveRunThroughCycle_.insert(workerThreadName);
    }

    if(isCycleCompletedForAllThreads())
    {
        // N‰m‰ rutiinit pit‰‰ lukita erikseen, jotta ei mene p‰‰llekk‰isi‰ lopetuksia
        std::lock_guard<std::mutex> lock(cycleCompletionMutex_);
        startHistoryLoaderThreadOnce();
        doLedChannelReportOfMissingServerData();
        clearThreadCycleData();
    }
}

void NFmiMissingDataOnServerReporter::startHistoryLoaderThreadOnce()
{
    if(!allThreadsHaveRunThroughFirstCycle_)
    {
        allThreadsHaveRunThroughFirstCycle_ = true;
        AfxGetMainWnd()->PostMessage(ID_MESSAGE_START_HISTORY_THREAD);
    }
}

void NFmiMissingDataOnServerReporter::doLedChannelReportOfMissingServerData()
{
    auto missingDataCount = (int)missingDataOnServerCount();
    double missingServerDataProcent = 100. * missingDataCount / queryDataOnServerCount_;
    CatLog::Severity missingDataSeverity = CatLog::Severity::Info;
    std::string message = ""; // defaulttina tyhj‰ viesti
    if(missingServerDataProcent >= 95.)
    {
        if(missingServerDataProcent >= 100.)
            message = "None of the " + std::to_string(queryDataOnServerCount_) + " queryData from the data-server were available";
        else
            message = NFmiValueString::GetStringWithMaxDecimalsSmartWay(missingServerDataProcent, 1) + " % of the queryData from the data-server weren't available";
        missingDataSeverity = CatLog::Severity::Error;
    }
    else if(missingServerDataProcent >= 75.)
    {
        message = NFmiValueString::GetStringWithMaxDecimalsSmartWay(missingServerDataProcent, 1) + " % of the queryData from the data-server weren't available";
        missingDataSeverity = CatLog::Severity::Warning;
    }

    std::string reporterName = "MissingServerData";
    if(missingDataSeverity >= CatLog::Severity::Warning)
        NFmiLedLightStatusSystem::ReportToChannelFromThread(NFmiLedChannel::OperationalInfo, reporterName, message, missingDataSeverity);
    else
        NFmiLedLightStatusSystem::StopReportToChannelFromThread(NFmiLedChannel::OperationalInfo, reporterName);
}


void NFmiMissingDataOnServerReporter::clearThreadCycleData()
{
    {
        // Tehd‰‰n omassa blokissa, jotta lukko aukeaa heti kuin mahdollista
        std::lock_guard<std::mutex> lock(fileFiltersWithNoServerDataFilesMutex_);
        fileFiltersWithNoServerDataFiles_.clear();
    }

    {
        // Tehd‰‰n omassa blokissa, jotta lukko aukeaa heti kuin mahdollista
        std::lock_guard<std::mutex> lock(loaderThreadsThatHaveRunThroughCycleMutex_);
        loaderThreadsThatHaveRunThroughCycle_.clear();
    }
}

// Lis‰t‰‰n annettu fileFilter puuttuvien sarver datojen listaan.
// Palauttaa myˆs true, jos asiasta pit‰‰ lokittaa, muuten palautetaan false.
bool NFmiMissingDataOnServerReporter::insertMissingDataFileFilter(const std::string& fileFilter)
{
    std::lock_guard<std::mutex> lock(fileFiltersWithNoServerDataFilesMutex_);
    auto insertIter = fileFiltersWithNoServerDataFiles_.insert(fileFilter);
    // Huom! lokitusta tehd‰‰n vain 1. p‰‰syklin aikana
    if(!allThreadsHaveRunThroughFirstCycle_)
    {
        // Raportoidaan vain siis jos on uusi filefiltteri (set luokan insert:in paluu arvon second on true) 
        // jolle ei lˆydy tiedostoa serverilt‰
        if(insertIter.second == true)
            return true;
    }
    return false;
}

size_t NFmiMissingDataOnServerReporter::completedCycleThreadCount()
{
    std::lock_guard<std::mutex> lock(loaderThreadsThatHaveRunThroughCycleMutex_);
    return loaderThreadsThatHaveRunThroughCycle_.size();
}

size_t NFmiMissingDataOnServerReporter::missingDataOnServerCount()
{
    std::lock_guard<std::mutex> lock(fileFiltersWithNoServerDataFilesMutex_);
    return fileFiltersWithNoServerDataFiles_.size();
}

bool NFmiMissingDataOnServerReporter::isCycleCompletedForAllThreads()
{
    return workerThreadCount_ <= completedCycleThreadCount();
}
