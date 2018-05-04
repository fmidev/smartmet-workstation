// ======================================================================
//
// Definition of a map of x-coordinates indexed by an y-coordinate.
// The container is used for holding the points where a collection
// of edges intersects integral Y-coordinates. Typical use involves
// intersecting all edges of a contour, and then filling the contour
// based on the x- and y-coordinates of the this container.
//
// This class is intended to be used only by NFmiImage Fill methods
// as a temporary data-holder.
//
// Typical usage:
//
//	NFmiImage img(400,400);
//	int color = img.Color(255,0,0);	// red
//	NFmiFillMap theMap(theTree);	// build from NFmiContourTree
//	img.Fill(theMap,color);		// fill image based on the map
//
// The map needed by Fill can also be built from a NFmiPath, or just
// common vectors of floats (or a vector of a pair of floats).
//
// For speed reasons the constructor also accepts as input
// limiting values for the Y coordinates, outside which
// nothing should be saved. The default values of the limits
// is kFloatMissing, which implies no limit.
//
// This is meaningful when we are rendering only a small part
// of the polygon, for example when zooming into the data.
//
// History:
//
// 13.08.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once

#include "imagine-config.h"

#ifdef IMAGINE_WITH_CAIRO
#error "Either Cairo or us"
#endif

#ifndef UNIX
#pragma warning(disable : 4786)  // poistaa n kpl VC++ k‰‰nt‰j‰n varoitusta
#endif

#include <map>        // maps
#include <vector>     // vectors

#include "NFmiImage.h"

namespace Imagine
{
// Typedefs to ease things

typedef std::vector<float> NFmiFillMapElement;
typedef std::map<float, NFmiFillMapElement> NFmiFillMapData;

class _FMI_DLL NFmiFillMap : public NFmiDrawable
{
 public:
  // Constructors, destructors:

  NFmiFillMap(float theLoLimit = kFloatMissing, float theHiLimit = kFloatMissing)
      : itsData(), itsLoLimit(theLoLimit), itsHiLimit(theHiLimit)
  {
  }

  virtual ~NFmiFillMap(void){};

  // Data access

  const NFmiFillMapData& MapData(void) const { return itsData; }
  // Adding a line, conic or cubic segment

  using NFmiDrawable::Add;

  void Add(float theX1, float theY1, float theX2, float theY2);

  void AddConic(float theX1, float theY1, float theX2, float theY2, float theX3, float theY3);

  void AddCubic(float theX1,
                float theY1,
                float theX2,
                float theY2,
                float theX3,
                float theY3,
                float theX4,
                float theY4);

  // Logical operations with another fillmap

  void Or(const NFmiFillMap& theMap);   // union
  void And(const NFmiFillMap& theMap);  // intersection
  // void Xor(const NFmiFillMap & theMap);		// exclusive or
  // void Substract(const NFmiFillMap & theMap);	// difference

  // Filling

  using NFmiDrawable::Fill;

  void Fill(NFmiImage& theImage,
            NFmiColorTools::Color theColor,
            NFmiColorTools::NFmiBlendRule theRule);

  void Fill(NFmiImage& theImage,
            const NFmiImage& thePattern,
            NFmiColorTools::NFmiBlendRule theRule,
            float theAlpha = 1.0,
            int theX = 0,
            int theY = 0);

 private:
  // Fast low-level specializations for each blending rule:

  //  template <class T> // 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n
  //  virheen takia.
  //  void Fill(NFmiImage & theImage,NFmiColorTools::Color theColor);

  //  template <class T> // 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n
  //  virheen takia.
  //  void Fill(NFmiImage & theImage,int r, int g, int b, int a);

  // Fast low-level pattern filling for each blending rule

  //  template <class T> // 2.1.2002/Marko Muutin static-funktioksi cpp-tiedostoon MSVC-k‰‰nt‰j‰n
  //  virheen takia.
  //  void Fill(NFmiImage & theImage,
  //	    const NFmiImage & thePattern,
  //	    float theAlpha, int theX, int theY);

  // Data-elements

  NFmiFillMapData itsData;
  float itsLoLimit;
  float itsHiLimit;
};

}  // namespace Imagine


// ======================================================================
