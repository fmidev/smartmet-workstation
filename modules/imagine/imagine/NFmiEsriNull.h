// ======================================================================
//
// Esri Shapefile Techinical Description, page 5
//
//
// Position	Field	Value	Type	Number	Endian
//
// 0		Type	0	int	1	little
//
// ======================================================================

#pragma once

#include "NFmiEsriElement.h"
#include "NFmiEsriBox.h"
#include "NFmiEsriBuffer.h"

namespace Imagine
{
using namespace NFmiEsriBuffer;  // Conversion tools

class NFmiEsriNull : public NFmiEsriElement
{
 public:
  // Constructors, destructors

  ~NFmiEsriNull(void) {}
  NFmiEsriNull(const NFmiEsriNull& theNull) : NFmiEsriElement(theNull) {}
  NFmiEsriNull(int theNumber = 0) : NFmiEsriElement(kFmiEsriNull, theNumber) {}
  // Copying

  NFmiEsriNull& operator=(const NFmiEsriNull& theNull)
  {
    if (this != &theNull) NFmiEsriElement::operator=(theNull);
    return *this;
  }

  virtual NFmiEsriElement* Clone() const { return new NFmiEsriNull(*this); }
  // Updating bounding boxes

  void Update(NFmiEsriBox& theBox) const {}
  void Project(const NFmiEsriProjector& theProjector) {}
  // Writing string buffer

  std::ostream& Write(std::ostream& os) const
  {
    os << LittleEndianInt(kFmiEsriNull);
    return os;
  }

  // Conversion to string buffer

  const std::string ToString(void) const { return LittleEndianInt(kFmiEsriNull); }
  // Calculating string buffer size

  int StringSize(void) const { return 4; }  // the type takes 4
};

}  // namespace Imagine


// ======================================================================
