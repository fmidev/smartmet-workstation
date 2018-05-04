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
// ======================================================================

#include "NFmiEsriMultiPointM.h"
#include "NFmiEsriBuffer.h"

using namespace Imagine::NFmiEsriBuffer;  // Conversion tools
using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Copy constructor
// ----------------------------------------------------------------------

NFmiEsriMultiPointM::NFmiEsriMultiPointM(const NFmiEsriMultiPointM& thePoints)
    : NFmiEsriMultiPoint(thePoints), itsBox(thePoints.itsBox), itsPoints(thePoints.itsPoints)
{
}

// ----------------------------------------------------------------------
// Assignment operator
// ----------------------------------------------------------------------

NFmiEsriMultiPointM& NFmiEsriMultiPointM::operator=(const NFmiEsriMultiPointM& thePoints)
{
  if (this != &thePoints)
  {
    NFmiEsriMultiPoint::operator=(thePoints);
    itsBox = thePoints.itsBox;
    itsPoints = thePoints.itsPoints;
  }
  return *this;
}

// ----------------------------------------------------------------------
// Cloning
// ----------------------------------------------------------------------

NFmiEsriElement* NFmiEsriMultiPointM::Clone() const { return new NFmiEsriMultiPointM(*this); }
// ----------------------------------------------------------------------
// Constructor based on a character buffer
// ----------------------------------------------------------------------

NFmiEsriMultiPointM::NFmiEsriMultiPointM(const string& theBuffer, int thePos, int theNumber)
    : NFmiEsriMultiPoint(theNumber, kFmiEsriMultiPointM), itsBox(), itsPoints()
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

    int measurepos = thePos + 40 + 16 * npoints + 16 + i * 8;
    Add(NFmiEsriPointM(LittleEndianDouble(theBuffer, pointpos),
                       LittleEndianDouble(theBuffer, pointpos + 8),
                       LittleEndianDouble(theBuffer, measurepos)));
  }
}

// ----------------------------------------------------------------------
// Calculating string buffer size
// ----------------------------------------------------------------------

int NFmiEsriMultiPointM::StringSize(void) const
{
  return (4  // the type	: 1 int
          +
          4 * 8  // bounding box : 4 doubles
          +
          4  // numpoints	: 1 int
          +
          NumPoints() * 2 * 8  // points	: 2n doubles
          +
          2 * 8  // mbox		: 2 doubles
          +
          NumPoints() * 8  // mvalues	: n doubles
          );
}

// ----------------------------------------------------------------------
// Writing element
// ----------------------------------------------------------------------

std::ostream& NFmiEsriMultiPointM::Write(ostream& os) const
{
  os << LittleEndianInt(Type()) << LittleEndianDouble(Box().Xmin())
     << LittleEndianDouble(Box().Ymin()) << LittleEndianDouble(Box().Xmax())
     << LittleEndianDouble(Box().Ymax()) << LittleEndianInt(NumPoints());

  int i;
  for (i = 0; i < NumPoints(); i++)
  {
    os << LittleEndianDouble(Points()[i].X()) << LittleEndianDouble(Points()[i].Y());
  }

  os << LittleEndianDouble(Box().Mmin()) << LittleEndianDouble(Box().Mmax());

  for (i = 0; i < NumPoints(); i++)  // 18.12.2001/Marko Redifinition of i removed.
    os << LittleEndianDouble(Points()[i].M());

  return os;
}

}  // namespace Imagine

// ======================================================================
