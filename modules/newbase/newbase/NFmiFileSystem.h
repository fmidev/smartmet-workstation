// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiFileSystem
 */
// ======================================================================

#pragma once

#include "NFmiDef.h"
#include <ctime>   // time_t
#include <list>    // for ListDirectory
#include <string>  // filenames are strings

// Koska Winkkari headerissa
// on makro joka muuttaa funktion nimeä:
//
// #ifdef UNICODE
// #define CreateDirectory  CreateDirectoryW
// #else
// #define CreateDirectory  CreateDirectoryA
// #endif // !UNICODE

#if defined CreateDirectory
#undef CreateDirectory
#endif

// sama juttu CopyFile*:n kanssa
#if defined CopyFile
#undef CopyFile
#endif

namespace NFmiFileSystem
{
// File access checking:

_FMI_DLL bool FileExists(const std::string &theFile);
_FMI_DLL bool FileReadable(const std::string &theFile);
_FMI_DLL bool FileWritable(const std::string &theFile);
_FMI_DLL bool FileExecutable(const std::string &theFile);
_FMI_DLL size_t FileSize(const std::string &theFile);
_FMI_DLL bool FileEmpty(const std::string &theFile);

// Filename completion
_FMI_DLL const std::string FileComplete(const std::string &theFile,
                                        const std::string &theSearchPath);

std::time_t FileAccessTime(const std::string &theFile);
std::time_t FileModificationTime(const std::string &theFile);
std::time_t FileChangedTime(const std::string &theFile);
std::time_t FindFile(const std::string &theFileFilter,
                     bool fSearchNewest,
                     std::string *theFoundFileName);

_FMI_DLL long FileAge(const std::string &theFile);

_FMI_DLL bool DirectoryExists(const std::string &theFile);
_FMI_DLL bool DirectoryReadable(const std::string &theFile);
_FMI_DLL bool DirectoryWritable(const std::string &theFile);

_FMI_DLL bool CreateDirectory(const std::string &thePath);
_FMI_DLL const std::string NewestFile(const std::string &thePath);
_FMI_DLL std::time_t NewestFileTime(const std::string &thePath);
_FMI_DLL std::time_t NewestFileTime(const std::list<std::string> &theFileList,
                                    const std::string &thePath);
_FMI_DLL std::string NewestFileName(const std::list<std::string> &theFileList,
                                    const std::string &thePath);
_FMI_DLL std::time_t NewestPatternFileTime(const std::string &thePattern);
_FMI_DLL std::string NewestPatternFileName(const std::string &thePattern);
_FMI_DLL std::string FirstPatternFileName(const std::string &thePattern);
_FMI_DLL std::string PathFromPattern(const std::string &thePattern);
_FMI_DLL std::string FileNameFromPath(const std::string &theTotalPathAndFileStr);
_FMI_DLL const std::list<std::string> DirectoryFiles(const std::string &thePath);
_FMI_DLL const std::list<std::string> PatternFiles(const std::string &thePattern);
_FMI_DLL const std::list<std::pair<std::string, std::time_t> > PatternFiles(
    const std::string &thePattern, std::time_t timeLimit);
_FMI_DLL const std::list<std::string> Directories(const std::string &thePath);

_FMI_DLL bool RemoveFile(const std::string &theFile);
_FMI_DLL bool RenameFile(const std::string &theOldFileName, const std::string &theNewFileName);
_FMI_DLL bool CopyFile(const std::string &theSrcFileName, const std::string &theDstFileName);

_FMI_DLL bool ReadFile2String(const std::string &theFileName,
                              std::string &theFileContent,
                              long theMaxByteSize = 1024 * 1024 * 500);  // 500 MB
_FMI_DLL bool ReadFileStart2String(const std::string &theFileName,
                                   std::string &theFileContent,
                                   long theBytesFromStart);

_FMI_DLL const std::string TemporaryFile(const std::string &thePath);
_FMI_DLL const std::string BaseName(const std::string &theName);
_FMI_DLL const std::string DirName(const std::string &theName);

_FMI_DLL void CleanDirectory(const std::string &thePath,
                             double theMaxFileAgeInHours,
                             std::list<std::string> *theDeletedFilesOut = 0);
_FMI_DLL void CleanFilePattern(const std::string &theFilePattern,
                               int theKeepFileCount,
                               std::list<std::string> *theDeletedFilesOut = 0);

_FMI_DLL std::string FindQueryData(const std::string &thePath);
_FMI_DLL bool IsCompressed(const std::string &theName);
_FMI_DLL std::string MakeAbsolutePath(const std::string &theOrigPath,
                                      const std::string &theWorkingDirectory);
_FMI_DLL bool IsAbsolutePath(const std::string &thePath);

_FMI_DLL void SafeFileSave(const std::string &theFileName, const std::string &theContents);

}  // namespace NFmiFileSystem

// ======================================================================
