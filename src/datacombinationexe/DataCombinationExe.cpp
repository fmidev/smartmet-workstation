// Query datojen yhdistely ohjelma saa seuraavat argumentit:
// 0. ohjelman nimi (ei v‰litet‰ t‰st‰)
// 1. work1 ("partial-data-filter,combined-data-filter,time-steps" comma separated values in quates!)
// 2. work2
// 3. ...

#include "NFmiFileString.h"
#include "NFmiLogger.h"
#include "CombineDataFileHelper.h"
#include <boost/algorithm/string.hpp>
#include <thread>

namespace
{
    class CombinationWork
    {
    public:
        std::string partialDataFilter_;
        std::string combinedDataFilter_;
        int timeSteps_ = 2;

        CombinationWork(const std::string &partialDataFilter, const std::string &combinedDataFilter, int timeSteps)
        :partialDataFilter_(partialDataFilter)
        ,combinedDataFilter_(combinedDataFilter)
        ,timeSteps_(timeSteps)
        {}
    };

    using CombinationWorkList = std::vector<CombinationWork>;

    std::shared_ptr<NFmiLogger> g_logger;

    std::string makeDailyTimeStamp()
    {
        NFmiMetTime aTime(1);
        return std::string(aTime.ToStr("YYYYMMDD", kEnglish));
    }

    void makeLogger(const std::string& theLogFilePath)
    {
        if(!theLogFilePath.empty())
        {
            NFmiFileString logFilePathStr(theLogFilePath);
            std::string logFilePath = logFilePathStr.Device() + logFilePathStr.Path();
            std::string logFileBaseName = logFilePathStr.FileName();
            auto dailyTimeStamp = ::makeDailyTimeStamp();
            boost::replace_all(logFileBaseName, "*", dailyTimeStamp);
            bool useBackUpSystem = true;
            bool cleanBackups = true;
            NFmiLogger::Period loggingPeriod = NFmiLogger::kDaily;
            g_logger = std::make_shared<NFmiLogger>(logFilePath, logFileBaseName, useBackUpSystem, cleanBackups, loggingPeriod);
        }
    }

    void logMessage(const std::string& theMessage, NFmiLogger::Level theLogLevel)
    {
        if(g_logger)
            g_logger->LogMessage(theMessage, theLogLevel);
    }

    void logWork(size_t workIndex, const CombinationWork& combinationWork)
    {
        logMessage(std::string("Work ") + std::to_string(workIndex + 1) + ":", NFmiLogger::kInfo);
        logMessage(std::string("partial-data-filter: ") + combinationWork.partialDataFilter_, NFmiLogger::kInfo);
        logMessage(std::string("combined-data-filter: ") + combinationWork.combinedDataFilter_, NFmiLogger::kInfo);
        logMessage(std::string("time-steps: ") + std::to_string(combinationWork.timeSteps_), NFmiLogger::kInfo);
    }

    void logArguments(const std::string& executableName, const CombinationWorkList & combinationWorks)
    {
        auto workCount = std::to_string(combinationWorks.size());
        logMessage(executableName + " starts with " + workCount + " jobs:", NFmiLogger::kInfo);
        logMessage("Work #: partial-data-filter, combined-data-filter, time-steps (header line)", NFmiLogger::kInfo);
        for(size_t workIndex = 0; workIndex < combinationWorks.size(); workIndex++)
        {
            logWork(workIndex, combinationWorks[workIndex]);
        }
    }

    std::string putStringInsideSingleQuates(const std::string &astring)
    {
        std::string inQuates = "'";
        inQuates += astring;
        inQuates += "'";
        return inQuates;
    }

    CombinationWork makeCombinationWorkFromArgument(std::string executableArgument)
    {
        std::vector<std::string> parameterStrings;
        boost::split(parameterStrings, executableArgument, boost::is_any_of(","));
        if(parameterStrings.size() != 3)
        {
            std::string message = "Didn't find 3 comma separated parameters from argument ";
            message += ::putStringInsideSingleQuates(executableArgument);
            throw std::runtime_error(message);
        }

        auto partialDataFilter = parameterStrings[0];
        if(partialDataFilter.empty())
        {
            std::string message = "Partial-data-filter part was empty in argument ";
            message += ::putStringInsideSingleQuates(executableArgument);
            throw std::runtime_error(message);
        }

        auto combinedDataFilter = parameterStrings[1];
        if(combinedDataFilter.empty())
        {
            std::string message = "Combined-data-filter part was empty in argument ";
            message += ::putStringInsideSingleQuates(executableArgument);
            throw std::runtime_error(message);
        }

        // stoi heitt‰‰ poikkeuksen jos ep‰onnistuu
        int timeSteps = std::stoi(parameterStrings[2]);
        return CombinationWork(partialDataFilter, combinedDataFilter, timeSteps);
    }
}

int main(int argc, const char* argv[]) 
{
    NFmiFileString exeFileNameStr(argv[0]);
    std::string exeName = exeFileNameStr.FileName();
    if(argc <= 2)
    {
        std::cout << "Error when executing " << exeName << ", not enough arguments, atleast 2 required:" << std::endl;
        std::cout << "1. log-file-path (e.g. C:\\log\\data_combination_log_*.txt, '*' will be replace with generated time-stamp)" << std::endl;
        std::cout << "2. combination-work-1" << std::endl;
        std::cout << "work-1 contains \"partial-filter,combined-filter,time-steps\"" << std::endl;
        std::cout << "'work' must be inside quates (\"\") and contain 3 comma separated parameters" << std::endl;
        std::cout << "Sample: \"C:\\data\\partial\\radar\\*_radar.sqd,C:\\data\\cached\\radar\\*_radar.sqd,65" << std::endl;
        std::cout << "3. optional combination-work-2" << std::endl;
        std::cout << "4.-n. optional work-3,4,...,n-1" << std::endl;
    }
    else
    {
        try
        {
            std::string logFilePath = argv[1];
            ::makeLogger(logFilePath);
            CombinationWorkList combinationWorks;
            for(int index = 2; index < argc; index++)
            {
                try
                {
                    combinationWorks.push_back(::makeCombinationWorkFromArgument(argv[index]));
                }
                catch(std::exception& e)
                {
                    std::string errorMessage = "Error with work argument ";
                    errorMessage += std::to_string(index - 1);
                    errorMessage += ": ";
                    errorMessage += e.what();
                    ::logMessage(errorMessage, NFmiLogger::kError);
                }
            }

            ::logArguments(exeName, combinationWorks);

            LoggingFunction loggingFunction = [&](const std::string& message)
            {
                ::logMessage(message, NFmiLogger::kInfo);
            };

            bool doLongDebugWait = false;
            for(const auto& combinationWork : combinationWorks)
            {
                // Kaikki tyˆt tehd‰‰n rebuild tyyliin, se on varmaa ja helpottaa logiikkaa
                // ja SSD aikakaudella se ei rasita levy I/O:n puolesta paljoakaan.
                CombineDataFileHelper combineDataFileHelper(combinationWork.partialDataFilter_, combinationWork.combinedDataFilter_, combinationWork.timeSteps_, true, loggingFunction);
                combineDataFileHelper.doDataCombinationWork();

                if(combinationWork.timeSteps_ == 99999)
                    doLongDebugWait = true;
            }

            if(doLongDebugWait)
                std::this_thread::sleep_for(std::chrono::minutes(10));

            ::logMessage("Successfull exit", NFmiLogger::kInfo);
            return 0; // onnistunut ulostulo
        }
        catch(std::exception &e)
        {
            ::logMessage(e.what(), NFmiLogger::kError);
        }
        catch(...)
        {
            ::logMessage(std::string("Unknow error with ") + exeName, NFmiLogger::kError);
        }
    }

    ::logMessage("Failed exit", NFmiLogger::kInfo);
    return 1; // virheellinen ulostulo
} 

/*

#include "NFmiFileSystem.h"
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"

void printParamInfo(const NFmiParam& param, NFmiFastQueryInfo& fastInfo, const std::string& fileName, bool* thisDataInfoPrinted)
{
    if(param.InterpolationMethod() == kLinearly)
    {
        if(*thisDataInfoPrinted == false)
        {
            *thisDataInfoPrinted = true;
            double xResoInKm = (fastInfo.Area()->WorldRect().Width() / fastInfo.GridXNumber()) / 1000.;
            double yResoInKm = (fastInfo.Area()->WorldRect().Height() / fastInfo.GridYNumber()) / 1000.;
            std::cout << fileName << " resoluutio [km] X = " << xResoInKm << " Y = " << yResoInKm << std::endl;
        }

        std::cout << "Param id = " << param.GetIdent() << ", name = " << param.GetName().CharPtr() << ", precision = " << param.Precision().CharPtr() << std::endl;
    }
}

bool ishighPrecision(std::string precisionStr)
{
    auto pos = precisionStr.find('.');
    if(pos != std::string::npos)
    {
        auto nextChar = precisionStr[pos + 1];
        if(nextChar >= '2')
            return true;
    }
    return false;
}

void printLessPrecisionParameters(const std::string& filePath, const std::string& fileName)
{
    NFmiQueryData qdata(filePath);
    if(qdata.IsGrid())
    {
        bool thisDataInfoPrinted = false;
        NFmiFastQueryInfo fastInfo(&qdata);
        for(fastInfo.ResetParam(); fastInfo.NextParam(); )
        {
            auto param = fastInfo.Param().GetParam();
            if(param)
            {
                if(!ishighPrecision(std::string(param->Precision())))
                {
                    printParamInfo(*param, fastInfo, fileName, &thisDataInfoPrinted);
                }
            }
        }
    }
}

int main(int argc, const char* argv[])
{
    std::string basePath = "D:\\SmartMet\\wrk\\data\\local\\";
    std::string filePattern = basePath + "*.?qd";
    auto files = NFmiFileSystem::PatternFiles(filePattern);
    for(const auto& fileName : files)
    {
        auto finalFilePath = basePath + fileName;
        printLessPrecisionParameters(finalFilePath, fileName);
    }
}

*/
