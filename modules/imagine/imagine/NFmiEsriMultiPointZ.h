// ======================================================================
//
// Esri Shapefile Techinical Description, page 16
//
//
// Position	Field	Value	Type	Number	Endian
//
// Byte 0	Type	8	int	1	little
// Byte 4	Box	Box	double	4	little
// Byte 36	NPoint	NPoints	int	1	little
// Byte 40	Points	Points	Point	NPoints	little
// Byte X	Zmin	Zmin	double	1	little
// Byte X+8	Zmax	Zmax	double	1	little
// Byte X+16	Zarry	Zarry	double	Npoints	little
// Byte Y*	Mmin	Mmin	double	1	little
// Byte Y+8*	Mmax	Mmax	double	1	little
// Byte Y+16*	Marry	Marry	double	NPoints	little
//
// Note:	X = 40 + 16 * NumPoints
//		Y = X + 16 + 8 * NumPoints
//
// ======================================================================

#pragma once

#include "NFmiEsriPointZ.h"
#include "NFmiEsriMultiPointM.h"
#include "NFmiEsriBox.h"

namespace Imagine
{
class NFmiEsriMultiPointZ : public NFmiEsriMultiPointM
{
 public:
  // Constructors, destructors

  ~NFmiEsriMultiPointZ(void) {}
  NFmiEsriMultiPointZ(const NFmiEsriMultiPointZ& thePoints);

  NFmiEsriMultiPointZ(int theNumber = 0, NFmiEsriElementType theType = kFmiEsriMultiPointZ)
      : NFmiEsriMultiPointM(theNumber, theType), itsBox(), itsPoints()
  {
  }

  NFmiEsriMultiPointZ(const std::string& theBuffer, int thePos = 0, int theNumber = 0);

  // Copying

  NFmiEsriMultiPointZ& operator=(const NFmiEsriMultiPointZ& thePoints);

  virtual NFmiEsriElement* Clone() const;

  // Data access

  const NFmiEsriBox& Box(void) const { return itsBox; }
  int NumPoints(void) const { return itsPoints.size(); }
  const std::vector<NFmiEsriPointZ>& Points(void) const { return itsPoints; }
  // This is intended to be used by projection etc methods

  void Points(const std::vector<NFmiEsriPointZ>& pts) { itsPoints = pts; }
  // Adding a point

  void Add(const NFmiEsriPointZ& thePoint)
  {
    itsPoints.push_back(thePoint);
    itsBox.Update(thePoint.X(), thePoint.Y(), thePoint.Z(), thePoint.M());
  }

  // Updating bounding boxes

  void Update(NFmiEsriBox& theBox) const { theBox.Update(itsBox); }
  // String buffer size, write and string

  int StringSize(void) const;
  std::ostream& Write(std::ostream& os) const;

 private:
  NFmiEsriBox itsBox;  // Bounding box

  // int	itsNumPoints;	// Number of points = size of itsPoints

  std::vector<NFmiEsriPointZ> itsPoints;
};

}  // namespace Imagine


// ======================================================================
