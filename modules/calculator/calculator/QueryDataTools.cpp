// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace QueryDataTools
 */
// ======================================================================
/*!
 * \namespace TextGen::QueryDataTools
 *
 * \brief Various small querydata related utility functions
 *
 * The namespace contains various useful utility functions
 * operating on querydata. The functions are not directly
 * available in newbase, but we do not wish to increase
 * the size of newbase unnecessarily with utility functions
 * which are mostly useful only in the text generator.
 *
 */
// ======================================================================

#include "QueryDataTools.h"

#include <newbase/NFmiFastQueryInfo.h>
#include "TextGenPosixTime.h"

namespace TextGen
{
namespace QueryDataTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Find integration index range
 *
 * The NFmiFastQueryInfo Time and ValidTime methods are
 * too slow to be used in loops, it is much better to use
 * the TimeIndex method. This function calculates
 * the desired loop indexes so that if you set the returned
 * start index, the querydata valid time is the first one
 * greater than or equal to the desired start time.
 * The end index is the first time greater than the
 * end time.
 *
 * \param theQI The query info
 * \param theStartTime The desired integration start time
 * \param theEndTime The desired end time
 * \param theStartIndex The variable into which the start index is stored
 * \param theEndIndex The variable into which the end index is stored
 * \return True, if both indexes were found
 */
// ----------------------------------------------------------------------

bool findIndices(NFmiFastQueryInfo& theQI,
                 const TextGenPosixTime& theStartTime,
                 const TextGenPosixTime& theEndTime,
                 unsigned long& theStartIndex,
                 unsigned long& theEndIndex)
{
  const unsigned long invalid = static_cast<unsigned long>(-1);

  if (!firstTime(theQI, theStartTime, theEndTime)) return false;
  theStartIndex = theQI.TimeIndex();
  if (!lastTime(theQI, theEndTime)) return false;
  theEndIndex = theQI.TimeIndex();

  return (theStartIndex != invalid && theEndIndex != invalid);
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the first integration time
 *
 * This effectively sets active the first time greater than or equal
 * to the given time. If there is no such time, false is returned.
 *
 * This is needed because newbase does not provide an equivalent
 * interface.
 *
 * \param theQI The query info
 * \param theTime The time to set
 * \param theEndTime The end time of the desired range
 * \return True if the time was set succesfully
 */
// ----------------------------------------------------------------------

bool firstTime(NFmiFastQueryInfo& theQI,
               const TextGenPosixTime& theTime,
               const TextGenPosixTime& theEndTime)
{
  // Abort if endtime is before start of date
  if (theQI.TimeDescriptor().FirstTime() > theEndTime) return false;

  theQI.FirstTime();
  unsigned long idx1 = theQI.TimeIndex();
  theQI.LastTime();
  unsigned long idx2 = theQI.TimeIndex();

  while (idx1 != idx2)
  {
    unsigned long idx = (idx1 + idx2) / 2;
    if (!theQI.TimeIndex(idx)) return false;
    if (!theQI.IsValidTime()) return false;
    if (theQI.ValidTime() >= theTime)
      idx2 = idx;
    else
      idx1 = idx + 1;
  }
  theQI.TimeIndex(idx1);

  return (theQI.IsValidTime() && theQI.ValidTime() >= theTime);
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the last integration time
 *
 * This effectively sets active the last time greater than
 * to the given time. If there is no such time, false is returned.
 *
 * This is needed because newbase does not provide an equivalent
 * interface.
 *
 * \param theQI The query info
 * \param theTime The time to set
 * \return True if the time was set succesfully
 */
// ----------------------------------------------------------------------

bool lastTime(NFmiFastQueryInfo& theQI, const TextGenPosixTime& theTime)
{
  theQI.FirstTime();
  unsigned long idx1 = theQI.TimeIndex();
  theQI.LastTime();
  unsigned long idx2 = theQI.TimeIndex();

  while (idx1 != idx2)
  {
    unsigned long idx = (idx1 + idx2) / 2;
    if (!theQI.TimeIndex(idx)) return false;

    if (!theQI.IsValidTime()) return false;
    if (theQI.ValidTime() > theTime)
      idx2 = idx;
    else
      idx1 = idx + 1;
  }
  theQI.TimeIndex(idx1);
  return (theQI.IsValidTime() && theQI.ValidTime() >= theTime);
}

}  // namespace QueryDataTools
}  // namespace TextGen

// ======================================================================
