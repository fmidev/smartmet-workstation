#include "StdAfx.h"
#include "QueryDataReading.h"
#include "NFmiFileSystem.h"
#include "NFmiFileString.h"
#include "NFmiMilliSecondTimer.h"
#include "NFmiValueString.h"
#include "NFmiQueryData.h"
#include "catlog/catlog.h"

namespace
{
    void LogQueryDataFileReadError(const std::string &theFileName, const std::string &theErrorMsgPart2)
    {
        CatLog::logMessage(std::string("FILEERROR: File read failed (") + theFileName + theErrorMsgPart2, CatLog::Severity::Error, CatLog::Category::Data);
    }

    void DoLatestDataReadingProblem(const NFmiFileString &theFileName, const std::string &theExceptionMsg, bool badAllocExeption)
    {
        std::string errMsg(theExceptionMsg);

        // Datan luku virhetilanteissa ei en‰‰ haluta siirt‰‰ ongelma tiedostoja pois monistakin syist‰.
        // Tehd‰‰n vain hieman erilaiset virheilmoitukset ja jos muisti on loppunut, varoitetaan k‰ytt‰j‰‰,
        // mutta vain 1. kerralla.
        std::string::size_type pos = theExceptionMsg.find("SmartMet:");
        if(badAllocExeption == false && pos == std::string::npos)
        {
            errMsg += "\nFile reading problem but file was not moved to error directory.";
        }
        else
        { // jos smartmet teksti lˆytyi messagesta, tai oli bad_alloc-exception, laitetaan t‰h‰n varoitus messageBox yhden kerran, ett‰
          // joku data ei ole luettu koska ei saatu varattua tarpeeksi isoa taulukkoa.
            static bool firstTime = true;
            if(firstTime)
            {
                firstTime = false;
                std::string msgTitle("Data file reading problem");
                std::string msgStr("There was data file reading problem with file:\n");
                msgStr += theFileName;
                msgStr += "\nthis message will be displayed only once.\n";
                msgStr += "You may restart program and try to read data again,\n";
                msgStr += "but it is possible that there is too much data in\n";
                msgStr += "this SmartMet configuration and too little memory in\n";
                msgStr += "this PC that this system will work properly.\n\n";

                msgStr += theExceptionMsg;
                CatLog::logMessage(msgStr, CatLog::Severity::Error, CatLog::Category::Data);
                ::MessageBox(AfxGetMainWnd()->GetSafeHwnd(), CA2T(msgStr.c_str()), CA2T(msgTitle.c_str()), MB_OK);
            }
        }
        throw std::runtime_error(errMsg);
    }
}

namespace QueryDataReading
{
    // theFileFilter voi olla tarkka tiedoston nimi polkuineen tai esim. Hirlam_data_*.sqd, jolloin haetaan viimeisin
    // kyseisell‰ filterill‰ lˆytyv‰st‰ tiedostoista.
    // Anna theLimitingTimeStamp arvo 0 jos haluat lukea tiedoston joka tapauksessa.
    std::unique_ptr<NFmiQueryData> ReadLatestDataWithFileFilterAfterTimeStamp(const std::string& theFileFilter, time_t theLimitingTimeStamp, std::string& theFileNameOut, time_t& theTimeStampOut)
    {
        std::unique_ptr<NFmiQueryData> data;
        theTimeStampOut = NFmiFileSystem::FindFile(theFileFilter, true, &theFileNameOut);
        if(theTimeStampOut && theTimeStampOut > theLimitingTimeStamp)
        {
            bool catchedAllready = false;
            bool badAllocHandled = false;
            NFmiFileString tmpFileName(theFileFilter);
            tmpFileName.FileName(theFileNameOut);
            std::string usedFileName = static_cast<char*>(tmpFileName);

            try
            {
                data = ReadDataFromFile(usedFileName, true);
                if(data == 0)
                    ::LogQueryDataFileReadError(usedFileName, "), zero-pointer returned");
            }
            catch(std::bad_alloc & e)
            {
                ::LogQueryDataFileReadError(usedFileName, "), bad-alloc thrown");

                catchedAllready = true;
                badAllocHandled = true;
                DoLatestDataReadingProblem(tmpFileName, e.what(), true);
            }
            catch(std::exception & e)
            {
                catchedAllready = true;
                if(badAllocHandled)
                    throw; // jos bad_alloc haarasta heitetty tuli t‰h‰n, heitet‰‰n se vain edelleen
                else
                {
                    ::LogQueryDataFileReadError(usedFileName, std::string("), std::exception thrown:\n") + e.what());
                    DoLatestDataReadingProblem(tmpFileName, e.what(), false);
                }
            }
            catch(char *msg) // HUOM! t‰m‰n (catch(char *msg)) pit‰‰ olla catch(std::exception & e) per‰ss‰ koska DoLatestDataReadingProblem heitt‰‰ std::runtime_error:in joka otettaisiin ttaas uudestaan kiinni
            {
                catchedAllready = true;
                ::LogQueryDataFileReadError(usedFileName, std::string("), char * -exception thrown:\n") + msg);
                DoLatestDataReadingProblem(tmpFileName, msg, false);
            }
            catch(...)
            {
                if(catchedAllready)
                    throw; // jos on jo k‰sitelty, heit‰ poikkeus vain eteenp‰in
                else
                {
                    ::LogQueryDataFileReadError(usedFileName, "), unknown-exception thrown");
                    DoLatestDataReadingProblem(tmpFileName, std::string("File read failed (") + usedFileName + "), unknown-exception thrown", false);
                }
            }
        }
        return data;
    }

    std::unique_ptr<NFmiQueryData> ReadDataFromFile(const std::string& fileName, bool useMemoryMapping)
    {
        std::unique_ptr<NFmiQueryData> data;
        NFmiMilliSecondTimer timer;
        timer.StartTimer();

        if(data == NULL)
        {
            if(fileName.empty() == false && NFmiFileSystem::FileExists(fileName) && NFmiFileSystem::FileEmpty(fileName) == false)
            {
                data = std::make_unique<NFmiQueryData>(fileName, useMemoryMapping);
            }
        }

        timer.StopTimer();
        if(data)
        {
            std::string logStr("Data file: '");
            logStr += fileName;
            logStr += "' reading lasted: ";
            logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() / 1000, 0);
            logStr += ".";
            logStr += NFmiValueString::GetStringWithMaxDecimalsSmartWay(timer.TimeDiffInMSeconds() % 1000, 0);
            logStr += " s.";
            CatLog::logMessage(logStr, CatLog::Severity::Info, CatLog::Category::Data);
        }

        return data;
    }
}

