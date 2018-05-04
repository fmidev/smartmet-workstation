// ======================================================================
/*!
 * \file NFmiEnumConverter.h
 * \brief Interface of class NFmiEnumConverter
 */
// ----------------------------------------------------------------------

#pragma once

#include "NFmiDef.h"
#include "NFmiGlobals.h"
#include "NFmiParameterName.h"
#include <cstring>
#include <list>
#include <map>
#include <string>

// Case insensitive < operator
struct myenumcomparator
{
  bool operator()(const char *a, const char *b) const
  {
#ifdef _MSC_VER
    // MSVC++ 2008 (or before) doesn't support strcasecmp-function so using _stricmp instead.
    return (::_stricmp(a, b) < 0);
#else
    return (::strcasecmp(a, b) < 0);
#endif
  }
};

class _FMI_DLL NFmiEnumConverter
{
 public:
  ~NFmiEnumConverter();
  NFmiEnumConverter(FmiEnumSpace theEnumspace = kParamNames);

  const char *ToCharPtr(int theValue);
  const std::string ToString(int theValue);

  int ToEnum(const char *s);
  int ToEnum(const std::string &theString) { return ToEnum(theString.c_str()); }
  std::list<std::string> Names();

  typedef std::map<const char *, int, myenumcomparator> storage_type;

 private:
  bool EnumTableInit();
  FmiEnumSpace itsEnumSpace;
  storage_type itsData;
  int itsBadEnum;

};  // class NFmiEnumConverter

// ======================================================================
