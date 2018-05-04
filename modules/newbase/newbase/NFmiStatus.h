// ======================================================================
/*!
 * \file NFmiStatus.h
 * \brief Interface of class NFmiStatus
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiString.h"

//! Undocumented

class _FMI_DLL NFmiStatus
{
 public:
  virtual ~NFmiStatus(void) {}
  NFmiStatus(void);
  NFmiStatus(const NFmiStatus& theStatus);

  NFmiStatus& operator=(const NFmiStatus& theStatus);
  operator bool(void) const;

  void ErrorLog(NFmiString& theErrorLog);
  void WarningLog(NFmiString& theWarningLog);
  void MessageLog(NFmiString& theMessageLog);

  void ErrorLog(const char* theErrorLog);
  void WarningLog(const char* theWarningLog);
  void MessageLog(const char* theMessageLog);

  bool IsError(void) const;
  bool IsWarning(void) const;
  bool IsMessage(void) const;

  const NFmiString ErrorLog(void) const;
  const NFmiString WarningLog(void) const;
  const NFmiString MessageLog(void) const;

  virtual std::ostream& Write(std::ostream& file) const;
  virtual std::istream& Read(std::istream& file);

 private:
  NFmiString itsErrorLog;
  NFmiString itsWarningLog;
  NFmiString itsMessageLog;

};  // class NFmiStatus

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should probably be const
 */
// ----------------------------------------------------------------------

inline NFmiStatus::operator bool(void) const { return !itsErrorLog.IsValue(); }

// ======================================================================
