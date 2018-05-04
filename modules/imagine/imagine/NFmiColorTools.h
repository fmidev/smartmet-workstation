// ======================================================================
/*!
 * \file NFmiColorTools.h
 * \brief Tools to handle RGBA colors.
 *
 * \b History:
 *
 * \li 21.08.2001 Mika Heiskanen\par
 * Implemented.
 *
 * \li 14.09.2001 Mika Heiskanen\par
 * Completed set of blending rules.
 *
 * \li 29.09.2001 Mika Heiskanen\par
 * Added \c kFmiColorOnOpaque and \c kFmiColorOnTransparent rules.
 *
 * \li 06.12.2001 Mika Heiskanen\par
 * Separated color blending templates into NFmiColorBlend.h.
 */
// ======================================================================

#pragma once

#include "imagine-config.h"

#include <newbase/NFmiDef.h>

#include <string>  // for color name definitions

namespace Imagine
{
//! Loki template to convert enumerated value to a class for dispatching
template <int v>
struct Blend2Type
{
  enum
  {
    value = v
  };
};

//! Utilities for dealing with RGBA colours.

namespace _FMI_DLL NFmiColorTools
{
  //! An RGBA colour is internally represented as an integer of the form 0xaarrggbb

  typedef int Color;

  // -------------------- special colors --------------------

  //! A missing color means the color is unknown.

  const Color MissingColor = -1;

  //! A "no-color" means no color is to be used. Strokes and fills are cancelled.

  const Color NoColor = -2;

  //! Helper variable to identify a transparent color.

  const Color TransparentColor = 0x7F000000;

  //! Helper variable to identify a black color

  const Color Black = 0;

  // -------------------- range definitions --------------------

  //! RGB values are in range 0-255.

  const int MaxRGB = 255;

  //! Alpha values are in range 0-127, as in the GD library.

  const int MaxAlpha = 127;

  // -------------------- opacity definitions --------------------

  //! A color is opaque when its alpha value is zero.

  const int Opaque = 0;

  //! A color is transparent when its alpha value is 127.

  const int Transparent = 127;

  // -------------------- extracting color components --------------------

  int GetAlpha(Color c);
  int GetRed(Color c);
  int GetGreen(Color c);
  int GetBlue(Color c);
  Color GetRGB(Color c);

  // -------------------- building color from components --------------------

  Color MakeColor(int r, int g, int b, int a = Opaque);
  Color SafeColor(int r, int g, int b, int a = Opaque);
  Color SafestColor(int r, int g, int b, int a = Opaque);

  // ------------ replacing individual color components -----------------

  Color ReplaceAlpha(Color c, int alpha);
  Color ReplaceRed(Color c, int red);
  Color ReplaceGreen(Color c, int green);
  Color ReplaceBlue(Color c, int blue);

  // -------------------- color intensity tools --------------------

  //! Intensity range is 0-255.

  const int MaxIntensity = 255;

  int Intensity(int r, int g, int b);
  inline int Intensity(Color c);

  // -------------------- color contrast tools --------------------

  // Adding contrast = increasing difference between light and dark colors
  // Reducing contrast = decreasing difference between light and dark colors

  //! Modify contrast into the given direction

  Color Contrast(Color theColor, int theSign);

  Color AddContrast(Color theColor);
  Color ReduceContrast(Color theColor);

  // -------------------- colour blending --------------------

  // Don't forget to update BlendNamesInit() when updating!

  //! All available colour blending rules.

  enum NFmiBlendRule
  {
    kFmiColorRuleMissing,
    kFmiColorClear,
    kFmiColorCopy,
    kFmiColorKeep,
    kFmiColorOver,
    kFmiColorUnder,
    kFmiColorIn,
    kFmiColorKeepIn,
    kFmiColorOut,
    kFmiColorKeepOut,
    kFmiColorAtop,
    kFmiColorKeepAtop,
    kFmiColorXor,

    kFmiColorPlus,
#ifndef IMAGINE_WITH_CAIRO
    kFmiColorMinus,
    kFmiColorAdd,
    kFmiColorSubstract,
    kFmiColorMultiply,
    kFmiColorDifference,
    kFmiColorCopyRed,
    kFmiColorCopyGreen,
    kFmiColorCopyBlue,
    kFmiColorCopyMatte,
    kFmiColorCopyHue,
    kFmiColorCopyLightness,
    kFmiColorCopySaturation,
    kFmiColorKeepMatte,
    kFmiColorKeepHue,
    kFmiColorKeepLightness,
    kFmiColorKeepSaturation,
    kFmiColorBumpmap,
    kFmiColorDentmap,
    kFmiColorAddContrast,
    kFmiColorReduceContrast,
    kFmiColorOnOpaque,
    kFmiColorOnTransparent
#endif
  };

  // -------------------- colour name conversion --------------------

  // Color name <-> color conversion.

  //! Convert colour name to colour.

  Color ColorValue(const std::string &theName);

  //! Convert colour to colour name.

  const std::string ColorName(const Color &theColor);

  //! Initialize colour name table.

  void ColorNamesInit(void);

  //! A general purpose string to colour conversion

  Color ToColor(const std::string &theColor);

  //! Utility used by ToColor for converting hex to dec

  Color HexToColor(const std::string &theHex);

  // -------------------- blend name conversion --------------------

  //! Convert blending rule name to enum.

  NFmiBlendRule BlendValue(const std::string &theName);

  //! Convert blending rule to string name.

  const std::string BlendName(const NFmiBlendRule &theRule);

  //! Initialize blending rule name table .

  void BlendNamesInit(void);

  // -------------------- color space conversion --------------------

  //! Convert RGB values to HLS values.

  void RGBtoHLS(int red, int green, int blue, double *h, double *l, double *s);

  //! Convert HLS values to RGB values

  void HLStoRGB(double h, double l, double s, int *r, int *g, int *b);

  //! Utility used by HLS -- RGB conversion functions.

  double hls_to_rgb_util(double m1, double m2, double h);

  // -------------------- miscellaneous --------------------

  //! Simplify a blending rule given the source color alpha value

  NFmiBlendRule Simplify(NFmiBlendRule theRule, int alpha);

  //! Interpolate linearly in HLS space between 2 colors.

  int Interpolate(Color c1, Color c2, float fraction);

  Color Simplify(Color c, int opaquethreshold, bool ignorealpha);
}

// ----------------------------------------------------------------------
/*!
 * \brief Extract alpha component from a color.
 */
// ----------------------------------------------------------------------

inline int NFmiColorTools::GetAlpha(Color c) { return (c >> 24) & 0x7F; }
// ----------------------------------------------------------------------
/*!
 * \brief Extract red component from a color.
 */
// ----------------------------------------------------------------------

inline int NFmiColorTools::GetRed(Color c) { return (c >> 16) & 0xFF; }
// ----------------------------------------------------------------------
/*!
 * \brief Extract green component from a color.
 */
// ----------------------------------------------------------------------

inline int NFmiColorTools::GetGreen(Color c) { return (c >> 8) & 0xFF; }
// ----------------------------------------------------------------------
/*!
 * \brief Extract blue component from a color.
 */
// ----------------------------------------------------------------------

inline int NFmiColorTools::GetBlue(Color c) { return (c & 0xFF); }
// ----------------------------------------------------------------------
/*!
 * \brief Extract RGB component from a color. Alpha will be zero (opaque color).
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::GetRGB(Color c) { return (c & 0xFFFFFF); }
// ----------------------------------------------------------------------
/*!
 * \brief Set new alpha component into the color.
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::ReplaceAlpha(Color c, int alpha)
{
  return ((c & 0x00FFFFFF) | (alpha << 24));
}

// ----------------------------------------------------------------------
/*!
 * \brief Set new red component into the color.
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::ReplaceRed(Color c, int red)
{
  return ((c & 0xFF00FFFF) | (red << 16));
}

// ----------------------------------------------------------------------
/*!
 * \brief Set new green component into the color.
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::ReplaceGreen(Color c, int green)
{
  return ((c & 0xFFFF00FF) | (green << 8));
}

// ----------------------------------------------------------------------
/*!
 * \brief Set new blue component into the color
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::ReplaceBlue(Color c, int blue)
{
  return ((c & 0xFFFFFF00) | (blue));
}

// ----------------------------------------------------------------------
/*!
 * Converting RGBA values into a color. No checks are made on the
 * input. The acceptable range of input for the RGB components
 * is 0 - MaxRGB, for the alpha component 0 - MaxAlpha.
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::MakeColor(int r, int g, int b, int a)
{
  return ((a << 24) + (r << 16) + (g << 8) + b);
}

// ----------------------------------------------------------------------
/*!
 * Converting RGBA values into a color. The acceptable range of input
 * for the RGB components is 0 - MaxRGB, for the alpha component 0 - MaxAlpha.
 * Any value greater than the upper limit is converted to the upper limit.
 * This makes building colours from positive definite linear combinations
 * safe.
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::SafeColor(int r, int g, int b, int a)
{
  int rr = (r > NFmiColorTools::MaxRGB) ? NFmiColorTools::MaxRGB : r;
  int gg = (g > NFmiColorTools::MaxRGB) ? NFmiColorTools::MaxRGB : g;
  int bb = (b > NFmiColorTools::MaxRGB) ? NFmiColorTools::MaxRGB : b;
  int aa = (a > NFmiColorTools::MaxAlpha) ? NFmiColorTools::MaxAlpha : a;

  return NFmiColorTools::MakeColor(rr, gg, bb, aa);

  // For some reason g++ does not get this right.
  //  return reinterpret_cast<NFmiColorTools::Color>
  //    ( ((a>NFmiColorTools::MaxAlpha) ? NFmiColorTools::MaxAlpha : a)<< 24 +
  //      ((r>NFmiColorTools::MaxRGB  ) ? NFmiColorTools::MaxRGB   : r)<< 16 +
  //      ((g>NFmiColorTools::MaxRGB  ) ? NFmiColorTools::MaxRGB   : g)<< 8 +
  //      ((b>NFmiColorTools::MaxRGB  ) ? NFmiColorTools::MaxRGB   : b) );
}

// ----------------------------------------------------------------------
/*!
 * Converting RGBA values into a color. The acceptable range of input
 * for the RGB components is 0 - MaxRGB, for the alpha component 0 - MaxAlpha.
 * Any value outside the valid range is converted to the nearest valid
 * value (zero or the maximum).
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::SafestColor(int r, int g, int b, int a)
{
  return ((((a < 0) ? 0 : (a > NFmiColorTools::MaxAlpha) ? NFmiColorTools::MaxAlpha : a) << 24) +
          (((r < 0) ? 0 : (r > NFmiColorTools::MaxRGB) ? NFmiColorTools::MaxRGB : r) << 16) +
          (((g < 0) ? 0 : (g > NFmiColorTools::MaxRGB) ? NFmiColorTools::MaxRGB : g) << 8) +
          ((b < 0) ? 0 : (b > NFmiColorTools::MaxRGB) ? NFmiColorTools::MaxRGB : b));
}

// ----------------------------------------------------------------------
/*!
 * Returns intensity of the given color in range 0-255. This is based
 * on a well known formula in floating point arithmetic. The integer
 * arithmetic version here is taken from ImageMagick.
 */
// ----------------------------------------------------------------------

inline int NFmiColorTools::Intensity(int r, int g, int b)
{
  return ((9798 * r + 19235 * g + 3735 * b) / 32768L);
}

inline int NFmiColorTools::Intensity(NFmiColorTools::Color c)
{
  return NFmiColorTools::Intensity(
      NFmiColorTools::GetRed(c), NFmiColorTools::GetGreen(c), NFmiColorTools::GetBlue(c));
}

// ----------------------------------------------------------------------
/*!
 * \brief Add contrast to given color
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::AddContrast(Color theColor)
{
  return Contrast(theColor, 1);
}

// ----------------------------------------------------------------------
/*!
 * \brief Reduce contrast in the given color
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::ReduceContrast(Color theColor)
{
  return Contrast(theColor, -1);
}

// ----------------------------------------------------------------------
/*!
 * \brief Simplify a color if possible.
 *
 * A color can be simplified when the user wishes to get rid of
 * the alpha channel, or to force it to extreme values.
 * A nonnegative opacity threshold opaquethreshold is used to divide colours
 * into opaque and transparent ones. An additional option ignorealpha
 * is used to indicate the alpha channel can be ignored completely, in which
 * case the color is made completely opaque.
 */
// ----------------------------------------------------------------------

inline NFmiColorTools::Color NFmiColorTools::Simplify(Color c,
                                                      int opaquethreshold,
                                                      bool ignorealpha)
{
  if (ignorealpha) return ReplaceAlpha(c, 0);
  if (GetAlpha(c) == NFmiColorTools::MaxAlpha) return TransparentColor;
  if (opaquethreshold < 0) return c;
  if (GetAlpha(c) < opaquethreshold)
    return ReplaceAlpha(c, 0);
  else
    return TransparentColor;
}

}  // namespace Imagine


// ----------------------------------------------------------------------
