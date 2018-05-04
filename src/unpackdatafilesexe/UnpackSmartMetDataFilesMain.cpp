#pragma once

// Ohjelma saa seuraavat argumentit:
// 0. ohjelman nimi (ei v‰litet‰ t‰st‰)
// 1. pakatun l‰hdetiedoston polku
// 2. puretun kohdetiedoston polku
// 3. kohdetiedoston lopullinen siirto polku
// 4. deletoidaanko pakattu l‰hdetiedosto (1=deletoi, 0=ei deletoida)
// 5. lokitiedoston nimi, jota NFmiLogger k‰ytt‰‰

#include <vector>
#include <memory>

#include "NFmiFileString.h"
#include "NFmiStringTools.h"
#include "NFmiLogger.h"
#include "bzip2util.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiFileSystem.h"

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

int main(int argc, const char* argv[]) 
{
    NFmiFileString exeFileNameStr(argv[0]);
    std::string exeName = exeFileNameStr.FileName();
    if(argc < 6)
    {
        std::cout << "Error when executing " << exeName << ", not enough arguments, 4/5 required:" << std::endl;
        std::cout << "1. packed-file-path, 2. unpacked-file-path, 3. unpacked-file-final-move-path" << std::endl;
        std::cout << "4. delete-packed-file-after (1=delete, 0=no delete), 5. log-file-path (default no log)" << std::endl;
    }
    else
    {
        std::string packedFilePath = argv[1];
        std::string unPackedFilePath = argv[2];
        std::string unPackedFinalFilePath = argv[3];
        bool deletePackedFileAfter = NFmiStringTools::Convert<bool>(argv[4]);
        std::string logFilePath = argv[5];
        std::shared_ptr<NFmiLogger> logger = ::MakeLogger(logFilePath);

        try
        {
            // puretaan ensin pakattutiedosto
       	    NFmiMilliSecondTimer timer;
            bool status = CFmiBzip2Helpers::UnpackBzip2DataFile(packedFilePath, unPackedFilePath, deletePackedFileAfter);
            timer.StopTimer();
            if(status)
            { 
                ::ReportSuccessfullUnpacking(logger, packedFilePath, unPackedFilePath, timer);
                // siirret‰‰n purettu tiedosto viel‰ lopulliseen paikkaan
                if(::MoveFileToFinalDestination(unPackedFilePath, unPackedFinalFilePath, logger))
                    return 0; // onnistunut ulostulo
            }
            else
            {
                ::ReportUnsuccessfullUnpacking(logger, packedFilePath, unPackedFilePath);
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
