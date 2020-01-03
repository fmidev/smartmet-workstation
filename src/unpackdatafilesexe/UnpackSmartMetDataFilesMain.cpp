#pragma once

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
