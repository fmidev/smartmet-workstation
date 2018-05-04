#ifndef IMAGINE_WITH_CAIRO

// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace Imagine::NFmiImageTools
 */
// ======================================================================

#include "NFmiImageTools.h"
#include "NFmiImage.h"

#include <stdexcept>

using namespace std;

namespace Imagine
{
namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Compress integer in range 0-255 to given number of bits
 *
 *  - 0 reduces to values 0,FF
 *  - 1 reduces to values 0,80,FF
 *  - 2 reduces to values 0,40,80,C0,FF
 *  - 3 reduces to values 0,20,40,...,FF
 *  - 4 reduces to values 0,10,20,...,FF
 *  - 5 reduces to values 0,08,10,...,FF
 *  - 6 reduces to values 0,04,08,...,FF
 *  - 7 reduces to values 0,02,04,...,FF
 *  - 8 reduces to values 0,01,02,...,FF
 *
 * Each value is rounded to the value nearest to it
 */
// ----------------------------------------------------------------------

inline int compress_bits(int theValue, int theBits)
{
  if (theBits < 0 || theBits > 8)
    throw runtime_error("Invalid number of bits in NFmiImageTools::compress_bits");
  if (theBits == 8) return theValue;
  int round = (theValue >> (8 - theBits - 1)) & 1;
  int value = (theValue >> (8 - theBits)) + round;
  return min(255, value << (8 - theBits));
}

}  // namespace anonymous

namespace NFmiImageTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Reduce the accuracy of the color components
 *
 * This is a poor mans color reduction algorithm.
 *
 * \param theImage The image to reduce unique colors from
 * \param theRedBits The desired accuracy of red bits
 * \param theGreenBits The desired accuracy of green bits
 * \param theBlueBits The desired accuracy of blue bits
 * \param theAlphaBits The desired accuracy of alpha bits
 */
// ----------------------------------------------------------------------

void CompressBits(
    NFmiImage& theImage, int theRedBits, int theGreenBits, int theBlueBits, int theAlphaBits)
{
  using namespace NFmiColorTools;

  if (theRedBits < 0 || theRedBits > 8 || theGreenBits < 0 || theGreenBits > 8 || theBlueBits < 0 ||
      theBlueBits > 8 || theAlphaBits < 0 || theAlphaBits > 8)
  {
    throw runtime_error("Color resolution must be 0-8 bits");
  }

  for (int j = 0; j < theImage.Height(); j++)
    for (int i = 0; i < theImage.Width(); i++)
    {
      Color& c = theImage(i, j);
      const int r = compress_bits(GetRed(c), theRedBits);
      const int g = compress_bits(GetGreen(c), theGreenBits);
      const int b = compress_bits(GetBlue(c), theBlueBits);
      const int a = compress_bits(GetAlpha(c), theAlphaBits);
      c = MakeColor(r, g, b, a);
    }
}

// ----------------------------------------------------------------------
/*!
 * \brief Determine image mime-type from image header
 *
 * Throws if the image format is unknown
 *
 * \param theFileName The file containing the image
 * \return "png", "jpeg", "gif", "pnm" or "pgm"
 */
// ----------------------------------------------------------------------

std::string MimeType(const string& theFileName)
{
  FILE* in;
  in = fopen(theFileName.c_str(), "rb");
  if (in == NULL) throw runtime_error("Unable to determine image type of '" + theFileName + "'");

  unsigned char strmagic[4];
  size_t num = fread(strmagic, 1, 4, in);
  fclose(in);

  if (num != 4) throw runtime_error("Failed to read image magic number from '" + theFileName + "'");

  unsigned long magic = (static_cast<unsigned long>(strmagic[0]) << 24) +
                        (static_cast<unsigned long>(strmagic[1]) << 16) +
                        (static_cast<unsigned long>(strmagic[2]) << 8) +
                        (static_cast<unsigned long>(strmagic[3]));

  if (magic == 0xffd8ffe0)
    return "jpeg";
  else if (magic == 0x89504e47)
    return "png";
  else if (magic == 0x47494638)
    return "gif";
  else if (strmagic[0] == 'P' && strmagic[1] == '6' && strmagic[2] == '\n')
    return "pnm";
  else if (strmagic[0] == 'P' && strmagic[1] == '5' && strmagic[2] == '\n')
    return "pgm";
  else if (strmagic[0] == 'I' && strmagic[1] == 'I' && strmagic[2] == '*')
    return "tiff";
  else
    throw runtime_error("Unknown image format in '" + theFileName + "'");
}

}  // namespace NFmiImageTools

}  // namespace Imagine

// ======================================================================

#endif
// IMAGINE_WITH_CAIRO
