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
  virtual ~NFmiColor(void) {}
  // Constructors

  // Default: 100% opacity is assumed
  NFmiColor(float aRedValue = 0.0,
            float aGreenValue = 0.0,
            float aBlueValue = 0.0,
            float aAlphaValue = 0.0);

  NFmiColor(const NFmiColor &aColor);

  // HUOM! tämä tekee rajojen tarkistuksen, siksi en tehnyt suoraa kopiota
  NFmiColor(const FmiRGBColor &aColor);

  // Color handling methods

  void SetRGB(float aRedValue, float aGreenValue, float aBlueValue);
  void SetRGB(const FmiRGBColor &aColor);
  void SetRGB(const NFmiColor &aColor);

  void SetRGBA(float aRedValue, float aGreenValue, float aBlueValue, float aAlphaValue = 0.0);

  void SetRGBA(const FmiRGBColor &aColor);
  void SetRGBA(const NFmiColor &aColor);

  void Red(float theRed) { itsColor.red = theRed; }
  void Green(float theGreen) { itsColor.green = theGreen; }
  void Blue(float theBlue) { itsColor.blue = theBlue; }
  void Alpha(float theAlpha) { itsColor.alpha = theAlpha; }
  float Red(void) const { return itsColor.red; }
  float Green(void) const { return itsColor.green; }
  float Blue(void) const { return itsColor.blue; }
  float Alpha(void) const { return itsColor.alpha; }
  // Taaksepäin yhteensopivuuden takia TFmiColor:in GetVäri-metodeja
  // toteutetaan tässä samallalailla kuten poistetussa TFmiColor-luokassa.

  float GetRed(void) const { return Red(); }
  float GetGreen(void) const { return Green(); }
  float GetBlue(void) const { return Blue(); }
  const FmiRGBColor &GetRGB(void) const { return itsColor; }
  const FmiRGBColor &GetRGBA(void) const { return itsColor; }
  void BlendColor(const NFmiColor &foregroundcolor, float value, float minvalue, float maxvalue);

  void Overlay(const NFmiColor &theForegroundColor);
  unsigned long GetPackedColor(void) const;

  void Mix(const NFmiColor &anOtherColor, float mixingRatio);

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
  itsColor.red = (aColor.red >= 0.0f) ? ((aColor.red <= 1.0f) ? aColor.red : 1.0f) : 0.0f;
  itsColor.green = (aColor.green >= 0.0f) ? ((aColor.green <= 1.0f) ? aColor.green : 1.0f) : 0.0f;
  itsColor.blue = (aColor.blue >= 0.0f) ? ((aColor.blue <= 1.0f) ? aColor.blue : 1.0f) : 0.0f;
  itsColor.alpha = (aColor.alpha >= 0.0f) ? ((aColor.alpha <= 1.0f) ? aColor.alpha : 1.0f) : 0.0f;
}

inline NFmiColor::NFmiColor(const NFmiColor &aColor) : itsColor(aColor.itsColor)
{
}

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
  itsColor.red = (aRedValue >= 0.0f) ? ((aRedValue <= 1.0f) ? aRedValue : 1.0f) : 0.0f;
  itsColor.green = (aGreenValue >= 0.0f) ? ((aGreenValue <= 1.0f) ? aGreenValue : 1.0f) : 0.0f;
  itsColor.blue = (aBlueValue >= 0.0f) ? ((aBlueValue <= 1.0f) ? aBlueValue : 1.0f) : 0.0f;
  itsColor.alpha = (aAlphaValue >= 0.0f) ? ((aAlphaValue <= 1.0f) ? aAlphaValue : 1.0f) : 0.0f;
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
// alpha value 0.0 !

inline void NFmiColor::SetRGB(float aRedValue, float aGreenValue, float aBlueValue)
{
  SetRGBA(aRedValue, aGreenValue, aBlueValue, 0.0);
}

inline void NFmiColor::SetRGB(const FmiRGBColor &aColor)
{
  SetRGBA(aColor);
  Alpha(0.0);
}

inline void NFmiColor::SetRGB(const NFmiColor &aColor)
{
  SetRGBA(aColor);
  Alpha(0.0);
}

// This method is meant to be used for colors with no transparency
inline void NFmiColor::BlendColor(const NFmiColor &foregroundcolor,
                                  float value,
                                  float minvalue,
                                  float maxvalue)
{
  float ratio = 0.0;

  if (value <= minvalue)
    ratio = 0.0;
  else if (value >= maxvalue)
    ratio = 1.0;
  else if ((minvalue < value) && (value < maxvalue))
    ratio = (value - minvalue) / (maxvalue - minvalue);

  NFmiColor backgroundcolor(itsColor.red, itsColor.green, itsColor.blue);

  NFmiColor tempColor(foregroundcolor * ratio + backgroundcolor * (1.0f - ratio));

  backgroundcolor.SetRGBA(tempColor);

  itsColor.red = backgroundcolor.Red();
  itsColor.green = backgroundcolor.Green();
  itsColor.blue = backgroundcolor.Blue();
}

inline void NFmiColor::Mix(const NFmiColor &anOtherColor, float mixingRatio)
{
  SetRGB(
      (1.F - mixingRatio) * static_cast<float>(itsColor.red) + mixingRatio * anOtherColor.Red(),
      (1.F - mixingRatio) * static_cast<float>(itsColor.green) + mixingRatio * anOtherColor.Green(),
      (1.F - mixingRatio) * static_cast<float>(itsColor.blue) + mixingRatio * anOtherColor.Blue());
}


// ======================================================================
