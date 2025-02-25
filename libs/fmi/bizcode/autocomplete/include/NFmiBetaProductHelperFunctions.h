#pragma once

#include "NFmiPathUtils.h"
#include "catlog/catlog.h"

#include <functional>

class CWnd;

namespace BetaProduct
{
    // **************************************************************
    // jotain Beta-product systeemiin liittyvi� yleisi� apufunktioita
    // **************************************************************

    using LogAndWarnFunctionType = std::function<void(const std::string &, const std::string&, CatLog::Severity , CatLog::Category , bool)>;

    bool GetFilePathFromUser(const std::string &theFileFilter, const std::string &theInitialDirectory, std::string &theFilePathOut, bool fLoadFile, const std::string& theInitialFileName, CWnd *parentView);
    bool GetFilePathFromUserTotal(const std::string& theFileFilter, const std::string& theInitialDirectory, std::string& theFilePathOut, bool fLoadFile, const std::string& theInitialFileName, const std::string& theFileExtension, const std::string& theRootDirectory, CWnd* parentView);
    void SetLoggerFunction(LogAndWarnFunctionType &theLoggerFunction);
    LogAndWarnFunctionType GetLoggerFunction();

    template<typename ObjectType>
    bool SaveObjectToKnownFileInJsonFormat(const ObjectType &theObject, const std::string &theAbsoluteFilePath, const std::string &theObjectName, bool justLogMessages)
    {
            std::string errorString;
            if(ObjectType::StoreInJsonFormat(theObject, theAbsoluteFilePath, errorString))
            {
                std::string logMessage = std::string("Saving ") + theObjectName + " to file: " + theAbsoluteFilePath;
                if(GetLoggerFunction())
                    GetLoggerFunction()(logMessage, "", CatLog::Severity::Info, CatLog::Category::Data, true); // Onnistumiset vain logitetaan aina
                return true;
            }
            else
            {
                if(GetLoggerFunction())
                    GetLoggerFunction()(errorString, std::string("Saving ") + theObjectName + " failed", CatLog::Severity::Error, CatLog::Category::Data, justLogMessages);
                return false;
            }
    }

    template<typename ObjectType>
    bool SaveObjectInJsonFormat(const ObjectType &theObject, std::string &theInitialPath, const std::string &theFileFilter, const std::string &theFileExtension, const std::string &theBaseDirectory, const std::string &theObjectName, const std::string& theInitialFileName, bool justLogMessages, std::string *theUsedAbsoluteFilePath, CWnd* parentView)
    {
        std::string filePath;
        if(GetFilePathFromUser(theFileFilter, theInitialPath, filePath, false, theInitialFileName, parentView))
        {
            filePath = PathUtils::getTrueFilePath(filePath, theBaseDirectory, theFileExtension);
            if(theUsedAbsoluteFilePath)
                *theUsedAbsoluteFilePath = filePath;
            theInitialPath = PathUtils::getPathSectionFromTotalFilePath(filePath);
            return SaveObjectToKnownFileInJsonFormat(theObject, filePath, theObjectName, justLogMessages); // Virheist� tulee my�s messageBox jos niin halutaan
        }
        return false;
    }

    template<typename ObjectType>
    bool LoadObjectFromKnownFileInJsonFormat(ObjectType &theObject, const std::string &theAbsoluteFilePath, const std::string &theObjectName, bool justLogMessages)
    {
        std::string errorString;
        if(ObjectType::ReadInJsonFormat(theObject, theAbsoluteFilePath, errorString))
        {
            std::string logMessage = std::string("Loading ") + theObjectName + " from file: " + theAbsoluteFilePath;
            if(GetLoggerFunction())
                GetLoggerFunction()(logMessage, "", CatLog::Severity::Debug, CatLog::Category::Data, true); // Onnistumiset vain logitetaan aina
            return true;
        }
        else
        {
            if(GetLoggerFunction())
                GetLoggerFunction()(errorString, std::string("Loading ") + theObjectName + " failed", CatLog::Severity::Error, CatLog::Category::Data, justLogMessages); // Virheist� tulee my�s messageBox jos niin halutaan
            return false;
        }
    }


    template<typename ObjectType>
    bool LoadObjectInJsonFormat(ObjectType &theObject, std::string &theInitialPath, const std::string &theFileFilter, const std::string &theFileExtension, const std::string &theBaseDirectory, const std::string &theObjectName, bool justLogMessages, std::string *theUsedAbsoluteFilePath, CWnd* parentView)
    {
        std::string filePath;
        if(GetFilePathFromUser(theFileFilter, theInitialPath, filePath, true, "", parentView))
        {
            if(theUsedAbsoluteFilePath)
                *theUsedAbsoluteFilePath = filePath;
            theInitialPath = PathUtils::getPathSectionFromTotalFilePath(filePath);
            return LoadObjectFromKnownFileInJsonFormat(theObject, filePath, theObjectName, justLogMessages);
        }
        return false;
    }

}; // BetaProduct
