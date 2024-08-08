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
        // samaa rataa kuin muutkin datat, tässä tarkistetaan osoittaako 'server' polku oikeasti lokaaliin hakemistoon.
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

bool NFmiMissingDataOnServerReporter::initialize(NFmiHelpDataInfoSystem & helpDataSystem)
{
    if(!initialized_)
    {
        queryDataOnServerCount_ = ::calcDataOnServerCount(helpDataSystem);
        if(queryDataOnServerCount_ > 0)
        {
            initialized_ = true;
            return true;
        }
    }
    return false;
}

// Tämä tekee tai valmistelee puuttuvien datojen raportointia kahdella tasolla:
// 1. Puuttuvan datan lokitus, mutta vain 1. syklin aikana
// 2. Puuttuvan datan nimen talletus, jotta tehdään sykli kohtaisia led-light status raportteja
void NFmiMissingDataOnServerReporter::doReportIfFileFilterHasNoRelatedDataOnServer(const NFmiCachedDataFileInfo& cachedDataFileInfo, const std::string& fileFilter)
{
    if(cachedDataFileInfo.itsTotalServerFileName.empty())
    {
        // Raportoidaan ne tapaukset, kun dataa ei löydy serveriltä. Tämä saattaa auttaa löytämään konffi ongelmia helpommin
        // Tätä funktiota kutsutaan useasta eri threadeista, joten pakko käyttää lukkoa ennen kuin laitetaan yhteiseen containeriin tavaraa
        bool logThisMissingData = insertMissingDataFileFilter(fileFilter);
        if(logThisMissingData)
        {
            CatLog::logMessage(std::string("Found no files from queryData server with filefilter: ") + fileFilter, CatLog::Severity::Debug, CatLog::Category::Data);
        }
    }
}

void NFmiMissingDataOnServerReporter::mainWorkerThreadCompletesCycle()
{
    haveRunThroughFirstCycle_ = true;
    doLedChannelReportOfMissingServerData();
    clearThreadCycleData();
}

void NFmiMissingDataOnServerReporter::doLedChannelReportOfMissingServerData()
{
    auto missingDataCount = (int)missingDataOnServerCount();
    double missingServerDataProcent = 100. * missingDataCount / queryDataOnServerCount_;
    CatLog::Severity missingDataSeverity = CatLog::Severity::Info;
    std::string message = ""; // defaulttina tyhjä viesti
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
    std::lock_guard<std::mutex> lock(fileFiltersWithNoServerDataFilesMutex_);
    fileFiltersWithNoServerDataFiles_.clear();
}

// Lisätään annettu fileFilter puuttuvien sarver datojen listaan.
// Palauttaa myös true, jos asiasta pitää lokittaa, muuten palautetaan false.
bool NFmiMissingDataOnServerReporter::insertMissingDataFileFilter(const std::string& fileFilter)
{
    std::lock_guard<std::mutex> lock(fileFiltersWithNoServerDataFilesMutex_);
    auto insertIter = fileFiltersWithNoServerDataFiles_.insert(fileFilter);
    // Huom! lokitusta tehdään vain 1. pääsyklin aikana
    if(!haveRunThroughFirstCycle_)
    {
        // Raportoidaan vain siis jos on uusi filefiltteri (set luokan insert:in paluu arvon second on true) 
        // jolle ei löydy tiedostoa serveriltä
        if(insertIter.second == true)
            return true;
    }
    return false;
}

size_t NFmiMissingDataOnServerReporter::missingDataOnServerCount()
{
    std::lock_guard<std::mutex> lock(fileFiltersWithNoServerDataFilesMutex_);
    return fileFiltersWithNoServerDataFiles_.size();
}
