// ======================================================================
//
// Line generalization functions for objects with X() and Y() methods.
//
// Sample usage:
//
// vector<NFmiPoint> tmp;
// ...
// float tolerance = 0.1;
// NFmiSimplifier simplifier(kFmiDouglasPeucker,tolerance);
// vector<NFmiPoint> result = simplifier.Simplify(tmp);
//
//
// The data is considered a line by default. If the first and last
// coordinates are equal, then the data is considered a polygon.
//
// Straight-line simplification
// ----------------------------
//
// Simplify line segments which can be exactly reduced to a single line.
//
//
// Douglas-Peucker simplification (maxdistance)
// --------------------------------------------
//
// Select end points.
// Find point furthest from the line defined by the end points.
// If the point is further than some tolerance
//     Keep the point, thus subdividing the line into two.
//     Recurse
// Else
//     Ignore the point
// Endif
//
// Visvaningam simplification (mintriangle)
// ----------------------------------------
//
// Each 3 adjacent points form a triangle.
// In each pass delete the triangle with the smallest area.
// When the smallest area reaches the desired tolerance, simplification
// is finished.
//
// Optimization:
//
// If a small triangle is surrounded by two larger triangles, it can be
// deleted immediately. The result is not exactly equivalent in certain
// special cases, but is worth it in speed.
//
// Important points
// ----------------
//
// An optional flag can be set to indicate that important points
// should be found and preserved. An important point is one
// which occurs more often than its neighbours.
//
// The map of important points is to be made with NFmiCounter
// and is passed as the third optional argument to Simplify().
//
// History:
//
// 08.11.2001 Mika Heiskanen
//
//	Implemented
//
// ======================================================================

#pragma once

#include "NFmiCounter.h"

#include <vector>
#include <stack>

namespace Imagine
{
template <class T>
class NFmiSimplifier
{
 public:
  // Different smoothening methods

  enum NFmiSimplifierMethod
  {
    kFmiSimplifierNone,
    kFmiSimplifierStraight,
    kFmiSimplifierMinDistance,
    kFmiSimplifierMaxDistance,
    kFmiSimplifierMinTriangle,
    kFmiSimplifierMaxTriangle
  };

 protected:
  const NFmiSimplifierMethod itsMethod;
  const double itsTolerance;
  const NFmiCounter<T> itsCounter;

 public:
  // Constructors, destructors

  ~NFmiSimplifier(void){};

  NFmiSimplifier(NFmiSimplifierMethod theSimplifier, double theTolerance)
      : itsMethod(theSimplifier), itsTolerance(theTolerance)
  {
  }

  NFmiSimplifier(NFmiSimplifierMethod theSimplifier,
                 double theTolerance,
                 const NFmiCounter<T>& theCounter)
      : itsMethod(theSimplifier), itsTolerance(theTolerance), itsCounter(theCounter)
  {
  }

  // Access to data members

  NFmiSimplifierMethod Method(void) { return itsMethod; }
  double Tolerance(void) const { return itsTolerance; }
  // Actual simplification methods

  const std::vector<T> Simplify(const std::vector<T>& theData) const
  {
    switch (itsMethod)
    {
      case kFmiSimplifierNone:
        return theData;
      case kFmiSimplifierStraight:
        return SimplifyStraight(theData);
      case kFmiSimplifierMinDistance:
        return SimplifyMinDistance(theData);
      case kFmiSimplifierMaxDistance:
        return SimplifyMaxDistance(theData);
      case kFmiSimplifierMinTriangle:
        return SimplifyMinTriangle(theData);
      case kFmiSimplifierMaxTriangle:
        return SimplifyMaxTriangle(theData);
      default:
        return std::vector<T>();
    }
  }

 private:
  // ------------------------------------------------------------
  // Simplify straight line segments into one segment only
  // ------------------------------------------------------------

  const std::vector<T> SimplifyStraight(const std::vector<T>& theData) const { return theData; }
  // ------------------------------------------------------------
  // Simplify so than the smallest deviation point from the
  // line formed by the adjacent points is deleted first.
  // Continues until the smallest deviation is greater than
  // the tolerance.
  // ------------------------------------------------------------

  const std::vector<T> SimplifyMinDistance(const std::vector<T>& theData) const { return theData; }
  // ------------------------------------------------------------
  // Simplify so that the maximum deviation point is kept
  // from the line formed adjancent points kept earlier.
  // This is the Douglas-Peucker algorithm.
  // ------------------------------------------------------------

  const std::vector<T> SimplifyMaxDistance(const std::vector<T>& theData) const
  {
    // The output data

    std::vector<T> out;

    // Special case of no line:

    if (theData.size() <= 1) return out;

    // The first point can be output already - we have atleast two points

    if (!theData.empty()) out.push_back(theData[0]);

    // Stack of vector indices for recursion, initialized with N-1

    std::stack<int> stk;
    stk.push(static_cast<int>(theData.size() - 1));

    // Using squared distances in comparisons is faster

    double epssq = itsTolerance * itsTolerance;

    int i = 0;
    do
    {
      // Find longest distance

      int j = stk.top();
      double dist = -1;

      int split;
      if (i + 1 < j)
      {
        // Precalculated parts of the distance formula

        double xixj = theData[j].X() - theData[i].X();
        double yiyj = theData[j].Y() - theData[i].Y();
        if (xixj != 0 || yiyj != 0)
        {
          // is really a line, not a point

          double dxdy = -yiyj * theData[i].X() + xixj * theData[i].Y();
          for (int k = i + 1; k < j; k++)
          {
            // distance from line
            double tmp = dxdy + theData[k].X() * yiyj - xixj * theData[k].Y();
            if (tmp < 0) tmp = -tmp;
            if (tmp > dist)
            {
              dist = tmp;
              split = k;
            }
          }
          dist *= dist / (xixj * xixj + yiyj * yiyj);
        }
        else
        {
          // is actually a point
          for (int k = i + 1; k < j; k++)
          {
            // distance from point
            double dx = theData[k].X() - theData[i].X();
            double dy = theData[k].Y() - theData[i].Y();
            double tmp = dx * dx + dy * dy;
            if (tmp > dist)
            {
              dist = tmp;
              split = k;
            }
          }
        }
      }

      // If max distance greater than tolerance, push the split
      // point into the stack for output and recursion.
      // Otherwise we can output the first point and start
      // scanning the next segment

      if (dist > epssq)
        stk.push(split);

      else
      {
        out.push_back(theData[stk.top()]);
        i = stk.top();
        stk.pop();
      }
    } while (!stk.empty());

    // As a final check, if we end up having a line shorter than
    // the tolerance, we return an empty path

    if (out.size() < 2)
      out.clear();
    else if (out.size() == 2)
    {
      double dx = out[1].X() - out[0].X();
      double dy = out[1].Y() - out[0].Y();
      if (dx * dx + dy * dy < epssq) out.clear();
    }

    return out;
  }

  // ------------------------------------------------------------
  // Simplify so that the smallest triangle formed by 3
  // adjacent points is deleted first.
  //
  // The area of a general triangle with coordinates p1,p2,p3 is
  //
  // 1/2*abs((x2y1-x1y2)+(x3y2-x2y3)+(x1y3-x3y1))
  // ------------------------------------------------------------

  const std::vector<T> SimplifyMinTriangle(const std::vector<T>& theData) const
  {
    std::vector<T> out;

    out = theData;

    // Special case of no line:

    if (theData.size() <= 1) return out;

    // Go through the data again and again, until nothing is deleted

    bool removed_some = true;
    bool found_small = true;

    while (removed_some && found_small)
    {
      // Output from work to out

      std::vector<T> work = out;
      out.clear();

      out.push_back(work[0]);

      double last_area = -1;
      double x1 = out[0].X();
      double y1 = out[0].Y();

      removed_some = false;
      found_small = false;

      for (int i = 1; i < work.size() - 1; i++)
      {
        // The coordinates now, at the previous and at the next point

        double x1 = work[i - 1].X();
        double y1 = work[i - 1].Y();
        double x2 = work[i].X();
        double y2 = work[i].Y();
        double x3 = work[i + 1].X();
        double y3 = work[i + 1].Y();

        // The area of the relevant triangle

        double area = abs(0.5 * (x2 * y1 - x1 * y2 + x3 * y2 - x2 * y3 + x1 * y3 - x3 * y1));

        // Delete previous triangle, if possible

        if (area < Tolerance()) found_small = true;

        if (last_area > 0 && last_area < area && last_area < Tolerance())
        {
          removed_some = true;
        }
        else
        {
        }
      }
    }

    return out;
  }

  // ------------------------------------------------------------
  // Simplify so that the largest triangle formed by 3 adjacent
  // points is kept first. This is very similar to Douglas-
  // Peucker, but using area as the criteria we might get
  // better shape preservation.
  //
  // The area of a general triangle with coordinates p1,p2,p3 is
  //
  // 1/2*abs((x2y1-x1y2)+(x3y2-x2y3)+(x1y3-x3y1))
  //
  // = 1/2*abs( y2(x3-x1) - x2(y3-y1) + (x1y3-x3y1) )
  //
  // ------------------------------------------------------------

  const std::vector<T> SimplifyMaxTriangle(const std::vector<T>& theData) const
  {
    // The output data

    std::vector<T> out;

    // Special case of no line:

    if (theData.size() <= 1) return out;

    // The first point can be output already - we have atleast two points

    if (!theData.empty()) out.push_back(theData[0]);

    // Stack of vector indices for recursion, initialized with N-1

    std::stack<int> stk;
    stk.push(static_cast<int>(theData.size() - 1));

    int i = 0;
    do
    {
      // Find largest area

      int j = stk.top();
      double area = -1;

      int split;
      if (i + 1 < j)
      {
        // 1/2*abs( y2(x3-x1) - x2(y3-y1) + (x1y3-x3y1) )

        double term3 = theData[i].X() * theData[j].Y() - theData[j].X() * theData[i].Y();

        double yiyj = theData[j].Y() - theData[i].Y();
        double xixj = theData[j].X() - theData[i].X();

        for (int k = i + 1; k < j; k++)
        {
          // The area
          double tmp = 0.5 * (theData[k].Y() * xixj - theData[k].X() * yiyj + term3);
          if (tmp < 0) tmp = -tmp;

          if (tmp > area)
          {
            area = tmp;
            split = k;
          }
        }
      }

      // If max area greater than tolerance, push the split
      // point into the stack for output and recursion.
      // Otherwise we can output the first point and start
      // scanning the next segment

      if (area > Tolerance())
        stk.push(split);

      else
      {
        out.push_back(theData[stk.top()]);
        i = stk.top();
        stk.pop();
      }
    } while (!stk.empty());

    // As a final check, if we end up having a line so short, that
    // having equal altitude would make the respective triangle
    // smaller than the tolerance, we return empty data

    if (out.size() < 2)
      out.clear();
    else if (out.size() == 2)
    {
      double dx = abs(out[1].X() - out[0].X());
      double dy = abs(out[1].Y() - out[0].Y());
      if (0.5 * dx * dy < Tolerance()) out.clear();
    }
    return out;
  }
};

}  // namespace Imagine


// ======================================================================
