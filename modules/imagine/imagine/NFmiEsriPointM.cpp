// ======================================================================
//
// Esri Shapefile Techinical Description, page 10
//
//
// Position	Field	Value	Type	Number	Endian
//
// Byte 0	Type	21	int	1	little
// Byte 4	X	X	double	1	little
// Byte 12	Y	Y	double	1	little
// Byte 20	M	M	double	1	little
//
// ======================================================================

#include "NFmiEsriPointM.h"
#include "NFmiEsriBuffer.h"

using namespace Imagine::NFmiEsriBuffer;  // Conversion tools
using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Copy constructor
// ----------------------------------------------------------------------

NFmiEsriPointM::NFmiEsriPointM(const NFmiEsriPointM& thePoint)
    : NFmiEsriPoint(thePoint), itsM(thePoint.itsM)
{
}

// ----------------------------------------------------------------------
// Copying
// ----------------------------------------------------------------------

NFmiEsriPointM& NFmiEsriPointM::operator=(const NFmiEsriPointM& thePoint)
{
  if (this != &thePoint)
  {
    NFmiEsriPoint::operator=(thePoint);
    itsM = thePoint.itsM;
  }
  return *this;
}

// ----------------------------------------------------------------------
// Cloning
// ----------------------------------------------------------------------

NFmiEsriElement* NFmiEsriPointM::Clone() const { return new NFmiEsriPointM(*this); }
// ----------------------------------------------------------------------
// Constructor based on a character buffer
// ----------------------------------------------------------------------

NFmiEsriPointM::NFmiEsriPointM(const string& theBuffer, int thePos, int theNumber)
    : NFmiEsriPoint(LittleEndianDouble(theBuffer, thePos + 4),
                    LittleEndianDouble(theBuffer, thePos + 12),
                    theNumber,
                    kFmiEsriPointM),
      itsM(LittleEndianDouble(theBuffer, thePos + 20))
{
}

// ----------------------------------------------------------------------
// Calculating string buffer size
// ----------------------------------------------------------------------

int NFmiEsriPointM::StringSize(void) const
{
  return (4 + 3 * 8);  // int + 3 doubles
}

// ----------------------------------------------------------------------
// Writing element
// ----------------------------------------------------------------------

std::ostream& NFmiEsriPointM::Write(ostream& os) const
{
  os << LittleEndianInt(Type()) << LittleEndianDouble(X()) << LittleEndianDouble(Y())
     << LittleEndianDouble(M());
  return os;
}

}  // namespace Imagine

// ======================================================================
