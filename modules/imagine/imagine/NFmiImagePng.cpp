// ======================================================================
//
// NFmiImage addendum - PNG reading and writing
//
// History:
//
// 15.10.2001 Mika Heiskanen
//
//	Implemented based on gd_png.c and PNG: The Definitive Guide
//
// ======================================================================

#include "NFmiImage.h"

#ifdef IMAGINE_FORMAT_PNG

#include <png.h>
#include <map>
#include <iostream>
#include <cstdlib>

// Define png_jmpbuf() in case we are using a pre-1.0.6 version of libpng
#ifndef png_jmpbuf
#define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
#endif

using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Read PNG image
// If w,h are positive, cropping is performed on the fly
// ----------------------------------------------------------------------

void NFmiImage::ReadPNG(FILE *in)
{
  // Read in the signature and check it

  unsigned char sig[8];

  fread(sig, 1, 8, in);
  if (!(png_check_sig(sig, 8))) throw NFmiImageCorruptError("Incorrect signature in PNG image");

  // Initialize PNG struct

  png_structp png_ptr = NULL;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) throw NFmiImageMemoryError("Insufficient memory to allocate PNG structure");

  // Initialize info struct

  png_infop info_ptr = NULL;
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
  }

  // Jumper

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    throw NFmiImageCorruptError("Invalid PNG header");
  }

  // Preparations

  png_init_io(png_ptr, in);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);  // read info up to image data

  png_uint_32 width, height;
  int bit_depth, color_type;

  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

  // Allocate the final image

  Allocate(width, height);

  // expand palette images to RGB, low-bit-depth grayscale images to 8 bits,
  // transparency chunks to full alpha channel; strip 16-bit-per-sample
  // images to 8 bits per sample; and convert grayscale to RGB[A]

  if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8) png_set_expand(png_ptr);
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand(png_ptr);
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand(png_ptr);

  if (bit_depth == 16) png_set_strip_16(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);

  // all transformations have been registered; now update info_ptr data,
  // get rowbytes and channels, and allocate image memory

  png_read_update_info(png_ptr, info_ptr);

  int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
  int channels = static_cast<int>(png_get_channels(png_ptr, info_ptr));

  // Image data holder

  unsigned char *row_data = static_cast<unsigned char *>(malloc(rowbytes));

  if (row_data == NULL)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    throw NFmiImageMemoryError("Insufficient memory to allocate PNG row data");
  }

  // Read in the data one row at a time

  png_bytep row_pointer = row_data;
  png_byte r, g, b;
  png_byte a = NFmiColorTools::Opaque;  // Correct value for RGB

  for (unsigned j = 0; j < height; j++)
  {
    png_read_row(png_ptr, row_pointer, NULL);

    int boxoffset = 0;
    for (unsigned i = 0; i < width; i++)
    {
      r = row_data[boxoffset++];
      g = row_data[boxoffset++];
      b = row_data[boxoffset++];
      if (channels == 4) a = NFmiColorTools::MaxAlpha - (row_data[boxoffset++]) / 2;
      (*this)(i, j) = NFmiColorTools::MakeColor(r, g, b, a);
    }
  }

  // Cleanup

  png_read_end(png_ptr, info_ptr);

  if (row_data != NULL)
  {
    free(row_data);
    row_data = NULL;
  }
  if (png_ptr && info_ptr)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    png_ptr = NULL;
    info_ptr = NULL;
  }
}

// ----------------------------------------------------------------------
// Write PNG image
// ----------------------------------------------------------------------

void NFmiImage::WritePNG(FILE *out) const
{
  int i, j;

  // Allocate png info variables

  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr) throw NFmiImageMemoryError("Insufficient memory to allocate PNG write structure");

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_write_struct(&png_ptr, NULL);
    throw NFmiImageMemoryError("Insufficient memory to allocate PNG info structure");
  }

  // make sure outfile is (re)opened in BINARY mode

  png_init_io(png_ptr, out);

  // compression options

  png_set_compression_level(png_ptr, itsPngQuality);
  png_set_filter(png_ptr, 0, itsPngFilter);

  // Establish whether a palette version can be made

  set<NFmiColorTools::Color> theColors;
  bool truecolor = true;

  int maxcolors = 255;
  // Maybe should set this to -1 ??
  int opaquethreshold = itsAlphaLimit;

  // Establish whether we're saving RGB or RGBA

  bool savealpha = itsSaveAlphaFlag && !IsOpaque(opaquethreshold);
  bool ignorealpha = !savealpha;

  if (itsForcePaletteFlag)
  {
    bool overflow = AddColors(theColors, maxcolors, opaquethreshold, ignorealpha);

    // Should force quantization here if overflow occurred
    // For now we'll just use truecolor instead as if the
    // flag was WantPaletteFlag instead

    truecolor = overflow;
  }
  else if (itsWantPaletteFlag)
  {
    truecolor = AddColors(theColors, maxcolors, opaquethreshold, ignorealpha);
  }

  // Establish optional sRGB rendering intent

  bool use_srgb = false;
  int srgb_intent = -1;
  if (!itsIntent.empty())
  {
    use_srgb = true;
    if (itsIntent == "Saturation")
      srgb_intent = PNG_sRGB_INTENT_SATURATION;
    else if (itsIntent == "Perceptual")
      srgb_intent = PNG_sRGB_INTENT_PERCEPTUAL;
    else if (itsIntent == "Absolute")
      srgb_intent = PNG_sRGB_INTENT_ABSOLUTE;
    else if (itsIntent == "Relative")
      srgb_intent = PNG_sRGB_INTENT_RELATIVE;
    else
      use_srgb = false;  // Unrecognized intent!
  }

  // Write a true-color image

  if (truecolor)

  {
    // Now, if the image contains only opacities 0 and 127, we can
    // save RGB instead of RGBA. However, we need to find some RGB value
    // not in use to identify the transparent color.
    // Not implemented yet!

    bool separate = false;
    NFmiColorTools::Color transcolor = NFmiColorTools::NoColor;

    if (savealpha) separate = IsFullyOpaqueOrTransparent(opaquethreshold);
    if (separate) transcolor = UnusedColor();

    bool rgba = (separate ? false : savealpha);

    png_set_IHDR(png_ptr,
                 info_ptr,
                 itsWidth,
                 itsHeight,
                 8,
                 rgba ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    if (separate)
    {
      png_color_16 trans_rgb_value;
      trans_rgb_value.red = NFmiColorTools::GetRed(transcolor);
      trans_rgb_value.green = NFmiColorTools::GetGreen(transcolor);
      trans_rgb_value.blue = NFmiColorTools::GetBlue(transcolor);
      png_set_tRNS(png_ptr, info_ptr, 0, 0, &trans_rgb_value);
    }

    // Handle sRGB or GAMMA

    if (use_srgb)
      png_set_sRGB(png_ptr, info_ptr, srgb_intent);
    else if (itsGamma > 0.0)
      png_set_gAMA(png_ptr, info_ptr, 1.0 / itsGamma);

    // write all chunks up to (but not including) first IDAT

    png_write_info(png_ptr, info_ptr);

    // start writing

    int channels = (rgba ? 4 : 3);

    // Image data holder

    unsigned char *row_data = static_cast<unsigned char *>(malloc(channels * itsWidth));
    if (row_data == NULL)
    {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      throw NFmiImageMemoryError("Insufficient memory to allocate PNG write structure");
    }

    // Write the data one row at a time

    png_bytep row_pointer = row_data;

    png_byte a;
    for (j = 0; j < itsHeight; j++)
    {
      int boxoffset = 0;
      for (i = 0; i < itsWidth; i++)
      {
        NFmiColorTools::Color c = (*this)(i, j);
        a = NFmiColorTools::GetAlpha(c);
        if (separate && a == NFmiColorTools::MaxAlpha) c = transcolor;

        row_data[boxoffset++] = static_cast<png_byte>(NFmiColorTools::GetRed(c));
        row_data[boxoffset++] = static_cast<png_byte>(NFmiColorTools::GetGreen(c));
        row_data[boxoffset++] = static_cast<png_byte>(NFmiColorTools::GetBlue(c));
        if (channels == 4) row_data[boxoffset++] = 255 - ((a << 1) + (a >> 7));
      }
      png_write_row(png_ptr, row_pointer);
    }

    free(row_data);
  }

  // Write palette-image

  else

  {
    // This will hold transparencies of non-opaque colors

    png_byte transparent_values[256];
    int num_transparent = 0;

    // This will hold RGB values

    png_color color_values[256];
    int num_colors = 0;

    // This will hold a mapping of colors-indexes

    map<NFmiColorTools::Color, int> colormap;

    set<NFmiColorTools::Color>::const_iterator iter;

    // Two passes over the data

    for (int pass = 1; pass <= 2; pass++)
      for (iter = theColors.begin(); iter != theColors.end(); iter++)
      {
        int a = NFmiColorTools::GetAlpha(*iter);
        bool addnow;
        if (pass == 1)
        {
          if (a == 0)
            addnow = false;
          else
          {
            transparent_values[num_transparent++] = 255 - ((a << 1) + (a >> 7));
            addnow = true;
          }
        }
        else
          addnow = (a == 0);

        if (addnow)
        {
          color_values[num_colors].red = NFmiColorTools::GetRed(*iter);
          color_values[num_colors].green = NFmiColorTools::GetGreen(*iter);
          color_values[num_colors].blue = NFmiColorTools::GetBlue(*iter);
          colormap.insert(make_pair(*iter, num_colors));
          num_colors++;
        }
      }

    // Determine bit-depth from num_colors

    int bit_depth;
    if (num_colors <= 2)
      bit_depth = 1;
    else if (num_colors <= 4)
      bit_depth = 2;
    else if (num_colors <= 16)
      bit_depth = 4;
    else
      bit_depth = 8;

    // Temporary bug fix:

    bit_depth = 8;

    png_set_IHDR(png_ptr,
                 info_ptr,
                 itsWidth,
                 itsHeight,
                 bit_depth,
                 PNG_COLOR_TYPE_PALETTE,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_set_packing(png_ptr);

    // Set the transparent palette

    if (num_transparent > 0)
      png_set_tRNS(png_ptr, info_ptr, transparent_values, num_transparent, NULL);

    // Set the opaque palette

    png_set_PLTE(png_ptr, info_ptr, color_values, num_colors);

    // Handle sRGB or GAMMA

    if (use_srgb)
      png_set_sRGB(png_ptr, info_ptr, srgb_intent);
    else if (itsGamma > 0.0)
      png_set_gAMA(png_ptr, info_ptr, 1.0 / itsGamma);

    // write all chunks up to (but not including) first IDAT

    png_write_info(png_ptr, info_ptr);

    // Image data holder

    unsigned char *row_data = static_cast<unsigned char *>(malloc(itsWidth));
    if (row_data == NULL)
    {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      throw NFmiImageMemoryError("Insufficient memory to allocate PNG write row data");
    }

    // Write the data one row at a time

    png_bytep row_pointer = row_data;

    NFmiColorTools::Color lastcolor = NFmiColorTools::NoColor;
    png_byte lastindex = 0;

    for (j = 0; j < itsHeight; j++)
    {
      for (i = 0; i < itsWidth; i++)
      {
        NFmiColorTools::Color c = (*this)(i, j);
        if (c == lastcolor)
          row_data[i] = lastindex;
        else
        {
          lastcolor = c;
          c = NFmiColorTools::Simplify(c, opaquethreshold, ignorealpha);
          lastindex = static_cast<png_byte>(colormap[c]);
          row_data[i] = lastindex;
        }
      }
      png_write_row(png_ptr, row_pointer);
    }

    free(row_data);
  }

  png_write_end(png_ptr, info_ptr);

  // done

  png_destroy_write_struct(&png_ptr, &info_ptr);
}

}  // namespace Imagine

#endif  // IMAGINE_FORMAT_PNG

// ======================================================================
