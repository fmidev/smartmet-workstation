// ======================================================================
//
// C++ Image API, defining basic image operations.
//
// See NFmiImage.h for documentation
//
// History:
//
// 31.07.2008 Asko Kauppi; adaptation to Cairo
// 13.08.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4996)  // VC++ k‰‰nt‰j‰ll‰ tulee paljon varoitusta 'turvattomien'
                                 // funktioiden k‰ytˆst‰, t‰ss‰ fopen pit‰isi muka korvata fopen_s
                                 // -funktiolla.
#endif

#include "NFmiColorBlend.h"
#include "NFmiColorReduce.h"

#include "NFmiImage.h"

#ifndef IMAGINE_WITH_CAIRO
#include "NFmiImageTools.h"
#endif

#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiStringTools.h>

#include <cstdlib>  // for rand, RAND_MAX
#include <iostream>
#include <algorithm>
#include <sstream>

extern "C" {
#ifndef UNIX
#include <io.h>  // Windows _mktemp
#endif
#ifdef IMAGINE_FORMAT_PNG
#include <png.h>  // for pnglib
#endif
}

using namespace std;

namespace Imagine
{
// ----------------------------------------------------------------------
// SimpleStroke filling via templates for each blending rule
// This one is optimized for blending rules when Color is
// faster to use than RGBA.
// ----------------------------------------------------------------------

// 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n virheen takia.
// Jouduin muuttamaan funktion nimen StrokeBasic2:ksi, koska muuten MSVC-k‰‰nt‰j‰ ei oikein
// osannut valita oikeaa funktiota.
// Jouduin laittamaan NFmiImage:n parametriksi (siihen annetaan kutsussa this-pointteri).
// Laiton viel‰ blenderin parametriksi, koska min‰ olen havainnut ongelmia MSVC-k‰‰nt‰j‰n kanssa,
// kun
// tekee seuraavanlaista koodia:
// Function1<MyClass1>();
// Pit‰‰ olla mieluummin:
// Function1(MyClass1());
#ifndef IMAGINE_WITH_CAIRO
template <class T>
static void StrokeBasic2(T theBlender,
                         float theX1,
                         float theY1,
                         float theX2,
                         float theY2,
                         NFmiColorTools::Color theColor,
                         NFmiImage &theImage)
{
  // Convert coodinates to device space

  int x1 = static_cast<int>(floor(theX1 + 0.5));
  int y1 = static_cast<int>(floor(theY1 + 0.5));
  int x2 = static_cast<int>(floor(theX2 + 0.5));
  int y2 = static_cast<int>(floor(theY2 + 0.5));

  // cout << "Rendering " << x1 << "," << y1 << " --> " << x2 << "," << y2
  // << " (" << theX1 << "," << theY1 << " --> " << theX2 << "," << theY2 << ")" << endl;

  // Reduce the number of cases by ordering endpoints
  // so that y2-y1 = deltaY >= 0

  if (y1 > y2)
  {
    swap(x1, x2);
    swap(y1, y2);
  }

  int dx = x2 - x1;
  int dy = y2 - y1;
  int dir = dx > 0 ? 1 : -1;
  dx = abs(dx);

  // Assign the first point

  //  (*this)(x1,y1) = T::Blend(theColor,(*this)(x1,y1));
  theImage(x1, y1) = theBlender.Blend(
      theColor,
      theImage(
          x1,
          y1));  // joudun k‰ytt‰m‰‰n .operaattoria :: osoituksen sijaan MSVC vaatii jostain syyst‰.

  if (dx > dy)
  {
    int adj1 = 2 * dy;
    int adj2 = adj1 - 2 * dx;
    int error = adj1 - dx;

    while (dx--)
    {
      if (error >= 0)
      {
        y1++;
        error += adj2;
      }
      else
        error += adj1;
      x1 += dir;

      //	  (*this)(x1,y1) = T::Blend(theColor,(*this)(x1,y1));
      theImage(x1, y1) = theBlender.Blend(theColor, theImage(x1, y1));  // joudun k‰ytt‰m‰‰n
                                                                        // .operaattoria ::
                                                                        // osoituksen sijaan MSVC
                                                                        // vaatii jostain syyst‰.
    }
  }
  else
  {
    int adj1 = 2 * dx;
    int adj2 = adj1 - 2 * dy;
    int error = adj1 - dy;
    while (dy--)
    {
      if (error >= 0)
      {
        x1 += dir;
        error += adj2;
      }
      else
        error += adj1;
      y1++;

      //	  (*this)(x1,y1) = T::Blend(theColor,(*this)(x1,y1));
      theImage(x1, y1) = theBlender.Blend(theColor, theImage(x1, y1));
    }
  }
}

// ----------------------------------------------------------------------
// SimpleStroke filling via templates for each blending rule
// This one is optimized for blending rules when RGBA is
// faster to use Color.
// ----------------------------------------------------------------------

// 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n virheen takia.
// Jouduin muuttamaan funktion nimen StrokeBasic2:ksi, koska muuten MSVC-k‰‰nt‰j‰ ei oikein
// osannut valita oikeaa funktiota.
// Jouduin laittamaan NFmiImage:n parametriksi (siihen annetaan kutsussa this-pointteri).
// Laiton viel‰ blenderin parametriksi, koska min‰ olen havainnut ongelmia MSVC-k‰‰nt‰j‰n kanssa,
// kun
// tekee seuraavanlaista koodia:
// Function1<MyClass1>();
// Pit‰‰ olla mieluummin:
// Function1(MyClass1());
template <class T>
static void StrokeBasic2(T theBlender,
                         float theX1,
                         float theY1,
                         float theX2,
                         float theY2,
                         int r,
                         int g,
                         int b,
                         int a,
                         NFmiImage &theImage)
{
  // Convert coodinates to device space

  int x1 = static_cast<int>(floor(theX1 + 0.5));
  int y1 = static_cast<int>(floor(theY1 + 0.5));
  int x2 = static_cast<int>(floor(theX2 + 0.5));
  int y2 = static_cast<int>(floor(theY2 + 0.5));

  // cout << "Rendering " << x1 << "," << y1 << " --> " << x2 << "," << y2
  // << " (" << theX1 << "," << theY1 << " --> " << theX2 << "," << theY2 << ")" << endl;

  // Reduce the number of cases by ordering endpoints
  // so that y2-y1 = deltaY >= 0

  if (y1 > y2)
  {
    swap(x1, x2);
    swap(y1, y2);
  }

  int dx = x2 - x1;
  int dy = y2 - y1;
  int dir = dx > 0 ? 1 : -1;
  dx = abs(dx);

  // Assign the first point

  //  (*this)(x1,y1) = T::Blend(r,g,b,a,(*this)(x1,y1));
  theImage(x1, y1) = theBlender.Blend(r, g, b, a, theImage(x1, y1));  // joudun k‰ytt‰m‰‰n
                                                                      // .operaattoria :: osoituksen
                                                                      // sijaan MSVC vaatii jostain
                                                                      // syyst‰.

  if (dx > dy)
  {
    int adj1 = 2 * dy;
    int adj2 = adj1 - 2 * dx;
    int error = adj1 - dx;

    while (dx--)
    {
      if (error >= 0)
      {
        y1++;
        error += adj2;
      }
      else
        error += adj1;
      x1 += dir;
      //	  (*this)(x1,y1) = T::Blend(r,g,b,a,(*this)(x1,y1));
      theImage(x1, y1) = theBlender.Blend(r, g, b, a, theImage(x1, y1));  // joudun k‰ytt‰m‰‰n
                                                                          // .operaattoria ::
                                                                          // osoituksen sijaan MSVC
                                                                          // vaatii jostain syyst‰.
    }
  }
  else
  {
    int adj1 = 2 * dx;
    int adj2 = adj1 - 2 * dy;
    int error = adj1 - dy;
    while (dy--)
    {
      if (error >= 0)
      {
        x1 += dir;
        error += adj2;
      }
      else
        error += adj1;
      y1++;
      //	  (*this)(x1,y1) = T::Blend(r,g,b,a,(*this)(x1,y1));
      theImage(x1, y1) = theBlender.Blend(r, g, b, a, theImage(x1, y1));  // joudun k‰ytt‰m‰‰n
                                                                          // .operaattoria ::
                                                                          // osoituksen sijaan MSVC
                                                                          // vaatii jostain syyst‰.
    }
  }
}

// ----------------------------------------------------------------------
// Image composition for a specific blending rule.
// The additional alpha component is the multiplier for the
// input image alpha channel.
// ----------------------------------------------------------------------

// 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n virheen takia.
// Jouduin muuttamaan funktion nimen Composite2:ksi, koska muuten MSVC-k‰‰nt‰j‰ ei oikein
// osannut valita oikeaa funktiota (luokalla on jo Composite-funktio).
// Jouduin laittamaan NFmiImage:n parametriksi (siihen annetaan kutsussa this-pointteri).
// Laiton viel‰ blenderin parametriksi, koska min‰ olen havainnut ongelmia MSVC-k‰‰nt‰j‰n kanssa,
// kun
// tekee seuraavanlaista koodia:
// Function1<MyClass1>();
// Pit‰‰ olla mieluummin:
// Function1(MyClass1());
template <class T>
static void Composite2(T theBlender,
                       const NFmiImage &thePattern,
                       int theX,
                       int theY,
                       float theAlpha,
                       NFmiImage &theThisImage)
{
  // Establish the pixels of the pattern inside the target image

  int i1 = std::max(0, -theX);
  int j1 = std::max(0, -theY);

  int i2 = std::min(thePattern.Width(), theThisImage.Width() - theX) - 1;
  int j2 = std::min(thePattern.Height(), theThisImage.Height() - theY) - 1;

  if (theAlpha == 1.0)
  {
    for (int j = j1; j <= j2; j++)
      for (int i = i1; i <= i2; i++)
        //	  (*this)(theX+i,theY+j) = T::Blend(thePattern(i,j),(*this)(theX+i,theY+j));
        theThisImage(theX + i, theY + j) = theBlender.Blend(
            thePattern(i, j), theThisImage(theX + i, theY + j));  // joudun k‰ytt‰m‰‰n .operaattoria
                                                                  // :: osoituksen sijaan MSVC
                                                                  // vaatii jostain syyst‰.
  }
  else
  {
    NFmiColorTools::Color c;
    const float beta = (1.0 - theAlpha) * NFmiColorTools::MaxAlpha;
    int a, aa;

    for (int j = j1; j <= j2; j++)
      for (int i = i1; i <= i2; i++)
      {
        c = thePattern(i, j);
        a = NFmiColorTools::GetAlpha(c);
        aa = static_cast<int>(theAlpha * a + beta);
        c = NFmiColorTools::ReplaceAlpha(c, aa);
        //	    (*this)(theX+i,theY+j) = T::Blend(c,(*this)(theX+i,theY+j));
        theThisImage(theX + i, theY + j) = theBlender.Blend(
            c, theThisImage(theX + i, theY + j));  // joudun k‰ytt‰m‰‰n .operaattoria :: osoituksen
                                                   // sijaan MSVC vaatii jostain syyst‰.
      }
  }
}
#endif
// not IMAGINE_WITH_CAIRO

// ----------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------

void NFmiImage::Destroy(void)
{
  if (itsPixels != NULL) delete[] itsPixels;
  itsPixels = NULL;
}

// ----------------------------------------------------------------------
// Constructor based on size.
// The image is initialized with the given color
// ----------------------------------------------------------------------

NFmiImage::NFmiImage(int theWidth, int theHeight, NFmiColorTools::Color theColor)
{
  DefaultOptions();
  Allocate(theWidth, theHeight);
  Erase(theColor);
}

// ----------------------------------------------------------------------
// Copy constructor
// ----------------------------------------------------------------------

NFmiImage::NFmiImage(const NFmiImage &theImage)
#ifndef IMAGINE_WITH_CAIRO
    : NFmiDrawable()
#endif
{
  DefaultOptions();
  itsType = theImage.itsType;
  Allocate(theImage.Width(), theImage.Height());
  memcpy(itsPixels, theImage.itsPixels, itsWidth * itsHeight * sizeof(NFmiColorTools::Color));
}

// ----------------------------------------------------------------------
// Constructor based on filename. The file type is autodetected.
// ----------------------------------------------------------------------
#ifndef IMAGINE_WITH_CAIRO
NFmiImage::NFmiImage(const string &theFileName) : itsPixels(NULL)
{
  DefaultOptions();
  Read(theFileName);
}
#endif

// ----------------------------------------------------------------------
// Constructor from raw Cairo ARGB_32 image data, for output using NFmiImage
// (format support wider than Cairo's, and color reduction)
//    -AKa 4-Aug-2008
// ----------------------------------------------------------------------
NFmiImage::NFmiImage(int w, int h, const NFmiColorTools::Color *data)
/* : NFmiDrawable() */
{
  DefaultOptions();
  Allocate(w, h);
  memcpy(itsPixels, data, w * h * sizeof(NFmiColorTools::Color));
}

// ----------------------------------------------------------------------
// Image default options
// ----------------------------------------------------------------------

void NFmiImage::DefaultOptions(void)
{
  itsType = "";
#ifdef IMAGINE_FORMAT_JPEG
  itsJpegQuality = 75;  // 0-100
#endif
#ifdef IMAGINE_FORMAT_PNG
  itsPngQuality = 6;                       // 0=none, 1=fast,9=slow
  itsPngFilter = PNG_FILTER_TYPE_DEFAULT;  // usually fastest and often best
#endif
#if (defined IMAGINE_FORMAT_JPEG) || (defined IMAGINE_FORMAT_PNG)
  itsAlphaLimit = -1;      // do not force opaque/transparent
  itsGamma = -1.0;         // negative means default gamma
  itsIntent = string("");  // no intent

  itsSaveAlphaFlag = true;      // yes, save alpha channel
  itsWantPaletteFlag = true;    // yes, try palette
  itsForcePaletteFlag = false;  // no, do not force palette
#endif
}

// ----------------------------------------------------------------------
// Assignment operator
// ----------------------------------------------------------------------

NFmiImage &NFmiImage::operator=(const NFmiImage &theImage)
{
  if (this != &theImage)
  {
    Reallocate(theImage.Width(), theImage.Height());
    itsType = theImage.itsType;
    memcpy(itsPixels, theImage.itsPixels, itsWidth * itsHeight * sizeof(NFmiColorTools::Color));
  }
  return *this;
}

// ----------------------------------------------------------------------
// Allocation utility
// ----------------------------------------------------------------------

void NFmiImage::Allocate(int theWidth, int theHeight)
{
  if (theWidth < 0 || theHeight < 0)
    throw runtime_error("Cannot allocate an image with negative dimensions");
  itsWidth = theWidth;
  itsHeight = theHeight;
  itsPixels = new int[theWidth * theHeight];  // Allocate
  if (itsPixels == NULL)
  {
    ostringstream os;
    os << "Insufficient memory to allocate image of size " << theWidth << "x" << theHeight;
    throw NFmiImageMemoryError(os.str());
  }
}

// ----------------------------------------------------------------------
// Re-allocation utility
// ----------------------------------------------------------------------

void NFmiImage::Reallocate(int theWidth, int theHeight)
{
  if (theWidth * theHeight == itsWidth * itsHeight)  // Same size - don't bother
    return;
  Destroy();
  Allocate(theWidth, theHeight);
}

// ----------------------------------------------------------------------
// Replace image with image in given file
// ----------------------------------------------------------------------
#ifndef IMAGINE_WITH_CAIRO
void NFmiImage::Read(const string &theFileName)
{
  // Make sure old contents are destroyed

  Destroy();

  string mime = NFmiImageTools::MimeType(theFileName);

  // Open the input file.

  FILE *in;
  in = fopen(theFileName.c_str(), "rb");

  if (in == NULL) throw NFmiImageOpenError(std::string("Failed to open image ") + theFileName);

  itsType = mime;

  if (mime == "gif") ReadGIF(in);
#ifdef IMAGINE_FORMAT_PNG
  else if (mime == "png")
    ReadPNG(in);
#endif
#ifdef IMAGINE_FORMAT_JPEG
  else if (mime == "jpeg")
    ReadJPEG(in);
#endif
  else if (mime == "pnm")
    ReadPNM(in);
  else if (mime == "pgm")
    ReadPGM(in);
  else
    throw NFmiImageFormatError("Unrecognized image format in '" + theFileName + "'");
  // Assert we got an image

  if (itsPixels == NULL)
    throw NFmiImageCorruptError(std::string("Failed to read image ") + theFileName);

  // Close the input file
  fclose(in);
}
#endif

// ----------------------------------------------------------------------
// Write image of desired type
// ----------------------------------------------------------------------

void NFmiImage::Write(const string &theFileName, const string &theType) const
{
  if (0)
    ;
#ifdef IMAGINE_FORMAT_PNG
  else if (theType == "png")
    WritePng(theFileName);
#endif
#ifdef IMAGINE_FORMAT_JPEG
  else if (theType == "jpeg" || theType == "jpg")
    WriteJpeg(theFileName);
#endif
  else if (theType == "gif")
    WriteGif(theFileName);
  else if (theType == "wbmp")
    WriteWbmp(theFileName);
  else if (theType == "pnm")
    WritePnm(theFileName);
  else if (theType == "pgm")
    WritePgm(theFileName);
  else
    throw runtime_error("Image format '" + theType + "' is not supported");
}

// ----------------------------------------------------------------------
// Write image as JPEG into given file with given quality.
// The quality should be negative to imply default quality, or
// in the range 0-95.
// ----------------------------------------------------------------------
#ifdef IMAGINE_FORMAT_JPEG
void NFmiImage::WriteJpeg(const string &theFileName) const
{
  const string dir = NFmiFileSystem::DirName(theFileName);
  const string tmp = NFmiFileSystem::TemporaryFile(dir);

  FILE *out;
  out = fopen(tmp.c_str(), "wb");
  if (out == NULL) throw runtime_error("Failed to open '" + theFileName + "' for writing a JPEG");
  WriteJPEG(out);
  fclose(out);

  bool status = NFmiFileSystem::RenameFile(tmp, theFileName);

  if (!status) throw runtime_error("Failed to write '" + theFileName + "'");
}
#endif

// ----------------------------------------------------------------------
// Write image as PNG into given file. The compression quality
// cannot be controlled.
// ----------------------------------------------------------------------
#ifdef IMAGINE_FORMAT_PNG
void NFmiImage::WritePng(const string &theFileName) const
{
  const string dir = NFmiFileSystem::DirName(theFileName);
  const string tmp = NFmiFileSystem::TemporaryFile(dir);

  FILE *out;
  out = fopen(tmp.c_str(), "wb");
  if (out == NULL) throw runtime_error("Failed to open '" + theFileName + "' for writing a PNG");
  WritePNG(out);
  fclose(out);

  bool status = NFmiFileSystem::RenameFile(tmp, theFileName);

  if (!status) throw runtime_error("Failed to write '" + theFileName + "'");
}
#endif

// ----------------------------------------------------------------------
// Write image as WBMP into given file. The compression quality
// cannot be controlled.
// ----------------------------------------------------------------------

void NFmiImage::WriteWbmp(const string &theFileName) const
{
  const string dir = NFmiFileSystem::DirName(theFileName);
  const string tmp = NFmiFileSystem::TemporaryFile(dir);

  FILE *out;
  out = fopen(tmp.c_str(), "wb");
  if (out == NULL) throw runtime_error("Failed to open '" + theFileName + "' for writing a WBMP");
  WriteWBMP(out);
  fclose(out);

  bool status = NFmiFileSystem::RenameFile(tmp, theFileName);

  if (!status) throw runtime_error("Failed to write '" + theFileName + "'");
}

// ----------------------------------------------------------------------
// Write image as GIF into given file.
// ----------------------------------------------------------------------

void NFmiImage::WriteGif(const string &theFileName) const
{
  const string dir = NFmiFileSystem::DirName(theFileName);
  const string tmp = NFmiFileSystem::TemporaryFile(dir);

  FILE *out;
  out = fopen(tmp.c_str(), "wb");
  if (out == NULL) throw runtime_error("Failed to open '" + theFileName + "' for writing a GIF");
  WriteGIF(out);
  fclose(out);

  bool status = NFmiFileSystem::RenameFile(tmp, theFileName);

  if (!status) throw runtime_error("Failed to write '" + theFileName + "'");
}

// ----------------------------------------------------------------------
// Write image as PNM into given file.
// ----------------------------------------------------------------------

void NFmiImage::WritePnm(const string &theFileName) const
{
  const string dir = NFmiFileSystem::DirName(theFileName);
  const string tmp = NFmiFileSystem::TemporaryFile(dir);

  FILE *out;
  out = fopen(tmp.c_str(), "wb");
  if (out == NULL) throw runtime_error("Failed to open '" + theFileName + "' for writing a PNM");
  WritePNM(out);
  fclose(out);

  bool status = NFmiFileSystem::RenameFile(tmp, theFileName);

  if (!status) throw runtime_error("Failed to write '" + theFileName + "'");
}

// ----------------------------------------------------------------------
// Write image as PGM into given file.
// ----------------------------------------------------------------------

void NFmiImage::WritePgm(const string &theFileName) const
{
  const string dir = NFmiFileSystem::DirName(theFileName);
  const string tmp = NFmiFileSystem::TemporaryFile(dir);

  FILE *out;
  out = fopen(tmp.c_str(), "wb");
  if (out == NULL) throw runtime_error("Failed to open '" + theFileName + "' for writing a PGM");
  WritePGM(out);
  fclose(out);

  bool status = NFmiFileSystem::RenameFile(tmp, theFileName);

  if (!status) throw runtime_error("Failed to write '" + theFileName + "'");
}

// ----------------------------------------------------------------------
// Fill the image with a desired colour. Note that alpha-blending
// is ignored, it is assumed that we wish to overwrite the desired
// colour. Note that the idea is to have this subroutine as fast
// as possible, hence gdImageSetPixel is not used.
// ----------------------------------------------------------------------

void NFmiImage::Erase(NFmiColorTools::Color theColor)
{
  // Quick exit if color is not real

  if (theColor == NFmiColorTools::NoColor) return;

  for (int i = 0; i < itsWidth * itsHeight; i++)
    itsPixels[i] = theColor;
}

// ----------------------------------------------------------------------
// Reduce colors in the image
// ----------------------------------------------------------------------

void NFmiImage::ReduceColors()
{
// First we must simplify the alpha channel as requested in the color save mode

#if (defined IMAGINE_FORMAT_JPEG) || (defined IMAGINE_FORMAT_PNG)
  bool savealpha = (itsSaveAlphaFlag && !IsOpaque(itsAlphaLimit));
  bool ignorealpha = !savealpha;

  for (int i = 0; i < itsWidth * itsHeight; i++)
  {
    itsPixels[i] = NFmiColorTools::Simplify(itsPixels[i], itsAlphaLimit, ignorealpha);
  }
#endif

  // Then we simplify

  NFmiColorReduce::AdaptiveReduce(*this);
}

// ----------------------------------------------------------------------
// Test whether the image is fully opaque. This information is used
// when saving images to save some space.
// ----------------------------------------------------------------------

bool NFmiImage::IsOpaque(int threshold) const
{
  int limit = (threshold < 0 ? 0 : threshold);
  for (int i = 0; i < itsWidth * itsHeight; i++)
    if (NFmiColorTools::GetAlpha(itsPixels[i]) > limit) return false;
  return true;
}

// ----------------------------------------------------------------------
// Test whether the image is fully opaque or transparent. This information
// can be used to ignore alpha channel when saving images.
// ----------------------------------------------------------------------

bool NFmiImage::IsFullyOpaqueOrTransparent(int threshold) const
{
  // If the separation threshold is set, each pixel is then obviously
  // forced to be either fully opaque or fully transparent

  if (threshold >= 0) return true;

  for (int i = 0; i < itsWidth * itsHeight; i++)
  {
    register int alpha = NFmiColorTools::GetAlpha(itsPixels[i]);
    if (!(alpha == NFmiColorTools::Opaque || alpha == NFmiColorTools::Transparent)) return false;
  }
  return true;
}

// ----------------------------------------------------------------------
// Find an unused RGB triple from the image data.
// This is intended to be used by PNG write when we can omit the
// alpha channel from true color data by pretending a particular
// RGB triple is transparent.
//
// Algorithm:
//
//	Pick a random RGB triple, and see if it is used.
//	Repeat until an unused triple is found.
//
// If the search takes too long, we return NoColor instead.
// This can always be distinguished from the RGB value, since
// the 'alpha channel' is nonzero in NoValue.
//
// ----------------------------------------------------------------------

NFmiColorTools::Color NFmiImage::UnusedColor(void) const
{
  // Semi-eternal loop until a missing RGB triple is found

  for (int try_number = 0; try_number < 1000; try_number++)
  {
    // A new color

    int r = rand() * NFmiColorTools::MaxRGB / RAND_MAX;
    int g = rand() * NFmiColorTools::MaxRGB / RAND_MAX;
    int b = rand() * NFmiColorTools::MaxRGB / RAND_MAX;

    NFmiColorTools::Color rgb = NFmiColorTools::MakeColor(r, g, b);

    // See if it is unused

    bool used = false;

    for (int i = 0; i < itsWidth * itsHeight; i++)
    {
      if (rgb == NFmiColorTools::GetRGB(itsPixels[i]))
      {
        used = true;
        break;
      }
    }

    // If it wasn't used, return the color

    if (!used) return rgb;
  }

  return NFmiColorTools::NoColor;
}

// ----------------------------------------------------------------------
// Add the colors used in the image into the given set of colors.
// If the size of the set exceeds the given maximum, the operation
// is aborted. A value <= 0 implies no limit.
// If opaquethreshold >=0 then all colors are made either fully
// transparent or opaque.
// The alpha channel of the colors is ignored if so specified.
// The method return true if an overflow occurred.
// ----------------------------------------------------------------------

bool NFmiImage::AddColors(set<NFmiColorTools::Color> &theSet,
                          int maxcolors,
                          int opaquethreshold,
                          bool ignorealpha) const
{
  // First check if the maximum number has already been exceeded

  int colorsnow = 0;
  if (maxcolors > 0)
  {
    colorsnow = theSet.size();
    if (colorsnow > maxcolors) return true;
  }

  // We keep a record of last 4 colors for extremely fast access

  NFmiColorTools::Color color1 = NFmiColorTools::NoColor;
  NFmiColorTools::Color color2 = NFmiColorTools::NoColor;
  NFmiColorTools::Color color3 = NFmiColorTools::NoColor;
  NFmiColorTools::Color color4 = NFmiColorTools::NoColor;

  NFmiColorTools::Color color;

  // Used when inserting a new color

  pair<set<NFmiColorTools::Color>::iterator, bool> iter;

  for (int i = 0; i < itsWidth * itsHeight; i++)
  {
    // The next color, without alpha if so desired

    color = itsPixels[i];

    color = NFmiColorTools::Simplify(color, opaquethreshold, ignorealpha);

    // Quick continue of just handled the same color

    if (color == color1 || color == color2 || color == color3 || color == color4) continue;

    // Otherwise add the color to the set

    iter = theSet.insert(color);

    // Increment total number and exit if needed

    if (iter.second && maxcolors > 0)
    {
      colorsnow++;
      if (colorsnow > maxcolors) return true;
    }

    // And shift back the last known colors

    color4 = color3;
    color3 = color2;
    color2 = color1;
    color1 = color;
  }

  // Succesfully added all colors to the set without exceeding any limit

  return false;
}

// ----------------------------------------------------------------------
// Stroke a non-antialiased 1 pixel wide line onto given image using
// various Porter-Duff rules
// ----------------------------------------------------------------------

#ifndef IMAGINE_WITH_CAIRO
void NFmiImage::StrokeBasic(float theX1,
                            float theY1,
                            float theX2,
                            float theY2,
                            NFmiColorTools::Color theColor,
                            NFmiColorTools::NFmiBlendRule theRule)
{
  // Quick exit if color is not real

  if (theColor == NFmiColorTools::NoColor) return;

  // Clip the coordinates

  float dx = theX2 - theX1;  // y = dy/dx*(x-x1)+y1
  float dy = theY2 - theY1;  // x = dx/dy*(y-y1)+x1

  float x1 = theX1;
  float y1 = theY1;
  float x2 = theX2;
  float y2 = theY2;

  // The top and right margins are calculated with pixel coordinates
  // in mind, while the input is real coordinates. In order to avoid
  // rounding up overflows, the last half-pixel line of the last
  // pixel must not be included, hence the offset used here is
  // marginally smaller < 0.5.
  //
  // What would happen if offset=0.5 is:
  //
  // A line would intersect at Width-0.5, or Height-0.5, and the
  // value would be rounded up in StrokeBasic primitives in NFmiImage.h,
  // causing a memory overlow and possibly a bus error.
  //
  // If the offset was zero, the last pixel rows would be only
  // partially included in the image, and tiny half-pixel errors
  // may appear when comparing images drawn with different methods.

  const float offset = 0.49f;  // pixel offset 0.5 minus some small epsilon

  const float xmargin = Width() - 1 + offset;
  const float ymargin = Height() - 1 + offset;

  if (x1 > x2)
  {
    swap(x1, x2);
    swap(y1, y2);
  }

  // This also makes sure we don't have to worry about dy/dx

  if (x1 > xmargin || x2 < 0) return;

  if (x1 < 0)
  {
    x1 = 0;
    y1 = dy / dx * (0 - theX1) + theY1;
  }
  if (x2 > xmargin)
  {
    x2 = xmargin;
    y2 = dy / dx * (xmargin - theX1) + theY1;
  }

  if (y1 > y2)
  {
    swap(x1, x2);
    swap(y1, y2);
  }

  // This also makes sure we don't have to worry about dx/dy

  if (y1 >= ymargin || y2 < 0) return;

  if (y1 < 0)
  {
    y1 = 0;
    x1 = dx / dy * (0 - theY1) + theX1;
  }
  if (y2 > ymargin)
  {
    y2 = ymargin;
    x2 = dx / dy * (ymargin - theY1) + theX1;
  }

  // When the color is opaque or transparent, some rules will simplify.
  // Instead of using ifs in the innermost loop, we will simplify the
  // rule itself here.

  int alpha = NFmiColorTools::GetAlpha(theColor);
  NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::Simplify(theRule, alpha);

  // If the result is ColorKeep, the source alpha is such that there
  // is nothing to do!

  if (rule == NFmiColorTools::kFmiColorKeep) return;

  // Otherwise we instantiate the appropriate fill routine

  int r = NFmiColorTools::GetRed(theColor);
  int g = NFmiColorTools::GetGreen(theColor);
  int b = NFmiColorTools::GetBlue(theColor);
  int a = NFmiColorTools::GetAlpha(theColor);

  switch (rule)
  {
    // Cases for which Color stroke is faster:

    case NFmiColorTools::kFmiColorClear:
      StrokeBasic2(NFmiColorBlendClear(), x1, y1, x2, y2, theColor, *this);
      break;
    case NFmiColorTools::kFmiColorCopy:
      StrokeBasic2(NFmiColorBlendCopy(), x1, y1, x2, y2, theColor, *this);
      break;
    case NFmiColorTools::kFmiColorAddContrast:
      StrokeBasic2(NFmiColorBlendAddContrast(), x1, y1, x2, y2, theColor, *this);
      break;
    case NFmiColorTools::kFmiColorReduceContrast:
      StrokeBasic2(NFmiColorBlendReduceConstrast(), x1, y1, x2, y2, theColor, *this);
      break;

    // Cases for which RGBA stroke is faster:

    case NFmiColorTools::kFmiColorOver:
      StrokeBasic2(NFmiColorBlendOver(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorUnder:
      StrokeBasic2(NFmiColorBlendUnder(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorIn:
      StrokeBasic2(NFmiColorBlendIn(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorKeepIn:
      StrokeBasic2(NFmiColorBlendKeepIn(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorOut:
      StrokeBasic2(NFmiColorBlendOut(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorKeepOut:
      StrokeBasic2(NFmiColorBlendKeepOut(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorAtop:
      StrokeBasic2(NFmiColorBlendAtop(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorKeepAtop:
      StrokeBasic2(NFmiColorBlendKeepAtop(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorXor:
      StrokeBasic2(NFmiColorBlendXor(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorPlus:
      StrokeBasic2(NFmiColorBlendPlus(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorMinus:
      StrokeBasic2(NFmiColorBlendMinus(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorAdd:
      StrokeBasic2(NFmiColorBlendAdd(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorSubstract:
      StrokeBasic2(NFmiColorBlendSubstract(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorMultiply:
      StrokeBasic2(NFmiColorBlendMultiply(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorDifference:
      StrokeBasic2(NFmiColorBlendDifference(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorCopyRed:
      StrokeBasic2(NFmiColorBlendCopyRed(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorCopyGreen:
      StrokeBasic2(NFmiColorBlendCopyGreen(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorCopyBlue:
      StrokeBasic2(NFmiColorBlendCopyBlue(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorCopyMatte:
      StrokeBasic2(NFmiColorBlendCopyMatte(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorCopyHue:
      StrokeBasic2(NFmiColorBlendCopyHue(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorCopyLightness:
      StrokeBasic2(NFmiColorBlendCopyLightness(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorCopySaturation:
      StrokeBasic2(NFmiColorBlendCopySaturation(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorKeepMatte:
      StrokeBasic2(NFmiColorBlendKeepMatte(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorKeepHue:
      StrokeBasic2(NFmiColorBlendKeepHue(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorKeepLightness:
      StrokeBasic2(NFmiColorBlendKeepLightness(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorKeepSaturation:
      StrokeBasic2(NFmiColorBlendKeepSaturation(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorBumpmap:
      StrokeBasic2(NFmiColorBlendBumpmap(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorDentmap:
      StrokeBasic2(NFmiColorBlendDentmap(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorOnOpaque:
      StrokeBasic2(NFmiColorBlendOnOpaque(), x1, y1, x2, y2, r, g, b, a, *this);
      break;
    case NFmiColorTools::kFmiColorOnTransparent:
      StrokeBasic2(NFmiColorBlendOnTransparent(), x1, y1, x2, y2, r, g, b, a, *this);
      break;

    // Some special cases
    case NFmiColorTools::kFmiColorKeep:
    case NFmiColorTools::kFmiColorRuleMissing:
      break;
  }
}
#endif

// ----------------------------------------------------------------------
// Composite image over another using given blending rule
// ----------------------------------------------------------------------

#ifndef IMAGINE_WITH_CAIRO
void NFmiImage::Composite(const NFmiImage &thePattern,
                          NFmiColorTools::NFmiBlendRule theRule,
                          NFmiAlignment theAlignment,
                          int theX,
                          int theY,
                          float theAlpha)
{
  // Adjust the coordinates based on the alignment

  int x = theX;
  int y = theY;

  switch (theAlignment)
  {
    case kFmiAlignCenter:
      y -= thePattern.Height() / 2;
    case kFmiAlignNorth:
      x -= thePattern.Width() / 2;
      break;

    case kFmiAlignEast:
      x -= thePattern.Width();
    case kFmiAlignWest:
      y -= thePattern.Height() / 2;
      break;

    case kFmiAlignSouthEast:
      y -= thePattern.Height();
    case kFmiAlignNorthEast:
      x -= thePattern.Width();
      break;

    case kFmiAlignSouth:
      x -= thePattern.Width() / 2;
    case kFmiAlignSouthWest:
      y -= thePattern.Height();
      break;

    case kFmiAlignNorthWest:
    case kFmiAlignMissing:
      break;
  }

  switch (theRule)
  {
    case NFmiColorTools::kFmiColorClear:
      Composite2(NFmiColorBlendClear(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorCopy:
      Composite2(NFmiColorBlendCopy(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorAddContrast:
      Composite2(NFmiColorBlendAddContrast(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorReduceContrast:
      Composite2(NFmiColorBlendReduceConstrast(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorOver:
      Composite2(NFmiColorBlendOver(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorUnder:
      Composite2(NFmiColorBlendUnder(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorIn:
      Composite2(NFmiColorBlendIn(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorKeepIn:
      Composite2(NFmiColorBlendKeepIn(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorOut:
      Composite2(NFmiColorBlendOut(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorKeepOut:
      Composite2(NFmiColorBlendKeepOut(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorAtop:
      Composite2(NFmiColorBlendAtop(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorKeepAtop:
      Composite2(NFmiColorBlendKeepAtop(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorXor:
      Composite2(NFmiColorBlendXor(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorPlus:
      Composite2(NFmiColorBlendPlus(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorMinus:
      Composite2(NFmiColorBlendMinus(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorAdd:
      Composite2(NFmiColorBlendAdd(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorSubstract:
      Composite2(NFmiColorBlendSubstract(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorMultiply:
      Composite2(NFmiColorBlendMultiply(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorDifference:
      Composite2(NFmiColorBlendDifference(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorCopyRed:
      Composite2(NFmiColorBlendCopyRed(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorCopyGreen:
      Composite2(NFmiColorBlendCopyGreen(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorCopyBlue:
      Composite2(NFmiColorBlendCopyBlue(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorCopyMatte:
      Composite2(NFmiColorBlendCopyMatte(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorCopyHue:
      Composite2(NFmiColorBlendCopyHue(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorCopyLightness:
      Composite2(NFmiColorBlendCopyLightness(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorCopySaturation:
      Composite2(NFmiColorBlendCopySaturation(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorKeepMatte:
      Composite2(NFmiColorBlendKeepMatte(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorKeepHue:
      Composite2(NFmiColorBlendKeepHue(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorKeepLightness:
      Composite2(NFmiColorBlendKeepLightness(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorKeepSaturation:
      Composite2(NFmiColorBlendKeepSaturation(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorBumpmap:
      Composite2(NFmiColorBlendBumpmap(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorDentmap:
      Composite2(NFmiColorBlendDentmap(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorOnOpaque:
      Composite2(NFmiColorBlendOnOpaque(), thePattern, x, y, theAlpha, *this);
      break;
    case NFmiColorTools::kFmiColorOnTransparent:
      Composite2(NFmiColorBlendOnTransparent(), thePattern, x, y, theAlpha, *this);
      break;

    // Some special cases
    case NFmiColorTools::kFmiColorKeep:
    case NFmiColorTools::kFmiColorRuleMissing:
      break;
  }
}
#endif

}  // namespace Imagine

// ======================================================================
