// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::QueryDataTools
 */
// ======================================================================

#ifndef TEXTGEN_QUERYDATATOOLS_H
#define TEXTGEN_QUERYDATATOOLS_H

class NFmiFastQueryInfo;
class TextGenPosixTime;

namespace TextGen
{
namespace QueryDataTools
{
bool findIndices(NFmiFastQueryInfo& theQI,
                 const TextGenPosixTime& theStartTime,
                 const TextGenPosixTime& theEndTime,
                 unsigned long& theStartIndex,
                 unsigned long& theEndIndex);

bool firstTime(NFmiFastQueryInfo& theQI,
               const TextGenPosixTime& theTime,
               const TextGenPosixTime& theEndTime);

bool lastTime(NFmiFastQueryInfo& theQI, const TextGenPosixTime& theTime);
}
}

#endif  // TEXTGEN_QUERYDATATOOLS_H

// ======================================================================
