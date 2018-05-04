// ======================================================================
/*!
 * \file NFmiColorTools.cpp
 * \brief Tools to handle RGBA colors.
 *
 * \b History:
 *
 * \li 21.08.2001 Mika Heiskanen\par
 * Implemented.
 */
// ======================================================================

// ======================================================================
/*!
 * \namespace Imagine::NFmiColorTools
 *
 * This provides methods to operator on RGBA colors by providing static
 * member functions. Hence the members can be and are intended to be used
 * as in
 *
 * \code
 * NFmiColorTools::Color color = NFmiColorTools::MakeColor(red,green,blue,alpha);
 * \endcode
 *
 * The better alternative would be to use a \c namespace, but they do not
 * work properly with the GNU C++ compiler.
 *
 * <b>Notes about colour blending with Porter-Duff rules:</b>
 *
 * A general Porter-Duff rule is expressed with formulas
 *
 * \f$C_d = C_s F_s + C_d F_d\f$
 *
 * \f$A_d = A_s F_s + A_d F_d\f$
 *
 * where \f$C_s\f$, \f$C_d\f$ stand for source and destination colour
 * components, \f$A_s\f$, \f$A_d\f$ source and destination alphas.
 * \f$F_s\f$ ans \f$F_d\f$ denote the fractions used in the blending as
 * defined by each compositing rule.
 *
 * Note that here having alpha value 1 implies an opaque colour and
 * zero a transparent colour, while in code we actuall prefer to use 0 for
 * opaque and 127 for transparent.
 *
 * Hence in all formulas we must substitute
 *
 * \f$ A_s = (\alpha_{max} - \alpha)/\alpha_{max}\f$
 *
 * \f$ 1-A_s = \alpha/\alpha_{max}\f$
 *
 * Also, the rules apply in premultiplied RGBA space, where each component
 * value has already been premultiplied by the alpha. We do this explicitly
 * when applying the rules.
 *
 * The rules as found from for example Java2D documentation are described
 * below. The name on the left corresponds to the chosen name, the name
 * below it the logical description of the operation.
 *
 * \li \b Clear\par
 * Both the color and the alpha of destination are cleared. Neither the
 * source nor the destination is used as input.
 *
 * \li \b Copy\par
 * The source is copied to the destination. The destination is not used
 * as input.
 *
 * \li \b Keep\par
 * The destination is kept. Useless operation for us.
 *
 * \li \b Over\par
 * The source is composited over the destination.
 *
 * \li \b Under\par
 * The destination is composited over the source and the result replaces the
 * destination. This is \b Over with the operands reversed.
 *
 * \li \b In\par
 * The part of the source lying inside of the destination replaces
 * the destination.
 *
 * \li \b KeepIn\par
 * The part of the destination lying inside of the source replaces the
 * destination. This is \b In with the operands reversed.
 *
 * \li \b Out\par
 * The part of the source lying outside of the destination replaces
 * the destination.
 *
 * \li \b KeepOut\par
 * The part of the destination lying outside of the source replaces
 * the destination. This is \b Out with the operands reversed.
 *
 * \li \b Atop\par
 * The part of the source inside the destination replaces the part inside
 * the destination.
 *
 * \li \b KeepAtop\par
 * The part of the destination inside the source replaces the part inside
 * the source in the destination. This is \b Atop reversed.
 *
 * \li \b Xor\par
 * Only non-overlapping areas of source and destination are kept.
 *
 * <b>Additional blending rules</b>
 *
 * In addition to Ported-Duff rules, we define miscellaneous blending-rule
 * type functions. Many of these are found for example from Imagemagick.
 *
 * \li \b Plus\par
 * Add RGBA values using Porter-Duff type rules.
 *
 * \li \b Minus\par
 * Substract RGBA values using Porter-Duff type rules.
 *
 * \li \b Add\par
 * Add RGBA values.
 *
 * \li \b Substract\par
 * Substract RGBA values.
 *
 * \li \b Multiply\par
 * Multiply RGBA values
 *
 * \li \b Difference\par
 * Absolute difference of RGBA values.
 *
 * \li \b Bumpmap\par
 * Adjust by intensity of source color.
 *
 * \li \b Dentmap\par
 * Adjust by intensity of destination color.
 *
 * \li \b CopyRed\par
 * Copy red component only.
 *
 * \li \b CopyGreen\par
 * Copy green component only.
 *
 * \li \b CopyBlue\par
 * Copy blue component only.
 *
 * \li \b CopyMatte\par
 * Copy opacity only.
 *
 * \li \b CopyHue\par
 * Copy hue component only.
 *
 * \li \b CopyLightness\par
 * Copy light component only.
 *
 * \li \b CopySaturation\par
 * Copy saturation component only.
 *
 * \li \b KeepMatte\par
 * Keep target matte only.
 *
 * \li \b KeepHue\par
 * Keep target hue only.
 *
 * \li \b KeepLightness\par
 * Keep target lightness only.
 *
 * \li \b KeepSaturation\par
 * Keep target saturation only.
 *
 * \li \b AddContrast\par
 * Enhance the contrast of target pixel.
 *
 * \li \b ReduceContrast\par
 * Reduce the contrast of target pixel.
 *
 * \li \b OnOpaque\par
 * Draw on opaque parts only.
 *
 * \li \b OnTransparent\par
 * Draw on transparent parts only.
 *
 * Note that ImageMagick Dissolve and Plus are equivalent.
 *
 */
// ======================================================================

#include "NFmiColorTools.h"

#include <newbase/NFmiGlobals.h>

#include <string>
#include <map>
#include <vector>
#include <algorithm>

namespace Imagine
{
using namespace std;

//! Map of recognized colour names.
/*!
 * A static map of recognized color names is initialized when
 * a name - rgb conversion is requested for the first time.
 * A map is used due to the large number of known colour names.
 */

static map<string, NFmiColorTools::Color> itsColorNames;

//! Map of recognized colour blending rule names.
/*!
 * A static map of recognized blending rule names is initialized when
 * a name - rule conversion is requested for the first time.
 * A map is used due to the largish number of available rules.
 */

static map<string, NFmiColorTools::NFmiBlendRule> itsBlendNames;

// ----------------------------------------------------------------------
/*!
 * Convert RGB to HLS values.
 * The RGB values are expected to be integers in range 0 - MaxRGB.
 * The LS values are floats in range 0-1, H in range 0-360.
 */
// ----------------------------------------------------------------------

void NFmiColorTools::RGBtoHLS(int red, int green, int blue, double *h, double *l, double *s)
{
  double v, m, delta, l1, h1, r, g, b;

  // Normalize input to range 0-1 for calculations

  r = static_cast<double>(red) / MaxRGB;
  g = static_cast<double>(green) / MaxRGB;
  b = static_cast<double>(blue) / MaxRGB;

  v = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
  m = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);

  l1 = (m + v) / 2;
  delta = v - m;
  if (l1 <= 0)
    *l = *h = *s = 0;
  else if (delta <= 0)
  {
    *l = l1;
    *h = *s = 0;
  }
  else
  {
    *s = (l1 < 0.5) ? delta / (v + m) : delta / (2 - v - m);
    *l = l1;
    if (r == v)
      h1 = (g - b) / delta;
    else if (g == v)
      h1 = 2 + (b - r) / delta;
    else
      h1 = 4 + (r - g) / delta;
    h1 = h1 * 60;
    if (h1 < 0) h1 = h1 + 360;
    *h = h1;
  }
}

// ----------------------------------------------------------------------
/*! Convert of HLS values to RGB values.
 * The RGB values are expected to be integers in range 0 - MaxRGB
 * The LS values are floats in range 0-1, H in range 0-360.
 */
// ----------------------------------------------------------------------

void NFmiColorTools::HLStoRGB(double h, double l, double s, int *r, int *g, int *b)
{
  double m1, m2;

  // handle greyscale separately
  if (s < 1e-10)
    *r = *g = *b = static_cast<int>(MaxRGB * l);
  else
  {
    m2 = (l < 0.5) ? l * (1 + s) : l + s - l * s;
    m1 = 2 * l - m2;
    *r = static_cast<int>(MaxRGB * hls_to_rgb_util(m1, m2, h + 120));
    *g = static_cast<int>(MaxRGB * hls_to_rgb_util(m1, m2, h));
    *b = static_cast<int>(MaxRGB * hls_to_rgb_util(m1, m2, h - 120));
  }
}

// Utility function needed by HLStoRGB

double NFmiColorTools::hls_to_rgb_util(double m1, double m2, double h)
{
  double hue = h;
  if (hue > 360) hue = hue - 360;
  if (hue < 0) hue = hue + 360;
  if (hue < 60)
    return m1 + (m2 - m1) * hue / 60;
  else if (hue < 180)
    return m2;
  else if (hue < 240)
    return m1 + (m2 - m1) * (240 - hue) / 60;
  else
    return m1;
}

// ----------------------------------------------------------------------
// Linear colour interpolation in HLS space.
//
// Often interpolation in RGB space results in LS values clearly
// different from the range defined by the interpolation limits.
// The result may be an unnecessarily dark gradient. To prevent
// this, interpolation can be performed in HLS space. The shorter
// path around the colour circle is chosen for interpolating H.
//
// Extrapolation is not performed, the limits are returned instead.
// ----------------------------------------------------------------------

NFmiColorTools::Color NFmiColorTools::Interpolate(Color c1, Color c2, float fraction)
{
  // Handle extrapolation

  if (fraction <= 0) return c1;
  if (fraction >= 1) return c2;

  double h1, l1, s1, h2, l2, s2, h, l, s;
  int r, g, b, a;

  // Covert to HLS space

  RGBtoHLS(GetRed(c1), GetGreen(c1), GetBlue(c1), &h1, &l1, &s1);
  RGBtoHLS(GetRed(c2), GetGreen(c2), GetBlue(c2), &h2, &l2, &s2);

  // Interpolate in HLS space

  l = l1 + (l2 - l1) * fraction;
  s = s1 + (s2 - s1) * fraction;
  if (abs(h1 - h2) <= 180)
    h = h1 + (h2 - h1) * fraction;
  else if (h1 <= h2)
    h = h1 + (h2 - 360 - h1) * fraction;
  else
    h = h1 - 360 + (h2 - h1 - 360) * fraction;
  if (h < 0) h += 360;
  if (h >= 360) h -= 360;

  // Back to RGB space

  HLStoRGB(h, l, s, &r, &g, &b);

  // Interpolate alpha

  a = static_cast<int>((1 - fraction) * GetAlpha(c1) + fraction * GetAlpha(c2));

  return MakeColor(r, g, b, a);
}

// ----------------------------------------------------------------------
// Adjust contrast in the desired direction.
// The code is taken from ImageMagick
// If theSign=1, contrast is enhanced, for -1 it is reduced.
// ----------------------------------------------------------------------

NFmiColorTools::Color NFmiColorTools::Contrast(NFmiColorTools::Color theColor, int theSign)
{
  // Get RGBA components

  int r = GetRed(theColor);
  int g = GetGreen(theColor);
  int b = GetBlue(theColor);
  int a = GetAlpha(theColor);

  // Transform to hls space

  double h, l, s;
  RGBtoHLS(r, g, b, &h, &l, &s);

  // Calculate adjustment

  const float m = 0.5;
  double adjustment = m * theSign * (m * (sin(3.14159265358979323846264 * (l - m)) + 1) - l);
  l += adjustment;
  l = std::max(std::min(l, 1.0), 0.0);

  // Transform back

  HLStoRGB(h, l, s, &r, &g, &b);

  return MakeColor(r, g, b, a);
}

// ----------------------------------------------------------------------
// Given a blending rule and the alpha of the source color, simplify
// the blending rule if possible so that rendering will occur at
// maximum speed.
//
// The simplifications are:
//
//	ColorOver      with alpha=0   (Fs=1,Fd=0)    ColorCopy
//	ColorKeepIn    with alpha=0   (Fs=0,Fd=1)    ColorKeep
//	ColorKeepOut   with alpha=0   (Fs=0,Fd=1)    ColorKeep
//    ColorKeepAtop  with alpha=0   (Fs=1-Ad,Fd=1) ColorUnder
//	ColorXor       with alpha=0   (Fs=1-Ad,Fd=1) ColorUnder
//	ColorKeepIn    with alpha=max (Fs=0,Fd=0)    ColorClear
//	ColorKeepOut   with alpha=max (Fs=0,Fd=0)    ColorClear
//	ColorAtop      with alpha=max (Fs=Ad,Fd=0)   ColorIn
//	ColorKeepAtop  with alpha=max (Fs=1-Ad,Fd=0) ColorKeepOut
//	ColorXor       with alpha=max (Fs=1-Ad,Fd=0) ColorKeepOut
//
// ----------------------------------------------------------------------

NFmiColorTools::NFmiBlendRule NFmiColorTools::Simplify(NFmiColorTools::NFmiBlendRule rule,
                                                       int alpha)
{
  if (alpha == NFmiColorTools::Opaque)
  {
    if (rule == NFmiColorTools::kFmiColorOver) return NFmiColorTools::kFmiColorCopy;

    if (rule == NFmiColorTools::kFmiColorKeepIn) return NFmiColorTools::kFmiColorKeep;

    if (rule == NFmiColorTools::kFmiColorKeepOut) return NFmiColorTools::kFmiColorKeep;

    if (rule == NFmiColorTools::kFmiColorKeepAtop) return NFmiColorTools::kFmiColorUnder;

    if (rule == NFmiColorTools::kFmiColorXor) return NFmiColorTools::kFmiColorUnder;
  }

  else if (alpha == NFmiColorTools::Transparent)
  {
    if (rule == NFmiColorTools::kFmiColorKeepIn) return NFmiColorTools::kFmiColorClear;

    if (rule == NFmiColorTools::kFmiColorKeepOut) return NFmiColorTools::kFmiColorClear;

    if (rule == NFmiColorTools::kFmiColorAtop) return NFmiColorTools::kFmiColorIn;

    if (rule == NFmiColorTools::kFmiColorKeepAtop) return NFmiColorTools::kFmiColorKeepOut;

    if (rule == NFmiColorTools::kFmiColorXor) return NFmiColorTools::kFmiColorKeepOut;
  }

  return rule;
}

// ----------------------------------------------------------------------
// General purpose string to color conversion
//
// Accepted formats:
//  - #aarrggbb  Hexadecimal RGBA
//  - #rrggbb    Hexadecimal RGB
//  - r,g,b,a    Decimal RGBA
//  - r,g,b      Decimal RGB
//
// Returns MissingColor for unrecognized names
// ----------------------------------------------------------------------

NFmiColorTools::Color NFmiColorTools::ToColor(const string &theColor)
{
  // Handle hex format number

  if (theColor[0] == '#') return HexToColor(theColor.substr(1));

  // Handle ascii format

  else if (theColor[0] < '0' || theColor[0] > '9')
  {
    string::size_type pos = theColor.find(",");
    if (pos == string::npos)
      return ColorValue(theColor);
    else
    {
      int value = -1;
      for (size_t i = pos + 1; i < theColor.length(); i++)
      {
        if (theColor[i] >= '0' && theColor[i] <= '9')
        {
          if (value < 0)
            value = theColor[i] - '0';
          else
            value = value * 10 + theColor[i] - '0';
        }
        else
          return MissingColor;
      }
      if (value < 0) return MissingColor;
      Color tmp = ColorValue(theColor.substr(0, pos));
      return ReplaceAlpha(tmp, value);
    }
  }

  // Handle decimal format

  else
  {
    vector<int> tmp;
    int value = -1;
    for (unsigned int i = 0; i < theColor.length(); i++)
    {
      if (theColor[i] >= '0' && theColor[i] <= '9')
      {
        if (value < 0)
          value = theColor[i] - '0';
        else
          value = value * 10 + theColor[i] - '0';
      }
      else if (theColor[i] == ',')
      {
        tmp.push_back(value);
        value = -1;
      }
      else
        return MissingColor;
    }
    if (value >= 0) tmp.push_back(value);

    if (tmp.size() == 3)
      return MakeColor(tmp[0], tmp[1], tmp[2], 0);
    else if (tmp.size() == 4)
      return MakeColor(tmp[0], tmp[1], tmp[2], tmp[3]);
    else
      return MissingColor;
  }
}

// ----------------------------------------------------------------------
// Utility function to convert hexadecimal numbers to a color
// ----------------------------------------------------------------------

NFmiColorTools::Color NFmiColorTools::HexToColor(const std::string &theHex)
{
  Color c = 0;
  for (unsigned int i = 0; i < theHex.length(); i++)
  {
    c <<= 4;
    if (theHex[i] >= '0' && theHex[i] <= '9')
      c += theHex[i] - '0';
    else if (theHex[i] >= 'A' && theHex[i] <= 'F')
      c += 10 + theHex[i] - 'A';
    else if (theHex[i] >= 'a' && theHex[i] <= 'f')
      c += 10 + theHex[i] - 'a';
    else
      return MissingColor;
  }
  return c;
}

// ----------------------------------------------------------------------
// Convert a color name to a color value
// Returns MissingColor if the color name is undefined
// ----------------------------------------------------------------------

NFmiColorTools::Color NFmiColorTools::ColorValue(const string &theName)
{
  ColorNamesInit();

  // Search for the string

  map<string, NFmiColorTools::Color>::iterator iter;
  iter = itsColorNames.find(theName);

  if (iter == itsColorNames.end())
    return MissingColor;
  else
    return iter->second;
}

// ----------------------------------------------------------------------
// Convert a color value to a color name
// Returns a MissingColorName if the name is unknown
// ----------------------------------------------------------------------

const string NFmiColorTools::ColorName(const NFmiColorTools::Color &theColor)
{
  ColorNamesInit();

  // Search for the color value

  map<string, NFmiColorTools::Color>::iterator iter;

  for (iter = itsColorNames.begin(); iter != itsColorNames.end(); ++iter)
  {
    if (iter->second == theColor) return iter->first;
  }
  static const string MissingColorName = string("");
  return MissingColorName;
}

// ----------------------------------------------------------------------
// Initialize table of SVG color names.
// See: http://www.w3.org/TR/SVG/types.html#ColorKeyWords
// ----------------------------------------------------------------------

void NFmiColorTools::ColorNamesInit(void)
{
  // Abort if already initialized

  if (!itsColorNames.empty()) return;

#define COLORINSERT(N, R, G, B) itsColorNames.insert(make_pair(string(N), MakeColor(R, G, B)))

  itsColorNames.insert(make_pair(string("none"), NoColor));
  itsColorNames.insert(make_pair(string("transparent"), TransparentColor));

  COLORINSERT("aliceblue", 240, 248, 255);
  COLORINSERT("antiquewhite", 250, 235, 215);
  COLORINSERT("aqua", 0, 255, 255);
  COLORINSERT("aquamarine", 127, 255, 212);
  COLORINSERT("azure", 240, 255, 255);
  COLORINSERT("beige", 245, 245, 220);
  COLORINSERT("bisque", 255, 228, 196);
  COLORINSERT("black", 0, 0, 0);
  COLORINSERT("blanchedalmond", 255, 235, 205);
  COLORINSERT("blue", 0, 0, 255);
  COLORINSERT("blueviolet", 138, 43, 226);
  COLORINSERT("brown", 165, 42, 42);
  COLORINSERT("burlywood", 222, 184, 135);
  COLORINSERT("cadetblue", 95, 158, 160);
  COLORINSERT("chartreuse", 127, 255, 0);
  COLORINSERT("chocolate", 210, 105, 30);
  COLORINSERT("coral", 255, 127, 80);
  COLORINSERT("cornflowerblue", 100, 149, 237);
  COLORINSERT("cornsilk", 255, 248, 220);
  COLORINSERT("crimson", 220, 20, 60);
  COLORINSERT("cyan", 0, 255, 255);
  COLORINSERT("darkblue", 0, 0, 139);
  COLORINSERT("darkcyan", 0, 139, 139);
  COLORINSERT("darkgoldenrod", 184, 134, 11);
  COLORINSERT("darkgray", 169, 169, 169);
  COLORINSERT("darkgreen", 0, 100, 0);
  COLORINSERT("darkgrey", 169, 169, 169);
  COLORINSERT("darkkhaki", 189, 183, 107);
  COLORINSERT("darkmagenta", 139, 0, 139);
  COLORINSERT("darkolivegreen", 85, 107, 47);
  COLORINSERT("darkorange", 255, 140, 0);
  COLORINSERT("darkorchid", 153, 50, 204);
  COLORINSERT("darkred", 139, 0, 0);
  COLORINSERT("darksalmon", 233, 150, 122);
  COLORINSERT("darkseagreen", 143, 188, 143);
  COLORINSERT("darkslateblue", 72, 61, 139);
  COLORINSERT("darkslategray", 47, 79, 79);
  COLORINSERT("darkslategrey", 47, 79, 79);
  COLORINSERT("darkturquoise", 0, 206, 209);
  COLORINSERT("darkviolet", 148, 0, 211);
  COLORINSERT("deeppink", 255, 20, 147);
  COLORINSERT("deepskyblue", 0, 191, 255);
  COLORINSERT("dimgray", 105, 105, 105);
  COLORINSERT("dimgrey", 105, 105, 105);
  COLORINSERT("dodgerblue", 30, 144, 255);
  COLORINSERT("firebrick", 178, 34, 34);
  COLORINSERT("floralwhite", 255, 250, 240);
  COLORINSERT("forestgreen", 34, 139, 34);
  COLORINSERT("fuchsia", 255, 0, 255);
  COLORINSERT("gainsboro", 220, 220, 220);
  COLORINSERT("ghostwhite", 248, 248, 255);
  COLORINSERT("gold", 255, 215, 0);
  COLORINSERT("goldenrod", 218, 165, 32);
  COLORINSERT("gray", 128, 128, 128);
  COLORINSERT("green", 0, 128, 0);
  COLORINSERT("greenyellow", 173, 255, 47);
  COLORINSERT("grey", 128, 128, 128);
  COLORINSERT("honeydew", 240, 255, 240);
  COLORINSERT("hotpink", 255, 105, 180);
  COLORINSERT("indianred", 205, 92, 92);
  COLORINSERT("indigo", 75, 0, 130);
  COLORINSERT("ivory", 255, 255, 240);
  COLORINSERT("khaki", 240, 230, 140);
  COLORINSERT("lavender", 230, 230, 250);
  COLORINSERT("lavenderblush", 255, 240, 245);
  COLORINSERT("lawngreen", 124, 252, 0);
  COLORINSERT("lemonchiffon", 255, 250, 205);
  COLORINSERT("lightblue", 173, 216, 230);
  COLORINSERT("lightcoral", 240, 128, 128);
  COLORINSERT("lightcyan", 224, 255, 255);
  COLORINSERT("lightgoldenrodyellow", 250, 250, 210);
  COLORINSERT("lightgray", 211, 211, 211);
  COLORINSERT("lightgreen", 144, 238, 144);
  COLORINSERT("lightgrey", 211, 211, 211);
  COLORINSERT("lightpink", 255, 182, 193);
  COLORINSERT("lightsalmon", 255, 160, 122);
  COLORINSERT("lightseagreen", 32, 178, 170);
  COLORINSERT("lightskyblue", 135, 206, 250);
  COLORINSERT("lightslategray", 119, 136, 153);
  COLORINSERT("lightslategrey", 119, 136, 153);
  COLORINSERT("lightsteelblue", 176, 196, 222);
  COLORINSERT("lightyellow", 255, 255, 224);
  COLORINSERT("lime", 0, 255, 0);
  COLORINSERT("limegreen", 50, 205, 50);
  COLORINSERT("linen", 250, 240, 230);
  COLORINSERT("magenta", 255, 0, 255);
  COLORINSERT("maroon", 128, 0, 0);
  COLORINSERT("mediumaquamarine", 102, 205, 170);
  COLORINSERT("mediumblue", 0, 0, 205);
  COLORINSERT("mediumorchid", 186, 85, 211);
  COLORINSERT("mediumpurple", 147, 112, 219);
  COLORINSERT("mediumseagreen", 60, 179, 113);
  COLORINSERT("mediumslateblue", 123, 104, 238);
  COLORINSERT("mediumspringgreen", 0, 250, 154);
  COLORINSERT("mediumturquoise", 72, 209, 204);
  COLORINSERT("mediumvioletred", 199, 21, 133);
  COLORINSERT("midnightblue", 25, 25, 112);
  COLORINSERT("mintcream", 245, 255, 250);
  COLORINSERT("mistyrose", 255, 228, 225);
  COLORINSERT("moccasin", 255, 228, 181);
  COLORINSERT("navajowhite", 255, 222, 173);
  COLORINSERT("navy", 0, 0, 128);
  COLORINSERT("oldlace", 253, 245, 230);
  COLORINSERT("olive", 128, 128, 0);
  COLORINSERT("olivedrab", 107, 142, 35);
  COLORINSERT("orange", 255, 165, 0);
  COLORINSERT("orangered", 255, 69, 0);
  COLORINSERT("orchid", 218, 112, 214);
  COLORINSERT("palegoldenrod", 238, 232, 170);
  COLORINSERT("palegreen", 152, 251, 152);
  COLORINSERT("paleturquoise", 175, 238, 238);
  COLORINSERT("palevioletred", 219, 112, 147);
  COLORINSERT("papayawhip", 255, 239, 213);
  COLORINSERT("peachpuff", 255, 218, 185);
  COLORINSERT("peru", 205, 133, 63);
  COLORINSERT("pink", 255, 192, 203);
  COLORINSERT("plum", 221, 160, 221);
  COLORINSERT("powderblue", 176, 224, 230);
  COLORINSERT("purple", 128, 0, 128);
  COLORINSERT("red", 255, 0, 0);
  COLORINSERT("rosybrown", 188, 143, 143);
  COLORINSERT("royalblue", 65, 105, 225);
  COLORINSERT("saddlebrown", 139, 69, 19);
  COLORINSERT("salmon", 250, 128, 114);
  COLORINSERT("sandybrown", 244, 164, 96);
  COLORINSERT("seagreen", 46, 139, 87);
  COLORINSERT("seashell", 255, 245, 238);
  COLORINSERT("sienna", 160, 82, 45);
  COLORINSERT("silver", 192, 192, 192);
  COLORINSERT("skyblue", 135, 206, 235);
  COLORINSERT("slateblue", 106, 90, 205);
  COLORINSERT("slategray", 112, 128, 144);
  COLORINSERT("slategrey", 112, 128, 144);
  COLORINSERT("snow", 255, 250, 250);
  COLORINSERT("springgreen", 0, 255, 127);
  COLORINSERT("steelblue", 70, 130, 180);
  COLORINSERT("tan", 210, 180, 140);
  COLORINSERT("teal", 0, 128, 128);
  COLORINSERT("thistle", 216, 191, 216);
  COLORINSERT("tomato", 255, 99, 71);
  COLORINSERT("turquoise", 64, 224, 208);
  COLORINSERT("violet", 238, 130, 238);
  COLORINSERT("wheat", 245, 222, 179);
  COLORINSERT("white", 255, 255, 255);
  COLORINSERT("whitesmoke", 245, 245, 245);
  COLORINSERT("yellow", 255, 255, 0);
  COLORINSERT("yellowgreen", 154, 205, 50);
}

// ----------------------------------------------------------------------
// Convert a blend name to a blend value
// Returns kFmiColorRuleMissing if the color name is undefined
// ----------------------------------------------------------------------

NFmiColorTools::NFmiBlendRule NFmiColorTools::BlendValue(const string &theName)
{
  BlendNamesInit();

  // Search for the string

  map<string, NFmiColorTools::NFmiBlendRule>::iterator iter;
  iter = itsBlendNames.find(theName);

  if (iter == itsBlendNames.end())
    return kFmiColorRuleMissing;
  else
    return iter->second;
}

// ----------------------------------------------------------------------
// Convert a blend rule to a blend name
// Returns a MissingBlendName if the name is unknown
// ----------------------------------------------------------------------

const string NFmiColorTools::BlendName(const NFmiColorTools::NFmiBlendRule &theRule)
{
  BlendNamesInit();

  // Search for the blend value

  map<string, NFmiColorTools::NFmiBlendRule>::iterator iter;

  for (iter = itsBlendNames.begin(); iter != itsBlendNames.end(); ++iter)
  {
    if (iter->second == theRule) return iter->first;
  }
  static const string MissingBlendName = string("");
  return MissingBlendName;
}
// ----------------------------------------------------------------------
// Initialize table of Porter-Duff blending rule names
// See the header file for the respective enum
// ----------------------------------------------------------------------

void NFmiColorTools::BlendNamesInit(void)
{
  // Abort if already initialized

  if (!itsBlendNames.empty()) return;

#define BLENDINSERT(N, B) itsBlendNames.insert(make_pair(string(N), B))

  BLENDINSERT("Clear", kFmiColorClear);
  BLENDINSERT("Copy", kFmiColorCopy);
  BLENDINSERT("Keep", kFmiColorKeep);
  BLENDINSERT("Over", kFmiColorOver);
  BLENDINSERT("Under", kFmiColorUnder);
  BLENDINSERT("In", kFmiColorIn);
  BLENDINSERT("KeepIn", kFmiColorKeepIn);
  BLENDINSERT("Out", kFmiColorOut);
  BLENDINSERT("KeepOut", kFmiColorKeepOut);
  BLENDINSERT("Atop", kFmiColorAtop);
  BLENDINSERT("KeepAtop", kFmiColorKeepAtop);
  BLENDINSERT("Xor", kFmiColorXor);

  BLENDINSERT("Plus", kFmiColorPlus);

#ifdef IMAGINE_WITH_CAIRO
  // Take "OnOpaque" as "Copy" for Cairo; used in selftests
  //
  BLENDINSERT("OnOpaque", kFmiColorCopy);
#else
  BLENDINSERT("Minus", kFmiColorMinus);
  BLENDINSERT("Add", kFmiColorAdd);
  BLENDINSERT("Substract", kFmiColorSubstract);
  BLENDINSERT("Multiply", kFmiColorMultiply);
  BLENDINSERT("Difference", kFmiColorDifference);
  BLENDINSERT("CopyRed", kFmiColorCopyRed);
  BLENDINSERT("CopyGreen", kFmiColorCopyGreen);
  BLENDINSERT("CopyBlue", kFmiColorCopyBlue);
  BLENDINSERT("CopyMatte", kFmiColorCopyMatte);
  BLENDINSERT("CopyHue", kFmiColorCopyHue);
  BLENDINSERT("CopyLightness", kFmiColorCopyLightness);
  BLENDINSERT("CopySaturation", kFmiColorCopySaturation);
  BLENDINSERT("KeepMatte", kFmiColorKeepMatte);
  BLENDINSERT("KeepHue", kFmiColorKeepHue);
  BLENDINSERT("KeepLightness", kFmiColorKeepLightness);
  BLENDINSERT("KeepSaturation", kFmiColorKeepSaturation);
  BLENDINSERT("Bumpmap", kFmiColorBumpmap);
  BLENDINSERT("Dentmap", kFmiColorDentmap);
  BLENDINSERT("AddContrast", kFmiColorAddContrast);
  BLENDINSERT("ReduceContrast", kFmiColorReduceContrast);

  BLENDINSERT("OnOpaque", kFmiColorOnOpaque);
  BLENDINSERT("OnTransparent", kFmiColorOnTransparent);
#endif
}

}  // namespace Imagine

//======================================================================
