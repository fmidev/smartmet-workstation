// ======================================================================
//
// Definition of a PostScript style path, with a ghostline extension.
//
// Notes on conic control points
// -----------------------------
//
// This defines a regular conic segment where x2,y2 is the control point
// when moving from x1,y1 to x3,y3
//
//   <M|L|G> x1 y1 <Q> x2 y2 <L|G> x3 y3
//
// The following defines a shorthand conic segment where x2,y2 is the
// control point when moving from (x1+x2)/2,(y1+y2)/2 to x3,y3
// to
//
//   <Q> x1 y1 <Q> x2 y2 <L|G> x3 y3
//
//   <Q> x1 y1 <Q> x2 y2 <Q> x3 y3
//
//
//
// History:
//
// 16.07.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once

#include "imagine-config.h"

// Essential includes

#include "NFmiPathElement.h"

#include "NFmiAlignment.h"
#include "NFmiColorTools.h"
#include "NFmiImage.h"

#ifdef IMAGINE_WITH_CAIRO
#include "ImagineXr.h"
#define CAIRO_NORMAL_LINE_WIDTH (0.4)  // 2.0 is normal Cairo default
#else
#include "NFmiDrawable.h"
#include "NFmiAffine.h"
#endif

#include <newbase/NFmiArea.h>

#include <deque>

#include <iostream>  // << is overloaded

class NFmiGrid;

namespace Imagine
{
class NFmiEsriBox;

typedef std::deque<NFmiPathElement> NFmiPathData;

// ----------------------------------------------------------------------
// A class defining a path
// ----------------------------------------------------------------------

class NFmiPath
#ifndef IMAGINE_WITH_CAIRO
    : public NFmiDrawable
#endif
{
 public:
  virtual ~NFmiPath() {}
  NFmiPath() : itsInsideOut(false), itsElements() {}
  NFmiPath(const NFmiPathData &thePathData, bool theInsideOut = false)
      : itsInsideOut(theInsideOut), itsElements(thePathData)
  {
  }

  // Tulostusoperaattorin ylikuormitus

  friend std::ostream &operator<<(std::ostream &os, const NFmiPath &thePath);

  // Data-access

  const NFmiPathData &Elements() const { return itsElements; }
  int Size() const { return static_cast<int>(itsElements.size()); }
  int Empty() const { return itsElements.empty(); }
  // Clear contents

  void Clear()
  {
    itsElements.clear();
    itsInsideOut = false;
  }

  // Add the given path element

  void Add(const NFmiPathElement &theElement) { itsElements.push_back(theElement); }
  // Add the given path element via its components

  void Add(NFmiPathOperation theOper, double theX, double theY)
  {
    itsElements.push_back(NFmiPathElement(theOper, theX, theY));
  }

  // Move to a new point

  void MoveTo(double theX, double theY)
  {
    itsElements.push_back(NFmiPathElement(kFmiMoveTo, theX, theY));
  }

  // Draw a line from current point to new point

  void LineTo(double theX, double theY)
  {
    itsElements.push_back(NFmiPathElement(kFmiLineTo, theX, theY));
  }

  // Draw an invisible line from current point to new point

  void GhostLineTo(double theX, double theY)
  {
    itsElements.push_back(NFmiPathElement(kFmiGhostLineTo, theX, theY));
  }

  // Draw a line from the given point to the first point

  void InsertLineTo(double theX, double theY)
  {
    itsElements.front().op = kFmiLineTo;
    itsElements.push_front(NFmiPathElement(kFmiMoveTo, theX, theY));
  }

  // Draw a line from the given point to the first point

  void InsertGhostLineTo(double theX, double theY)
  {
    itsElements.front().op = kFmiGhostLineTo;
    itsElements.push_front(NFmiPathElement(kFmiMoveTo, theX, theY));
  }

  // Add a conic control point

  void ConicTo(double theX, double theY)
  {
    itsElements.push_back(NFmiPathElement(kFmiConicTo, theX, theY));
  }

  // Add a cubic control point

  void CubicTo(double theX, double theY)
  {
    itsElements.push_back(NFmiPathElement(kFmiCubicTo, theX, theY));
  }

  // Close the last subpath with a line

  void CloseLineTo() { DoCloseLineTo(kFmiLineTo); }
  // Close the path with an invisible line

  void CloseGhostLineTo() { DoCloseLineTo(kFmiGhostLineTo); }
  // Append a path without a joining line. The first moveto
  // may be omitted if the coordinate is the same as the
  // endpoint of the current path. This is a required feature
  // for some path simplification algorithms.

  void Add(const NFmiPath &thePath)
  {
    if (thePath.Empty()) return;

    NFmiPathData::const_iterator it = thePath.itsElements.begin();
    // strip leading moveto if the coordinate is the same as last end point
    if (!Empty() && (*it).op == kFmiMoveTo && (*it).x == itsElements.back().x &&
        (*it).y == itsElements.back().y)
      ++it;

    itsElements.insert(itsElements.end(), it, thePath.itsElements.end());
  }

  // Append a path using a line of desired type

  void Add(const NFmiPath &thePath, bool fExact);

  // Insert a path using a line of desired type

  void Insert(const NFmiPath &thePath, bool fExact);

  // Append a reversed path using a line of desired type

  void AddReverse(const NFmiPath &thePath, bool fExact);

  // Simplify using Douglas-Peucker algorithm. The input is the
  // maximum allowed error for any line segment.

  void Simplify(double epsilon = 0.0);

  // Simplify long straight line segments

  void SimplifyLines(double offset = 0.0);

  // Return SVG-string description

  std::string SVG(bool relative_moves = false, bool removeghostlines = true) const;

  // Test if the path is Pacific

  bool IsPacificView() const;

  // Make Pacific if so requested

  NFmiPath PacificView(bool pacific) const;

  // Make Atlantic if so requested
  NFmiPath AtlanticView(bool atlantic) const;

// Add the path to a fill map

#ifndef IMAGINE_WITH_CAIRO
  void Add(NFmiFillMap &theMap) const;
#endif
  // Add a string representation of a path to the path

  void Add(const std::string &theString);

  // Rotate the path by the given decimal degrees

  void Rotate(double theAngle);

  // Translate the path by the given amount

  void Translate(double theX, double theY);

  // Scale the path by the given amount

  void Scale(double theScale);

  // Scale the path by the given amounts in x- and y-directions

  void Scale(double theXScale, double theYScale);

// Apply a general affine transformation
#ifndef IMAGINE_WITH_CAIRO
  void Transform(NFmiAffine &theAffine);
#endif
  // Align path to desired corner

  void Align(NFmiAlignment theAlignment, double theX = 0.0, double theY = 0.0);

  // Project
  void Project(const NFmiArea *const theArea);

  void InvProject(const NFmiArea *const theArea);

  void InvGrid(const NFmiGrid *const theGrid);

  // Stroke onto given image using various Porter-Duff rules

  void Stroke(ImagineXr_or_NFmiImage &img,
              double theWidth,
              NFmiColorTools::Color theColor,
              NFmiColorTools::NFmiBlendRule theRule = NFmiColorTools::kFmiColorCopy) const;

#ifdef IMAGINE_WITH_CAIRO
  void Stroke(ImagineXr &img,
              NFmiColorTools::Color theColor,
              NFmiColorTools::NFmiBlendRule theRule = NFmiColorTools::kFmiColorCopy) const
  {
    Stroke(img, CAIRO_NORMAL_LINE_WIDTH, theColor, theRule);
  }
#else
  void Stroke(NFmiImage &img,
              NFmiColorTools::Color theColor,
              NFmiColorTools::NFmiBlendRule theRule = NFmiColorTools::kFmiColorCopy) const;
#endif
  // Return the bounding box

  NFmiEsriBox BoundingBox() const;

  NFmiPath Clip(double theX1, double theY1, double theX2, double theY2, double theMargin = 0) const;
  NFmiPath Clip(
      const NFmiArea *const theArea) const;  // HUOM! toimii vain rajaviivoille oikein, ei alueille

  void InsideOut() { itsInsideOut = !itsInsideOut; }
  bool IsInsideOut() const { return itsInsideOut; }
#ifdef IMAGINE_WITH_CAIRO
  void Fill(ImagineXr &img, NFmiColorTools::Color col, NFmiColorTools::NFmiBlendRule rule) const
  {
    img.Fill(itsElements, col, rule);
  }

  void Fill(ImagineXr &img,
            const ImagineXr &pattern,
            NFmiColorTools::NFmiBlendRule rule,
            double factor) const
  {
    img.Fill(itsElements, pattern, rule, factor);
  }
#endif

  /***
  */
 private:
  // Close the last subpath with an invisible or visible line
  //
  void DoCloseLineTo(NFmiPathOperation theOper);

  // Floating point number to string
  // This is needed because stringstream version of SVG() does not
  // work using g++ v2.95, instead we must format the numbers by
  // ourselves for string concatenation operations
  //
  std::string ftoa(double theValue) const;

  // Simplification subroutines
  //
  void SimplifyTrivial();

  // Data-members
  //
  bool itsInsideOut;
  NFmiPathData itsElements;
};

}  // namespace Imagine


// ----------------------------------------------------------------------
