// ======================================================================
//
// NFmiImage addendum - JPEG reading and writing
//
// History:
//
// 15.10.2001 Mika Heiskanen
//
//	Implemented based on gd_jpeg.c and JPEG sample programs
//
// ======================================================================

#include "NFmiImage.h"

#ifdef IMAGINE_FORMAT_JPEG

// JPEG group idiots.. I wasted hours on this one

extern "C" {
#include <jpeglib.h>
}

using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// Read JPEG image
// ----------------------------------------------------------------------

void NFmiImage::ReadJPEG(FILE *in)
{
  // This struct contains the JPEG decompression parameters and pointers to
  // working space (which is allocated as needed by the JPEG library).

  struct jpeg_decompress_struct cinfo;

  // Step 1: allocate and initialize JPEG decompression object

  // We set up the normal JPEG error routines

  struct jpeg_error_mgr jerr;
  memset(&jerr, 0, sizeof(jerr));
  cinfo.err = jpeg_std_error(&jerr);

  // Now we can initialize the JPEG decompression object.

  jpeg_create_decompress(&cinfo);

  // Step 2: specify data source (eg, a file)

  jpeg_stdio_src(&cinfo, in);

  // Step 3: read file parameters with jpeg_read_header()

  static_cast<void>(jpeg_read_header(&cinfo, TRUE));

  // We can ignore the return value from jpeg_read_header since
  //   (a) suspension is not possible with the stdio data source, and
  //   (b) we passed TRUE to reject a tables-only JPEG file as an error.
  // See libjpeg.doc for more info.

  // Step 4: set parameters for decompression

  //  Force the image into RGB colorspace

  cinfo.out_color_space = JCS_RGB;

  // Step 5: Start decompressor

  static_cast<void>(jpeg_start_decompress(&cinfo));

  // We can ignore the return value since suspension is not possible
  // with the stdio data source.

  // We may need to do some setup of our own at this point before reading
  // the data.  After jpeg_start_decompress() we have the correct scaled
  // output image dimensions available, as well as the output colormap
  // if we asked for color quantization.
  // In this example, we need to make an output work buffer of the right size.

  // JSAMPLEs per row in output buffer

  // int row_stride = cinfo.output_width * cinfo.output_components;

  if (cinfo.output_components != 3)
    throw runtime_error("Failed to create 3 output channels in JPEG");

  // Make a one-row-high sample array that will go away when done with image

  JSAMPROW row = static_cast<JSAMPROW>(calloc(cinfo.output_width * 3, sizeof(JSAMPLE)));
  if (row == NULL) throw runtime_error("Failed to allocate memory for a JPEG data row");

  JSAMPROW rowptr[1];
  rowptr[0] = row;

  // Step 6: while (scan lines remain to be read)
  //           jpeg_read_scanlines(...);

  // This is the only NFmiImage method being used:

  Allocate(cinfo.output_width, cinfo.output_height);

  // Here we use the library's state variable cinfo.output_scanline as the
  // loop counter, so that we don't have to keep track ourselves.

  for (unsigned int j = 0; j < cinfo.output_height; j++)
  {
    // jpeg_read_scanlines expects an array of pointers to scanlines.
    // Here the array is only one element long, but you could ask for
    // more than one scanline at a time if that's more convenient.

    static_cast<void>(jpeg_read_scanlines(&cinfo, rowptr, 1));

    // Save the data

    for (unsigned int i = 0; i < cinfo.output_width; i++)
    {
      int r = row[3 * i];
      int g = row[3 * i + 1];
      int b = row[3 * i + 2];
      (*this)(i, j) = NFmiColorTools::MakeColor(r, g, b);
    }
  }

  // Step 7: Finish decompression

  static_cast<void>(jpeg_finish_decompress(&cinfo));

  // Step 8: Release JPEG decompression object

  // This is an important step since it will release a good deal of memory.

  jpeg_destroy_decompress(&cinfo);
}

// ----------------------------------------------------------------------
// Write JPEG image with desired quality (0-100)
// ----------------------------------------------------------------------

void NFmiImage::WriteJPEG(FILE *out) const
{
  int i, j;

  // This struct contains the JPEG compression parameters and pointers to
  // working space (which is allocated as needed by the JPEG library).
  // It is possible to have several such structures, representing multiple
  // compression/decompression processes, in existence at once.  We refer
  // to any one struct (and its associated working data) as a "JPEG object".

  struct jpeg_compress_struct cinfo;

  // This struct represents a JPEG error handler.  It is declared separately
  // because applications often want to supply a specialized error handler
  // (see the second half of this file for an example).  But here we just
  // take the easy way out and use the standard error handler, which will
  // print a message on stderr and call exit() if compression fails.
  // Note that this struct must live as long as the main JPEG parameter
  // struct, to avoid dangling-pointer problems.

  struct jpeg_error_mgr jerr;

  // Step 1: allocate and initialize JPEG compression object

  // We have to set up the error handler first, in case the initialization
  // step fails.  (Unlikely, but it could happen if you are out of memory.)
  // This routine fills in the contents of struct jerr, and returns jerr's
  // address which we place into the link field in cinfo.

  cinfo.err = jpeg_std_error(&jerr);

  // Now we can initialize the JPEG compression object.

  jpeg_create_compress(&cinfo);

  // Step 2: specify data destination (eg, a file)

  jpeg_stdio_dest(&cinfo, out);

  // Step 3: set parameters for compression

  // First we supply a description of the input image.
  // Four fields of the cinfo struct must be filled in:

  cinfo.image_width = itsWidth;  // image width and height, in pixels
  cinfo.image_height = itsHeight;
  cinfo.input_components = 3;      // # of color components per pixel
  cinfo.in_color_space = JCS_RGB;  // colorspace of input image

  // Now use the library's routine to set default compression parameters.
  // (You must set at least cinfo.in_color_space before calling this,
  // since the defaults depend on the source color space.)

  jpeg_set_defaults(&cinfo);

  // Now you can set any non-default parameters you wish to.
  // Here we just illustrate the use of quality (quantization table) scaling:

  jpeg_set_quality(&cinfo, itsJpegQuality, TRUE);

  // Step 4: Start compressor

  // TRUE ensures that we will write a complete interchange-JPEG file.
  // Pass TRUE unless you are very sure of what you're doing.

  jpeg_start_compress(&cinfo, TRUE);

  // Step 5: while (scan lines remain to be written)
  //           jpeg_write_scanlines(...);

  // To keep things simple, we pass one scanline per call; you can pass
  // more if you wish, though.

  JSAMPROW row = 0;
  JSAMPROW rowptr[1];

  row = static_cast<JSAMPROW>(
      calloc(1, cinfo.image_width * cinfo.input_components * sizeof(JSAMPLE)));

  if (row == 0)
  {
    jpeg_destroy_compress(&cinfo);
    throw NFmiImageMemoryError("Insufficient memory to allocate JPEG image");
  }
  rowptr[0] = row;

  for (j = 0; j < itsHeight; j++)
  {
    int offset = 0;
    for (i = 0; i < itsWidth; i++)
    {
      NFmiColorTools::Color c = (*this)(i, j);
      row[offset++] = NFmiColorTools::GetRed(c);
      row[offset++] = NFmiColorTools::GetGreen(c);
      row[offset++] = NFmiColorTools::GetBlue(c);
    }
    static_cast<void>(jpeg_write_scanlines(&cinfo, rowptr, 1));
  }

  // Step 6: Finish compression

  jpeg_finish_compress(&cinfo);

  // Step 7: release JPEG compression object

  // This is an important step since it will release a good deal of memory.

  jpeg_destroy_compress(&cinfo);

  // Done
}

}  // namespace Imagine

#endif  // IMAGINE_FORMAT_JPEG

// ======================================================================
