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

#include "NFmiEsriPointZ.h"
#include "NFmiEsriBuffer.h"

using namespace Imagine::NFmiEsriBuffer;  // Conversion tools
using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Copy constructor
// ----------------------------------------------------------------------

NFmiEsriPointZ::NFmiEsriPointZ(const NFmiEsriPointZ& thePoint)
    : NFmiEsriPointM(thePoint), itsZ(thePoint.itsZ)
{
}

// ----------------------------------------------------------------------
// Copying
// ----------------------------------------------------------------------

NFmiEsriPointZ& NFmiEsriPointZ::operator=(const NFmiEsriPointZ& thePoint)
{
  if (this != &thePoint)
  {
    NFmiEsriPointM::operator=(thePoint);
    itsZ = thePoint.itsZ;
  }
  return *this;
}

// ----------------------------------------------------------------------
// Cloning
// ----------------------------------------------------------------------

NFmiEsriElement* NFmiEsriPointZ::Clone() const { return new NFmiEsriPointZ(*this); }
// ----------------------------------------------------------------------
// Constructor based on a character buffer
// ----------------------------------------------------------------------

NFmiEsriPointZ::NFmiEsriPointZ(const string& theBuffer, int thePos, int theNumber)
    : NFmiEsriPointM(LittleEndianDouble(theBuffer, thePos + 4),
                     LittleEndianDouble(theBuffer, thePos + 12),
                     LittleEndianDouble(theBuffer, thePos + 28),
                     theNumber,
                     kFmiEsriPointZ),
      itsZ(LittleEndianDouble(theBuffer, thePos + 20))
{
}

// ----------------------------------------------------------------------
// Calculating string buffer size
// ----------------------------------------------------------------------

int NFmiEsriPointZ::StringSize(void) const
{
  return (4 + 4 * 8);  // int + 4 doubles
}

// ----------------------------------------------------------------------
// Writing element
// ----------------------------------------------------------------------

std::ostream& NFmiEsriPointZ::Write(ostream& os) const
{
  os << LittleEndianInt(Type()) << LittleEndianDouble(X()) << LittleEndianDouble(Y())
     << LittleEndianDouble(Z()) << LittleEndianDouble(M());
  return os;
}

}  // namespace Imagine

// ======================================================================
