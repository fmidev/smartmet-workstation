/*!
 *  \file NFmiSmartToolInfo.cpp \par
 *  Luokka pitää sisällään SmartTool:ia ja SmartToolDialogia
 *  koskevia tietoja.
 */

#include "NFmiSmartToolInfo.h"
#include <newbase/NFmiFileString.h>
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiSettings.h>
#include <fstream>
#include <iterator>
#include "boost/filesystem.hpp"

using namespace std;

NFmiSmartToolInfo::NFmiSmartToolInfo(void)
    : itsCurrentScript(),
      itsScriptFileExtension(),
      itsCurrentScriptName(),
      itsLoadDirectory(),
      itsRootLoadDirectory(),
      itsDBCheckerFileName(),
      itsDBCheckerText(),
      fMakeDBCheckAtSend(false),
      fIsThereDBCheckScript(false)
{
}

NFmiSmartToolInfo::~NFmiSmartToolInfo(void)
{
}
// luetaan  asetukset nykyään NFmiSetting-luokasta
bool NFmiSmartToolInfo::Init(const std::string &theLoadDirectory)
{
  LoadDirectory(theLoadDirectory, false);
  if (LoadSettings())
  {
    LoadScript(itsCurrentScriptName);
    return true;  // sillä ei ole väliä, vaikka viimeistä skriptiä ei saisikaan luettua kunhan
                  // asetukset on saatu
  }
  return false;
}

bool NFmiSmartToolInfo::LoadScript(const std::string &theScriptName)
{
  string fullFileName(GetFullScriptFileName(theScriptName));
  if (NFmiFileSystem::ReadFile2String(fullFileName, itsCurrentScript))
  {
    itsCurrentScriptName = theScriptName;
    return SaveSettings();
  }
  return false;
}

// theScriptName -parametrissa on mukana itsRootLoadDirectory:n suhteen polku esim. xxx\yyy\macro.st
bool NFmiSmartToolInfo::SpeedLoadScript(const std::string &theScriptName)
{
  // fullFileName saadaan yhdistämällä root ja annettu suhteellinen polku
  string fullFileName = itsRootLoadDirectory + theScriptName;
  // Lisäksi halutaan asettaa itsLoadDirectory osoittamaan annettuun suhteelliseen polkuun
  boost::filesystem::path loadPath = theScriptName;
  itsLoadDirectory = itsRootLoadDirectory + loadPath.parent_path().string() + "\\";

  if (NFmiFileSystem::ReadFile2String(fullFileName, itsCurrentScript))
  {
    itsCurrentScriptName = loadPath.stem().string();
    return SaveSettings();
  }
  return false;
}

bool NFmiSmartToolInfo::SaveScript(const std::string &theScriptName)
{
  string fullFileName(GetFullScriptFileName(theScriptName));
  if (WriteScript2File(fullFileName, itsCurrentScript))
  {
    itsCurrentScriptName = theScriptName;
    return SaveSettings();
  }
  return false;
}

bool NFmiSmartToolInfo::LoadDBChecker(void)
{
  if (NFmiFileSystem::ReadFile2String(itsDBCheckerFileName, itsDBCheckerText))
  {
    fIsThereDBCheckScript = true;
    return true;
  }
  fIsThereDBCheckScript = false;
  return false;
}

bool NFmiSmartToolInfo::SaveDBChecker(void)
{
  return WriteScript2File(itsDBCheckerFileName, itsDBCheckerText);
}

bool NFmiSmartToolInfo::WriteScript2File(const std::string &theFileName,
                                         const std::string &theScript)
{
  if (!theFileName.empty())
  {
    ofstream out(theFileName.c_str(), std::ios::binary);
    if (out)
    {
      out << theScript;
      return !out.fail();
    }
  }
  return false;
}

bool NFmiSmartToolInfo::LoadSettings(void)
{
  try
  {
    itsScriptFileExtension =
        NFmiSettings::Require<string>("MetEditor::SmartToolInfo::ScriptFileExtension");
    itsCurrentScriptName =
        NFmiSettings::Require<string>("MetEditor::SmartToolInfo::CurrentScriptName");
    fMakeDBCheckAtSend = NFmiSettings::Require<bool>("MetEditor::SmartToolInfo::MakeDBCheckAtSend");
    return true;
  }
  catch (exception & /* e */)
  {
    return false;
  }
}

bool NFmiSmartToolInfo::SaveSettings(void)
{
  try
  {
    NFmiSettings::Set(
        "MetEditor::SmartToolInfo::ScriptFileExtension", itsScriptFileExtension, true);
    NFmiSettings::Set("MetEditor::SmartToolInfo::CurrentScriptName", itsCurrentScriptName, true);
    NFmiSettings::Set(
        "MetEditor::SmartToolInfo::MakeDBCheckAtSend", fMakeDBCheckAtSend ? "true" : "false", true);
    return true;
  }
  catch (exception & /* e */)
  {
    return false;
  }
}

bool NFmiSmartToolInfo::ScriptExist(const std::string &theScriptName)
{
  checkedVector<string> names = GetScriptNames();
  checkedVector<string>::iterator it = std::find(names.begin(), names.end(), theScriptName);
  if (it != names.end())
    return true;
  return false;
}
checkedVector<std::string> NFmiSmartToolInfo::GetScriptNames(void)
{
  checkedVector<string> names;

  // listataan alkuun hakemistot ja jos ei olla rootissa, laitetaan vielä ..-hakemsito mukaan
  std::list<std::string> directories = NFmiFileSystem::Directories(itsLoadDirectory);
  std::list<std::string>::iterator itDir = directories.begin();
  std::list<std::string>::iterator itEndDir = directories.end();
  for (; itDir != itEndDir; ++itDir)
  {
    // "this"-hakemistoa ei laiteta
    if (*itDir == ".")
      continue;
    // jos ollaan ns. root hakemistossa, ei up-hakemistoa laiteta
    if (itsLoadDirectory == itsRootLoadDirectory && *itDir == "..")
      continue;
    std::string name("<");
    name += *itDir;
    name += ">";
    names.push_back(name);
  }

  string pattern(itsLoadDirectory);
  pattern += "*.";
  pattern += itsScriptFileExtension;
  list<string> tmpList = NFmiFileSystem::PatternFiles(pattern);

  list<string>::iterator it = tmpList.begin();
  for (; it != tmpList.end(); ++it)
  {
    NFmiFileString fileString(*it);
    names.push_back(string(static_cast<char *>(fileString.Header())));
  }
  return names;
}

std::string NFmiSmartToolInfo::GetFullScriptFileName(const std::string &theScriptName)
{
  string fullFileName(itsLoadDirectory);
  fullFileName += theScriptName;
  fullFileName += ".";
  fullFileName += itsScriptFileExtension;
  return fullFileName;
}

bool NFmiSmartToolInfo::RemoveScript(const std::string &theScriptName)
{
  string fullFileName(GetFullScriptFileName(theScriptName));
  return NFmiFileSystem::RemoveFile(fullFileName);
}

void NFmiSmartToolInfo::LoadDirectory(const std::string &newValue, bool fSaveSettings)
{
  itsLoadDirectory = newValue;
  if (itsLoadDirectory[itsLoadDirectory.size() - 1] == '/')
    itsLoadDirectory[itsLoadDirectory.size() - 1] = '\\';
  else if (itsLoadDirectory[itsLoadDirectory.size() - 1] != '\\')
    itsLoadDirectory += "\\";
  itsRootLoadDirectory = itsLoadDirectory;
  if (fSaveSettings)
    SaveSettings();
}

/*
 * poistaa viimeisen osan polusta c:\data\src\inc\ -> c:\data\src\
 * eli inc pois esimerkistä
 */

static void RemoveLastPartOfDirectory(string &thePath)
{
  NFmiStringTools::TrimR(thePath, '\\');
  NFmiStringTools::TrimR(thePath, '/');
  string::size_type pos1 = thePath.find_last_of('/');
  string::size_type pos2 = thePath.find_last_of('\\');
  string::size_type usedPos = string::npos;
  if (pos1 != string::npos && pos2 != string::npos)
  {
    if (pos1 < pos2)
      usedPos = pos2;
    else
      usedPos = pos1;
  }
  else if (pos1 != string::npos)
    usedPos = pos1;
  else if (pos2 != string::npos)
    usedPos = pos2;

  if (usedPos != string::npos)
    thePath = string(thePath.begin(), thePath.begin() + usedPos + 1);
}

void NFmiSmartToolInfo::SetCurrentLoadDirectory(const std::string &newValue)
{  // tässä ei aseteta root-directoria
   // nimi tulee <> sulkujen sisällä joten ne on poistettava ensin
  std::string usedDirectoryName(newValue);
  NFmiStringTools::TrimL(usedDirectoryName, '<');
  NFmiStringTools::TrimR(usedDirectoryName, '>');

  if (usedDirectoryName == "..")
  {
    RemoveLastPartOfDirectory(itsLoadDirectory);
  }
  else
  {
    itsLoadDirectory += usedDirectoryName;
    itsLoadDirectory += "\\";
  }
}

// Jos itsLoadDirectory     on C:\xxx\yyy\zzz
// ja itsRootLoadDirectory  on C:\xxx
// tällöin tämä funktio palauttaa arvon: yyy\zzz
std::string NFmiSmartToolInfo::GetRelativeLoadPath() const
{
  std::string relativePath = itsLoadDirectory;
  NFmiStringTools::ReplaceAll(relativePath, itsRootLoadDirectory, "");
  return relativePath;
}
