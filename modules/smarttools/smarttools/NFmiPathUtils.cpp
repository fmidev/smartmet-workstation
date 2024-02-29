#include "NFmiPathUtils.h"

#include <newbase/NFmiFileString.h>
#include <newbase/NFmiSettings.h>
#include <newbase/NFmiStringTools.h>
#include "boost/algorithm/string/replace.hpp"
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

namespace
{
bool hasDriveLetterInPath(const std::string &filePath, bool mustHaveDriveLetter)
{
  // 1) Minimum of 3 characters in path that start with a-z letter, then comes ':' and third letter
  // is slash character in either way '\' or '/'
  if (filePath.size() >= 3)
  {
    if (::isalpha(filePath[0]) && filePath[1] == ':' && filePath[2] == '\\' || filePath[2] == '/')
      return true;
  }

  // With 'source' paths set mustHaveDriveLetter to true and with 'destination' paths to false.
  if (!mustHaveDriveLetter)
  {
    // 2) Minimum of 3 characters in path that starts with double slashes // or \\ and then a
    // letter, that means in Windows that this is absolute network path (e.g.
    // \\server\path_in_server) and it's treated like path with drive-letter in it.
    if (filePath.size() >= 3)
    {
      if ((filePath[0] == '\\' || filePath[0] == '/') &&
          (filePath[1] == '\\' || filePath[1] == '/') && ::isalpha(filePath[2]))
        return true;
    }
  }
  return false;
}

std::vector<std::string> split(std::string path, char d)
{
  std::vector<std::string> r;
  int j = 0;
  for (int i = 0; i < path.length(); i++)
  {
    if (path[i] == d)
    {
      std::string cur = path.substr(j, i - j);
      if (cur.length())
      {
        r.push_back(cur);
      }
      j = i + 1;  // start of next match
    }
  }
  if (j < path.length())
  {
    r.push_back(path.substr(j));
  }
  return r;
}

std::string simplifyUnixPath(std::string path)
{
  std::vector<std::string> ps = split(path, '/');
  std::string p = "";
  std::vector<std::string> st;
  for (int i = 0; i < ps.size(); i++)
  {
    if (ps[i] == "..")
    {
      if (st.size() > 0)
      {
        st.pop_back();
      }
    }
    else if (ps[i] != ".")
    {
      st.push_back(ps[i]);
    }
  }
  for (int i = 0; i < st.size(); i++)
  {
    p += "/" + st[i];
  }
  return p.length() ? p : "/";
}

}  // namespace

namespace PathUtils
{
// If filePath is absolute path without drive letter, but usedAbsoluteBaseDirectory contains drive
// letter with ':' character, add that driveletter and ':' character to filePath's start. Presumes
// that filePath is absolute path with or without drive letter like "\path\zzz" or "c:\path\zzz".
// Presumes that usedAbsoluteBaseDirectory has absolute path with drive letter like "c:\path\zzz".
std::string fixMissingDriveLetterToAbsolutePath(const std::string &filePath,
                                                const std::string &usedAbsoluteBaseDirectory)
{
  if (!::hasDriveLetterInPath(filePath, false) &&
      ::hasDriveLetterInPath(usedAbsoluteBaseDirectory, true))
  {
    std::string pathWithDriveLetter(usedAbsoluteBaseDirectory.begin(),
                                    usedAbsoluteBaseDirectory.begin() + 2);
    return pathWithDriveLetter + filePath;
  }
  else
    return filePath;
}

std::string getAbsoluteFilePath(const std::string &filePath,
                                const std::string &usedAbsoluteBaseDirectory)
{
  std::string finalAbsoluteFilePath;
  NFmiFileString fileString(filePath);
  if (fileString.IsAbsolutePath())
  {
    finalAbsoluteFilePath =
        fixMissingDriveLetterToAbsolutePath(filePath, usedAbsoluteBaseDirectory);
  }
  else
  {
    std::string absolutePath = usedAbsoluteBaseDirectory;
    if (!lastCharacterIsSeparator(absolutePath))
    {
      absolutePath += kFmiDirectorySeparator;
    }
    absolutePath += filePath;
    finalAbsoluteFilePath = absolutePath;
  }
  return simplifyWindowsPath(finalAbsoluteFilePath);
}

std::string getPathSectionFromTotalFilePath(const std::string &theFilePath)
{
  NFmiFileString filePath(theFilePath);
  std::string directoryPart = filePath.Device();
  directoryPart += filePath.Path();
  return simplifyWindowsPath(directoryPart);
}

// Esim. theAbsoluteFilePath = D:\xxx\yyy\zzz\macro.mac, theBaseDirectory = D:\xxx ja
// theStrippedFileExtension = mac
// => yyy\zzz\macro
std::string getRelativeStrippedFileName(const std::string &theAbsoluteFilePath,
                                        const std::string &theBaseDirectory,
                                        const std::string &theStrippedFileExtension)
{
  std::string relativeFilePath = getRelativePathIfPossible(theAbsoluteFilePath, theBaseDirectory);
  std::string usedFileExtension = theStrippedFileExtension;
  if (usedFileExtension.size() && usedFileExtension[0] != '.')
    usedFileExtension =
        "." + theStrippedFileExtension;  // pitää mahdollisesti lisätä . -merkki alkuun
  return NFmiStringTools::ReplaceAll(relativeFilePath, usedFileExtension, "");
}

// Lisätään loppuun kenoviiva, jos siellä ei jo sellaista ole.
void addDirectorySeparatorAtEnd(std::string &thePathInOut)
{
  if (thePathInOut.size() && thePathInOut[thePathInOut.size() - 1] != '\\' &&
      thePathInOut[thePathInOut.size() - 1] != '/')
    thePathInOut += kFmiDirectorySeparator;
}

// Yrittää palauttaa annetusta theFilePath:ista sen suhteellisen osion, joka jää jäljelle
// theBaseDirectoryPath:in jälkeen. Jos theFilePath:in ja theBaseDirectoryPath eivät osu
// yhteen, palautetaan originaali arvo.
// Jos theFilePath on suhteellinen polku, palautetaan originaali arvo.
// Esim1: "C:\xxx\data.txt", "C:\xxx"   => "data.txt"
// Esim2: "C:\xxx\data.txt", "C:\yyy"   => "C:\xxx\data.txt"
// Esim3: "xxx\data.txt", "\xxx"      => "xxx\data.txt"
std::string getRelativePathIfPossible(const std::string &theFilePath,
                                      const std::string &theBaseDirectoryPath)
{
  if (!theBaseDirectoryPath.empty())
  {
    NFmiFileString filePathString(fixPathSeparators(theFilePath));
    NFmiFileString baseDirectoryPathString(fixPathSeparators(theBaseDirectoryPath));
    if (filePathString.IsAbsolutePath() && baseDirectoryPathString.IsAbsolutePath())
    {
      auto usedFilePath = doDriveLetterFix(filePathString, baseDirectoryPathString);
      std::string filePathLowerCase(usedFilePath);
      NFmiStringTools::LowerCase(filePathLowerCase);
      std::string baseDirectoryPathLowerCase(theBaseDirectoryPath);
      NFmiStringTools::LowerCase(baseDirectoryPathLowerCase);

      std::string::size_type pos = filePathLowerCase.find(baseDirectoryPathLowerCase);
      if (pos != std::string::npos)
      {
        std::string relativePath(usedFilePath.begin() + theBaseDirectoryPath.size(),
                                 usedFilePath.end());
        std::string::size_type pos2 = relativePath.find_first_not_of("\\/");
        std::string finalRelativePath;
        if (pos2 != std::string::npos)
        {
          // Otetaan vielä polun alusta pois mahdolliset kenoviivat
          finalRelativePath = std::string(relativePath.begin() + pos2, relativePath.end());
        }
        else
        {
          finalRelativePath = relativePath;
        }
        return finalRelativePath;
      }
    }
  }

  return theFilePath;
}

// Yrittää hakea tiedostolle sen lopullisen absoluuttisen polun extensioineen kaikkineen.
// Esim1 "beta1" "D:\betaProducts" "BetaProd"                           =>
// D:\betaProducts\beta1.BetaProd" Esim2 "D:\betaProducts\beta1.1" "D:\betaProducts" "BetaProd" =>
// D:\betaProducts\beta1.1.BetaProd" Esim3 "D:\betaProducts\beta1.BetaProd" "D:\betaProducts"
// "BetaProd"  => D:\betaProducts\beta1.BetaProd" Esim4 "xxx\beta1" "D:\betaProducts" "BetaProd" =>
// D:\betaProducts\xxx\beta1.BetaProd"
std::string getTrueFilePath(const std::string &theOriginalFilePath,
                            const std::string &theRootDirectory,
                            const std::string &theFileExtension,
                            bool *extensionAddedOut)
{
  if (extensionAddedOut)
    *extensionAddedOut = false;
  std::string filePath = theOriginalFilePath;
  NFmiStringTools::Trim(filePath);  // Siivotaan annetusta polusta alusta ja lopusta white spacet

  if (filePath.empty())
    return filePath;
  else
  {
    // Tutkitaan onko kyseessä absoluuttinen vai suhteellinen polku
    // ja tehdään lopullisesti tutkittava polku.
    NFmiFileString fileString(filePath);
    std::string finalFilePath;
    if (fileString.IsAbsolutePath())
      finalFilePath = filePath;
    else
    {
      finalFilePath = theRootDirectory;
      addDirectorySeparatorAtEnd(finalFilePath);
      finalFilePath += filePath;
    }

    // Lisätään vielä tarvittaessa polkuun tiedoston wmr -pääte
    std::string fileExtension = fileString.Extension();
    if (fileExtension.empty())
    {
      finalFilePath += "." + theFileExtension;
      if (extensionAddedOut)
        *extensionAddedOut = true;
    }
    else
    {
      // Vaikka tiedostonimessä olisi extensio, se ei tarkoita että se olisi oikean tyyppinen (esim.
      // beta4.1, missä '1' on väärän tyyppinen extensio)
      NFmiStringTools::LowerCase(fileExtension);
      std::string wantedFileExtensionLowerCase = theFileExtension;
      NFmiStringTools::LowerCase(wantedFileExtensionLowerCase);
      if (fileExtension != wantedFileExtensionLowerCase)
      {
        finalFilePath += "." + theFileExtension;
        if (extensionAddedOut)
          *extensionAddedOut = true;
      }
    }
    return simplifyWindowsPath(finalFilePath);
  }
}

// Cloud system based configurations try to use non-drive-letter absolute paths. But when you browse
// for file there is allways drive letter involved.  In those cases, remove drive letter from files
// absolute path, e.g.: C:\basepath\macros\macro.vmr   =>   \basepath\macros\macro.vmr
std::string doDriveLetterFix(const NFmiFileString &filePathString,
                             const NFmiFileString &baseDirectoryPathString)
{
  if (baseDirectoryPathString.Device() == "")
    return std::string(filePathString.Path() + filePathString.FileName());
  else
    return std::string(filePathString);
}

std::string simplifyWindowsPath(const std::string &pathstring)
{
  auto fixedSeparatorPathString = fixPathSeparators(pathstring);
  std::experimental::filesystem::path originalPath(fixedSeparatorPathString);
  // Tähän tulee windowsissa esim. D:
  auto rootNamePath = originalPath.root_name();
  // Tähän tulee absoluuttinen polku ilman driveria, esim. \xxx\yyy
  std::string basicRootPathString =
      originalPath.root_directory().string() + originalPath.relative_path().string();
  auto unixRootPathString = boost::replace_all_copy(basicRootPathString, "\\", "/");
  auto simplifiedUnixRootPathString = simplifyUnixPath(unixRootPathString);
  auto simplifiedWindowsRootPathString =
      rootNamePath.string() + boost::replace_all_copy(simplifiedUnixRootPathString, "/", "\\");
  if (lastCharacterIsSeparator(pathstring))
    PathUtils::addDirectorySeparatorAtEnd(simplifiedWindowsRootPathString);
  return simplifiedWindowsRootPathString;
}

std::string fixPathSeparators(const std::string &pathstring)
{
  std::experimental::filesystem::path originalPath(pathstring);
  // Käännetään kaikki separaattorit oikein päin
  originalPath = originalPath.make_preferred();
  // Poistetaan kaikki tupla tai useammat peräkkäiset separaattorit
  // Esim. dir1\\dir2\dir3\file => dir1\dir2\dir3\file
  // paitsi jos kyse on Winows polusta missä on serveri nimi alussa
  // Esim. tässä ei alun tuplaa 'korjata': \\servername\dir1\dir2\file
  auto fixedPathString = originalPath.string();
  if (fixedPathString.size() < 2)
  {
    return fixedPathString;
  }

  std::string searchStr{kFmiDirectorySeparator, kFmiDirectorySeparator};
  std::string replaceStr{kFmiDirectorySeparator};
  bool isWinServerStart = (fixedPathString.front() == kFmiDirectorySeparator &&
                           fixedPathString[1] == kFmiDirectorySeparator);
  if (isWinServerStart)
  {
    // poistetaan 1. kirjain tässä poikkeustapauksessa
    fixedPathString.erase(0, 1);
  }

  for (;;)
  {
    // Tehdään korjauksia niin kauan, kunnes tulos-stringin koko ei eroa originaalin koosta.
    auto originalStr = fixedPathString;
    boost::replace_all(fixedPathString, searchStr, replaceStr);
    if (originalStr.size() == fixedPathString.size())
    {
      break;
    }
  }

  if (isWinServerStart)
  {
    std::string finalStr;
    finalStr += kFmiDirectorySeparator;
    finalStr += fixedPathString;
    return finalStr;
  }
  else
    return fixedPathString;
}

bool lastCharacterIsSeparator(const std::string &aPath)
{
  if (aPath.empty())
    return false;
  else
    return (aPath.back() == '\\' || aPath.back() == '/');
}

std::string getFixedAbsolutePathFromSettings(const std::string &theSettingsKey,
                                             const std::string &theAbsoluteWorkingPath,
                                             bool fEnsureEndDirectorySeparator)
{
  std::string settingPath = NFmiSettings::Require<std::string>(theSettingsKey);
  return makeFixedAbsolutePath(settingPath, theAbsoluteWorkingPath, fEnsureEndDirectorySeparator);
}

std::string makeFixedAbsolutePath(const std::string &thePath,
                                  const std::string &theAbsoluteWorkingPath,
                                  bool fEnsureEndDirectorySeparator)
{
  auto fixedPath = getAbsoluteFilePath(thePath, theAbsoluteWorkingPath);
  if (fEnsureEndDirectorySeparator)
    addDirectorySeparatorAtEnd(fixedPath);
  return fixedPath;
}

std::string getFilename(const std::string &filePath)
{
  std::experimental::filesystem::path originalPath(filePath);
  return originalPath.stem().string();
}

}  // namespace PathUtils
