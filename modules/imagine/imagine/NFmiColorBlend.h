// ======================================================================
/*!
 * \file NFmiColorBlend.h
 * \brief Templates for blending RGBA colors.
 *
 * \b History:
 *
 * \li 06.12.2001 Mika Heiskanen\par
 * Implemented by separating code from NFmiColorTools.h. The idea
 * is to make this file library-internal, so that GNU linker would
 * be able to compile programs using NFmiColorTools. NFmiBlendRule
 * enumeration was intentionally left in NFmiColorTools.h so that
 * external programs could still use the rules, if not blend directly.
 */
// ======================================================================

#pragma once

#include "NFmiColorTools.h"
#include <cstdlib>

#ifdef __BORLANDC__
using std::abs;
#endif

namespace Imagine
{
//
// Porter-Duff and other miscellaneous blending rules
//
// ======================================================================

// For some code we define versions for efficiency rules.
// The first version takes a two colors and blends them,
// the second assumes the source color will be used heavily,
// and that the individual color components are extracted
// in advance outside a rendering loop for speed.
//
// Two versions are defined only for some of the more important rules.

// ----------------------------------------------------------------------
// Porter-Duff Clear : Fs=0, Fd=0
// ----------------------------------------------------------------------

struct NFmiColorBlendClear
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::Black;
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::Black;
  }
};

// ----------------------------------------------------------------------
// Porter-Duff Copy : Fs=1, Fd=0
// ----------------------------------------------------------------------

struct NFmiColorBlendCopy
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return src;
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::MakeColor(srcr, srcg, srcb, srca);
  }
};

// ----------------------------------------------------------------------
// Porter-Duff Keep : Fs=0, Fd=1
// ----------------------------------------------------------------------

struct NFmiColorBlendKeep
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return dst;
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return dst;
  }
};

// ----------------------------------------------------------------------
// Porter-Duff Over : Fs=1, Fd=1-As
// ----------------------------------------------------------------------

struct NFmiColorBlendOver
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    // This optimization is for banging partly opaque/transparent images onto an image
    int srca = NFmiColorTools::GetAlpha(src);
    if (srca == NFmiColorTools::Opaque) return src;
    if (srca == NFmiColorTools::Transparent) return dst;

    // We inlined this from the other Blend function for best possible speed.
    // The decision is based on profiling a time critical contouring application.
    // We also got rid of a couple local variables.

    int srcp = NFmiColorTools::MaxAlpha - srca;
    int dsta = NFmiColorTools::GetAlpha(dst);
    int dstp = (NFmiColorTools::MaxAlpha - dsta) * srca / NFmiColorTools::MaxAlpha;

    return NFmiColorTools::MakeColor(
        (srcp * NFmiColorTools::GetRed(src) + dstp * NFmiColorTools::GetRed(dst)) /
            NFmiColorTools::MaxAlpha,
        (srcp * NFmiColorTools::GetGreen(src) + dstp * NFmiColorTools::GetGreen(dst)) /
            NFmiColorTools::MaxAlpha,
        (srcp * NFmiColorTools::GetBlue(src) + dstp * NFmiColorTools::GetBlue(dst)) /
            NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    // This is as in ImageMagick

    if (srca == NFmiColorTools::Transparent) return dst;

    int srcp = NFmiColorTools::MaxAlpha - srca;
    int dsta = NFmiColorTools::GetAlpha(dst);
    int dstp = (NFmiColorTools::MaxAlpha - dsta) * srca / NFmiColorTools::MaxAlpha;

    return NFmiColorTools::MakeColor(
        (srcp * srcr + dstp * NFmiColorTools::GetRed(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcg + dstp * NFmiColorTools::GetGreen(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcb + dstp * NFmiColorTools::GetBlue(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Porter-Duff Under : Fs=1-Ad, Fd=1
// ----------------------------------------------------------------------

struct NFmiColorBlendUnder
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int srcp = (NFmiColorTools::MaxAlpha - srca) * srca / NFmiColorTools::MaxAlpha;
    int dsta = NFmiColorTools::GetAlpha(dst);
    int dstp = NFmiColorTools::MaxAlpha - dsta;

    return NFmiColorTools::SafeColor(
        (srcp * srcr + dstp * NFmiColorTools::GetRed(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcg + dstp * NFmiColorTools::GetGreen(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcb + dstp * NFmiColorTools::GetBlue(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Porter-Duff In : Fs=Ad, Fd=0
// ----------------------------------------------------------------------

struct NFmiColorBlendIn
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int srcp = (NFmiColorTools::MaxAlpha - srca) * (NFmiColorTools::MaxAlpha - dsta) /
               NFmiColorTools::MaxAlpha;

    return NFmiColorTools::SafeColor(srcp * srcr / NFmiColorTools::MaxAlpha,
                                     srcp * srcg / NFmiColorTools::MaxAlpha,
                                     srcp * srcb / NFmiColorTools::MaxAlpha,
                                     srcp * srca / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Porter-Duff KeepIn : Fs=0, Fd=As
// ----------------------------------------------------------------------

struct NFmiColorBlendKeepIn
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int dstp = (NFmiColorTools::MaxAlpha - dsta) * (NFmiColorTools::MaxAlpha - srca) /
               NFmiColorTools::MaxAlpha;
    return NFmiColorTools::SafeColor(
        dstp * NFmiColorTools::GetRed(dst) / NFmiColorTools::MaxAlpha,
        dstp * NFmiColorTools::GetGreen(dst) / NFmiColorTools::MaxAlpha,
        dstp * NFmiColorTools::GetBlue(dst) / NFmiColorTools::MaxAlpha,
        dstp * NFmiColorTools::GetAlpha(dst) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Porter-Duff Out : Fs=1-Ad, Fd=0
// ----------------------------------------------------------------------

struct NFmiColorBlendOut
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int srcp = (NFmiColorTools::MaxAlpha - dsta) * dsta / NFmiColorTools::MaxAlpha;

    return NFmiColorTools::SafeColor(srcp * srcr / NFmiColorTools::MaxAlpha,
                                     srcp * srcg / NFmiColorTools::MaxAlpha,
                                     srcp * srcb / NFmiColorTools::MaxAlpha,
                                     srcp * srca / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Porter-Duff KeepOut : Fs=0, Fd=1-As
// ----------------------------------------------------------------------

struct NFmiColorBlendKeepOut
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int dstp = (NFmiColorTools::MaxAlpha - dsta) * srca / NFmiColorTools::MaxAlpha;

    return NFmiColorTools::SafeColor(
        dstp * NFmiColorTools::GetRed(dst) / NFmiColorTools::MaxAlpha,
        dstp * NFmiColorTools::GetGreen(dst) / NFmiColorTools::MaxAlpha,
        dstp * NFmiColorTools::GetBlue(dst) / NFmiColorTools::MaxAlpha,
        dstp * NFmiColorTools::GetAlpha(dst) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Porter-Duff Atop : Fs=Ad, Fd=1-As
// ----------------------------------------------------------------------

struct NFmiColorBlendAtop
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int srcp = (NFmiColorTools::MaxAlpha - srca) * (NFmiColorTools::MaxAlpha - dsta) /
               NFmiColorTools::MaxAlpha;
    int dstp = (NFmiColorTools::MaxAlpha - dsta) * srca / NFmiColorTools::MaxAlpha;

    return NFmiColorTools::SafeColor(
        (srcp * srcr + dstp * NFmiColorTools::GetRed(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcg + dstp * NFmiColorTools::GetGreen(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcb + dstp * NFmiColorTools::GetBlue(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Porter-Duff KeepAtop : Fs=1-Ad, Fd=As
// ----------------------------------------------------------------------

struct NFmiColorBlendKeepAtop
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int srcp = (NFmiColorTools::MaxAlpha - srca) * dsta / NFmiColorTools::MaxAlpha;
    int dstp = (NFmiColorTools::MaxAlpha - dsta) * (NFmiColorTools::MaxAlpha - srca) /
               NFmiColorTools::MaxAlpha;

    return NFmiColorTools::SafeColor(
        (srcp * srcr + dstp * NFmiColorTools::GetRed(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcg + dstp * NFmiColorTools::GetGreen(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcb + dstp * NFmiColorTools::GetBlue(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Porter-Duff Xor : Fs=1-Ad, Fd=1-As
// ----------------------------------------------------------------------

struct NFmiColorBlendXor
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int srcp = (NFmiColorTools::MaxAlpha - srca) * dsta / NFmiColorTools::MaxAlpha;
    int dstp = (NFmiColorTools::MaxAlpha - dsta) * (NFmiColorTools::MaxAlpha - srca) /
               NFmiColorTools::MaxAlpha;
    return NFmiColorTools::SafeColor(
        (srcp * srcr + dstp * NFmiColorTools::GetRed(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcg + dstp * NFmiColorTools::GetGreen(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcb + dstp * NFmiColorTools::GetBlue(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }
};

// ======================================================================
// Additional rules not by Porter-Duff
// ======================================================================

// ----------------------------------------------------------------------
// Color Plus - Equals Porter-Duff with Fs=1, Fd=1
// ----------------------------------------------------------------------

struct NFmiColorBlendPlus
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int srcp = (NFmiColorTools::MaxAlpha - srca);
    int dstp = (NFmiColorTools::MaxAlpha - dsta);
    return NFmiColorTools::SafeColor(
        (srcp * srcr + dstp * NFmiColorTools::GetRed(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcg + dstp * NFmiColorTools::GetGreen(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcb + dstp * NFmiColorTools::GetBlue(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Color Minus : Equals Porter-Duff with Fs=1, Fd=-1
// ----------------------------------------------------------------------

struct NFmiColorBlendMinus
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int srcp = (NFmiColorTools::MaxAlpha - srca);
    int dstp = -(NFmiColorTools::MaxAlpha - dsta);
    return NFmiColorTools::SafestColor(
        (srcp * srcr + dstp * NFmiColorTools::GetRed(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcg + dstp * NFmiColorTools::GetGreen(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcb + dstp * NFmiColorTools::GetBlue(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Color Add - add components directly
// ----------------------------------------------------------------------

struct NFmiColorBlendAdd
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::SafeColor(srcr + NFmiColorTools::GetRed(dst),
                                     srcg + NFmiColorTools::GetGreen(dst),
                                     srcb + NFmiColorTools::GetBlue(dst),
                                     srca + NFmiColorTools::GetAlpha(dst));
  }
};

// ----------------------------------------------------------------------
// Color Substract - substract components directly
// ----------------------------------------------------------------------

struct NFmiColorBlendSubstract
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::SafestColor(srcr - NFmiColorTools::GetRed(dst),
                                       srcg - NFmiColorTools::GetGreen(dst),
                                       srcb - NFmiColorTools::GetBlue(dst),
                                       srca - NFmiColorTools::GetAlpha(dst));
  }
};

// ----------------------------------------------------------------------
// Color Multiply - multiply components directly
// ----------------------------------------------------------------------

struct NFmiColorBlendMultiply
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::SafeColor(
        srcr * NFmiColorTools::GetRed(dst) / NFmiColorTools::MaxRGB,
        srcg * NFmiColorTools::GetGreen(dst) / NFmiColorTools::MaxRGB,
        srcb * NFmiColorTools::GetBlue(dst) / NFmiColorTools::MaxRGB,
        srca * NFmiColorTools::GetAlpha(dst) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Color Difference - absolute difference of components directly
// ----------------------------------------------------------------------

struct NFmiColorBlendDifference
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::SafeColor(abs(srcr - NFmiColorTools::GetRed(dst)),
                                     abs(srcg - NFmiColorTools::GetGreen(dst)),
                                     abs(srcb - NFmiColorTools::GetBlue(dst)),
                                     abs(srca - NFmiColorTools::GetAlpha(dst)));
  }
};

// ----------------------------------------------------------------------
// Color CopyRed - Copy red component only
// ----------------------------------------------------------------------

struct NFmiColorBlendCopyRed
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReplaceRed(dst, NFmiColorTools::GetRed(src));
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReplaceRed(dst, srcr);
  }
};

// ----------------------------------------------------------------------
// Color CopyGreen - Copy green component only
// ----------------------------------------------------------------------

struct NFmiColorBlendCopyGreen
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReplaceGreen(dst, NFmiColorTools::GetGreen(src));
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReplaceGreen(dst, srcg);
  }
};

// ----------------------------------------------------------------------
// Color CopyBlue - Copy blue component only
// ----------------------------------------------------------------------

struct NFmiColorBlendCopyBlue
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReplaceBlue(dst, NFmiColorTools::GetBlue(src));
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReplaceBlue(dst, srcb);
  }
};

// ----------------------------------------------------------------------
// Color CopyMatte - Copy matte component only
// ----------------------------------------------------------------------

struct NFmiColorBlendCopyMatte
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReplaceAlpha(dst, NFmiColorTools::GetAlpha(src));
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReplaceAlpha(dst, srca);
  }
};

// ----------------------------------------------------------------------
// Color CopyHue - Copy hue component only
// ----------------------------------------------------------------------

struct NFmiColorBlendCopyHue
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    double hsrc, lsrc, ssrc;
    NFmiColorTools::RGBtoHLS(srcr, srcg, srcb, &hsrc, &lsrc, &ssrc);

    double hdst, ldst, sdst;
    NFmiColorTools::RGBtoHLS(NFmiColorTools::GetRed(dst),
                             NFmiColorTools::GetGreen(dst),
                             NFmiColorTools::GetBlue(dst),
                             &hdst,
                             &ldst,
                             &sdst);
    int r, g, b;
    NFmiColorTools::HLStoRGB(hsrc, ldst, sdst, &r, &g, &b);

    return NFmiColorTools::MakeColor(r, g, b, NFmiColorTools::GetAlpha(dst));
  }
};

// ----------------------------------------------------------------------
// Color CopyLightness - Copy lightness component only
// ----------------------------------------------------------------------

struct NFmiColorBlendCopyLightness
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    double hsrc, lsrc, ssrc;
    NFmiColorTools::RGBtoHLS(srcr, srcg, srcb, &hsrc, &lsrc, &ssrc);

    double hdst, ldst, sdst;
    NFmiColorTools::RGBtoHLS(NFmiColorTools::GetRed(dst),
                             NFmiColorTools::GetGreen(dst),
                             NFmiColorTools::GetBlue(dst),
                             &hdst,
                             &ldst,
                             &sdst);
    int r, g, b;
    NFmiColorTools::HLStoRGB(hdst, lsrc, sdst, &r, &g, &b);

    return NFmiColorTools::MakeColor(r, g, b, NFmiColorTools::GetAlpha(dst));
  }
};

// ----------------------------------------------------------------------
// Color CopySaturation - Copy saturation component only
// ----------------------------------------------------------------------

struct NFmiColorBlendCopySaturation
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    double hsrc, lsrc, ssrc;
    NFmiColorTools::RGBtoHLS(srcr, srcg, srcb, &hsrc, &lsrc, &ssrc);

    double hdst, ldst, sdst;
    NFmiColorTools::RGBtoHLS(NFmiColorTools::GetRed(dst),
                             NFmiColorTools::GetGreen(dst),
                             NFmiColorTools::GetBlue(dst),
                             &hdst,
                             &ldst,
                             &sdst);

    int r, g, b;
    NFmiColorTools::HLStoRGB(hdst, ldst, ssrc, &r, &g, &b);

    return NFmiColorTools::MakeColor(r, g, b, NFmiColorTools::GetAlpha(dst));
  }
};

// ----------------------------------------------------------------------
// Color KeepMatte - Keep matte only
// ----------------------------------------------------------------------

struct NFmiColorBlendKeepMatte
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::MakeColor(srcr, srcg, srcb, NFmiColorTools::GetAlpha(dst));
  }
};

// ----------------------------------------------------------------------
// Color KeepHue - Keep hue only
// ----------------------------------------------------------------------

struct NFmiColorBlendKeepHue
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    double hsrc, lsrc, ssrc;
    NFmiColorTools::RGBtoHLS(srcr, srcg, srcb, &hsrc, &lsrc, &ssrc);

    double hdst, ldst, sdst;
    NFmiColorTools::RGBtoHLS(NFmiColorTools::GetRed(dst),
                             NFmiColorTools::GetGreen(dst),
                             NFmiColorTools::GetBlue(dst),
                             &hdst,
                             &ldst,
                             &sdst);
    int r, g, b;
    NFmiColorTools::HLStoRGB(hdst, lsrc, ssrc, &r, &g, &b);

    return NFmiColorTools::MakeColor(r, g, b, srca);
  }
};

// ----------------------------------------------------------------------
// Color KeepLightness - Keep lightness only
// ----------------------------------------------------------------------

struct NFmiColorBlendKeepLightness
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    double hsrc, lsrc, ssrc;
    NFmiColorTools::RGBtoHLS(srcr, srcg, srcb, &hsrc, &lsrc, &ssrc);

    double hdst, ldst, sdst;
    NFmiColorTools::RGBtoHLS(NFmiColorTools::GetRed(dst),
                             NFmiColorTools::GetGreen(dst),
                             NFmiColorTools::GetBlue(dst),
                             &hdst,
                             &ldst,
                             &sdst);
    int r, g, b;
    NFmiColorTools::HLStoRGB(hsrc, ldst, ssrc, &r, &g, &b);

    return NFmiColorTools::MakeColor(r, g, b, srca);
  }
};

// ----------------------------------------------------------------------
// Color KeepSaturation - Keep saturation only
// ----------------------------------------------------------------------

struct NFmiColorBlendKeepSaturation
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    double hsrc, lsrc, ssrc;
    NFmiColorTools::RGBtoHLS(srcr, srcg, srcb, &hsrc, &lsrc, &ssrc);

    double hdst, ldst, sdst;
    NFmiColorTools::RGBtoHLS(NFmiColorTools::GetRed(dst),
                             NFmiColorTools::GetGreen(dst),
                             NFmiColorTools::GetBlue(dst),
                             &hdst,
                             &ldst,
                             &sdst);
    int r, g, b;
    NFmiColorTools::HLStoRGB(hsrc, lsrc, sdst, &r, &g, &b);

    return NFmiColorTools::MakeColor(r, g, b, srca);
  }
};

// ----------------------------------------------------------------------
// Color Bumpmap - Bumpmap destination by source intensity
// ----------------------------------------------------------------------

struct NFmiColorBlendBumpmap
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int intensity = NFmiColorTools::Intensity(srcr, srcg, srcb);
    return NFmiColorTools::SafeColor(
        intensity * NFmiColorTools::GetRed(dst) / NFmiColorTools::MaxIntensity,
        intensity * NFmiColorTools::GetGreen(dst) / NFmiColorTools::MaxIntensity,
        intensity * NFmiColorTools::GetBlue(dst) / NFmiColorTools::MaxIntensity,
        intensity * NFmiColorTools::GetAlpha(dst) / NFmiColorTools::MaxIntensity);
  }
};

// ----------------------------------------------------------------------
// Color DentMap - Bumpmap source by destination intensity
// ----------------------------------------------------------------------

struct NFmiColorBlendDentmap
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int intensity = NFmiColorTools::Intensity(dst);
    return NFmiColorTools::SafeColor(intensity * srcr / NFmiColorTools::MaxIntensity,
                                     intensity * srcg / NFmiColorTools::MaxIntensity,
                                     intensity * srcb / NFmiColorTools::MaxIntensity,
                                     intensity * srca / NFmiColorTools::MaxIntensity);
  }
};

// ----------------------------------------------------------------------
// Color AddContrast - Add contrast to target pixel
// ----------------------------------------------------------------------

struct NFmiColorBlendAddContrast
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::AddContrast(dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::AddContrast(dst);
  }
};

// ----------------------------------------------------------------------
// Color ReduceContrast - Reduce contrast of target pixel
// ----------------------------------------------------------------------

struct NFmiColorBlendReduceConstrast
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReduceContrast(dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    return NFmiColorTools::ReduceContrast(dst);
  }
};

// ----------------------------------------------------------------------
// Color OnOpaque - Draw on opaque parts only
// ----------------------------------------------------------------------

struct NFmiColorBlendOnOpaque
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);

    int dstp = srca + dsta - srca * dsta / NFmiColorTools::MaxAlpha;
    int srcp = NFmiColorTools::MaxAlpha - dstp;

    return NFmiColorTools::SafeColor(
        (srcp * srcr + dstp * NFmiColorTools::GetRed(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcg + dstp * NFmiColorTools::GetGreen(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcb + dstp * NFmiColorTools::GetBlue(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }
};

// ----------------------------------------------------------------------
// Color OnTransparent - Draw on transparent parts only
// ----------------------------------------------------------------------

struct NFmiColorBlendOnTransparent
{
  static inline NFmiColorTools::Color Blend(NFmiColorTools::Color src, NFmiColorTools::Color dst)
  {
    return Blend(NFmiColorTools::GetRed(src),
                 NFmiColorTools::GetGreen(src),
                 NFmiColorTools::GetBlue(src),
                 NFmiColorTools::GetAlpha(src),
                 dst);
  }

  static inline NFmiColorTools::Color Blend(
      int srcr, int srcg, int srcb, int srca, NFmiColorTools::Color dst)
  {
    int dsta = NFmiColorTools::GetAlpha(dst);
    int dstb = NFmiColorTools::MaxAlpha - dsta;

    int dstp = srca + dstb - srca * dstb / NFmiColorTools::MaxAlpha;
    int srcp = NFmiColorTools::MaxAlpha - dstp;

    return NFmiColorTools::SafeColor(
        (srcp * srcr + dstp * NFmiColorTools::GetRed(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcg + dstp * NFmiColorTools::GetGreen(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srcb + dstp * NFmiColorTools::GetBlue(dst)) / NFmiColorTools::MaxAlpha,
        (srcp * srca + dstp * dsta) / NFmiColorTools::MaxAlpha);
  }
};

}  // namespace Imagine


// ----------------------------------------------------------------------
