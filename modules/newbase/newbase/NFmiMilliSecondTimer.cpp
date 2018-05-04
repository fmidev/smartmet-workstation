// ======================================================================
/*!
 * \file NFmiMilliSecondTimer.cpp
 * \brief Implementation of class NFmiMilliSecondTimer
 */

#include "NFmiMilliSecondTimer.h"
#include "NFmiValueString.h"

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiMilliSecondTimer::NFmiMilliSecondTimer()
{
  ftime(&itsTime1);
  ftime(&itsTime2);
}

std::string NFmiMilliSecondTimer::EasyTimeDiffStr(int theDiffInMS, bool fIgnoreMilliSeconds)
{
  static const double dayInMS = 1000. * 60 * 60 * 24;
  static const double hourInMS = 1000. * 60 * 60;
  static const double minuteInMS = 1000. * 60;
  int diffInMS = theDiffInMS;
  auto days = static_cast<int>(diffInMS / dayInMS);
  if (days > 0) diffInMS = static_cast<int>(diffInMS - days * dayInMS);
  auto hours = static_cast<int>(diffInMS / hourInMS);
  if (hours > 0) diffInMS = static_cast<int>(diffInMS - hours * hourInMS);
  auto minutes = static_cast<int>(diffInMS / minuteInMS);
  if (minutes > 0) diffInMS = static_cast<int>(diffInMS - minutes * minuteInMS);
  auto seconds = static_cast<int>(diffInMS / 1000.);
  int msecs = diffInMS % 1000;
  std::string result;
  bool printRest = false;
  if (days > 0)
  {
    printRest = true;
    result += NFmiStringTools::Convert<int>(days) + " d ";
  }
  if (hours > 0 || printRest)
  {
    printRest = true;
    result += NFmiStringTools::Convert<int>(hours) + " h ";
  }
  if (minutes > 0 || printRest)
  {
    // printRest = true;
    result += NFmiStringTools::Convert<int>(minutes) + " m ";
  }
  //	sekunnit tulee aina
  result += NFmiStringTools::Convert<int>(seconds) + " s ";
  if (fIgnoreMilliSeconds == false)
  {
    NFmiValueString valStr(msecs, "%03d");
    result += valStr.CharPtr();
    result += " ms ";
  }
  return result;
}

std::string NFmiMilliSecondTimer::EasyTimeDiffStr(bool fIgnoreMilliSeconds) const
{
  int diffInMS = TimeDiffInMSeconds();
  return NFmiMilliSecondTimer::EasyTimeDiffStr(diffInMS, fIgnoreMilliSeconds);
}
