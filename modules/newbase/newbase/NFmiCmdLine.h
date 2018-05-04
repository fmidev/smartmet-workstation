// ======================================================================
/*!
 * \file NFmiCmdLine.h
 * \brief Interface of class NFmiCmdLine
 */
// ======================================================================

#pragma once

#include "NFmiStatus.h"

//! Undocumented
class _FMI_DLL NFmiCmdLine
{
 public:
  ~NFmiCmdLine(void);
  NFmiCmdLine(int argc, const char** argv, const char* optionsallowed);
  NFmiCmdLine(const NFmiString& theString, const char* optallow);

  char* Command(void) const;
  int NumberofOptions(void) const;
  int NumberofParameters(void) const;
  const char* Parameter(int i) const;
  char OptionLetter(int i) const;
  const char* OptionValue(int i) const;
  const char* OptionValue(char c) const;
  int isOption(char c) const;

  const NFmiStatus& Status(void) const;

 protected:
  NFmiStatus itsStatus;
  void Init(int argc, const char** argv, const char* optionsallowed);
  bool GetSpaceStringValue(NFmiString& theString, NFmiString& theValueString);
  int GetSpaceStringSize(const NFmiString& theString);

 private:
  NFmiCmdLine(const NFmiCmdLine& theCmdLine);
  NFmiCmdLine& operator=(const NFmiCmdLine& theCmdLine);

  int itsArgc;
  char** itsArgv;
  char* itsOptionsAllowed;
  char* itsCommand;
  char* itsOptionLetters;
  char** itsOptionValues;
  int itsOptionCount;
  int itsParameterCount;
  char** itsParameters;

};  // class NFmiCmdLine

// ======================================================================
