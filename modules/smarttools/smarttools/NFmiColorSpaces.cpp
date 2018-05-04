// NFmiColorSpaces.cpp
//
// NFmiColorSpaces-namespacen ja siinä olevien luokkien toteutus.

#include "NFmiColorSpaces.h"

// NFmiColorSpaces::RGB_color -struct
NFmiColorSpaces::RGB_color::RGB_color(void) : r(0), g(0), b(0)
{
}

NFmiColorSpaces::RGB_color::RGB_color(int r_, int g_, int b_) : r(r_), g(g_), b(b_)
{
}

NFmiColorSpaces::RGB_color::RGB_color(const NFmiColor &theColor)
    : r(static_cast<int>(FmiRound(theColor.GetRed() * 255.f))),
      g(static_cast<int>(FmiRound(theColor.GetGreen() * 255.f))),
      b(static_cast<int>(FmiRound(theColor.GetBlue() * 255.f)))
{
}

// NFmiColorSpaces::HSL_color -struct
NFmiColorSpaces::HSL_color::HSL_color(void) : h(0), s(0), l(0)
{
}

NFmiColorSpaces::HSL_color::HSL_color(int h_, int s_, int l_) : h(h_), s(s_), l(l_)
{
}

// Color conversio funktioita:
// <summary>
// Converts RGB to HSL.
// </summary>
// <param name="red">Red value, must be in [0,255].</param>
// <param name="green">Green value, must be in [0,255].</param>
// <param name="blue">Blue value, must be in [0,255].</param>
NFmiColorSpaces::HSL_color NFmiColorSpaces::RGBtoHSL(int red, int green, int blue)
{
  double h = 0, s = 0, l = 0;
  // normalize red, green, blue values

  double r = (double)red / 255.0;
  double g = (double)green / 255.0;
  double b = (double)blue / 255.0;

  double max1 = FmiMax(r, FmiMax(g, b));
  double min1 = FmiMin(r, FmiMin(g, b));

  // hue
  if (max1 == min1)
  {
    h = 0;  // undefined
  }
  else if (max1 == r && g >= b)
  {
    h = 60.0 * (g - b) / (max1 - min1);
  }
  else if (max1 == r && g < b)
  {
    h = 60.0 * (g - b) / (max1 - min1) + 360.0;
  }
  else if (max1 == g)
  {
    h = 60.0 * (b - r) / (max1 - min1) + 120.0;
  }
  else if (max1 == b)
  {
    h = 60.0 * (r - g) / (max1 - min1) + 240.0;
  }

  // luminance
  l = (max1 + min1) / 2.0;

  // saturation
  if (l == 0 || max1 == min1)
  {
    s = 0;
  }
  else if (0 < l && l <= 0.5)
  {
    s = (max1 - min1) / (max1 + min1);
  }
  else if (l > 0.5)
  {
    s = (max1 - min1) / (2 - (max1 + min1));  //(max1-min1 > 0)?
  }

  return HSL_color(static_cast<int>(FmiRound(h)),
                   static_cast<int>(FmiRound(s * 100)),
                   static_cast<int>(FmiRound(l * 100)));
}

// <summary>
// Converts HSL to RGB.
// </summary>
// <param name="h">Hue, must be in [0, 360].</param>
// <param name="s">Saturation, must be in [0, 1].</param>
// <param name="l">Luminance, must be in [0, 1].</param>
NFmiColorSpaces::RGB_color NFmiColorSpaces::HSLtoRGB(double h, double s, double l)
{
  if (s == 0)
  {
    // achromatic color (gray scale)
    return RGB_color(static_cast<int>(FmiRound(l * 255.0)),
                     static_cast<int>(FmiRound(l * 255.0)),
                     static_cast<int>(FmiRound(l * 255.0)));
  }
  else
  {
    double q = (l < 0.5) ? (l * (1.0 + s)) : (l + s - (l * s));
    double p = (2.0 * l) - q;

    double Hk = h / 360.0;
    double T[3];
    T[0] = Hk + (1.0 / 3.0);  // Tr
    T[1] = Hk;                // Tb
    T[2] = Hk - (1.0 / 3.0);  // Tg

    for (int i = 0; i < 3; i++)
    {
      if (T[i] < 0)
        T[i] += 1.0;
      if (T[i] > 1)
        T[i] -= 1.0;

      if ((T[i] * 6) < 1)
      {
        T[i] = p + ((q - p) * 6.0 * T[i]);
      }
      else if ((T[i] * 2.0) < 1)  //(1.0/6.0)<=T[i] && T[i]<0.5
      {
        T[i] = q;
      }
      else if ((T[i] * 3.0) < 2)  // 0.5<=T[i] && T[i]<(2.0/3.0)
      {
        T[i] = p + (q - p) * ((2.0 / 3.0) - T[i]) * 6.0;
      }
      else
        T[i] = p;
    }

    return RGB_color(static_cast<int>(FmiRound(T[0] * 255.0)),
                     static_cast<int>(FmiRound(T[1] * 255.0)),
                     static_cast<int>(FmiRound(T[2] * 255.0)));
  }
}

// theBrightningFactor on prosentti luku, jolla annettua väriä saadaan vaalennettua tai
// tummennettua.
// jos prosentti luku on > 0, vaalenee väri, jos se on < 0, tummenee väri.
NFmiColor NFmiColorSpaces::GetBrighterColor(const NFmiColor &theColor, double theBrightningFactor)
{
  RGB_color rgbCol(theColor);
  HSL_color hslCol = NFmiColorSpaces::RGBtoHSL(rgbCol.r, rgbCol.g, rgbCol.b);
  double lightness = hslCol.l;
  if (lightness)
    lightness = lightness + lightness * (theBrightningFactor / 100.);
  else  // jos lightness oli 0, pitää vain lisätä kirkastus kerroin
    lightness = theBrightningFactor;

  if (lightness > 100)
    lightness = 100;
  if (lightness < 0)
    lightness = 0;

  RGB_color rgbCol2 = NFmiColorSpaces::HSLtoRGB(hslCol.h, hslCol.s / 100., lightness / 100.);

  return NFmiColor(static_cast<float>(rgbCol2.r) / 255.f,
                   static_cast<float>(rgbCol2.g) / 255.f,
                   static_cast<float>(rgbCol2.b) / 255.f,
                   theColor.Alpha());
}
