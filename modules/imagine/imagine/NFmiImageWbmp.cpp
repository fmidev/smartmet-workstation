// ======================================================================
//
// NFmiImage addendum - WBMP reading and writing
//
// History:
//
// 15.10.2001 Mika Heiskanen
//
// Implemented based on WAP-237-WAEMT-20010515-a section 4.5.1.
// and by using GD and ImageMagick code.
//
// ======================================================================

#include "NFmiImage.h"

#include <iostream>

using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
/*!
 * \brief An utility function to write a multibyte integer
 */
// ----------------------------------------------------------------------

void writembint(int theValue, FILE *out)
{
  int cnt = 0;
  int accu = 0;
  while (accu != theValue)
    accu += theValue & 0x7f << 7 * cnt++;
  for (int l = cnt - 1; l > 0; l--)
    fputc(0x80 | (theValue & 0x7f << 7 * l) >> 7 * l, out);
  fputc(theValue & 0x7f, out);
}

// ----------------------------------------------------------------------
// Write WBMP image
// ----------------------------------------------------------------------

void NFmiImage::WriteWBMP(FILE *out) const
{
  fputc(0, out);  // multibyteinteger 0 = WBMP type 0
  fputc(0, out);  // WBMP type 0 extension field is always zero

  writembint(itsWidth, out);
  writembint(itsHeight, out);

  for (int j = 0; j < itsHeight; j++)
  {
    int bitpos = 8;
    int octet = 0;
    for (int i = 0; i < itsWidth; i++)
    {
      int intensity = NFmiColorTools::Intensity(operator()(i, j));
      octet |= (intensity > 128 ? 1 : 0) << --bitpos;
      if (bitpos == 0)
      {
        fputc(octet, out);
        bitpos = 8;
        octet = 0;
      }
    }
    if (bitpos != 8) fputc(octet, out);
  }
}

}  // namespace Imagine

// ======================================================================
