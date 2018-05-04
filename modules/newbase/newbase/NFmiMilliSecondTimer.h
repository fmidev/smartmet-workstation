// ======================================================================
/*!
 * \file NFmiMilliSecondTimer.h
 * \brief Interface of class NFmiMilliSecondTimer
 */
// ======================================================================
/*!
 * \class NFmiMilliSecondTimer
 *
 * Määrittely luokalle, jolla pystyy mittaamaan aikaa ainakin teoreettisesti
 * millisekunnin tarkkuudella. Todellisuudessa tarkkuus kai rajoittuu
 * muistaakseni 18 Hz:n tarkkuuteen eli n. 0.0555... sekunnin tarkkuuteen.
 */
// ======================================================================

#pragma once

#include "NFmiGlobals.h"
#include "NFmiStringTools.h"

extern "C" {
#include <sys/timeb.h>
}

//! Luokka koodin nopeusmittauksia varten

class _FMI_DLL NFmiMilliSecondTimer
{
 public:
  NFmiMilliSecondTimer();

  void StartTimer();
  void StopTimer();
  int TimeDiffInMSeconds() const;
  int CurrentTimeDiffInMSeconds()
      const;  // alkuajan ja nykyhetken erotus milli sekunneissa (eli kelloa ei pysäytetä)
  std::string EasyTimeDiffStr(bool fIgnoreMilliSeconds = false) const;
  static std::string EasyTimeDiffStr(int theDiffInMS, bool fIgnoreMilliSeconds = false);

  // Deprecated:
  void FirstTime();
  void SecondTime();

 private:
  int CalcTimeDiffInMS(const struct timeb &theTime1, const struct timeb &theTime2) const;

  struct timeb itsTime1;  //!< The start time
  struct timeb itsTime2;  //!< The end time

};  // class NFmiMilliSecondTimer

// ----------------------------------------------------------------------
/*!
 * Starts the timer
 *
 * \deprecated
 */
// ----------------------------------------------------------------------

inline void NFmiMilliSecondTimer::FirstTime() { ftime(&itsTime1); }
// ----------------------------------------------------------------------
/*!
 * Ends the timer
 *
 * \deprecated
 */
// ----------------------------------------------------------------------

inline void NFmiMilliSecondTimer::SecondTime() { ftime(&itsTime2); }
// ----------------------------------------------------------------------
/*!
 * Starts the timer
 */
// ----------------------------------------------------------------------

inline void NFmiMilliSecondTimer::StartTimer() { FirstTime(); }
// ----------------------------------------------------------------------
/*!
 * Ends the timer
 */
// ----------------------------------------------------------------------

inline void NFmiMilliSecondTimer::StopTimer() { SecondTime(); }
// ----------------------------------------------------------------------
/*!
 * Returns the measured time in milliseconds
 *
 * \return The measured time in milliseconds
 * \todo Make the method const
 */
// ----------------------------------------------------------------------

inline int NFmiMilliSecondTimer::TimeDiffInMSeconds() const
{
  return CalcTimeDiffInMS(itsTime1, itsTime2);
}

inline int NFmiMilliSecondTimer::CalcTimeDiffInMS(const struct timeb &theTime1,
                                                  const struct timeb &theTime2) const
{
  int seconds = static_cast<int>(theTime2.time - theTime1.time);
  int mseconds = seconds * 1000 + (theTime2.millitm - theTime1.millitm);
  return mseconds;
}

inline int NFmiMilliSecondTimer::CurrentTimeDiffInMSeconds() const
{
  struct timeb currentTime;
  ftime(&currentTime);
  return CalcTimeDiffInMS(itsTime1, currentTime);
}

// ======================================================================
