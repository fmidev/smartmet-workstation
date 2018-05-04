// ======================================================================
/*!
 * \file WindChillQueryInfo.h
 * \brief Interface of class WindChillQueryInfo
 */
// ======================================================================

#ifndef WINDCHILLQUERYINFO_H
#define WINDCHILLQUERYINFO_H

#include <newbase/NFmiFastQueryInfo.h>

class WindChillQueryInfo : public NFmiFastQueryInfo
{
 public:
  WindChillQueryInfo(const NFmiFastQueryInfo& theInfo);

  float GetFloatValue(unsigned long theIndex) const;

  using NFmiFastQueryInfo::FloatValue;

 private:
  long itsParameterOffset;
};

#endif
