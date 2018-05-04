#ifndef IMAGINE_WITH_CAIRO

// ======================================================================
//
// Definition of a map of x-coordinates indexed by an y-coordinate.
//
// See NFmiFillMap.h for documentation.
//
// History:
//
// 13.08.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#include "NFmiFillMap.h"
#include "NFmiColorBlend.h"
#include <algorithm>
#include <cmath>

using namespace std;

// A help utility to avoid eps-size errors in filling due to
// for example small rounding errors in projections

namespace
{
float myround(float theValue)
{
  if (abs(theValue - round(theValue)) < 0.001)  // 0.001 pixels is meaningless
    return round(theValue);
  else
    return theValue;
}
}

namespace Imagine
{
// ----------------------------------------------------------------------
// Low level driver for filling when having source RGBA separated
// is faster than having a composite Color
// ----------------------------------------------------------------------

// 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n virheen takia.
// Muutin nimeksi Fill2
// Laiton viel‰ blenderin parametriksi, koska min‰ olen havainnut ongelmia MSVC-k‰‰nt‰j‰n kanssa,
// kun
// tekee seuraavanlaista koodia:
// Function1<MyClass1>();
// Pit‰‰ olla mieluummin:
// Function1(MyClass1());
template <class T>
static void Fill2(T theBlender,
                  NFmiImage &theImage,
                  int red,
                  int green,
                  int blue,
                  int alpha,
                  NFmiFillMapData &theData)
{
  // The iterator for traversing the data is not const, because we
  // sort the x-coordinates

  NFmiFillMapData::iterator iter = theData.begin();

  // Iterate over all y-coordinates in the map

  for (; iter != theData.end(); ++iter)
  {
    // Skip the y-coordinate if it is outside the image dimensions

    float y = iter->first;

    if (y < 0 || y >= theImage.Height()) continue;

    int j = static_cast<int>(y);

    // Otherwise iterate over all the x-coordinates, and fill
    // with the even-odd rule. First we must sort the data.
    // I'm not sure how optimized sort() is for vectors, so
    // I'll handle the special case of 2 elements separately

    if (iter->second.size() == 2)
    {
      if (iter->second[0] > iter->second[1]) swap(iter->second[0], iter->second[1]);
    }
    else
      sort(iter->second.begin(), iter->second.end());

    // We have no active x-coordinate yet

    float x1 = kFloatMissing;

    NFmiFillMapElement::const_iterator dataiter = iter->second.begin();

    for (; dataiter != iter->second.end(); ++dataiter)
    {
      float x2 = *dataiter;

      // If last x was invalid, set new beginning of line

      if (x1 == kFloatMissing) x1 = x2;

      // Otherwise we have a line to fill, from x1 to x2
      // Note that due to the rounding method, we may
      // end up with i1>i2 below. This could result in
      // gaps when rendering very thin figures. To avoid
      // this we must modify either i1 or i2 so that
      // a single pixel is rendered. We modify the one
      // further away from the center of the fill area

      else
      {
        int i1 = static_cast<int>(ceil(x1));
        int i2 = static_cast<int>(floor(x2));

        // If intersection has integer X coordinate, x1 would be
        // interior, x2 exterior

        if (x2 == i2) i2--;

        // Check the line is atleast partially inside

        if (i1 >= theImage.Width() || i2 < 0)
          ;  // x1 is invalidated later
        else
        {
          // Check we fill atleast one pixel

          if (i1 > i2)
          {
            float xmid = (x1 + x2) / 2;
            if (abs(i1 - xmid) < abs(i2 - xmid))
              i2 = i1;
            else
              i1 = i2;
          }

          // Draw only the area inside the image

          i1 = std::max(i1, 0);
          i2 = std::min(i2, theImage.Width() - 1);

          for (int i = i1; i <= i2; ++i)
            //		    theImage(i,j) = T::Blend(red,green,blue,alpha,theImage(i,j));
            theImage(i, j) = theBlender.Blend(
                red, green, blue, alpha, theImage(i, j));  // joudun k‰ytt‰m‰‰n .operaattoria ::
                                                           // osoituksen sijaan MSVC vaatii jostain
                                                           // syyst‰.
        }

        // And invalidate x1
        x1 = kFloatMissing;
      }
    }
  }
}

// ----------------------------------------------------------------------
// Low level driver for filling when having source RGBA separated
// is slower than having a composite Color. These are mostly simple
// blending rules, such as ColorCopy.
// The code is identical to the case above, apart from the ColorBlend
// template specialization.
// ----------------------------------------------------------------------

// 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n virheen takia.
// Muutin nimeksi Fill2
// Laiton viel‰ blenderin parametriksi, koska min‰ olen havainnut ongelmia MSVC-k‰‰nt‰j‰n kanssa,
// kun
// tekee seuraavanlaista koodia:
// Function1<MyClass1>();
// Pit‰‰ olla mieluummin:
// Function1(MyClass1());
template <class T>
static void Fill2(T theBlender,
                  NFmiImage &theImage,
                  NFmiColorTools::Color theColor,
                  NFmiFillMapData &theData)
{
  // The iterator for traversing the data is not const, because we
  // sort the x-coordinates

  NFmiFillMapData::iterator iter = theData.begin();

  // Iterate over all y-coordinates in the map

  for (; iter != theData.end(); ++iter)
  {
    // Skip the y-coordinate if it is outside the image dimensions

    float y = iter->first;

    if (y < 0 || y >= theImage.Height()) continue;

    int j = static_cast<int>(y);

    // Otherwise iterate over all the x-coordinates, and fill
    // with the even-odd rule. First we must sort the data.
    // I'm not sure how optimized sort() is for vectors, so
    // I'll handle the special case of 2 elements separately

    if (iter->second.size() == 2)
    {
      if (iter->second[0] > iter->second[1]) swap(iter->second[0], iter->second[1]);
    }
    else
      sort(iter->second.begin(), iter->second.end());

    // We have no active x-coordinate yet

    float x1 = kFloatMissing;

    NFmiFillMapElement::const_iterator dataiter = iter->second.begin();

    for (; dataiter != iter->second.end(); ++dataiter)
    {
      float x2 = *dataiter;

      // If last x was invalid, set new beginning of line

      if (x1 == kFloatMissing) x1 = x2;

      // Otherwise we have a line to fill, from x1 to x2
      // Note that due to the rounding method, we may
      // end up with i1>i2 below. This could result in
      // gaps when rendering very thin figures. To avoid
      // this we must modify either i1 or i2 so that
      // a single pixel is rendered. We modify the one
      // further away from the center of the fill area

      else
      {
        int i1 = static_cast<int>(ceil(x1));
        int i2 = static_cast<int>(floor(x2));

        // If intersection has integer X coordinate, x1 would be
        // interior, x2 exterior

        if (x2 == i2) i2--;

        // Check the line is atleast partially inside

        if (i1 >= theImage.Width() || i2 < 0)
          ;  // x1 is invalidated later
        else
        {
          // Check we fill atleast one pixel

          if (i1 > i2)
          {
            float xmid = (x1 + x2) / 2;
            if (abs(i1 - xmid) < abs(i2 - xmid))
              i2 = i1;
            else
              i1 = i2;
          }

          // Draw only the area inside the image

          i1 = std::max(i1, 0);
          i2 = std::min(i2, theImage.Width() - 1);

          for (int i = i1; i <= i2; ++i)
            //		    theImage(i,j) = T::Blend(theColor,theImage(i,j));
            theImage(i, j) = theBlender.Blend(theColor, theImage(i, j));  // joudun k‰ytt‰m‰‰n
                                                                          // .operaattoria ::
                                                                          // osoituksen sijaan MSVC
                                                                          // vaatii jostain syyst‰.
        }

        // And invalidate x1

        x1 = kFloatMissing;
      }
    }
  }
}

// ----------------------------------------------------------------------
// Low level driver for pattern filling, with extra alpha factor.
// ----------------------------------------------------------------------

// 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n virheen takia.
// Muutin nimeksi Fill2
// Laiton viel‰ blenderin parametriksi, koska min‰ olen havainnut ongelmia MSVC-k‰‰nt‰j‰n kanssa,
// kun
// tekee seuraavanlaista koodia:
// Function1<MyClass1>();
// Pit‰‰ olla mieluummin:
// Function1(MyClass1());
template <class T>
static void Fill2(T theBlender,
                  NFmiImage &theImage,
                  const NFmiImage &thePattern,
                  float theAlpha,
                  int theX,
                  int theY,
                  NFmiFillMapData &theData)
{
  // The iterator for traversing the data is not const, because we
  // sort the x-coordinates

  NFmiFillMapData::iterator iter = theData.begin();

  // Pattern related variables

  int patw = thePattern.Width();
  int path = thePattern.Height();
  int patj, pati;
  NFmiColorTools::Color patc;

  // Iterate over all y-coordinates in the map

  for (; iter != theData.end(); ++iter)
  {
    // Skip the y-coordinate if it is outside the image dimensions

    float y = iter->first;

    if (y < 0 || y >= theImage.Height()) continue;

    int j = static_cast<int>(y);

    patj = (j + theY) % path;

    // Otherwise iterate over all the x-coordinates, and fill
    // with the even-odd rule. First we must sort the data.
    // I'm not sure how optimized sort() is for vectors, so
    // I'll handle the special case of 2 elements separately

    if (iter->second.size() == 2)
    {
      if (iter->second[0] > iter->second[1]) swap(iter->second[0], iter->second[1]);
    }
    else
      sort(iter->second.begin(), iter->second.end());

    // We have no active x-coordinate yet

    float x1 = kFloatMissing;

    NFmiFillMapElement::const_iterator dataiter = iter->second.begin();

    for (; dataiter != iter->second.end(); ++dataiter)
    {
      float x2 = *dataiter;

      // If last x was invalid, set new beginning of line

      if (x1 == kFloatMissing) x1 = x2;

      // Otherwise we have a line to fill, from x1 to x2
      // Note that due to the rounding method, we may
      // end up with i1>i2 below. This could result in
      // gaps when rendering very thin figures. To avoid
      // this we must modify either i1 or i2 so that
      // a single pixel is rendered. We modify the one
      // further away from the center of the fill area

      else
      {
        int i1 = static_cast<int>(ceil(x1));
        int i2 = static_cast<int>(floor(x2));

        // If intersection has integer X coordinate, x1 would be
        // interior, x2 exterior

        if (x2 == i2) i2--;

        // Check the line is atleast partially inside

        if (i1 >= theImage.Width() || i2 < 0)
          ;  // x1 is invalidated later
        else
        {
          // Check we fill atleast one pixel

          if (i1 > i2)
          {
            float xmid = (x1 + x2) / 2;
            if (abs(i1 - xmid) < abs(i2 - xmid))
              i2 = i1;
            else
              i1 = i2;
          }

          // Draw only the area inside the image

          i1 = std::max(i1, 0);
          i2 = std::min(i2, theImage.Width() - 1);

          for (int i = i1; i <= i2; ++i)
          {
            pati = (i + theX) % patw;
            patc = thePattern(pati, patj);
            if (theAlpha != 1.0)
            {
              int a = NFmiColorTools::GetAlpha(patc);
              int aa = static_cast<int>(a + (1.0 - theAlpha) * (NFmiColorTools::MaxAlpha - a));
              patc = NFmiColorTools::ReplaceAlpha(patc, aa);
            }
            //		      theImage(i,j) = T::Blend(patc,theImage(i,j));
            theImage(i, j) = theBlender.Blend(patc, theImage(i, j));  // joudun k‰ytt‰m‰‰n
                                                                      // .operaattoria :: osoituksen
                                                                      // sijaan MSVC vaatii jostain
                                                                      // syyst‰.
          }
        }

        // And invalidate x1

        x1 = kFloatMissing;
      }
    }
  }
}

// ----------------------------------------------------------------------
// A method to add an edge into the map
// ----------------------------------------------------------------------

void NFmiFillMap::Add(float theX1, float theY1, float theX2, float theY2)
{
  // Ignore invalid coordinates

  if (theX1 == kFloatMissing || theY1 == kFloatMissing || theX2 == kFloatMissing ||
      theY2 == kFloatMissing)
    return;

  // Ignore lines completely outside the area

  if (itsLoLimit != kFloatMissing && std::max(theY1, theY2) < itsLoLimit) return;

  if (itsHiLimit != kFloatMissing && std::min(theY1, theY2) > itsHiLimit) return;

  // The parametric equation of the line is:
  //
  // x = x1 + s*(x2-x1)
  // y = y1 + s*(y2-y1)
  //
  // where s=[0,1]
  //
  // Solving for s from the latter we get
  //
  //          y-y1
  // x = x1 + ----- * (x2-x1) = x1 + k * (y-y1) = (x1 - k*y1) + k*y
  //          y2-y1
  //
  // The equation is always valid, since the case y1=y2 has already been
  // handled.

  float x1 = myround(theX1);
  float y1 = myround(theY1);
  float x2 = myround(theX2);
  float y2 = myround(theY2);

  // First, we ignore horizontal lines, they are meaningless
  // when filling with horizontal lines.

  if (y1 == y2) return;

  if (y1 > y2)
  {
    swap(x1, x2);
    swap(y1, y2);
  }

  int lo = static_cast<int>(ceil(y1));
  int hi = static_cast<int>(floor(y2));

  // If limits are active, we can speed things up by clipping the limits

  if (itsLoLimit != kFloatMissing && lo < itsLoLimit) lo = static_cast<int>(ceil(itsLoLimit));

  if (itsHiLimit != kFloatMissing && hi > itsHiLimit) hi = static_cast<int>(floor(itsHiLimit));

  // We don't want to intersect ymin, it is handled
  // by the line connected to this one, except at
  // the bottom!

  if (y2 <= 0) return;

  if (static_cast<float>(lo) == y1 && y1 > 0) lo++;

  // We precalculate k and x1+k*y1 for speed
  // Should in principle remove the multiplication too,
  // but realistically speaking this cost quite small,
  // and a decent compiler will make fast code anyway.

  float k = (x2 - x1) / (y2 - y1);
  float tmp = x1 - k * y1;

  for (int j = lo; j <= hi; ++j)
    itsData[j].push_back(tmp + k * j);
}

// ----------------------------------------------------------------------
// A method to add a conic segment into the map
// ----------------------------------------------------------------------

void NFmiFillMap::AddConic(
    float theX1, float theY1, float theX2, float theY2, float theX3, float theY3)
{
  // Ignore invalid coordinates

  if (theX1 == kFloatMissing || theY1 == kFloatMissing || theX2 == kFloatMissing ||
      theY2 == kFloatMissing || theX3 == kFloatMissing || theY3 == kFloatMissing)
    return;

  // NOT IMPLEMENTED
}

// ----------------------------------------------------------------------
// A method to add a cubic segment into the map
// ----------------------------------------------------------------------

void NFmiFillMap::AddCubic(float theX1,
                           float theY1,
                           float theX2,
                           float theY2,
                           float theX3,
                           float theY3,
                           float theX4,
                           float theY4)
{
  // Ignore invalid coordinates

  if (theX1 == kFloatMissing || theY1 == kFloatMissing || theX2 == kFloatMissing ||
      theY2 == kFloatMissing || theX3 == kFloatMissing || theY3 == kFloatMissing ||
      theX4 == kFloatMissing || theY4 == kFloatMissing)
    return;

  // NOT IMPLEMENTED
}

// ----------------------------------------------------------------------
// Logical OR with another fillmap
// ----------------------------------------------------------------------

void NFmiFillMap::Or(const NFmiFillMap &theMap)
{
  // Traverse through the map, performing OR with each Y separately

  NFmiFillMapData::const_iterator theiter;  // theMap.itsData iterator
  NFmiFillMapData::iterator iter;           // this.itsData iterator

  for (theiter = theMap.MapData().begin(); theiter != theMap.MapData().end(); ++theiter)
  {
    // Find the Y-coordinate from my own map

    float y = theiter->first;
    iter = itsData.find(y);

    // If the value did not exist, add all of it since this is OR

    if (iter == itsData.end()) itsData[y] = theiter->second;

    // Otherwise we must OR with old x-coordinate data

    else
    {
      NFmiFillMapElement xvec1 = iter->second;
      NFmiFillMapElement xvec2 = theiter->second;
      NFmiFillMapElement xvec;

      sort(xvec1.begin(), xvec1.end());
      sort(xvec2.begin(), xvec2.end());
      unsigned int pos1 = 0;
      unsigned int pos2 = 0;
      float tail = 0.0f;

      while (pos1 <= xvec1.size() - 2 || pos2 <= xvec2.size() - 2)
      {
        if (pos1 >= xvec1.size() || xvec1[pos1] > xvec2[pos2]) swap(xvec1, xvec2);
        if (xvec.size() == 0 || xvec1[pos1] > tail)
        {
          xvec.push_back(xvec1[pos1++]);
          xvec.push_back(xvec1[pos1++]);
        }
        else if (xvec.size() != 0)
        {
          xvec[xvec.size() - 1] = xvec1[pos1 + 1];
          pos1 += 2;
        }
        tail = xvec[xvec.size() - 1];
      }
      iter->second = xvec;
    }
  }
}

// ----------------------------------------------------------------------
// Logical AND with another fillmap
// ----------------------------------------------------------------------

void NFmiFillMap::And(const NFmiFillMap &theMap)
{
  // Iterate through my data, doing AND at every Y

  NFmiFillMapData::const_iterator theiter;  // theMap.itsData iterator
  NFmiFillMapData::iterator iter;           // this.itsData iterator

  for (iter = itsData.begin(); iter != itsData.end();)
  {
    float y = iter->first;
    theiter = theMap.MapData().find(y);

    // If found no match, then must remove the Y

    if (theiter == theMap.MapData().end()) itsData.erase(iter++);  // must be postfix ++ !!!

    // Otherwise must perform AND

    else
    {
      NFmiFillMapElement xvec1 = iter->second;
      NFmiFillMapElement xvec2 = theiter->second;
      iter->second.clear();

      sort(xvec1.begin(), xvec1.end());
      sort(xvec2.begin(), xvec2.end());
      unsigned int pos1 = 0;
      unsigned int pos2 = 0;

      while (pos1 <= xvec1.size() - 2 && pos2 <= xvec2.size() - 2)
      {
        // Make sure xvec1 starts earlier

        if (xvec1[pos1] > xvec2[pos2]) swap(xvec1, xvec2);

        // If xvec1 ends before xvec2 starts, skip forward in xvec1

        if (xvec1[pos1 + 1] <= xvec2[pos2]) pos1 += 2;

        // Otherwise there is a common linesegment

        iter->second.push_back(std::max(xvec1[pos1], xvec2[pos2]));
        iter->second.push_back(std::min(xvec1[pos1 + 1], xvec2[pos2 + 1]));

        // Advance the segment which ends earlier

        if (xvec1[pos1 + 1] <= xvec2[pos2 + 1])
          pos1 += 2;
        else
          pos2 += 2;
      }

      // And advance to the next Y coordinate

      ++iter;
    }
  }
}

// ----------------------------------------------------------------------
// Filling a FillMap onto an image using the desired color and blending
// rule.
//
// Important note:
//
//	Most higher level objects which are filled create a NFmiFillMap
//	object to do the job. Hence this is the most time critical
//	fill method in the library.
//
// ----------------------------------------------------------------------

void NFmiFillMap::Fill(NFmiImage &theImage,
                       NFmiColorTools::Color theColor,
                       NFmiColorTools::NFmiBlendRule theRule)
{
  // Quick exit if color is not real

  if (theColor == NFmiColorTools::NoColor) return;

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
    // Cases for which Color fill is faster:
    case NFmiColorTools::kFmiColorClear:
      Fill2(NFmiColorBlendClear(), theImage, theColor, itsData);
      break;
    case NFmiColorTools::kFmiColorCopy:
      Fill2(NFmiColorBlendCopy(), theImage, theColor, itsData);
      break;
    case NFmiColorTools::kFmiColorAddContrast:
      Fill2(NFmiColorBlendAddContrast(), theImage, theColor, itsData);
      break;
    case NFmiColorTools::kFmiColorReduceContrast:
      Fill2(NFmiColorBlendReduceConstrast(), theImage, theColor, itsData);
      break;

    // CasesNFmiColorTools:: for which RGBA fill is faster:
    case NFmiColorTools::kFmiColorOver:
      Fill2(NFmiColorBlendOver(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorUnder:
      Fill2(NFmiColorBlendUnder(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorIn:
      Fill2(NFmiColorBlendIn(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepIn:
      Fill2(NFmiColorBlendKeepIn(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorOut:
      Fill2(NFmiColorBlendOut(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepOut:
      Fill2(NFmiColorBlendKeepOut(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorAtop:
      Fill2(NFmiColorBlendAtop(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepAtop:
      Fill2(NFmiColorBlendKeepAtop(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorXor:
      Fill2(NFmiColorBlendXor(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorPlus:
      Fill2(NFmiColorBlendPlus(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorMinus:
      Fill2(NFmiColorBlendMinus(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorAdd:
      Fill2(NFmiColorBlendAdd(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorSubstract:
      Fill2(NFmiColorBlendSubstract(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorMultiply:
      Fill2(NFmiColorBlendMultiply(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorDifference:
      Fill2(NFmiColorBlendDifference(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyRed:
      Fill2(NFmiColorBlendCopyRed(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyGreen:
      Fill2(NFmiColorBlendCopyGreen(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyBlue:
      Fill2(NFmiColorBlendCopyBlue(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyMatte:
      Fill2(NFmiColorBlendCopyMatte(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyHue:
      Fill2(NFmiColorBlendCopyHue(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyLightness:
      Fill2(NFmiColorBlendCopyLightness(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorCopySaturation:
      Fill2(NFmiColorBlendCopySaturation(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepMatte:
      Fill2(NFmiColorBlendKeepMatte(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepHue:
      Fill2(NFmiColorBlendKeepHue(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepLightness:
      Fill2(NFmiColorBlendKeepLightness(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepSaturation:
      Fill2(NFmiColorBlendKeepSaturation(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorBumpmap:
      Fill2(NFmiColorBlendBumpmap(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorDentmap:
      Fill2(NFmiColorBlendDentmap(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorOnOpaque:
      Fill2(NFmiColorBlendOnOpaque(), theImage, r, g, b, a, itsData);
      break;
    case NFmiColorTools::kFmiColorOnTransparent:
      Fill2(NFmiColorBlendOnTransparent(), theImage, r, g, b, a, itsData);
      break;

    // Some special cases
    case NFmiColorTools::kFmiColorKeep:
    case NFmiColorTools::kFmiColorRuleMissing:
      break;
  }
}

// ----------------------------------------------------------------------
// Pattern filling a FillMap onto an image using the given blending rule.
// The given blending rule may not necessarily make sense in association
// with pattern filling, but no rule is omitted since somebody may
// come up with unexpected use for the rules.
// ----------------------------------------------------------------------

void NFmiFillMap::Fill(NFmiImage &theImage,
                       const NFmiImage &thePattern,
                       NFmiColorTools::NFmiBlendRule theRule,
                       float theAlpha,
                       int theX,
                       int theY)
{
  switch (theRule)
  {
    case NFmiColorTools::kFmiColorClear:
      Fill2(NFmiColorBlendClear(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorCopy:
      Fill2(NFmiColorBlendCopy(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorAddContrast:
      Fill2(NFmiColorBlendAddContrast(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorReduceContrast:
      Fill2(NFmiColorBlendReduceConstrast(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorOver:
      Fill2(NFmiColorBlendOver(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorUnder:
      Fill2(NFmiColorBlendUnder(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorIn:
      Fill2(NFmiColorBlendIn(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepIn:
      Fill2(NFmiColorBlendKeepIn(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorOut:
      Fill2(NFmiColorBlendOut(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepOut:
      Fill2(NFmiColorBlendKeepOut(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorAtop:
      Fill2(NFmiColorBlendAtop(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepAtop:
      Fill2(NFmiColorBlendKeepAtop(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorXor:
      Fill2(NFmiColorBlendXor(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorPlus:
      Fill2(NFmiColorBlendPlus(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorMinus:
      Fill2(NFmiColorBlendMinus(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorAdd:
      Fill2(NFmiColorBlendAdd(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorSubstract:
      Fill2(NFmiColorBlendSubstract(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorMultiply:
      Fill2(NFmiColorBlendMultiply(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorDifference:
      Fill2(NFmiColorBlendDifference(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyRed:
      Fill2(NFmiColorBlendCopyRed(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyGreen:
      Fill2(NFmiColorBlendCopyGreen(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyBlue:
      Fill2(NFmiColorBlendCopyBlue(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyMatte:
      Fill2(NFmiColorBlendCopyMatte(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyHue:
      Fill2(NFmiColorBlendCopyHue(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorCopyLightness:
      Fill2(NFmiColorBlendCopyLightness(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorCopySaturation:
      Fill2(NFmiColorBlendCopySaturation(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepMatte:
      Fill2(NFmiColorBlendKeepMatte(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepHue:
      Fill2(NFmiColorBlendKeepHue(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepLightness:
      Fill2(NFmiColorBlendKeepLightness(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorKeepSaturation:
      Fill2(NFmiColorBlendKeepSaturation(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorBumpmap:
      Fill2(NFmiColorBlendBumpmap(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorDentmap:
      Fill2(NFmiColorBlendDentmap(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorOnOpaque:
      Fill2(NFmiColorBlendOnOpaque(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;
    case NFmiColorTools::kFmiColorOnTransparent:
      Fill2(NFmiColorBlendOnTransparent(), theImage, thePattern, theAlpha, theX, theY, itsData);
      break;

    // Some special cases
    case NFmiColorTools::kFmiColorKeep:
    case NFmiColorTools::kFmiColorRuleMissing:
      break;
  }
}

}  // namespace Imagine

// ======================================================================

#endif
// IMAGINE_WITH_CAIRO
