// ======================================================================
//
// A simple class to hold the name and type of an attribute.
//
// The idea is that these are common for possibly thousands of elements,
// using them collectively via a pointer will save space.
//
// History:
//
// 30.08.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once

#include <newbase/NFmiGlobals.h>
#include <string>

namespace Imagine
{
enum NFmiEsriAttributeType
{
  kFmiEsriString,
  kFmiEsriInteger,
  kFmiEsriDouble
};

class _FMI_DLL NFmiEsriAttributeName
{
 public:
  // Constructor, destructor

  ~NFmiEsriAttributeName(void) {}
  NFmiEsriAttributeName(const std::string& theName,
                        NFmiEsriAttributeType theType,
                        int theFieldLength = -1,
                        int theDecimalCount = -1,
                        int theLength = -1)
      : itsName(theName),
        itsType(theType),
        itsFieldLength(static_cast<short>(theFieldLength)),
        itsDecimalCount(static_cast<short>(theDecimalCount)),
        itsLength(theLength)
  {
  }

  // Special helper constructors based on the associated data type
  // The idea is to avoid switch statements when calling the constructor

  NFmiEsriAttributeName(const std::string& theName,
                        const std::string& /* theValue */,
                        int theLength = -1)
      : itsName(theName),
        itsType(kFmiEsriString),
        itsFieldLength(-1),
        itsDecimalCount(-1),
        itsLength(theLength)
  {
  }

  NFmiEsriAttributeName(const std::string& theName,
                        int /* theValue */,
                        int theFieldLength = -1,
                        int theDecimalCount = -1)
      : itsName(theName),
        itsType(kFmiEsriInteger),
        itsFieldLength(static_cast<short>(theFieldLength)),
        itsDecimalCount(static_cast<short>(theDecimalCount)),
        itsLength(-1)
  {
  }

  NFmiEsriAttributeName(const std::string& theName,
                        double /* theValue */,
                        int theFieldLength = -1,
                        int theDecimalCount = -1)
      : itsName(theName),
        itsType(kFmiEsriDouble),
        itsFieldLength(static_cast<short>(theFieldLength)),
        itsDecimalCount(static_cast<short>(theDecimalCount)),
        itsLength(-1)
  {
  }

  // Data-access

  const std::string& Name(void) const { return itsName; }
  NFmiEsriAttributeType Type(void) const { return itsType; }
  short FieldLength() const { return itsFieldLength; }
  short DecimalCount() const { return itsDecimalCount; }
  short Length() const { return static_cast<short>(itsLength); }
 private:
  NFmiEsriAttributeName(void);

  std::string itsName;
  NFmiEsriAttributeType itsType;

  short itsFieldLength;
  short itsDecimalCount;
  int itsLength;
};

}  // namespace Imagine


// ======================================================================
