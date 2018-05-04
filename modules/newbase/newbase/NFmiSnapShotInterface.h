// ======================================================================
/*!
 * \file NFmiSnapShotInterface.h
 * \brief Interface of class NFmiSnapShortInterface
 */
// ======================================================================

#pragma once

#include "NFmiQueryData.h"
#include "NFmiString.h"  // Added by ClassView
#include <ctime>

#ifdef UNIX
const NFmiString kFmiWorkingDirectory("/tmp/snapshot");
const NFmiString kFmiSourceDirectory("/var/www/share/querydata");
#else
const NFmiString kFmiWorkingDirectory("d:\\weto\\wrk\\data\\in\\");
const NFmiString kFmiSourceDirectory("\\\\vespa\\editdata\\");
#endif

//! Undocumented
class _FMI_DLL NFmiSnapShotInterface
{
 public:
  virtual ~NFmiSnapShotInterface(void);
  NFmiSnapShotInterface(NFmiString theDataFileName = "KEPA",
                        NFmiString theWorkingDirectory = kFmiWorkingDirectory,
                        NFmiString theSourceDirectory = kFmiSourceDirectory,
                        time_t theUpdateInterval = /*24*60**/ 60);

  bool IsValid(void);
  bool Update(NFmiQueryInfo** theInfo);

 protected:
  bool ReadData(void);

 protected:
  bool fIsValid;

 private:
  NFmiSnapShotInterface(const NFmiSnapShotInterface& theFace);
  NFmiSnapShotInterface& operator=(const NFmiSnapShotInterface theFace);

  time_t itsUpdateInterval;
  NFmiQueryInfo* itsInfo;
  NFmiQueryData* itsData;
  NFmiString itsDataFileName;
  NFmiString itsSourceDirectory;
  NFmiString itsWorkingDirectory;
  time_t itsStartingTime;

};  // class NFmiSnapShortInterface

// ======================================================================
