// ======================================================================
//
// C++ Image API, defining basic image operations.
//
// This is a wrapper for the Cairo graphics library, with some additional
// methods defined to improve performance.
//
// History:
//
// 31.07.2008 Asko Kauppi; GD replaced by Cairo
// 13.08.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once


#include "NFmiColorTools.h"
#include "NFmiAlignment.h"

#ifndef IMAGINE_WITH_CAIRO
#include "NFmiDrawable.h"
#endif

#include <set>      // for sets
#include <cstdio>
#include <stdexcept>

#ifdef __BORLANDC__
using std::FILE;
#endif

// IMAGINE_IGNORE_FORMATS (old define, this is compatibility code) disables
// JPEG and PNG formats.
//
// If either IMAGINE_FORMAT_JPEG or IMAGINE_FORMAT_PNG is on, only that support
// is built in.
//
// If no defines are there, both JPEG and PNG are on (compatibility to how
// it was)
//  --AKa 7-Oct-2008
//
#ifdef IMAGINE_IGNORE_FORMATS
#undef IMAGINE_FORMAT_JPEG
#undef IMAGINE_FORMAT_PNG
#elif(!defined IMAGINE_FORMAT_JPEG) && (!defined IMAGINE_FORMAT_PNG)
#define IMAGINE_FORMAT_JPEG
#define IMAGINE_FORMAT_PNG
#endif

namespace Imagine
{
//! Generic NFmiImage exception from which all others are derived
struct NFmiImageError : public std::runtime_error
{
  NFmiImageError(const std::string &s) : std::runtime_error(s) {}
};

//! Insufficient memory error
struct NFmiImageMemoryError : public NFmiImageError
{
  NFmiImageMemoryError(const std::string &s) : NFmiImageError(s) {}
};

//! Opening image file fails
struct NFmiImageOpenError : public NFmiImageError
{
  NFmiImageOpenError(const std::string &s) : NFmiImageError(s) {}
};

//! Unrecognized image format
struct NFmiImageFormatError : public NFmiImageError
{
  NFmiImageFormatError(const std::string &s) : NFmiImageError(s) {}
};

//! Corrupt image data
struct NFmiImageCorruptError : public NFmiImageError
{
  NFmiImageCorruptError(const std::string &s) : NFmiImageError(s) {}
};

class NFmiImage
#ifndef IMAGINE_WITH_CAIRO
    : public NFmiDrawable
#endif
{
 private:
  // Data elements
  //
  int itsWidth;
  int itsHeight;
  std::string itsType;  // when read from a file

  NFmiColorTools::Color *itsPixels;

// Various options
//
#ifdef IMAGINE_FORMAT_JPEG
  int itsJpegQuality;  // JPEG compression quality, 0-100
#endif
#ifdef IMAGINE_FORMAT_PNG
  int itsPngQuality;  // PNG compression level, 0-9
  int itsPngFilter;   // PNG filter
#endif
#if (defined IMAGINE_FORMAT_JPEG) || (defined IMAGINE_FORMAT_PNG)
  int itsAlphaLimit;      // alpha<=limit is considered opaque
  float itsGamma;         // Gamma, 1-3 is reasonable
  std::string itsIntent;  // Rendering intent (PNG)

  bool itsSaveAlphaFlag;     // true if alpha is to be saved
  bool itsWantPaletteFlag;   // true if palette is desired when possible
  bool itsForcePaletteFlag;  // true if palette is to be forced
#endif

  /********************/
 public:
  // Constructors, destructors
  //
  NFmiImage(int theWidth = 0, int theHeight = 0, NFmiColorTools::Color theColor = 0);
  NFmiImage(const NFmiImage &theImg);

#ifndef IMAGINE_WITH_CAIRO
  NFmiImage(const std::string &theFileName);
#endif

  // AKa 4-Aug-2008: Constructor based on raw data from Cairo ImageSurface
  //
  NFmiImage(int w, int h, const NFmiColorTools::Color *data);

  virtual ~NFmiImage() { Destroy(); }
  // Data access
  //
  int Width() const { return itsWidth; }
  int Height() const { return itsHeight; }
  const std::string &Type() const { return itsType; }
  // All constructors call this to set the default options
  //
  void DefaultOptions();

// Access to individual options

#ifdef IMAGINE_FORMAT_JPEG
  int JpegQuality(void) const { return itsJpegQuality; }
  void JpegQuality(int quality) { itsJpegQuality = quality; }
#endif
#ifdef IMAGINE_FORMAT_PNG
  int PngQuality(void) const { return itsPngQuality; }
  void PngQuality(int quality) { itsPngQuality = quality; }
#endif
#if (defined IMAGINE_FORMAT_JPEG) || (defined IMAGINE_FORMAT_PNG)
  int AlphaLimit(void) const { return itsAlphaLimit; }
  bool SaveAlpha(void) const { return itsSaveAlphaFlag; }
  bool WantPalette(void) const { return itsWantPaletteFlag; }
  bool ForcePalette(void) const { return itsForcePaletteFlag; }
  float Gamma(void) const { return itsGamma; }
  const std::string Intent(void) const { return itsIntent; }
  void AlphaLimit(int limit) { itsAlphaLimit = limit; }
  void SaveAlpha(bool flag) { itsSaveAlphaFlag = flag; }
  void WantPalette(bool flag) { itsWantPaletteFlag = flag; }
  void ForcePalette(bool flag) { itsForcePaletteFlag = flag; }
  void Gamma(float value) { itsGamma = value; }
  void Intent(const std::string &value) { itsIntent = value; }
#endif

  // This makes  A(i,j) = B(x,y) work
  //
  NFmiColorTools::Color &operator()(int i, int j) const { return itsPixels[j * itsWidth + i]; }
  // Assignment operator

  NFmiImage &operator=(const NFmiImage &theImage);
  NFmiImage &operator=(const NFmiColorTools::Color theColor);

// Reading an image
//
#ifndef IMAGINE_WITH_CAIRO
  void Read(const std::string &fn);
#endif

  // Writing the image
  //
  void Write(const std::string &fn, const std::string &type) const;

#ifdef IMAGINE_FORMAT_JPEG
  void WriteJpeg(const std::string &theFileName) const;
#endif
#ifdef IMAGINE_FORMAT_PNG
  void WritePng(const std::string &theFileName) const;
#endif
  void WriteWbmp(const std::string &theFileName) const;
  void WriteGif(const std::string &theFileName) const;
  void WritePnm(const std::string &theFileName) const;
  void WritePgm(const std::string &theFileName) const;

  void ReduceColors();

  // Erasing image with desired colour
  //
  void Erase(NFmiColorTools::Color theColor);

  // A simple non-antialiased line of width 1 pixel
  //
  void StrokeBasic(float theX1,
                   float theY1,
                   float theX2,
                   float theY2,
                   NFmiColorTools::Color theColor,
                   NFmiColorTools::NFmiBlendRule theRule);

  // Composite image over another
  //
  void Composite(const NFmiImage &theImage,
                 NFmiColorTools::NFmiBlendRule theRule,
                 NFmiAlignment theAlignment = kFmiAlignNorthWest,
                 int theX = 0,
                 int theY = 0,
                 float theAlpha = 1.0);

  /******
  */
 private:
  // Constructor, destructor utilities
  //
  void Destroy();
  void Allocate(int width, int height);
  void Reallocate(int width, int height);

// Reading and writing various image formats
#ifdef IMAGINE_FORMAT_JPEG
  void ReadJPEG(FILE *in);
  void WriteJPEG(FILE *out) const;
#endif
#ifdef IMAGINE_FORMAT_PNG
  void ReadPNG(FILE *in);
  void WritePNG(FILE *out) const;
#endif
  void WritePNM(FILE *out) const;
  void ReadPNM(FILE *out);

  void WritePGM(FILE *out) const;
  void ReadPGM(FILE *out);

  void WriteWBMP(FILE *out) const;

  void ReadGIF(FILE *in);
  void WriteGIF(FILE *out) const;

  // Test whether the image is opaque
  //
  bool IsOpaque(int threshold = -1) const;

  // Test whether the image is fully opaque/transparent
  //
  bool IsFullyOpaqueOrTransparent(int threshold = -1) const;

  // Returns an RGB value which does NOT occur in the image
  //
  NFmiColorTools::Color UnusedColor() const;

  // Put image colors into the given set
  //
  bool AddColors(std::set<NFmiColorTools::Color> &theSet,
                 int maxcolors = -1,
                 int opaquethreshold = -1,
                 bool ignoreAlpha = false) const;

  // Strokebasic low level drivers for each blending rule
  // By default we use the generally faster RGBA version below
  // Specializations later on in the code may overload the latter
  // interface below for defining possible optimized versions.

  //  template <class T> // 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n
  //  virheen takia.
  //  void StrokeBasic(float theX1, float theY1, float theX2, float theY2,
  //		   int r, int g, int b, int a);

  //  template <class T> // 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n
  //  virheen takia.
  //  void StrokeBasic(float theX1, float theY1,
  //		   float theX2, float theY2,
  //		   NFmiColorTools::Color theColor);

  // Composite low level drivers for each blending rule

  //  template <class T> // 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n
  //  virheen takia.
  //  void Composite(const NFmiImage & theImage,
  //		 int theX, int theY, float theAlpha);
};

}  // namespace Imagine


// ----------------------------------------------------------------------
