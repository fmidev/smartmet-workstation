// Ohjelma saa seuraavat argumentit:
// 0. ohjelman nimi (ei v‰litet‰ t‰st‰)
// 1. pakatun l‰hdetiedoston polku
// 2. kohdetiedoston lopullinen siirto polku
// 3. deletoidaanko pakattu l‰hdetiedosto (1=deletoi, 0=ei deletoida)
// 4. 7-zip ohjelman polku, komento (a = pakkaa), optioita (-y silent yes), jos t‰ss‰ on arvoja, k‰ytet‰‰n purkuun 7-zip ohjelmaa
// 5. Optionaalinen lokitiedoston nimi, jota NFmiLogger k‰ytt‰‰, jos argumentti on "" (kaksi lainausmerkki‰), on annettu polku tyhj‰, ja loggausta ei k‰ytet‰

#include "stdafx.h"

#include <vector>
#include <memory>

#include "NFmiFileString.h"
#include "NFmiStringTools.h"
#include "NFmiLogger.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiFileSystem.h"
#include "execute-command-in-separate-process.h"

static void LogMessage(std::shared_ptr<NFmiLogger> &theLogger, const std::string &theMessage, NFmiLogger::Level theLogLevel)
{
    if(theLogger)
        theLogger->LogMessage(theMessage, theLogLevel);
}

static void ReportSuccessfullUnpacking(std::shared_ptr<NFmiLogger> &theLogger, const std::string &thePackedFilePath, const std::string &theUnpackedFilePath, const NFmiMilliSecondTimer &theTimer)
{
    if(theLogger)
    {
        std::string logStr("Unpack file [");
        logStr += theTimer.EasyTimeDiffStr();
        logStr += "]: ";
        logStr += thePackedFilePath;
        logStr += " to file: ";
        logStr += theUnpackedFilePath;
        ::LogMessage(theLogger, logStr, NFmiLogger::kInfo);
    }
}

static void ReportUnsuccessfullUnpacking(std::shared_ptr<NFmiLogger> &theLogger, const std::string &thePackedFilePath, const std::string &theUnpackedFilePath)
{
    if(theLogger)
    {
        std::string errStr("Unknown error when trying to unpack file: ");
        errStr += thePackedFilePath;
        errStr += " to file: ";
        errStr += theUnpackedFilePath;
        ::LogMessage(theLogger, errStr, NFmiLogger::kError);
    }
}

static void LogRenameFailure(std::shared_ptr<NFmiLogger> &theLogger, const std::string &theUnPackedFilePath, const std::string &theUnPackedFinalFilePath)
{
    if(theLogger)
    {
	    std::string logStr("Copying data file was success but renaming tmp file failed\n");
	    logStr += theUnPackedFilePath;
	    logStr += " -> ";
	    logStr += theUnPackedFinalFilePath;
        ::LogMessage(theLogger, logStr, NFmiLogger::kError);
    }
}

static bool MoveFileToFinalDestination(const std::string &theUnPackedFilePath, const std::string &theUnPackedFinalFilePath, std::shared_ptr<NFmiLogger> &theLogger)
{
    // Rename:a data-tiedosto lopulliseen nimeen ja hakemistoon.
    if(NFmiFileSystem::RenameFile(theUnPackedFilePath, theUnPackedFinalFilePath))
		return true;
	else
	{
        ::LogRenameFailure(theLogger, theUnPackedFilePath, theUnPackedFinalFilePath);
		// jos rename ep‰onnistui, ei poisteta tmp-tiedostoa ainakaan viel‰, ett‰ j‰‰ jotain n‰yttˆ‰ ongelmista
        return false;
	}
}

static std::shared_ptr<NFmiLogger> MakeLogger(const std::string &theLogFilePath)
{
    std::shared_ptr<NFmiLogger> logger;
    if(!theLogFilePath.empty())
    {
        NFmiFileString logFilePathStr(theLogFilePath);
        std::string logFilePath = logFilePathStr.Device() + logFilePathStr.Path();
        std::string logFileBaseName = logFilePathStr.FileName();
        bool useBackUpSystem = true;
        bool cleanBackups = true;
        NFmiLogger::Period loggingPeriod = NFmiLogger::kDaily;
        logger = std::make_shared<NFmiLogger>(logFilePath, logFileBaseName, useBackUpSystem, cleanBackups, loggingPeriod);
    }
    return logger;
}

static std::string GetWorkingDirectory(const std::string &packedFilePath)
{
    NFmiFileString fileString = packedFilePath;
    std::string workingDirectory = fileString.Device();
    workingDirectory += fileString.Path();
    return workingDirectory;
}

static std::string GetUnpackedTmpFilePath(const std::string& packedFilePath)
{
    auto lastDotPos = packedFilePath.find_last_of('.');
    if(lastDotPos != std::string::npos)
        return std::string(packedFilePath.begin(), packedFilePath.begin() + lastDotPos);
    else
    {
        std::string errorMessage = "Error while trying to find packed file's last extension with file-path: ";
        errorMessage += packedFilePath;
        throw std::runtime_error(errorMessage);
    }
}

int main(int argc, const char* argv[]) 
{
    NFmiFileString exeFileNameStr(argv[0]);
    std::string exeName = exeFileNameStr.FileName();
    if(argc < 5)
    {
        std::cout << "Error when executing " << exeName << ", not enough arguments, 6 required (1 optional):" << std::endl;
        std::cout << "1. packed-file-path (e.g. TMP_timestamp_ecmwf_surface.sqd.7z_TMP)" << std::endl;
        std::cout << "2. unpacked-file-final-move-path" << std::endl;
        std::cout << "3. delete-packed-file-after (1=delete, 0=no delete)"  << std::endl;
        std::cout << "4. 7-zip executable path, z-zip is used to unpack packed file" << std::endl;
        std::cout << "5. log-file-path (optional, no need to give)" << std::endl;
    }
    else
    {
        std::string packedFilePath = argv[1];
        std::string unPackedFinalFilePath = argv[2];
        bool deletePackedFileAfter = NFmiStringTools::Convert<bool>(argv[3]);
        std::string _7zipUtilBaseString = argv[4];
        std::string logFilePath;
        if(argc > 5)
            logFilePath = argv[5];
        std::shared_ptr<NFmiLogger> logger = ::MakeLogger(logFilePath);

        try
        {
            // puretaan ensin pakattutiedosto
       	    NFmiMilliSecondTimer timer;
            // 7-zip arguments for unpacking (x is extraction command, -y is option for "silent yes" to all possible questions)
            std::string _7zipUtilExecutionString = "\"" + _7zipUtilBaseString + "\" x -y " + packedFilePath;
            // Pit‰‰ selvitt‰‰ working directory, jotta 7-zip osaa purkaa paketin sinne
            std::string workingDirectory = ::GetWorkingDirectory(packedFilePath);
            std::string unpackedTmpFilePath = ::GetUnpackedTmpFilePath(packedFilePath);

            bool status = CFmiProcessHelpers::ExecuteCommandInSeparateProcess(_7zipUtilExecutionString, false, false, SW_HIDE, true, NORMAL_PRIORITY_CLASS, &workingDirectory);
            timer.StopTimer();
            if(deletePackedFileAfter)
                NFmiFileSystem::RemoveFile(packedFilePath);
            if(status)
            { 
                ::ReportSuccessfullUnpacking(logger, packedFilePath, unpackedTmpFilePath, timer);
                // siirret‰‰n purettu tiedosto viel‰ lopulliseen paikkaan
                if(::MoveFileToFinalDestination(unpackedTmpFilePath, unPackedFinalFilePath, logger))
                    return 0; // onnistunut ulostulo
            }
            else
            {
                ::ReportUnsuccessfullUnpacking(logger, packedFilePath, unpackedTmpFilePath);
            }
        }
        catch(std::exception &e)
        {
            ::LogMessage(logger, e.what(), NFmiLogger::kError);
        }
        catch(...)
        {
            ::LogMessage(logger, std::string("Unknow error in ") + exeName, NFmiLogger::kError);
        }
    }
    return 1; // virheellinen ulostulo
} 

/*
#include <iostream>
#include <regex>
#include <string>

int main() {
    std::string input1 = "SomeWord";               // Should match (word itself)
    std::string input2 = "SomeWord[1h]";           // Should match (word with [1h])
    std::string input3 = "SomeWord[0.5H]";         // Should match (word with [0.5H])
    std::string input4 = "Word_123[10h]";          // Should match (word with [10h])
    std::string input5 = "InvalidWord[10m]";       // Should not match (invalid 'm')
    std::string input6 = "SomeWord[5h]extra";      // Should not match (extra after ])
    std::string input7 = "SomeWord[5h";            // Should not match (missing closing bracket)
    std::string input8 = "Word_with_space [5h]";   // Should not match (space before bracket)

    // Regular expression pattern based on the requirements:
    std::regex pattern(R"(^[A-Za-z0-9_]+(\[([0-9]*\.?[0-9]+[hH])\])?$)");

    // Function to test the match
    auto testRegexMatch = [&](const std::string& input) {
        if(std::regex_match(input, pattern)) {
            std::cout << "Match: " << input << std::endl;
        }
        else {
            std::cout << "No match: " << input << std::endl;
        }
        };

    // Test different input strings
    testRegexMatch(input1);  // Should match (word itself)
    testRegexMatch(input2);  // Should match (word with [1h])
    testRegexMatch(input3);  // Should match (word with [0.5H])
    testRegexMatch(input4);  // Should match (word with [10h])
    testRegexMatch(input5);  // Should not match (invalid 'm')
    testRegexMatch(input6);  // Should not match (extra characters after ])
    testRegexMatch(input7);  // Should not match (missing closing bracket)
    testRegexMatch(input8);  // Should not match (space before bracket)

    return 0;
}
*/
/*
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiQueryDataUtil.h"

static NFmiVPlaceDescriptor MakeReversedVPlaceDescriptor(const NFmiVPlaceDescriptor& sourceVPlaceDescriptor)
{
    auto sourceLevelBag = sourceVPlaceDescriptor.LevelBag();
    NFmiLevelBag reversedLevelBag;
    for(sourceLevelBag.Reset(kBackward); sourceLevelBag.Previous();)
    {
        reversedLevelBag.AddLevel(*sourceLevelBag.Level());
    }
    return NFmiVPlaceDescriptor(reversedLevelBag);
}

int main(int argc, const char* argv[])
{
    // K‰‰nnet‰‰n 1. parametrina annetun querydatan level-j‰rjestys ja talletetaan
    // tulos uuteen 2. parametrina annettuun tiedostoon.
    try
    {
        std::string sourceFilePath = argv[1];
        std::string destinationFilePath = argv[2];
        NFmiQueryData sourceData(sourceFilePath);
        NFmiFastQueryInfo sourceInfo(&sourceData);
        NFmiVPlaceDescriptor reversedLevels = ::MakeReversedVPlaceDescriptor(sourceInfo.VPlaceDescriptor());
        NFmiQueryInfo reversedMetaInfo(sourceInfo.ParamDescriptor(), sourceInfo.TimeDescriptor(), sourceInfo.HPlaceDescriptor(), reversedLevels);
        auto destinationData = NFmiQueryDataUtil::CreateEmptyData(reversedMetaInfo);
        if(!destinationData)
        {
            std::cerr << "Error: Unable to create new querydata" << std::endl;
            return 1;
        }


        NFmiFastQueryInfo destInfo(destinationData);
        for(sourceInfo.ResetLevel(); sourceInfo.NextLevel(); )
        {
            if(destInfo.Level(*sourceInfo.Level()))
            {
                for(sourceInfo.ResetParam(), destInfo.ResetParam(); sourceInfo.NextParam() && destInfo.NextParam(); )
                {
                    for(sourceInfo.ResetLocation(), destInfo.ResetLocation(); sourceInfo.NextLocation() && destInfo.NextLocation(); )
                    {
                        for(sourceInfo.ResetTime(), destInfo.ResetTime(); sourceInfo.NextTime() && destInfo.NextTime(); )
                        {
                            destInfo.FloatValue(sourceInfo.FloatValue());
                        }
                    }
                }
            }
        }

        destinationData->Write(destinationFilePath);

        return 0;
    }
    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
*/
/*
#include "NFmiDrawParam.h"
#include "NFmiFileSystem.h"
#include "NFmiPathUtils.h"
#include <fstream>

int main(int argc, const char* argv[])
{
    // Tulostetaan annetun 1. argumenttina annetun hakemiston drawParam tiedostoista, mik‰
    // on niiden itsStationSymbolColorShadeClassCount arvon jakauma.
    try
    {
        std::string drawParamDirectory = argv[1];
        PathUtils::addDirectorySeparatorAtEnd(drawParamDirectory);
        std::string drawParamFilePattern = drawParamDirectory + "*.dpa";
        auto fileNames = NFmiFileSystem::PatternFiles(drawParamFilePattern);
        std::map<int, int> valueDistribution;
        for(const auto& fileName : fileNames)
        {
            auto fileNamePath = drawParamDirectory + fileName;
            NFmiDrawParam drawParam;
            std::ifstream in(fileNamePath, std::ios_base::in | std::ios_base::binary);
            if(in)
            {
                in >> drawParam;
                valueDistribution[drawParam.StationSymbolColorShadeClassCount()]++;
                if(drawParam.StationSymbolColorShadeClassCount() <= 3)
                {
                    std::cout << fileNamePath << std::endl;
                }
            }
        }

        for(const auto& valuePair : valueDistribution)
        {
            std::cout << valuePair.first << ": " << valuePair.second << std::endl;
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
*/

/*
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiStringTools.h"
#include <fstream>

int main(int argc, const char* argv[])
{
    // Tulostetaan annetun 1. argumenttina annetun datan 2. parametrina annetun par-id:n arvot.
    // Tulostetaan pilkulla erotettujen indeksien aika-askeleet (alkavat 0:sta).
    try
    {
        std::string dataFileName = argv[1];
        std::string parIdStr = argv[2];
        auto parId = static_cast<FmiParameterName>(std::stoi(parIdStr));
        std::string timeIndexListStr = argv[3];
        auto timeIndexList = NFmiStringTools::Split<std::vector<int>>(timeIndexListStr, ",");
        NFmiQueryData data(dataFileName);
        NFmiFastQueryInfo fastInfo(&data);
        if(!fastInfo.Param(parId))
        {
            std::string errorMessage = "par-id: ";
            errorMessage += std::to_string(parId);
            errorMessage += " was not found from given data";
            throw std::runtime_error(errorMessage);
        }

        for(fastInfo.ResetLevel(); fastInfo.NextLevel(); )
        {
            for(auto timeIndex : timeIndexList)
            {
                if(!fastInfo.TimeIndex(timeIndex))
                {
                    std::cerr << "Time index " << timeIndex << " was not in given data" << std::endl;
                }
                else
                {
                    if(fastInfo.IsGrid())
                    {
                        int fieldWidth = 7;
                        auto locationSize = fastInfo.SizeLocations();
                        auto columnCount = fastInfo.GridXNumber();
                        std::cout << std::endl;
                        for(unsigned long locationIndex = 0; locationIndex < locationSize; locationIndex++)
                        {
                            fastInfo.LocationIndex(locationIndex);
                            if(locationIndex > 0 && locationIndex % columnCount == 0)
                            {
                                std::cout << std::endl;
                            }
                            std::cout << std::setw(fieldWidth) << fastInfo.FloatValue();
                        }
                    }
                    else
                    {
                        throw std::runtime_error("Station data output is not implemented yet");
                    }
                }
            }
        }

        return 0;
    }
    catch(std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
*/

/*
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "NFmiPathUtils.h"
#include "NFmiQueryDataUtil.h"
#include "NFmiTimeList.h"
#include "NFmiStringTools.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiProducerName.h"
#include <fstream>
#include <thread>
#include "boost\algorithm\string\replace.hpp"
#include "boost/filesystem.hpp"

static std::string MakeNextSubDirectoryPath(const std::string& currentDirectory, const std::string& subDirectoryName)
{
    auto nextSubDirectoryPath = currentDirectory;
    PathUtils::addDirectorySeparatorAtEnd(nextSubDirectoryPath);
    nextSubDirectoryPath += subDirectoryName;
    return nextSubDirectoryPath;
}

static void ChangeTime(NFmiMetTime& changedTime, int changeTimesByDays)
{
    changedTime.ChangeByDays(changeTimesByDays);
}

static NFmiTimeDescriptor MakeNewTimeDescrptor(NFmiTimeDescriptor copyOfSourceTimeDescriptor, int changeTimesByDays)
{
    if(copyOfSourceTimeDescriptor.UseTimeList())
    {
        auto &timeList = *copyOfSourceTimeDescriptor.ValidTimeList();
        for(timeList.Reset(); timeList.Next(); )
        {
            ::ChangeTime(*timeList.Current(), changeTimesByDays);
        }
    }
    else
    {
        auto &timebag = *copyOfSourceTimeDescriptor.ValidTimeBag();
        timebag.MoveByMinutes(changeTimesByDays * 24 * 60);
    }
    auto changedOriginTime = copyOfSourceTimeDescriptor.OriginTime();
    changedOriginTime.ChangeByDays(changeTimesByDays);
    copyOfSourceTimeDescriptor.OriginTime(changedOriginTime);
    return copyOfSourceTimeDescriptor;
}

std::unique_ptr<NFmiQueryData> CreateChangedQueryData(NFmiFastQueryInfo& sourceFastInfo, int changeTimesByDays)
{
    auto newTimeDescriptor = ::MakeNewTimeDescrptor(sourceFastInfo.TimeDescriptor(), changeTimesByDays);
    auto newQueryData = std::make_unique<NFmiQueryData>(*sourceFastInfo.RefQueryData());
    newQueryData->Info()->SetTimeDescriptor(newTimeDescriptor);
    return newQueryData;
}

// Timestampissa pit‰‰ olla 12 numeroa per‰kk‰in, jotka on eroteltu muista nimen osista '_' merkill‰.
// Palautetaan pair, jossa korjattu polku string:ina ja uuden aikaleiman aika NFmiMetTime:na.
static std::pair<std::string, NFmiStaticTime> MakeChangedTimeStampFilePath(std::string filePath, int changeTimesByDays)
{
    NFmiStaticTime usedTime = NFmiMetTime::gMissingTime;
    NFmiStaticTime usedTimeWithDDHHMMss_mask = NFmiMetTime::gMissingTime;
    std::string newFilePath = filePath;
    NFmiFileString fileString(filePath);
    std::string fileNameWithoutExtension = fileString.Header();
    auto fileNameParts = NFmiStringTools::Split(fileNameWithoutExtension, "_");
    for(auto fileNamePart : fileNameParts)
    {
        try
        {
            // Katsotaan voidaanko fileNamePart konvertoida numeroksi. Jos voidaan, silloin tarkastellaan
            // kuinka eripituisista timestampeista voisi rakentaa ajan
            double tmpValue = NFmiStringTools::Convert<double>(fileNamePart);
            NFmiTime aTime;
            auto fileNamePartSize = fileNamePart.size();
            if(fileNamePartSize == 12 || fileNamePartSize == 14)
            {
                auto usedTimeMask = (fileNamePartSize == 12) ? kYYYYMMDDHHMM : kYYYYMMDDHHMMSS;
                aTime.FromStr(fileNamePart, usedTimeMask);
                // Valitettavasti ChangeByHours aina nolla sekunnit v‰kisin
                auto currentSeconds = aTime.GetSec();
                aTime.ChangeByDays(changeTimesByDays);
                aTime.SetSec(currentSeconds);
                std::string newTimeStamp = aTime.ToStr(usedTimeMask);
                boost::replace_all(newFilePath, fileNamePart, newTimeStamp);
                usedTime = aTime;
                break; // lopetetaan kun on lˆytynyt 1. kunnon aikaleima tiedostonimest‰
            }
            else if(fileNamePartSize == 8)
            {
                // Oletus maski on muotoa DDHHmmSS (n‰in ainakin smartmet_roadmodel_skandinavia -tiedostoissa),
                // valitettavasti se ei toimi oikein FromStr metodin kanssa ja t‰ss‰ pit‰‰ tehd‰ erillist‰ kikkailua.
                auto YYYYMMstr = aTime.ToStr("YYYYMM");
                auto totalTimeString = YYYYMMstr + fileNamePart;
                aTime.FromStr(totalTimeString, kYYYYMMDDHHMMSS);
                // Valitettavasti ChangeByHours aina nolla sekunnit v‰kisin
                auto currentSeconds = aTime.GetSec();
                aTime.ChangeByDays(changeTimesByDays);
                aTime.SetSec(currentSeconds);
                std::string newTimeStamp = aTime.ToStr("DDHHmmSS");
                boost::replace_all(newFilePath, fileNamePart, newTimeStamp);
                if(usedTimeWithDDHHMMss_mask == NFmiMetTime::gMissingTime)
                    usedTimeWithDDHHMMss_mask = aTime;
            }
        }
        catch(...)
        {
        }
    }

    if(usedTime == NFmiMetTime::gMissingTime)
        return std::make_pair(newFilePath, usedTimeWithDDHHMMss_mask);
    else
        return std::make_pair(newFilePath, usedTime);
}

static time_t MakeTimeT(const NFmiStaticTime& newModifiedTime)
{
    struct tm timeStruct;
    timeStruct.tm_sec = newModifiedTime.GetSec();
    timeStruct.tm_min = newModifiedTime.GetMin();
    timeStruct.tm_hour = newModifiedTime.GetHour();
    timeStruct.tm_mday = newModifiedTime.GetDay();
    timeStruct.tm_mon = newModifiedTime.GetMonth() - 1;     // tm months start from 0
    timeStruct.tm_year = newModifiedTime.GetYear() - 1900;  // tm years start from 1900
    timeStruct.tm_wday = -1;
    timeStruct.tm_yday = -1;
    timeStruct.tm_isdst = -1;

    return ::mktime(&timeStruct);
}

static void StoreParallerQueryData(NFmiQueryData& newQueryData, const std::string& queryDataFilePath, const NFmiStaticTime & newModifiedTime)
{
    newQueryData.Write(queryDataFilePath);
    // Muutetaan luodun tiedoston modified time annetun ajan mukaiseksi
    boost::filesystem::path boostPath = queryDataFilePath;
    boost::filesystem::last_write_time(boostPath, ::MakeTimeT(newModifiedTime));
}

static void DoQueryDataWork(const std::string& queryDataFileName, const std::string& currentDirectory, const std::string& parallerDirectory, int changeTimesByDays)
{
    auto queryDataFileSourcePath = ::MakeNextSubDirectoryPath(currentDirectory, queryDataFileName);
    auto parallerQueryDataFileSourcePath = ::MakeNextSubDirectoryPath(parallerDirectory, queryDataFileName);
    auto responsePair = ::MakeChangedTimeStampFilePath(parallerQueryDataFileSourcePath, changeTimesByDays);
    parallerQueryDataFileSourcePath = responsePair.first;
    try
    {
        std::cerr << "Doing queryData file " << queryDataFileSourcePath << std::endl;
        NFmiQueryData sourceData(queryDataFileSourcePath);
        NFmiFastQueryInfo sourceFastInfo(&sourceData);
        auto newQueryData = ::CreateChangedQueryData(sourceFastInfo, changeTimesByDays);
        if(newQueryData)
            ::StoreParallerQueryData(*newQueryData, parallerQueryDataFileSourcePath, responsePair.second);
        else
            std::cerr << "Error: couldn't create newQueryData from " << queryDataFileSourcePath << std::endl;
    }
    catch(std::exception& e)
    {
        std::cerr << "Error with file: " << queryDataFileSourcePath << std::endl;
        std::cerr << e.what() << std::endl;
    }
}

static void DoFilesOnDirectory(const std::string& currentDirectory, const std::string& parallerDirectory, int changeTimesByDays)
{
    // Tiedostot tulevat aikaj‰rjestyksess‰ vanhimmasta uusimpaan, johtuen tiedostonimiss‰ olevista aikaleimoista.
    auto files = NFmiFileSystem::DirectoryFiles(currentDirectory);
    for(const auto& queryDataFileName : files)
    {
        ::DoQueryDataWork(queryDataFileName, currentDirectory, parallerDirectory, changeTimesByDays);
    }
}

static void DoRecursiveDirectoryWork(const std::string& currentDirectory, const std::string& parallerDirectory, int changeTimesByDays)
{
    std::cerr << "Doing directory " << currentDirectory << std::endl;
    if(NFmiFileSystem::DirectoryExists(currentDirectory))
    {
        if(NFmiFileSystem::CreateDirectory(parallerDirectory))
        {
            std::cerr << "Created paraller directory " << parallerDirectory << std::endl;
            DoFilesOnDirectory(currentDirectory, parallerDirectory, changeTimesByDays);
            auto subDirectories = NFmiFileSystem::Directories(currentDirectory);
            for(const auto &subDirectoryName : subDirectories)
            {
                if(subDirectoryName != "." && subDirectoryName != "..")
                {
                    auto nextSubDirectoryPath = ::MakeNextSubDirectoryPath(currentDirectory, subDirectoryName);
                    auto nextParallerSubDirectoryPath = ::MakeNextSubDirectoryPath(parallerDirectory, subDirectoryName);
                    DoRecursiveDirectoryWork(nextSubDirectoryPath, nextParallerSubDirectoryPath, changeTimesByDays);
                }
            }
        }
        else
            std::cerr << "Error: couldn't create paraller directory " << parallerDirectory << std::endl;
    }
    else
        std::cerr << "Error: non existing given directory " << parallerDirectory << std::endl;
}

static int CalculateRoundedDaysDifference(const NFmiMetTime& wallClockTime, const NFmiMetTime& usedDataWallClockTime)
{
    long differenceInMinutes = wallClockTime.DifferenceInMinutes(usedDataWallClockTime);
    int roundedDaysDifference = boost::math::iround(differenceInMinutes / (24. * 60.));
    return roundedDaysDifference;
}

// Tutkitaan annetusta queryData hakemistosta local hakemistoa.
// K‰yd‰‰n l‰pi sen kaikki qData tiedostot.
// T‰rkeimpi‰ ovat synop ja metar tiedostot, jos niit‰ lˆytyy, k‰ytet‰‰n sen datasetin 'sein‰kelloaikana'
// mink‰ tahansa synop/metar datan viimeisint‰ validTimea. 
// Jos t‰ll‰ist‰ ei lˆydy, k‰ytet‰‰n mink‰ tahansa hiladatan (oletetaan ett‰ on ennuste) myˆh‰isint‰ ensimm‰ist‰ validTimea.
// Jos t‰ll‰ist‰ ei lˆydy, k‰ytet‰‰n mink‰ tahansa asemadatan (oletetaan ett‰ on havainto)  myˆh‰isint‰ viimeist‰ validTimea.
// Lˆydetyn sein‰kelloajan avulla lasketaan siirtym‰ p‰iviss‰, joka palautetaan.
static int CalculateActualChangeTimesByDays(const std::string& pathToQueryDataFiles)
{
    auto localDirectory = pathToQueryDataFiles;
    PathUtils::addDirectorySeparatorAtEnd(localDirectory);
    localDirectory += "local";
    if(NFmiFileSystem::DirectoryExists(localDirectory))
    {
        PathUtils::addDirectorySeparatorAtEnd(localDirectory);
        auto qDataFilePattern = localDirectory;
        qDataFilePattern += "*.?qd";
        auto files = NFmiFileSystem::PatternFiles(qDataFilePattern);
        if(!files.empty())
        {
            NFmiMetTime wallClockTime;
            NFmiMetTime synopBasedWallClock = NFmiMetTime::gMissingTime;
            NFmiMetTime modelDataBasedWallClock = NFmiMetTime::gMissingTime;
            NFmiMetTime obsDataBasedWallClock = NFmiMetTime::gMissingTime;
            for(const auto& fileName : files)
            {
                try
                {
                    auto finalFilePath = localDirectory + fileName;
                    NFmiQueryData qData(finalFilePath);
                    NFmiFastQueryInfo fastInfo(&qData);
                    if(fastInfo.IsGrid())
                    {
                        const auto& firstValidTime = fastInfo.TimeDescriptor().FirstTime();
                        if(modelDataBasedWallClock < firstValidTime)
                            modelDataBasedWallClock = firstValidTime;
                    }
                    else
                    {
                        const auto& lastValidTime = fastInfo.TimeDescriptor().LastTime();
                        auto producerId = fastInfo.Producer()->GetIdent();
                        if(producerId == kFmiSYNOP || producerId == kFmiMETAR)
                        {
                            if(synopBasedWallClock < lastValidTime)
                                synopBasedWallClock = lastValidTime;
                        }
                        else
                        {
                            if(obsDataBasedWallClock < lastValidTime)
                                obsDataBasedWallClock = lastValidTime;
                        }
                    }
                }
                catch(...)
                { }
            }
            if(synopBasedWallClock != NFmiMetTime::gMissingTime)
            {
                int usedDifferenceInDays = ::CalculateRoundedDaysDifference(wallClockTime, synopBasedWallClock);
                std::cerr << "Using synop/metar based qData as original wall-clock time and final change time in days is " << std::to_string(usedDifferenceInDays) << std::endl;
                return usedDifferenceInDays;
            }
            else if(modelDataBasedWallClock != NFmiMetTime::gMissingTime)
            {
                int usedDifferenceInDays = ::CalculateRoundedDaysDifference(wallClockTime, modelDataBasedWallClock);
                std::cerr << "Using model based qData as original wall-clock time and final change time in days is " << std::to_string(usedDifferenceInDays) << std::endl;
                return usedDifferenceInDays;
            }
            else if(obsDataBasedWallClock != NFmiMetTime::gMissingTime)
            {
                int usedDifferenceInDays = ::CalculateRoundedDaysDifference(wallClockTime, obsDataBasedWallClock);
                std::cerr << "Using other-observation based qData as original wall-clock time and final change time in days is " << std::to_string(usedDifferenceInDays) << std::endl;
                return usedDifferenceInDays;
            }
            else
                throw std::runtime_error("Error: the 'local' sub-directory didn't contain any usable qdata files, cannot calculate changeTimesByDays value");
        }
        else
            throw std::runtime_error("Error: the 'local' sub-directory didn't contain any qdata files, cannot calculate changeTimesByDays value");
    }
    else
        throw std::runtime_error("Error: the 'local' sub-directory didn't exist at all, cannot calculate changeTimesByDays value");
}

int main(int argc, const char* argv[])
{
    // Ohjelma k‰y rekursiivisesti l‰pi kaikki tiedostot ja hakemistot, jotka ovat
    // annetulla pathToQueryDataFiles -polulla ja ja tekee seuraavia asioita:
    // 1) Luo uusi data muistiin, joissa on siirretty kaikkia datoja sein‰kelloajan suhteen.
    //    Siirrot tapahtuvat niin ett‰ tunnit ja minuutit s‰ilyv‰t, ainoastaan p‰iv‰m‰‰r‰
    //    tulee nykyhetkeen. 
    // 2) Etsit‰‰n datoista uusin data ja siirrot tapahtuvat sen suhteessa, eli lasketaan
    //    siirtym‰ p‰iviss‰. 
    // 3) Talleta uusi data rinnakkaiseen puurakenteeseen, joka alkaa newBasePath -polusta.
    // 4) Uuden datan nimessa oleva aikaleima on myˆs muokattu samalla siirtym‰ll‰.
    std::string pathToQueryDataFiles = argv[1];
    std::string newBasePath = argv[2];
    auto changeTimesByDays = ::CalculateActualChangeTimesByDays(pathToQueryDataFiles);
    DoRecursiveDirectoryWork(pathToQueryDataFiles, newBasePath, changeTimesByDays);

    return 1;
}
*/

/*
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiValueString.h"
#include "NFmiMilliSecondTimer.h"

#include <fstream>

template<typename T>
std::function<T> make_function(T* t) {
    return { t };
}

static float conversionFast(float value, int)
{
    return value;
}

static float conversionSlow(float value, int)
{
    NFmiValueString valueStr(value, "%f.1");
    return valueStr;
}

static void FillData(NFmiFastQueryInfo& fastInfo1, NFmiFastQueryInfo& fastInfo2, int precision, const std::string &name, std::function<float(float, int)> &conversionFunction)
{
    std::cout << name << " starts..." << std::endl;
    NFmiNanoSecondTimer timer;
    for(fastInfo1.ResetParam(), fastInfo2.ResetParam(); fastInfo1.NextParam(), fastInfo2.NextParam(); )
    {
        std::cout << "#Par";
        for(fastInfo1.ResetLocation(), fastInfo2.ResetLocation(); fastInfo1.NextLocation(), fastInfo2.NextLocation(); )
        {
//            std::cout << "L";
            auto latlonNew = fastInfo1.LatLon();
            latlonNew.X(latlonNew.X() + 0.1);
            for(fastInfo1.ResetLevel(), fastInfo2.ResetLevel(); fastInfo1.NextLevel(), fastInfo2.NextLevel(); )
            {
                for(fastInfo1.ResetTime(), fastInfo2.ResetTime(); fastInfo1.NextTime(), fastInfo2.NextTime(); )
                {
                    auto valueOrig = fastInfo1.InterpolatedValue(latlonNew);
                    auto valueNew = conversionFunction(valueOrig, precision);
                    fastInfo2.FloatValue(valueNew);
                }
            }
        }
    }
    std::cout << std::endl << name << " took " << timer.elapsedTimeInSecondsString() << std::endl;
}

int main(int argc, const char* argv[])
{
    std::string dataFileName = "D:\\SmartMet\\wrk\\data\\local\\202106090853_hirlam_skandinavia_mallipinta.sqd";
    NFmiQueryData data1(dataFileName);
    NFmiFastQueryInfo fastInfo1(&data1);
    std::unique_ptr<NFmiQueryData> data2(data1.Clone());
    NFmiFastQueryInfo fastInfo2(data2.get());
    ::FillData(fastInfo1, fastInfo2, 1, "fast", ::make_function(conversionFast));
    ::FillData(fastInfo1, fastInfo2, 1, "slow", ::make_function(conversionSlow));
    ::FillData(fastInfo1, fastInfo2, 1, "fast", ::make_function(conversionFast));

    return 0;
}
*/
/*
#include "NFmiQueryData.h"
#include "NFmiFastQueryInfo.h"
#include "NFmiValueString.h"
#include <fstream>

int main(int argc, const char* argv[])
{
    std::string dataFileName = argv[1];
    NFmiQueryData data(dataFileName);
    NFmiFastQueryInfo fastInfo(&data);
    auto amdarCase = (fastInfo.SizeLocations() == 1);
    std::string soundingName = amdarCase ? "amdar sounding" : "sounding";
    int hasDataSoundingCounter = 1;
    int soundingCounter = 1;

    std::string outputTxtFileName = argv[2];
    bool printMissingSoundingLine = false;
    if(argc > 3)
    {
        int fourthArgumentValue = std::stoi(std::string(argv[3]));
        if(fourthArgumentValue != 0)
            printMissingSoundingLine = true;
    }
    std::ofstream output(outputTxtFileName.c_str(), std::ios::binary);
    if(output)
    {
        const size_t usedWitdth = 10;
        output.fill(' ');
        output << "Print out separate non-missing " << soundingName << "(s) from given '" << dataFileName << "' querydata file\n";
        for(fastInfo.ResetTime(); fastInfo.NextTime(); )
        {
            for(fastInfo.ResetLocation(); fastInfo.NextLocation(); )
            {
                bool soundingHasData = false;
                std::stringstream temporaryOutput;
                temporaryOutput << std::setprecision(1) << std::fixed;

                temporaryOutput << "\n";
                temporaryOutput << std::to_string(printMissingSoundingLine ? soundingCounter : hasDataSoundingCounter);
                temporaryOutput << ". " << soundingName << " from time " << fastInfo.Time().ToStr("YYYY MM.DD. HH:mm:SS Utc", kEnglish).CharPtr() << std::endl;
                auto headerLineStr = temporaryOutput.str();
                headerLineStr.pop_back();
                if(!amdarCase)
                {
                    auto location = fastInfo.Location();
                    temporaryOutput << "Location: " << location->GetName().CharPtr() << " (id = " << location->GetIdent() << ")" << std::endl;
                }
                temporaryOutput << "      ";
                for(fastInfo.ResetParam(); fastInfo.NextParam(); )
                {
                    // Tulostetaan jokaiselle amdarille oma parametri otsikko rivi
                    std::string usedName = fastInfo.Param().GetParamName().CharPtr();
                    if(usedName.size() > usedWitdth)
                        usedName.resize(usedWitdth);
                    temporaryOutput << std::setw(usedWitdth) << usedName << "|";
                }
                temporaryOutput << std::endl;

                for(fastInfo.ResetLevel(); fastInfo.NextLevel(); )
                {
                    temporaryOutput << "L-" << std::to_string(fastInfo.LevelIndex()) << ": ";
                    if(fastInfo.LevelIndex() < 10)
                        temporaryOutput << " ";
                    for(fastInfo.ResetParam(); fastInfo.NextParam(); )
                    {
                        auto value = fastInfo.FloatValue();
                        if(value == kFloatMissing)
                            temporaryOutput << std::setw(usedWitdth) << "-" << " ";
                        else
                        {
                            soundingHasData = true;
                            temporaryOutput << std::setw(usedWitdth) << value << " ";
                        }
                    }
                    temporaryOutput << std::endl;
                }
                if(soundingHasData)
                {
                    hasDataSoundingCounter++;
                    output << temporaryOutput.str();
                }
                else if(printMissingSoundingLine)
                {
                    output << headerLineStr << " was completely missing...";

                }
                soundingCounter++;
            }
        }
        return 0;
    }

    return 1;
}
*/

/*
#include "NFmiArea.h"
#include "NFmiAreaFactory.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiHelpDataInfo.h"
#include "NFmiSettings.h"

int main(int argc, const char* argv[])
{
    NFmiNanoSecondTimer timer;
    // Testi 1: luodaan samoja area olioita per‰kk‰in. 
    // 1. olion luonti NFmiAreaFactory::Create funktiolla ja Clone kest‰‰ n. 0.05 s (n. 1000x hitaampaan kuin vanhalla versiolla).
    // Sen j‰lkeen luonti + Clone kest‰‰ 'vain' n. 0.002 s (silti n. 100x hitaampaan kuin vanhalla versiolla).

    std::string legacyAreaString = "stereographic,20,90,60:6,51.3,49,70.2";
    int totalSameAreaCreationCount = 33;
    for(int index = 0; index < totalSameAreaCreationCount; index++)
    {
        boost::shared_ptr<NFmiArea> area = NFmiAreaFactory::Create(legacyAreaString);
        if(area)
        {
            boost::shared_ptr<NFmiArea> areaClone(area->Clone());
        }
        std::cerr << "Area #" << index << ": " << timer.elapsedTimeInSecondsString() << std::endl;
        timer.restart();
    }

    // Testi 2: Luodaan monia samoja area olioita per‰kk‰in monimutkaisemmassa ymp‰ristˆss‰. 
    // T‰ss‰ k‰ytet‰‰n smarttools kirjaston NFmiHelpDataInfoSystem luokkaa.
    // Esim. kun luodaan cinesat:iin liittyvi‰ olioita, niill‰ on kaikilla sama area, mutta niiden 
    // luonti kest‰‰ aina n. 0.02 sekuntia ja jos niist‰ teht‰isiin kopio Clone:lla (nyt ei tehd‰),
    // kest‰isi sekin aina n. 0.02 s. Eli jostain syyst‰ t‰ll‰isell‰ asetelmalla toiminnot ovat 
    // aina n. 1000x hitaampia kuin vanhalla newbase:lla.

    // Testiin liittyy 9 konffia, jotka laitetaan /editorConfs hakemistoon.
    // Aikaa voi mitata lis‰‰m‰ll‰ joku ajan mittaus ja cout -tulostus NFmiHelpDataInfo::InitFromSettings 
    // metodiin NFmiAreaFactory::Create funktio kutsun ymp‰rille (ks. NFmiNanoSecondTimer:in k‰yttˆ‰ yll‰).

    std::string baseControlPath = "/editorConfs/";
    NFmiSettings::Read(baseControlPath + "data_scand.conf");
    NFmiSettings::Read(baseControlPath + "paths_dev.conf");
    NFmiSettings::Read(baseControlPath + "local_cache_settings_scand.conf");
    NFmiSettings::Read(baseControlPath + "static_settings.conf");
    NFmiSettings::Read(baseControlPath + "cinesat_scand_satel_image.conf");
    NFmiSettings::Read(baseControlPath + "meteosat8_scand_satel_image.conf");
    NFmiSettings::Read(baseControlPath + "noaa_scand_satel_image.conf");
    NFmiSettings::Read(baseControlPath + "seviri_scand_satel_image.conf");
    NFmiSettings::Read(baseControlPath + "satel_image_paths_fmi.conf");

    NFmiHelpDataInfoSystem helpDataInfoSystem;

    try
    {
        helpDataInfoSystem.InitFromSettings("MetEditor::HelpData",
            baseControlPath,
            "/editorConfs/*_help_pal_data.sqd",
            "help_pal_data");
    }
    catch(std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    std::cerr << "After helpDataInfoSystem.InitFromSettings: " << timer.elapsedTimeInSecondsString() << std::endl;
    timer.restart();

    return 0;
}
*/
/*
#include "NFmiCmdLine.h"
#include "NFmiQueryData.h"
#include "NFmiWindFix.h"

using namespace std;

string getUsageString()
{
    string usageString = "Usage: recalculatewindparameters input output";
    usageString += "\n\nPurpose: recalculate wind related parameters with WD and WS,";
    usageString += "\npossible recalculated parameters are: u, v and wind-vector";
    return usageString;
}

void usage()
{
    cerr << getUsageString() << endl;
}

int run(int argc, const char* argv[])
{
    NFmiCmdLine cmdline(argc, argv, "");

    // Tarkistetaan optioiden oikeus:
    if(cmdline.Status().IsError())
    {
        string errorString = "Invalid command line:\n";
        errorString += cmdline.Status().ErrorLog().CharPtr();
        errorString += "\n";
        errorString += getUsageString();
        throw runtime_error(errorString);
    }

    if(cmdline.NumberofParameters() != 2)
    {
        string errorString = "Invalid number of command line arguments (2 needed):\n";
        errorString += getUsageString();
        throw runtime_error(errorString);
    }

    string inputfile = cmdline.Parameter(1);
    string outputfile = cmdline.Parameter(2);
    // T‰ss‰ queryData halutaan lukea ilman memory-mappausta (joka olisi read-only), koska dataan tehd‰‰n 
    // muutoksia ja se talletetaan lopuksi eri tiedostoon.
    NFmiQueryData qd(inputfile, false);
    if(NFmiWindFix::FixWinds(qd))
        qd.Write(outputfile);
    return 0;
}

int main(int argc, const char* argv[])
{
    try
    {
        return run(argc, argv);
    }
    catch(exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}
*/
