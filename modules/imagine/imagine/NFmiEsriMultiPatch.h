// ======================================================================
//
// Esri Shapefile Techinical Description, page 20
//
//
// Position	Field	Value	Type	Number	Endian
//
// Byte 0	Type	31	int	1	little
// Byte 4	Box	Box	double	4	little
// Byte 36	NParts	NParts	int	1	little
// Byte 40	NPoints	NPoints	int	1	little
// Byte 44	Parts	Parts	int	NParts	little
// Byte W	Ptypes	Ptypes	int	NParts	little
// Byte X	Points	Points	point	NPoints	little
// Byte Y	Zmin	Zmin	double	1	little
// Byte Y+8	Zmax	Zmax	double	1	little
// Byte Y+16	Zarray	Zarray	double	NPoints	little
// Byte Z*	Mmin	Mmin	double	1	little
// Byte Z+8*	Mmax	Mmax	double	1	little
// Byte Z+16*	Marray	Marray	double	NPoints	little
//
// Note: W = 44 + 4 * NumParts
//	 X = W + 4 * NumParts
//	 Y = X + 16 * NumPoints
//	 Z = Y + 16 + 8 * NumPoints
//
// ======================================================================

#pragma once

#include "NFmiEsriPointZ.h"
#include "NFmiEsriBox.h"

#include <vector>

namespace Imagine
{
// ESRI Shapefile Technical Description, page 20
//
// These are the part types that can be in a multipatch

enum NFmiEsriMultiPatchType
{
  kFmiEsriTriangleStrip = 0,
  kFmiEsriTriangleFan = 1,
  kFmiEsriOuterRing = 2,
  kFmiEsriInnerRing = 3,
  kFmiEsriFirstRing = 4,
  kFmiEsriRing = 5
};

class NFmiEsriMultiPatch : public NFmiEsriElement
{
 public:
  // Constructors, destructors

  ~NFmiEsriMultiPatch(void) {}
  NFmiEsriMultiPatch(int theNumber = 0)
      : NFmiEsriElement(kFmiEsriMultiPatch, theNumber),
        itsBox(),
        itsParts(),
        itsPartTypes(),
        itsPoints()
  {
  }

  NFmiEsriMultiPatch(const NFmiEsriMultiPatch& theElement);

  NFmiEsriMultiPatch(const std::string& theBuffer, int thePos = 0, int theNumber = 0);

  // Copying

  virtual NFmiEsriElement* Clone() const;

  // Data access

  const NFmiEsriBox& Box(void) const { return itsBox; }
  int NumPoints(void) const { return itsPoints.size(); }
  int NumParts(void) const { return itsParts.size(); }
  const std::vector<int>& Parts(void) const { return itsParts; }
  const std::vector<NFmiEsriMultiPatchType>& PartTypes(void) const { return itsPartTypes; }
  const std::vector<NFmiEsriPointZ>& Points(void) const { return itsPoints; }
  // This is intended to be used by projection etc methods

  void Points(const std::vector<NFmiEsriPointZ>& pts) { itsPoints = pts; }
  // Adding data

  void Add(const NFmiEsriPointZ& thePoint)
  {
    itsPoints.push_back(thePoint);
    itsBox.Update(thePoint.X(), thePoint.Y(), thePoint.Z(), thePoint.M());
    if (NumParts() == 0)
    {
      itsParts.push_back(0);
      itsPartTypes.push_back(kFmiEsriFirstRing);
    }
  }

  // Adding data and beginning a new part

  void AddPart(const NFmiEsriPointZ& thePoint, NFmiEsriMultiPatchType theType)
  {
    itsParts.push_back(NumPoints());  // index of next free location
    itsPartTypes.push_back(theType);  // the new type
    Add(thePoint);                    // and the first point in it
  }

  // Updating bounding boxes

  void Update(NFmiEsriBox& theBox) const { theBox.Update(itsBox); }
  // String buffer size, write and string

  int StringSize(void) const;
  std::ostream& Write(std::ostream& os) const;

  // Projection

  void Project(const NFmiEsriProjector& theProjector);

 private:
  NFmiEsriMultiPatch();

  NFmiEsriBox itsBox;  // Bounding box
  // int	itsNumParts;	// Number of parts = size of itsParts
  // int	itsNumPoints;	// Number of points = size of itsPoints

  std::vector<int> itsParts;                         // 1st point in part
  std::vector<NFmiEsriMultiPatchType> itsPartTypes;  // Part types
  std::vector<NFmiEsriPointZ> itsPoints;             // Points for all parts
};

}  // namespace Imagine


// ======================================================================
