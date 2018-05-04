// ======================================================================
//
// Esri Shapefile Techinical Description, page 15
//
//
// Position	Field	Value	Type	Number	Endian
//
// Byte 0	Type	11	int	1	little
// Byte 4	X	X	double	1	little
// Byte 12	Y	Y	double	1	little
// Byte 20	Z	Z	double	1	little
// Byte 28	Measure	M	double	1	little
//
// ======================================================================

#pragma once

#include "NFmiEsriPointM.h"
#include "NFmiEsriBox.h"

namespace Imagine
{
class _FMI_DLL NFmiEsriPointZ : public NFmiEsriPointM
{
 public:
  // Constructors, destructors

  ~NFmiEsriPointZ(void) {}
  NFmiEsriPointZ(const NFmiEsriPointZ& thePoint);

  NFmiEsriPointZ(double theX,
                 double theY,
                 double theZ,
                 double theM,
                 int theNumber = 0,
                 NFmiEsriElementType theType = kFmiEsriPointZ)
      : NFmiEsriPointM(theX, theY, theM, theNumber, theType), itsZ(theZ)
  {
  }

  NFmiEsriPointZ(const std::string& theBuffer, int thePos = 0, int theNumber = 0);

  // Copying

  NFmiEsriPointZ& operator=(const NFmiEsriPointZ& thePoint);

  virtual NFmiEsriElement* Clone() const;

  // Data access

  double Z(void) const { return itsZ; }
  void Z(double theZ) { itsZ = theZ; }
  // Updating bounding boxes

  void Update(NFmiEsriBox& theBox) const { theBox.Update(X(), Y(), Z(), M()); }
  // String buffer size, write and string

  int StringSize(void) const;
  std::ostream& Write(std::ostream& os) const;

 private:
  NFmiEsriPointZ(void);

  double itsZ;  // Z-coordinate
};

}  // namespace Imagine


// ======================================================================
