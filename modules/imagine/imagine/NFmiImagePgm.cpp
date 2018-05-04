// ======================================================================
//
// NFmiImage addendum - PGM reading and writing
// ======================================================================

#include "NFmiImage.h"
#include <newbase/NFmiStringTools.h>

#include <iostream>
#include <sstream>

using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Read PGM image
// ----------------------------------------------------------------------

void NFmiImage::ReadPGM(FILE *in)
{
  const int maxbufsize = 1024;
  char buffer[maxbufsize + 1];

  // Safety against empty lines in future fgets calls
  buffer[0] = ' ';

  // The first line contains the P5 header

  fseek(in, 3, SEEK_CUR);

  // Then there may be multiple comment lines or empty lines

  fgets(buffer, maxbufsize, in);
  while (buffer[0] == '#' || buffer[0] == '\n')
  {
    fgets(buffer, maxbufsize, in);
    if (strlen(buffer) == 0) throw runtime_error("Invalid PGM image data");
  }

  // Then there are the dimensions X Y

  int width, height;
  {
    istringstream input(buffer);
    input >> width >> height;
    if (input.bad()) throw runtime_error("Failed to read PGM dimensions");
    if (width <= 0 || height <= 0) throw runtime_error("PGM dimensions must be positive");
  }

  // And the size of each color component (normally 255)

  int colorsize;
  {
    fgets(buffer, maxbufsize, in);
    istringstream input(buffer);
    input >> colorsize;
    if (input.bad()) throw runtime_error("Failed to read PGM color size");
    if (colorsize != 255)
      throw runtime_error("Only colorsize 255 is supported for PGM images (size=" +
                          NFmiStringTools::Convert(colorsize) + ")");
  }

  // And then there is the raw data

  Allocate(width, height);

  for (int j = 0; j < height; j++)
    for (int i = 0; i < width; i++)
    {
      const int gray = fgetc(in);
      if (gray == EOF) throw runtime_error("PGM data ends abruptly");
      (*this)(i, j) = NFmiColorTools::MakeColor(gray, gray, gray);
    }
}

// ----------------------------------------------------------------------
// Write PGM image
// ----------------------------------------------------------------------

void NFmiImage::WritePGM(FILE *out) const
{
  // First write the P5 header

  fprintf(out, "P5\n");

  // Then write the image dimensions

  fprintf(out, "%d %d\n", itsWidth, itsHeight);

  // Then write the color component size, which is always 255 in Imagine

  fprintf(out, "255\n");

  // Then write the image data itself

  for (int j = 0; j < itsHeight; j++)
    for (int i = 0; i < itsWidth; i++)
    {
      const NFmiColorTools::Color color = (*this)(i, j);
      const int gray = NFmiColorTools::Intensity(color);
      fprintf(out, "%c", gray);
    }
}

}  // namespace Imagine

// ======================================================================
