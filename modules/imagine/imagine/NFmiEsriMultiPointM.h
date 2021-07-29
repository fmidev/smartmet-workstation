// ======================================================================
//
// Esri Shapefile Techinical Description, page 11
//
//
// Position	Field	Value	Type	Number	Endian
//
// Byte 0	Type	28	int	1	little
// Byte 4	Box	Box	double	4	little
// Byte 36	NPoint	NPoints	int	1	little
// Byte 40	Points	Points	Point	NPoints	little
// Byte X*	Mmin	Mmin	double	1	little
// Byte X+8*	Mmax	Mmax	double	1	little
// Byte X+16*	Marry	Marry	double	NPoints	little
//
// Note: X = 40 + 16 * NumPoints
//
// ======================================================================

#pragma once

#include "NFmiEsriPointM.h"
#include "NFmiEsriMultiPoint.h"
#include "NFmiEsriBox.h"

namespace Imagine
{
class NFmiEsriMultiPointM : public NFmiEsriMultiPoint
{
 public:
  // Constructors, destructors

  ~NFmiEsriMultiPointM(void) {}
  NFmiEsriMultiPointM(const NFmiEsriMultiPointM& thePoints);

  NFmiEsriMultiPointM(int theNumber = 0, NFmiEsriElementType theType = kFmiEsriMultiPointM)
      : NFmiEsriMultiPoint(theNumber, theType), itsBox(), itsPoints()
  {
  }

  NFmiEsriMultiPointM(const std::string& theBuffer, int thePos = 0, int theNumber = 0);

  // Copying

  NFmiEsriMultiPointM& operator=(const NFmiEsriMultiPointM& thePoints);

  virtual NFmiEsriElement* Clone() const;

  // Data access

  const NFmiEsriBox& Box(void) const { return itsBox; }
  int NumPoints(void) const { return itsPoints.size(); }
  const std::vector<NFmiEsriPointM>& Points(void) const { return itsPoints; }
  // This is intended to be used by projection etc methods

  void Points(const std::vector<NFmiEsriPointM>& pts) { itsPoints = pts; }
  // Adding a point

  void Add(const NFmiEsriPointM& thePoint)
  {
    itsPoints.push_back(thePoint);
    itsBox.Update(thePoint.X(), thePoint.Y(), thePoint.M());
  }

  // Updating bounding boxes

  void Update(NFmiEsriBox& theBox) const { theBox.Update(itsBox); }
  // String buffer size, write and string

  int StringSize(void) const;
  std::ostream& Write(std::ostream& os) const;

 private:
  NFmiEsriBox itsBox;  // Bounding box

  // int	itsNumPoints;	// Number of points = size of itsPoints

  std::vector<NFmiEsriPointM> itsPoints;
};

}  // namespace Imagine


// ======================================================================
