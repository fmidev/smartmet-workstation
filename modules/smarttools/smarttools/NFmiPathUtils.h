#pragma once

#include <string>

class NFmiFileString;

namespace PathUtils
{
std::string fixMissingDriveLetterToAbsolutePath(const std::string &filePath,
                                                const std::string &usedAbsoluteBaseDirectory);
std::string getAbsoluteFilePath(const std::string &filePath,
                                const std::string &usedAbsoluteBaseDirectory);
std::string getPathSectionFromTotalFilePath(const std::string &theFilePath);
void addDirectorySeparatorAtEnd(std::string &thePathInOut);
std::string getRelativePathIfPossible(const std::string &theFilePath,
                                      const std::string &theBaseDirectoryPath);
std::string getTrueFilePath(const std::string &theOriginalFilePath,
                            const std::string &theRootDirectory,
                            const std::string &theFileExtension,
                            bool *extensionAddedOut = nullptr);
std::string getRelativeStrippedFileName(const std::string &theAbsoluteFilePath,
                                        const std::string &theBaseDirectory,
                                        const std::string &theStrippedFileExtension);
std::string doDriveLetterFix(const NFmiFileString &filePathString,
                             const NFmiFileString &baseDirectoryPathString);
std::string simplifyWindowsPath(const std::string &pathstring);
bool lastCharacterIsSeparator(const std::string &aPath);
std::string getFixedAbsolutePathFromSettings(const std::string &theSettingsKey,
                                             const std::string &theAbsoluteWorkingPath,
                                             bool fEnsureEndDirectorySeparator = false);
std::string makeFixedAbsolutePath(const std::string &thePath,
                                  const std::string &theAbsoluteWorkingPath,
                                  bool fEnsureEndDirectorySeparator = false);
std::string getFilename(const std::string &filePath);
std::string fixPathSeparators(const std::string &pathstring);

}  // namespace PathUtils
