#include "NFmiMacroParamFunctions.h"
#include "NFmiStringTools.h"

using namespace std;

namespace MacroParam
{

    // oletus: thePath loppuu aina /-merkkiin (ellei tyhjä)
    void GetFileNames(const std::string &thePath, const std::string &theName, std::string &theDrawParamFileName, std::string &theMacroFileName, NFmiInfoData::Type theDataType)
    {
        string drawParamFileName(thePath);
        if(!theName.empty())
            drawParamFileName += theName;
        else
            throw runtime_error("NFmiMacroParamSystem::GetFileNames: no name for macroParam");
        string macroFileName(drawParamFileName);
        drawParamFileName += ".dpa";
        if(theDataType == NFmiInfoData::kQ3MacroParam)
            macroFileName += ".q3";
        else
            macroFileName += ".st";

        theDrawParamFileName = drawParamFileName;
        theMacroFileName = macroFileName;
    }

    // palauttaa tämän hetkisen suhteellisen polun ja ilman kenoviivoja alussa ja lopussa.
    // esim. "euroMakrot" tai "euroMakrot\analyysi"
    std::string GetRelativePath(const std::string &theRootPath, const std::string &theCurrentPath)
    {
        if(theRootPath == theCurrentPath)
            return std::string();
        std::string::size_type pos = ci_find_substr(theCurrentPath, theRootPath);
        if(pos != ci_string_not_found)
        {
            std::string tmp(theCurrentPath.begin() + pos + theRootPath.size(), theCurrentPath.end());
            NFmiStringTools::TrimL(tmp, kFmiDirectorySeparator);
            NFmiStringTools::TrimR(tmp, kFmiDirectorySeparator);
            return tmp;
        }
        throw std::runtime_error("NFmiMacroParamSystem::RelativePath - Polut ovat sekaisin, suhteellista polkua ei voi muodostaa.");
    }

    // poistaa viimeisen osan polusta
    // c:\data\src\inc\ -> c:\data\src\
    // eli inc pois esimerkistä
    void RemoveLastPartOfDirectory(string &thePath)
    {
        NFmiStringTools::TrimR(thePath, '\\');
        NFmiStringTools::TrimR(thePath, '/');
        string::size_type pos1 = thePath.find_last_of('/');
        string::size_type pos2 = thePath.find_last_of('\\');
        string::size_type usedPos = string::npos;
        if(pos1 != string::npos && pos2 != string::npos)
        {
            if(pos1 < pos2)
                usedPos = pos2;
            else
                usedPos = pos1;
        }
        else if(pos1 != string::npos)
            usedPos = pos1;
        else if(pos2 != string::npos)
            usedPos = pos2;

        if(usedPos != string::npos)
            thePath = string(thePath.begin(), thePath.begin() + usedPos + 1);
    }

    // tämä funktio muuttaa annetut polut seuraavaan muotoon:
    // 1. kaikki polkuerottimet ovat yksinkertaisia /-merkkejä
    // 2. kaikki polut loppuvat /-merkkiin
    // HUOM! ei poista vielä ylimääräisiä, jos on useita /-merkkejä peräkkäin
    std::string ConvertPathToOneUsedFormat(const std::string &thePath)
    {
        const char wantedDeliminor = kFmiDirectorySeparator;
        std::string tmp(thePath);
        NFmiStringTools::ReplaceChars(tmp, '/', wantedDeliminor);
        if(!(tmp.empty()) && tmp[tmp.size() - 1] != wantedDeliminor)
            tmp += wantedDeliminor;
        return tmp;
    }
}
