// ======================================================================
/*!
 * \file NFmiFileString.h
 * \brief Interface of class NFmiFileString
 */
// ======================================================================

#pragma once

#include "NFmiString.h"

//! Undocumented
class _FMI_DLL NFmiFileString : public NFmiString
{
 public:
  ~NFmiFileString(void);
  NFmiFileString(void);
  NFmiFileString(const NFmiFileString& theFileStr);
  NFmiFileString(const NFmiString& theStr);

  const NFmiString Extension(void) const;
  const NFmiString FileName(void) const;
  const NFmiString Header(void) const;
  const NFmiString Path(void) const;
  const NFmiString Device(void) const;
  const NFmiString Directory(void) const;

  bool HasExtension(void) const;
  bool IsAbsolutePath(void) const;

  void Extension(const NFmiString& theExtension);
  void FileName(const NFmiString& theExtension);
  void Header(const NFmiString& theExtension);
  void Path(const NFmiString& theExtension);
  void Device(const NFmiString& theExtension);

  void AddDirectory(const NFmiString& theDirectory);
  void DeleteDirectory(void);
  void ReplaceDirectory(const NFmiString& theDirectory);

  void Extension(const char* theExtension);
  void FileName(const char* theExtension);
  void Header(const char* theExtension);
  void Path(const char* theExtension);
  void Device(const char* theExtension);

  void NormalizeDelimiter(void);
  void ChangeScandinavian(void);

  const NFmiString PathWithoutLastDirectory(void);

};  // class NFmiFileString

// ======================================================================
