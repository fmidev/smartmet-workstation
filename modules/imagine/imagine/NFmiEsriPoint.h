// ======================================================================
//
// Esri Shapefile Techinical Description, page 6
//
//
// Position	Field	Value	Type	Number	Endian
//
// Byte 0	Type	1	int	1	little
// Byte 4	X	X	double	1	little
// Byte 12	Y	Y	double	1	little
//
// ======================================================================

#pragma once

#include "NFmiEsriElement.h"
#include "NFmiEsriBox.h"

namespace Imagine
{
class _FMI_DLL NFmiEsriPoint : public NFmiEsriElement
{
 public:
  // Constructors, destructors

  ~NFmiEsriPoint(void) {}
  NFmiEsriPoint(const NFmiEsriPoint& thePoint);

  NFmiEsriPoint(double theX,
                double theY,
                int theNumber = 0,
                NFmiEsriElementType theType = kFmiEsriPoint)
      : NFmiEsriElement(theType, theNumber), itsX(theX), itsY(theY)
  {
  }

  NFmiEsriPoint(const std::string& theBuffer, int thePos = 0, int theNumber = 0);

  // Copying

  NFmiEsriPoint& operator=(const NFmiEsriPoint& thePoint);

  virtual NFmiEsriElement* Clone() const;

  // Data access

  double X(void) const { return itsX; }
  double Y(void) const { return itsY; }
  void X(double theX) { itsX = theX; }
  void Y(double theY) { itsY = theY; }
  // Updating bounding boxes

  void Update(NFmiEsriBox& theBox) const { theBox.Update(itsX, itsY); }
  // String buffer size, write and string

  int StringSize(void) const;
  std::ostream& Write(std::ostream& os) const;

  // Projection

  void Project(const NFmiEsriProjector& theProjector);

 private:
  NFmiEsriPoint(void);

  double itsX;  // X coordinate
  double itsY;  // Y coordinate
};

}  // namespace Imagine


// ======================================================================
