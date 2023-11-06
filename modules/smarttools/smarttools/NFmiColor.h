// ======================================================================
/*!
 * \file
 * \brief Interface of class NFmiColor
 */
// ======================================================================

#pragma once

#include "FmiColorTypes.h"

#include <newbase/NFmiGlobals.h>
#include <iostream>

class NFmiColor
{
 public:
  // Destructors
  virtual ~NFmiColor() {}
  // Constructors

  // Default: 100% opacity when aAlphaValue = 0.f
  NFmiColor(float aRedValue = 0.f,
            float aGreenValue = 0.f,
            float aBlueValue = 0.f,
            float aAlphaValue = 0.f);

  NFmiColor(const NFmiColor &aColor);

  // HUOM! tämä tekee rajojen tarkistuksen, siksi en tehnyt suoraa kopiota
  NFmiColor(const FmiRGBColor &aColor);

  // Color handling methods

  void SetRGB(float aRedValue, float aGreenValue, float aBlueValue);
  void SetRGB(const FmiRGBColor &aColor);
  void SetRGB(const NFmiColor &aColor);

  void SetRGBA(float aRedValue, float aGreenValue, float aBlueValue, float aAlphaValue = 0.f);

  void SetRGBA(const FmiRGBColor &aColor);
  void SetRGBA(const NFmiColor &aColor);

  void Red(float theRed) { itsColor.red = theRed; }
  void Green(float theGreen) { itsColor.green = theGreen; }
  void Blue(float theBlue) { itsColor.blue = theBlue; }
  void Alpha(float theAlpha) { itsColor.alpha = theAlpha; }
  float Red() const { return itsColor.red; }
  float Green() const { return itsColor.green; }
  float Blue() const { return itsColor.blue; }
  float Alpha() const { return itsColor.alpha; }
  // Taaksepäin yhteensopivuuden takia TFmiColor:in GetVäri-metodeja
  // toteutetaan tässä samallalailla kuten poistetussa TFmiColor-luokassa.

  float GetRed() const { return Red(); }
  float GetGreen() const { return Green(); }
  float GetBlue() const { return Blue(); }
  const FmiRGBColor &GetRGB() const { return itsColor; }
  const FmiRGBColor &GetRGBA() const { return itsColor; }
  void BlendColor(const NFmiColor &foregroundcolor, float value, float minvalue, float maxvalue);

  void Overlay(const NFmiColor &theForegroundColor);
  unsigned long GetPackedColor() const;

  void Mix(const NFmiColor &anOtherColor, float mixingRatio, bool mixAlpha = false);
  bool IsFullyTransparent() const;
  void InvertAlphaChannel();
  static FmiColorValue ColorChannelLimitCheck(FmiColorValue colorChannelValue);

  // Operators

  NFmiColor &operator=(const NFmiColor &aColor)
  {
    itsColor = aColor.itsColor;
    return *this;
  }

  bool operator==(const NFmiColor &theColor) const
  {
    if (itsColor.red == theColor.Red() && itsColor.green == theColor.Green() &&
        itsColor.blue == theColor.Blue() && itsColor.alpha == theColor.Alpha())
      return true;
    return false;
  }

  bool operator!=(const NFmiColor &theColor) const
  {
    if (itsColor.red != theColor.Red() || itsColor.green != theColor.Green() ||
        itsColor.blue != theColor.Blue() || itsColor.alpha != theColor.Alpha())
      return true;

    return false;
  }

  // NOTE: Using the following operators won't generate
  // any new (other than default) alpha channel value for
  // the color to be returned!

  friend NFmiColor operator+(const NFmiColor &aColor, const NFmiColor &theOtherColor)
  {
    NFmiColor color(aColor.Red() + theOtherColor.Red(),
                    aColor.Green() + theOtherColor.Green(),
                    aColor.Blue() + theOtherColor.Blue());
    return color;
  }

  friend NFmiColor operator-(const NFmiColor &aColor, const NFmiColor &theOtherColor)
  {
    NFmiColor color(aColor.Red() - theOtherColor.Red(),
                    aColor.Green() - theOtherColor.Green(),
                    aColor.Blue() - theOtherColor.Blue());
    return color;
  }

  NFmiColor operator*(const float theValue) const
  {
    NFmiColor color(itsColor.red * theValue, itsColor.green * theValue, itsColor.blue * theValue);
    return color;
  }

  virtual std::ostream &Write(std::ostream &file) const
  {
    file << itsColor.red << " " << itsColor.green << " " << itsColor.blue << " " << itsColor.alpha;
    return file;
  }

  virtual std::istream &Read(std::istream &file)
  {
    file >> itsColor.red >> itsColor.green >> itsColor.blue >> itsColor.alpha;
    return file;
  }

 private:
  FmiRGBColor itsColor;
};

// Taaksepäin yhteensopivuuden takia TFmiColor (poistettu luokka)
// määritellään tässä samaksi kuin NFmiColor.

typedef NFmiColor TFmiColor;

inline void NFmiColor::SetRGBA(const FmiRGBColor &aColor)
{
  itsColor.red = NFmiColor::ColorChannelLimitCheck(aColor.red);
  itsColor.green = NFmiColor::ColorChannelLimitCheck(aColor.green);
  itsColor.blue = NFmiColor::ColorChannelLimitCheck(aColor.blue);
  itsColor.alpha = NFmiColor::ColorChannelLimitCheck(aColor.alpha);
}

inline NFmiColor::NFmiColor(const NFmiColor &aColor) : itsColor(aColor.itsColor) {}

inline NFmiColor::NFmiColor(const FmiRGBColor &aColor) : itsColor()
{
  SetRGBA(aColor);
}

inline std::ostream &operator<<(std::ostream &os, const NFmiColor &ob)
{
  return ob.Write(os);
}

inline std::istream &operator>>(std::istream &os, NFmiColor &ob)
{
  return ob.Read(os);
}

inline void NFmiColor::SetRGBA(float aRedValue,
                               float aGreenValue,
                               float aBlueValue,
                               float aAlphaValue)
{
  itsColor.red = NFmiColor::ColorChannelLimitCheck(aRedValue);
  itsColor.green = NFmiColor::ColorChannelLimitCheck(aGreenValue);
  itsColor.blue = NFmiColor::ColorChannelLimitCheck(aBlueValue);
  itsColor.alpha = NFmiColor::ColorChannelLimitCheck(aAlphaValue);
}

inline void NFmiColor::SetRGBA(const NFmiColor &aColor)
{
  SetRGBA(aColor.Red(), aColor.Green(), aColor.Blue(), aColor.Alpha());
}

inline NFmiColor::NFmiColor(float aRedValue, float aGreenValue, float aBlueValue, float aAlphaValue)
    : itsColor()
{
  SetRGBA(aRedValue, aGreenValue, aBlueValue, aAlphaValue);
}

// The following RGB() methods simply call RGBA() methods with
// alpha value 0.f !

inline void NFmiColor::SetRGB(float aRedValue, float aGreenValue, float aBlueValue)
{
  SetRGBA(aRedValue, aGreenValue, aBlueValue, 0.f);
}

inline void NFmiColor::SetRGB(const FmiRGBColor &aColor)
{
  SetRGBA(aColor);
  Alpha(0.f);
}

inline void NFmiColor::SetRGB(const NFmiColor &aColor)
{
  SetRGBA(aColor);
  Alpha(0.f);
}

// This method is meant to be used for colors with no transparency
inline void NFmiColor::BlendColor(const NFmiColor &foregroundcolor,
                                  float value,
                                  float minvalue,
                                  float maxvalue)
{
  float ratio = 0.f;

  if (value <= minvalue)
    ratio = 0.f;
  else if (value >= maxvalue)
    ratio = 1.f;
  else if ((minvalue < value) && (value < maxvalue))
    ratio = (value - minvalue) / (maxvalue - minvalue);

  NFmiColor backgroundcolor(itsColor.red, itsColor.green, itsColor.blue);

  NFmiColor tempColor(foregroundcolor * ratio + backgroundcolor * (1.f - ratio));

  backgroundcolor.SetRGBA(tempColor);

  itsColor.red = backgroundcolor.Red();
  itsColor.green = backgroundcolor.Green();
  itsColor.blue = backgroundcolor.Blue();
}

inline void NFmiColor::Mix(const NFmiColor &anOtherColor, float mixingRatio, bool mixAlpha)
{
  SetRGB(
      (1.f - mixingRatio) * static_cast<float>(itsColor.red) + mixingRatio * anOtherColor.Red(),
      (1.f - mixingRatio) * static_cast<float>(itsColor.green) + mixingRatio * anOtherColor.Green(),
      (1.f - mixingRatio) * static_cast<float>(itsColor.blue) + mixingRatio * anOtherColor.Blue());
  if (mixAlpha)
    Alpha((1.f - mixingRatio) * static_cast<float>(itsColor.alpha) +
          mixingRatio * anOtherColor.Alpha());
}

inline bool NFmiColor::IsFullyTransparent() const
{
  return itsColor.alpha >= 1.f;
}

// On tapauksia, missä on talletettu värin alpha väärin päin.
// Tällä funktiolla korjataan tuollainen toistaiseksi.
inline void NFmiColor::InvertAlphaChannel()
{
  auto invertedAlpha = 1.f - Alpha();
  Alpha(NFmiColor::ColorChannelLimitCheck(invertedAlpha));
}

inline FmiColorValue NFmiColor::ColorChannelLimitCheck(FmiColorValue colorChannelValue)
{
  return (colorChannelValue >= 0.f) ? ((colorChannelValue <= 1.f) ? colorChannelValue : 1.f) : 0.f;
}

// ======================================================================
