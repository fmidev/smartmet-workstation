#include "NFmiPathUtils.h"
#include "NFmiFileString.h"
#include "NFmiStringTools.h"

namespace
{
    // min 3 characters in path that start with a-z letter, then comes ':' and third letter is slash character in either way '\' or '/'
    bool hasDriveLetterInPath(const std::string &filePath)
    {
        if(filePath.size() >= 3)
        {
            if(::isalpha(filePath[0]) && filePath[1] == ':' && filePath[2] == '\\' || filePath[2] == '/')
                return true;
        }
        return false;
    }

}

namespace PathUtils
{
    // If filePath is absolute path without drive letter, but usedAbsoluteBaseDirectory contains drive letter with ':' character,
    // add that driveletter and ':' character to filePath's start.
    // Presumes that filePath is absolute path with or without drive letter like "\path\zzz" or "c:\path\zzz".
    // Presumes that usedAbsoluteBaseDirectory has absolute path with drive letter like "c:\path\zzz".
    std::string fixMissingDriveLetterToAbsolutePath(const std::string &filePath, const std::string &usedAbsoluteBaseDirectory)
    {
        if(!::hasDriveLetterInPath(filePath) && ::hasDriveLetterInPath(usedAbsoluteBaseDirectory))
        {
            std::string pathWithDriveLetter(usedAbsoluteBaseDirectory.begin(), usedAbsoluteBaseDirectory.begin() + 2);
            return pathWithDriveLetter + filePath;
        }
        else
            return filePath;
    }

    std::string getAbsoluteFilePath(const std::string &filePath, const std::string &usedAbsoluteBaseDirectory)
    {
        NFmiFileString fileString(filePath);
        if(fileString.IsAbsolutePath())
        {
            return fixMissingDriveLetterToAbsolutePath(filePath, usedAbsoluteBaseDirectory);
        }
        else
        {
            std::string absolutePath = usedAbsoluteBaseDirectory;
            absolutePath += kFmiDirectorySeparator;
            absolutePath += filePath;
            return absolutePath;
        }
    }

    std::string getPathSectionFromTotalFilePath(const std::string &theFilePath)
    {
        NFmiFileString filePath(theFilePath);
        std::string directoryPart = filePath.Device();
        directoryPart += filePath.Path();
        return directoryPart;
    }

    // Esim. theAbsoluteFilePath = D:\xxx\yyy\zzz\macro.mac, theBaseDirectory = D:\xxx ja theStrippedFileExtension = mac
    // => yyy\zzz\macro
    std::string getRelativeStrippedFileName(const std::string &theAbsoluteFilePath, const std::string &theBaseDirectory, const std::string &theStrippedFileExtension)
    {
        std::string relativeFilePath = getRelativePathIfPossible(theAbsoluteFilePath, theBaseDirectory);
        std::string usedFileExtension = theStrippedFileExtension;
        if(usedFileExtension.size() && usedFileExtension[0] != '.')
            usedFileExtension = "." + theStrippedFileExtension; // pitää mahdollisesti lisätä . -merkki alkuun
        return NFmiStringTools::ReplaceAll(relativeFilePath, usedFileExtension, "");
    }

    // Lisätään loppuun kenoviiva, jos siellä ei jo sellaista ole.
    void addDirectorySeparatorAtEnd(std::string &thePathInOut)
    {
        if(thePathInOut.size() && thePathInOut[thePathInOut.size() - 1] != '\\' && thePathInOut[thePathInOut.size() - 1] != '/')
            thePathInOut += kFmiDirectorySeparator;
    }

    // Yrittää palauttaa annetusta theFilePath:ista sen suhteellisen osion, joka jää jäljelle
    // theBaseDirectoryPath:in jälkeen. Jos theFilePath:in ja theBaseDirectoryPath eivät osu 
    // yhteen, palautetaan originaali arvo.
    // Jos theFilePath on suhteellinen polku, palautetaan originaali arvo.
    // Esim1: "C:\xxx\data.txt", "C:\xxx"   => "data.txt"
    // Esim2: "C:\xxx\data.txt", "C:\yyy"   => "C:\xxx\data.txt"
    // Esim3: "xxx\data.txt", "\xxx"      => "xxx\data.txt"
    std::string getRelativePathIfPossible(const std::string &theFilePath, const std::string &theBaseDirectoryPath)
    {
        if(!theBaseDirectoryPath.empty())
        {
            NFmiFileString filePathString(theFilePath);
            NFmiFileString baseDirectoryPathString(theBaseDirectoryPath);
            if(filePathString.IsAbsolutePath() && baseDirectoryPathString.IsAbsolutePath())
            {
                auto usedFilePath = doDriveLetterFix(filePathString, baseDirectoryPathString);
                std::string filePathLowerCase(usedFilePath);
                NFmiStringTools::LowerCase(filePathLowerCase);
                std::string baseDirectoryPathLowerCase(theBaseDirectoryPath);
                NFmiStringTools::LowerCase(baseDirectoryPathLowerCase);

                std::string::size_type pos = filePathLowerCase.find(baseDirectoryPathLowerCase);
                if(pos != std::string::npos)
                {
                    std::string relativePath(usedFilePath.begin() + theBaseDirectoryPath.size(), usedFilePath.end());
                    std::string::size_type pos2 = relativePath.find_first_not_of("\\/");
                    if(pos2 != std::string::npos)
                    {
                        // Otetaan vielä polun alusta pois mahdolliset kenoviivat
                        return std::string(relativePath.begin() + pos2, relativePath.end());
                    }
                    else
                        return relativePath;
                }
            }
        }

        return theFilePath;
    }

    // Yrittää hakea tiedostolle sen lopullisen absoluuttisen polun extensioineen kaikkineen.
    // Esim1 "beta1" "D:\betaProducts" "BetaProd"                           => D:\betaProducts\beta1.BetaProd"
    // Esim2 "D:\betaProducts\beta1.1" "D:\betaProducts" "BetaProd"         => D:\betaProducts\beta1.1.BetaProd"
    // Esim3 "D:\betaProducts\beta1.BetaProd" "D:\betaProducts" "BetaProd"  => D:\betaProducts\beta1.BetaProd"
    // Esim4 "xxx\beta1" "D:\betaProducts" "BetaProd"                       => D:\betaProducts\xxx\beta1.BetaProd"
    std::string getTrueFilePath(const std::string &theOriginalFilePath, const std::string &theRootDirectory, const std::string &theFileExtension, bool *extensionAddedOut)
    {
        if(extensionAddedOut)
            *extensionAddedOut = false;
        std::string filePath = theOriginalFilePath;
        NFmiStringTools::Trim(filePath); // Siivotaan annetusta polusta alusta ja lopusta white spacet

        if(filePath.empty())
            return filePath;
        else
        {
            // Tutkitaan onko kyseessä absoluuttinen vai suhteellinen polku 
            // ja tehdään lopullisesti tutkittava polku.
            NFmiFileString fileString(filePath);
            std::string finalFilePath;
            if(fileString.IsAbsolutePath())
                finalFilePath = filePath;
            else
            {
                finalFilePath = theRootDirectory;
                addDirectorySeparatorAtEnd(finalFilePath);
                finalFilePath += filePath;
            }

            // Lisätään vielä tarvittaessa polkuun tiedoston wmr -pääte
            std::string fileExtension = fileString.Extension();
            if(fileExtension.empty())
            {
                finalFilePath += "." + theFileExtension;
                if(extensionAddedOut)
                    *extensionAddedOut = true;
            }
            else
            {
                // Vaikka tiedostonimessä olisi extensio, se ei tarkoita että se olisi oikean tyyppinen (esim. beta4.1, missä '1' on väärän tyyppinen extensio)
                NFmiStringTools::LowerCase(fileExtension);
                std::string wantedFileExtensionLowerCase = theFileExtension;
                NFmiStringTools::LowerCase(wantedFileExtensionLowerCase);
                if(fileExtension != wantedFileExtensionLowerCase)
                {
                    finalFilePath += "." + theFileExtension;
                    if(extensionAddedOut)
                        *extensionAddedOut = true;
                }
            }
            return finalFilePath;
        }
    }

    // Cloud system based configurations try to use non-drive-letter absolute paths. But when you browse for file there is allways
    // drive letter involved.  In those cases, remove drive letter from files absolute path, e.g.:
    // C:\basepath\macros\macro.vmr   =>   \basepath\macros\macro.vmr
    std::string doDriveLetterFix(const NFmiFileString &filePathString, const NFmiFileString &baseDirectoryPathString)
    {
        if(baseDirectoryPathString.Device() == "")
            return std::string(filePathString.Path() + filePathString.FileName());
        else
            return std::string(filePathString);
    }
}
