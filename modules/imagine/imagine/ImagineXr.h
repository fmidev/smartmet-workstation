/*
* ImagineXr.h
*
* Replacement for 'Imagine' library, using Cairo drawing
*/

#pragma once

#include "imagine-config.h"

#ifndef IMAGINE_WITH_CAIRO
#error "Please define IMAGINE_WITH_CAIRO to use us."
#endif

#include "NFmiColorTools.h"
#include "NFmiAlignment.h"
#include "NFmiPathElement.h"

#include <cairomm/cairomm.h>

#include <string>
#include <deque>

using namespace std;
using namespace Cairo;

using namespace Imagine;
using namespace Imagine::NFmiColorTools;

#ifdef _MSC_VER
typedef int int32_t;
#endif

class ImagineXr
{
 private:
  /* Either 'pdf_surf' or 'image_surf' is non-NULL; tried to use just a single
  * 'Cairo::Surface' object, but they differ enough and casting 'RefPtr' proved
  * to be tedious.
  */
  Cairo::RefPtr<Cairo::PdfSurface> pdf_surf;
  Cairo::RefPtr<Cairo::ImageSurface> image_surf;

  const Cairo::RefPtr<Cairo::Surface> surf() const
  {
    if (pdf_surf)
      return pdf_surf;
    else
      return image_surf;
  }

  const string fn;   // filename to write to ('PdfSurface' needs it up front)
  const string fmt;  // "pdf"/"png"/"gif"/...

  /* 'PdfSurface' does not remember its width & height; we need to keep
  * copy of the constructor values.
  */
  /*const*/ int width, height;

  Cairo::RefPtr<Cairo::Context> cr;  // same 'cr' and 'face' used by all operations
  Cairo::RefPtr<Cairo::FontFace> face;

  NFmiColorTools::Color font_bg_color;  // color behind a text, or 'NFmiColorTools::NoColor'
  int font_bg_mx;                       // margin x,y
  int font_bg_my;

  void SetRGB(NFmiColorTools::Color col);

  void SetRGBA(NFmiColorTools::Color col);

  void SetBlend(enum NFmiColorTools::NFmiBlendRule rule);

  bool SetPath(const std::deque<NFmiPathElement> &path);

  static void ApplyAlignment(enum NFmiAlignment alignment, int &x, int &y, int w, int h);

  /*
  * Cairo has alpha 0..255 whereas NFmiColor has 0..127, so we cannot give
  * whole data blocks directly between them.
  */
  const int32_t *ARGB_32() const;

  /*******************/
 public:
  ImagineXr(int width, int height, const string &to_filename, const string &fmt);
  ImagineXr(const string &from_png_filename);  // reads a PNG file as base image
  ImagineXr(int w, int h);
  ImagineXr(const ImagineXr &o);

  ~ImagineXr();

  int Width() const { return width; }
  int Height() const { return height; }
  const string &Filename() const { return fn; }
  const string &Format() const { return fmt; }
  const NFmiColorTools::Color *NFmiColorBuf(NFmiColorTools::Color *buf) const;

  NFmiColorTools::Color operator()(int x, int y) const;

  bool IsTransparent(int x, int y) const;

  void Erase(const NFmiColorTools::Color &color);

  void Composite(const ImagineXr &img2,
                 NFmiColorTools::NFmiBlendRule rule = NFmiColorTools::kFmiColorCopy,
                 NFmiAlignment alignment = kFmiAlignNorthWest,
                 int x = 0,
                 int y = 0,
                 float opaque = 1.0);

  void Stroke(const std::deque<NFmiPathElement> &path,
              float line_width,
              NFmiColorTools::Color color,
              NFmiColorTools::NFmiBlendRule rule);

  void Fill(const std::deque<NFmiPathElement> &path,
            NFmiColorTools::Color color,
            NFmiBlendRule rule);

  void Fill(const std::deque<NFmiPathElement> &path,
            const ImagineXr &pattern,
            NFmiBlendRule rule = kFmiColorCopy,
            float opaque = 1.0);

  void MakeFace(const string &fontspec,
                const NFmiColorTools::Color backcolor = NFmiColorTools::NoColor,
                int xm = 0,
                int ym = 0);

  bool DrawFace(int x,
                int y,
                const std::string &text_utf8,
                NFmiColorTools::Color color = NFmiColorTools::Black,
                NFmiAlignment alignment = kFmiAlignNorthWest,
                NFmiBlendRule rule = kFmiColorCopy);

  void Write() const;
};

