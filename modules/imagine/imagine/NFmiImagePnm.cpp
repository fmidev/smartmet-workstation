// ======================================================================
//
// NFmiImage addendum - PNM reading and writing
// ======================================================================

#include "NFmiImage.h"
#include <newbase/NFmiStringTools.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Read PNM image
// ----------------------------------------------------------------------

void NFmiImage::ReadPNM(FILE *in)
{
  const int maxbufsize = 1024;
  char buffer[maxbufsize + 1];

  // Safety against empty lines in future fgets calls
  buffer[0] = ' ';

  // The first line contains the P6 header

  fseek(in, 3, SEEK_CUR);

  // Then there may be multiple comment lines

  fgets(buffer, maxbufsize, in);
  while (buffer[0] == '#' || buffer[0] == '\n')
  {
    fgets(buffer, maxbufsize, in);
    if (strlen(buffer) == 0) throw runtime_error("Invalid PNM image data");
  }

  // Then there are the dimensions X Y

  int width, height;
  {
    istringstream input(buffer);
    input >> width >> height;
    if (input.bad()) throw runtime_error("Failed to read PNM dimensions");
    if (width <= 0 || height <= 0) throw runtime_error("PNM dimensions must be positive");
  }

  // And the size of each color component (normally 255)

  int colorsize;
  {
    fgets(buffer, maxbufsize, in);
    istringstream input(buffer);
    input >> colorsize;
    if (input.bad()) throw runtime_error("Failed to read PNM color size");
    if (colorsize != 255)
      throw runtime_error("Only colorsize 255 is supported for PNM images (size=" +
                          NFmiStringTools::Convert(colorsize) + ")");
  }

  // And then there is the raw data

  Allocate(width, height);

  for (int j = 0; j < height; j++)
    for (int i = 0; i < width; i++)
    {
      const int r = fgetc(in);
      const int g = fgetc(in);
      const int b = fgetc(in);
      if (r == EOF || g == EOF || b == EOF) throw runtime_error("PNM data ends abruptly");
      (*this)(i, j) = NFmiColorTools::MakeColor(r, g, b);
    }
}

// ----------------------------------------------------------------------
// Write PNM image
// ----------------------------------------------------------------------

void NFmiImage::WritePNM(FILE *out) const
{
  // First write the P6 header

  fprintf(out, "P6\n");

  // Then write the image dimensions

  fprintf(out, "%d %d\n", itsWidth, itsHeight);

  // Then write the color component size, which is always 255 in Imagine

  fprintf(out, "255\n");

  // Then write the image data itself

  for (int j = 0; j < itsHeight; j++)
    for (int i = 0; i < itsWidth; i++)
    {
      const NFmiColorTools::Color color = (*this)(i, j);
      const unsigned char red = NFmiColorTools::GetRed(color);
      const unsigned char green = NFmiColorTools::GetGreen(color);
      const unsigned char blue = NFmiColorTools::GetBlue(color);
      fprintf(out, "%c%c%c", red, green, blue);
    }
}

}  // namespace Imagine

// ======================================================================
