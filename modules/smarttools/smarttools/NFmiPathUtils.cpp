#include "NFmiPathUtils.h"

#include "NFmiFileString.h"
#include "NFmiStringTools.h"
#include "NFmiSettings.h"
#include "boost/algorithm/string/replace.hpp"
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

namespace
{
// min 3 characters in path that start with a-z letter, then comes ':' and third letter is slash
// character in either way '\' or '/'
bool hasDriveLetterInPath(const std::string &filePath)
{
  if (filePath.size() >= 3)
  {
    if (::isalpha(filePath[0]) && filePath[1] == ':' && filePath[2] == '\\' || filePath[2] == '/')
      return true;
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
  if (!::hasDriveLetterInPath(filePath) && ::hasDriveLetterInPath(usedAbsoluteBaseDirectory))
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
  NFmiFileString fileString(filePath);
  if (fileString.IsAbsolutePath())
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
        "." + theStrippedFileExtension;  // pit‰‰ mahdollisesti lis‰t‰ . -merkki alkuun
  return NFmiStringTools::ReplaceAll(relativeFilePath, usedFileExtension, "");
}

// Lis‰t‰‰n loppuun kenoviiva, jos siell‰ ei jo sellaista ole.
void addDirectorySeparatorAtEnd(std::string &thePathInOut)
{
  if (thePathInOut.size() && thePathInOut[thePathInOut.size() - 1] != '\\' &&
      thePathInOut[thePathInOut.size() - 1] != '/')
    thePathInOut += kFmiDirectorySeparator;
}

// Yritt‰‰ palauttaa annetusta theFilePath:ista sen suhteellisen osion, joka j‰‰ j‰ljelle
// theBaseDirectoryPath:in j‰lkeen. Jos theFilePath:in ja theBaseDirectoryPath eiv‰t osu
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
    NFmiFileString filePathString(theFilePath);
    NFmiFileString baseDirectoryPathString(theBaseDirectoryPath);
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
        if (pos2 != std::string::npos)
        {
          // Otetaan viel‰ polun alusta pois mahdolliset kenoviivat
          return std::string(relativePath.begin() + pos2, relativePath.end());
        }
        else
          return relativePath;
      }
    }
  }

  return theFilePath;
}

// Yritt‰‰ hakea tiedostolle sen lopullisen absoluuttisen polun extensioineen kaikkineen.
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
  if (extensionAddedOut) *extensionAddedOut = false;
  std::string filePath = theOriginalFilePath;
  NFmiStringTools::Trim(filePath);  // Siivotaan annetusta polusta alusta ja lopusta white spacet

  if (filePath.empty())
    return filePath;
  else
  {
    // Tutkitaan onko kyseess‰ absoluuttinen vai suhteellinen polku
    // ja tehd‰‰n lopullisesti tutkittava polku.
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

    // Lis‰t‰‰n viel‰ tarvittaessa polkuun tiedoston wmr -p‰‰te
    std::string fileExtension = fileString.Extension();
    if (fileExtension.empty())
    {
      finalFilePath += "." + theFileExtension;
      if (extensionAddedOut) *extensionAddedOut = true;
    }
    else
    {
      // Vaikka tiedostonimess‰ olisi extensio, se ei tarkoita ett‰ se olisi oikean tyyppinen (esim.
      // beta4.1, miss‰ '1' on v‰‰r‰n tyyppinen extensio)
      NFmiStringTools::LowerCase(fileExtension);
      std::string wantedFileExtensionLowerCase = theFileExtension;
      NFmiStringTools::LowerCase(wantedFileExtensionLowerCase);
      if (fileExtension != wantedFileExtensionLowerCase)
      {
        finalFilePath += "." + theFileExtension;
        if (extensionAddedOut) *extensionAddedOut = true;
      }
    }
    return finalFilePath;
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
  std::experimental::filesystem::path originalPath(pathstring);
  // K‰‰nnet‰‰n varmuuden vuoksi kaikki separaattorit ensin windows tyylisiksi
  originalPath = originalPath.make_preferred();
  // T‰h‰n tulee windowsissa esim. D:
  auto rootNamePath = originalPath.root_name();
  // T‰h‰n tulee absoluuttinen polku ilman driveria, esim. \xxx\yyy
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

bool lastCharacterIsSeparator(const std::string &aPath)
{
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
  fixedPath = simplifyWindowsPath(fixedPath);
  if (fEnsureEndDirectorySeparator) addDirectorySeparatorAtEnd(fixedPath);
  return fixedPath;
}

std::string getFilename(const std::string& filePath) 
{
  std::experimental::filesystem::path originalPath(filePath);
  return originalPath.stem().string();
}


}  // namespace PathUtils
