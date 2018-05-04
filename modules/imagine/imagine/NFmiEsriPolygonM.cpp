// ======================================================================
//
// Esri Shapefile Techinical Description, page 13
//
//
// Position	Field	Value	Type	Number	Endian
//
// Byte 0	Type	5	int	1	little
// Byte 4	Box	Box	double	4	little
// Byte 36	NParts	NParts	int	1	little
// Byte 40	NPoints	NPoints	int	1	little
// Byte 44	Parts	Parts	int	NParts	little
// Byte X	Points	Points	point	NPoints	little
// Byte Y*	Mmin	Mmin	double	1	little
// Byte Y+8*	Mmax	Mmax	double	1	little
// Byte Y+16*	Marray	Marray	double	NPoints	little
//
// ======================================================================

#include "NFmiEsriPolygonM.h"
#include "NFmiEsriBuffer.h"

using namespace Imagine::NFmiEsriBuffer;  // Conversion tools
using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Copy constructor
// ----------------------------------------------------------------------

NFmiEsriPolygonM::NFmiEsriPolygonM(const NFmiEsriPolygonM& thePolygon)
    : NFmiEsriPolygon(thePolygon),
      itsBox(thePolygon.itsBox),
      itsParts(thePolygon.itsParts),
      itsPoints(thePolygon.itsPoints)
{
}

// ----------------------------------------------------------------------
// Assignment operator
// ----------------------------------------------------------------------

NFmiEsriPolygonM& NFmiEsriPolygonM::operator=(const NFmiEsriPolygonM& thePolygon)
{
  if (this != &thePolygon)
  {
    NFmiEsriPolygon::operator=(thePolygon);
    itsBox = thePolygon.itsBox;
    itsParts = thePolygon.itsParts;
    itsPoints = thePolygon.itsPoints;
  }
  return *this;
}

// ----------------------------------------------------------------------
// Cloning
// ----------------------------------------------------------------------

NFmiEsriElement* NFmiEsriPolygonM::Clone() const { return new NFmiEsriPolygonM(*this); }
// ----------------------------------------------------------------------
// Constructor based on a character buffer
// ----------------------------------------------------------------------

NFmiEsriPolygonM::NFmiEsriPolygonM(const string& theBuffer, int thePos, int theNumber)
    : NFmiEsriPolygon(theNumber, kFmiEsriPolygonM), itsBox(), itsParts(), itsPoints()
{
  int nparts = LittleEndianInt(theBuffer, thePos + 36);
  int npoints = LittleEndianInt(theBuffer, thePos + 40);

  // Speed up by reserving enough space already

  itsParts.reserve(itsParts.size() + nparts);
  itsPoints.reserve(itsPoints.size() + npoints);

  // Establish the parts

  int i = 0;
  for (i = 0; i < nparts; i++)
    itsParts.push_back(LittleEndianInt(theBuffer, thePos + 44 + 4 * i));

  // And the points

  for (i = 0; i < npoints; i++)
  {
    int pointpos = thePos + 44 + 4 * nparts + 16 * i;
    int measurepos = thePos + 44 + 4 * nparts + 16 * npoints + 16 + 8 * i;
    Add(NFmiEsriPointM(LittleEndianDouble(theBuffer, pointpos),
                       LittleEndianDouble(theBuffer, pointpos + 8),
                       LittleEndianDouble(theBuffer, measurepos)));
  }
}

// ----------------------------------------------------------------------
// Calculating string buffer size
// ----------------------------------------------------------------------

int NFmiEsriPolygonM::StringSize(void) const
{
  return (4  // the type	: 1 int
          +
          4 * 8  // bounding box : 4 doubles
          +
          4  // numparts	: 1 int
          +
          4  // numpoints	: 1 int
          +
          NumParts() * 4  // parts	: np ints
          +
          NumPoints() * 2 * 8  // points	: 2n doubles
          +
          2 * 8  // mbox		: 2 doubles
          +
          NumPoints() * 8  // mvalues	: n doubles
          );
}

// ----------------------------------------------------------------------
// Write the element
// ----------------------------------------------------------------------

std::ostream& NFmiEsriPolygonM::Write(ostream& os) const
{
  os << LittleEndianInt(Type()) << LittleEndianDouble(Box().Xmin())
     << LittleEndianDouble(Box().Ymin()) << LittleEndianDouble(Box().Xmax())
     << LittleEndianDouble(Box().Ymax()) << LittleEndianInt(NumParts())
     << LittleEndianInt(NumPoints());

  int i = 0;
  for (i = 0; i < NumParts(); i++)
    os << LittleEndianInt(Parts()[i]);

  for (i = 0; i < NumPoints(); i++)
  {
    os << LittleEndianDouble(Points()[i].X()) << LittleEndianDouble(Points()[i].Y());
  }

  os << LittleEndianDouble(Box().Mmin()) << LittleEndianDouble(Box().Mmax());

  for (i = 0; i < NumPoints(); i++)
    os << LittleEndianDouble(Points()[i].M());

  return os;
}

}  // namespace Imagine

// ======================================================================
