#include "LocalCacheCleaning.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiCachedDataFileInfo.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "NFmiCaseStudySystem.h"
#include "catlog/catlog.h"
#include "CtrlViewFunctions.h"
#include "SmartMetDocumentInterface.h"

namespace
{
    // Jos t‰m‰ on false, se est‰‰ datan latauksen cacheen ja hakemistojen siivouksen
    bool gLoadDataAtStartUp = true;
    // Onko SmartMet ns. operatiivisessa moodissa, eli silloin se automaattisesti latailee uusia datoja cacheen ja silloin myˆs vanhoja pit‰‰ siivoilla pois.
    // Joskus halutaan ett‰ uutta dataa ei lueta eik‰ vanhoja siivota, silloin t‰m‰ moodi pit‰‰ laittaa pois p‰‰lt‰.
    // Myˆs jos ollaan ns. no-data/tiputus moodissa, t‰llˆin ei saa tuhota vanhoja tiedostoja.
    bool gAutoLoadNewCacheDataMode = true;
    // T‰h‰n otetaan GeneralDocista siivousintervalli, oletus 0.16 [h] eli n. 10 minuutin v‰lein.
    double gCacheCleaningIntervalInHours = 0.16;

    NFmiMilliSecondTimer gLocalCacheCleanerTimer;
    // Kun ohjelma halutaan sulkea (k‰ytt‰j‰), t‰h‰n tulee tieto asiasta
    std::shared_ptr<NFmiStopFunctor> gStopFunctorPtr;

    // T‰m‰ heitt‰‰ erikoispoikkeuksen, jos k‰ytt‰j‰ on halunnut sulkea ohjelman.
    void CheckIfProgramWantsToStop()
    {
        NFmiQueryDataUtil::CheckIfStopped(gStopFunctorPtr.get());
    }

    std::string DeletedFilesStr(const std::list<std::string>& theDeletedFiles)
    {
        std::string str;
        for(auto filePath : theDeletedFiles)
        {
            if(!str.empty())
                str += "\n";
            boost::replace_all(filePath, "\\/", "\\");
            str += filePath;
        }
        return str;
    }

    void CleanDirectory(const std::string& theDirectory, double theKeepHours)
    {
        try
        {
            std::list<std::string> deletedFiles;
            NFmiFileSystem::CleanDirectory(theDirectory, theKeepHours, &deletedFiles);
            if(deletedFiles.size())
            {
                std::string logStr("QueryDataToLocalCacheLoaderThread - CleanDirectory: ");
                logStr += theDirectory;
                logStr += ", keep-hour: ";
                logStr += NFmiStringTools::Convert(theKeepHours);
                logStr += "\nDeleted files:\n";
                logStr += ::DeletedFilesStr(deletedFiles);
                CatLog::logMessage(logStr, CatLog::Severity::Debug, CatLog::Category::Data);
            }
        }
        catch(...)
        {
            // ei tehd‰ toistaiseksi mit‰‰n...
        }
    }

    void CleanFilePattern(const std::string& theFilePattern, int theKeepMaxFiles)
    {
        try
        {
            std::string logMessageStart("QueryDataToLocalCacheLoaderThread - CleanFilePattern: ");
            logMessageStart += theFilePattern;
            logMessageStart += ",\nkeep-max-files = ";
            logMessageStart += NFmiStringTools::Convert(theKeepMaxFiles);
            logMessageStart += ",\nDeleted files:\n";
            CtrlViewUtils::DeleteFilesWithPatternAndLog(theFilePattern, logMessageStart, CatLog::Severity::Debug, CatLog::Category::Data, theKeepMaxFiles);
        }
        catch(...)
        {
            // ei tehd‰ toistaiseksi mit‰‰n...
        }
    }

    int CalcMaxKeepFileCount(const NFmiHelpDataInfo& helpDataInfo, NFmiCaseStudySystem& caseStudySystem)
    {
        NFmiInfoData::Type dataType = helpDataInfo.DataType();
        if(NFmiInfoData::IsLatestOnlyBasedData(dataType))
        {
            return 1; // n‰it‰ datatyyppeja on turhaa s‰ilˆ‰ 1 enemp‰‰
        }
        else
        {
            auto localCacheFileCount = 3;
            auto* caseStudyDataFile = caseStudySystem.FindCaseStudyDataFile(helpDataInfo.Name());
            if(caseStudyDataFile)
                localCacheFileCount = caseStudyDataFile->DataFileWinRegValues().LocalCacheDataCount();
            return localCacheFileCount;
        }
    }

    // Siivotaan combine-data cachet.
    // N‰m‰ olivat omissa alihakemistoissaan ja niille oli omat keep-files m‰‰r‰t.
    // siivotaan kuitenkin jokaista ali-hakemistoa yhteisen keepMaxDays:in mukaan.
    void CleanCombineDataCache(NFmiHelpDataInfoSystem& theHelpDataSystem)
    {
        CheckIfProgramWantsToStop();
        for(size_t i = 0; i < theHelpDataSystem.DynamicHelpDataInfos().size(); i++)
        {
            NFmiHelpDataInfo& helpData = theHelpDataSystem.DynamicHelpDataInfo(static_cast<int>(i));
            CheckIfProgramWantsToStop();
            if(NFmiCachedDataFileInfo::IsDataCached(helpData) && helpData.IsCombineData())
            {
                double keepDays = theHelpDataSystem.CacheFileKeepMaxDays();
                if(keepDays > 0)
                {
                    NFmiFileString usedFileFilterStr = helpData.UsedFileNameFilter(theHelpDataSystem);
                    std::string cacheDir = usedFileFilterStr.Device();
                    cacheDir += usedFileFilterStr.Path();
                    ::CleanDirectory(cacheDir, keepDays * 24);
                }
                ::CleanFilePattern(helpData.UsedFileNameFilter(theHelpDataSystem), helpData.CombineDataMaxTimeSteps() + 3);
            }
        }
    }

    // HUOM! siivouksessa ei tarkisteta onko jokin data k‰ytˆss‰ vai ei (NFmiHelpDataInfo:n IsEnabled-metodi tarkistus), vanhoja tiedostoja ei 
// j‰tet‰ levyille lojumaan, vaikka joku data on joskus otettu pois k‰ytˆst‰.
    void CleanCache(NFmiHelpDataInfoSystem& theHelpDataSystem)
    {
        CheckIfProgramWantsToStop();
        // 1. siivotaan ensin pois kaikki yli halutun aikam‰‰reen olevat tiedostot
        if(theHelpDataSystem.CacheFileKeepMaxDays() > 0)
            ::CleanDirectory(theHelpDataSystem.LocalDataLocalDirectory(), theHelpDataSystem.CacheFileKeepMaxDays() * 24);
        // 2. siivotaan tmp-hakemistosta kaikki yli puoli tuntia vanhemmat tiedostot (jos ne eiv‰t lukossa), oletetaan
        // ett‰ yhden tiedoston kopiointi ei kest‰ yli puolta tuntia, vaan kyse on jostain virheest‰.
        CheckIfProgramWantsToStop();
        ::CleanDirectory(theHelpDataSystem.LocalDataTmpDirectory(), 0.5);

        auto& caseStudySystem = SmartMetDocumentInterface::GetSmartMetDocumentInterfaceImplementation()->CaseStudySystem();

        // 3. siivotaan pois file-pattern -kohtaisesti ylim‰‰r‰iset tiedostot n-kpl
        for(size_t i = 0; i < theHelpDataSystem.DynamicHelpDataInfos().size(); i++)
        {
            CheckIfProgramWantsToStop();
            NFmiHelpDataInfo& helpDataInfo = theHelpDataSystem.DynamicHelpDataInfo(static_cast<int>(i));
            if(NFmiCachedDataFileInfo::IsDataCached(helpDataInfo) && helpDataInfo.IsCombineData() == false)
                ::CleanFilePattern(helpDataInfo.UsedFileNameFilter(theHelpDataSystem), ::CalcMaxKeepFileCount(helpDataInfo, caseStudySystem));
        }

        // HUOM! yhdistelm‰ datat pit‰‰ siivota t‰ss‰ erillisill‰ asetuksilla ja 
        // hieman eri lailla ja sit‰ varten on oma funktio.
        ::CleanCombineDataCache(theHelpDataSystem);
    }

} // nameless namespace ends

namespace LocalCacheCleaning
{
    void InitLocalCacheCleaning(bool loadDataAtStartUp, bool autoLoadNewCacheDataMode, double cacheCleaningIntervalInHours, std::shared_ptr<NFmiStopFunctor> &stopFunctorPtr)
    {
        gLoadDataAtStartUp = loadDataAtStartUp;
        gAutoLoadNewCacheDataMode = autoLoadNewCacheDataMode;
        gCacheCleaningIntervalInHours = cacheCleaningIntervalInHours;
        gStopFunctorPtr = stopFunctorPtr;
    }

    void DoPossibleLocalCacheCleaning(NFmiHelpDataInfoSystem& helpDataSystem)
    {
        if(!IsDataCopyingRoutinesOn() || !helpDataSystem.DoCleanCache())
        {
//            CatLog::logMessage("Cache cleaning disabled.", CatLog::Severity::Debug, CatLog::Category::Data, true);
            return;
        }

//        CatLog::logMessage("Checking if it's cache cleaning time yet.", CatLog::Severity::Debug, CatLog::Category::Data, true);
        int cleaningIntervalInMS = int(gCacheCleaningIntervalInHours * 60 * 60 * 1000);
        if(gLocalCacheCleanerTimer.CurrentTimeDiffInMSeconds() > cleaningIntervalInMS)
        {
            ::CleanCache(helpDataSystem);
            // Aloitetaan taas uusi siivous ajan lasku
            gLocalCacheCleanerTimer.StartTimer();
        }
    }

    bool LoadDataAtStartUp()
    {
        return gLoadDataAtStartUp;
    }

    void LoadDataAtStartUp(bool newValue)
    {
        gLoadDataAtStartUp = newValue;
    }

    bool AutoLoadNewCacheDataMode()
    {
        return gAutoLoadNewCacheDataMode;
    }

    void AutoLoadNewCacheDataMode(bool newValue)
    {
        gAutoLoadNewCacheDataMode = newValue;
    }

    bool IsDataCopyingRoutinesOn()
    {
        return (gLoadDataAtStartUp && gAutoLoadNewCacheDataMode);
    }

} // LocalCacheCleaning namespace ends
