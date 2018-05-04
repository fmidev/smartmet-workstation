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

#include "NFmiEsriMultiPointZ.h"
#include "NFmiEsriBuffer.h"

using namespace Imagine::NFmiEsriBuffer;  // Conversion tools
using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Copy constructor
// ----------------------------------------------------------------------

NFmiEsriMultiPointZ::NFmiEsriMultiPointZ(const NFmiEsriMultiPointZ& thePoints)
    : NFmiEsriMultiPointM(thePoints), itsBox(thePoints.itsBox), itsPoints(thePoints.itsPoints)
{
}

// ----------------------------------------------------------------------
// Assignment operator
// ----------------------------------------------------------------------

NFmiEsriMultiPointZ& NFmiEsriMultiPointZ::operator=(const NFmiEsriMultiPointZ& thePoints)
{
  if (this != &thePoints)
  {
    NFmiEsriMultiPointM::operator=(thePoints);
    itsBox = thePoints.itsBox;
    itsPoints = thePoints.itsPoints;
  }
  return *this;
}

// ----------------------------------------------------------------------
// Cloning
// ----------------------------------------------------------------------

NFmiEsriElement* NFmiEsriMultiPointZ::Clone() const { return new NFmiEsriMultiPointZ(*this); }
// ----------------------------------------------------------------------
// Constructor based on a character buffer
// ----------------------------------------------------------------------

NFmiEsriMultiPointZ::NFmiEsriMultiPointZ(const string& theBuffer, int thePos, int theNumber)
    : NFmiEsriMultiPointM(theNumber, kFmiEsriMultiPointZ), itsBox(), itsPoints()
{
  int npoints = LittleEndianInt(theBuffer, thePos + 36);

  // Speed up by reserving enough space already

  itsPoints.reserve(itsPoints.size() + npoints);

  for (int i = 0; i < npoints; i++)
  {
    // Start position 40, then 2 doubles (16) for each point

    int pointpos = thePos + 40 + i * 16;

    // Start position 40+16*n, then 2 doubles for range, then
    // 1 double for each measure

    int zpos = thePos + 40 + 16 * npoints + 16 + i * 8;

    // Offset to measure

    int mpos = zpos + 16 + 8 * npoints;

    Add(NFmiEsriPointZ(LittleEndianDouble(theBuffer, pointpos),
                       LittleEndianDouble(theBuffer, pointpos + 8),
                       LittleEndianDouble(theBuffer, zpos),
                       LittleEndianDouble(theBuffer, mpos)));
  }
}

// ----------------------------------------------------------------------
// Calculating string buffer size
// ----------------------------------------------------------------------

int NFmiEsriMultiPointZ::StringSize(void) const
{
  return (4  // the type	: 1 int
          +
          4 * 8  // bounding box : 4 doubles
          +
          4  // numpoints	: 1 int
          +
          NumPoints() * 2 * 8  // points	: 2n doubles
          +
          2 * 8  // zbox		: 2 doubles
          +
          NumPoints() * 8  // zvalues	: n doubles
          +
          2 * 8  // mbox		: 2 doubles
          +
          NumPoints() * 8  // mvalues	: n doubles
          );
}

// ----------------------------------------------------------------------
// Write the element
// ----------------------------------------------------------------------

std::ostream& NFmiEsriMultiPointZ::Write(ostream& os) const
{
  os << LittleEndianInt(Type()) << LittleEndianDouble(Box().Xmin())
     << LittleEndianDouble(Box().Ymin()) << LittleEndianDouble(Box().Xmax())
     << LittleEndianDouble(Box().Ymax()) << LittleEndianInt(NumPoints());

  int i;
  for (i = 0; i < NumPoints(); i++)
  {
    os << LittleEndianDouble(Points()[i].X()) << LittleEndianDouble(Points()[i].Y());
  }

  os << LittleEndianDouble(Box().Zmin()) << LittleEndianDouble(Box().Zmax());

  for (i = 0; i < NumPoints(); i++)  // 18.12.2001/Marko Redifinition of i removed.
    os << LittleEndianDouble(Points()[i].Z());

  os << LittleEndianDouble(Box().Mmin()) << LittleEndianDouble(Box().Mmax());

  for (i = 0; i < NumPoints(); i++)  // 18.12.2001/Marko Redifinition of i removed.
    os << LittleEndianDouble(Points()[i].M());

  return os;
}

}  // namespace Imagine

// ======================================================================
