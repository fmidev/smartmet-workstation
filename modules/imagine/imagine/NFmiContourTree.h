// ======================================================================
/*!
 * \file NFmiContourTree.h
 * \brief Definition of a class to calculate 2D contour polygons.
 *
 */
// ======================================================================
/*!
 * \class NFmiContourTree
 *
 * Algorithm assumptions:
 *
 * Assume a closed contour is desired for function value range
 * zmin..zmax. One of zmin/zmax may be defined missing, which
 * implies the desired function value range is half open
 *
 *   -# \f$(zmin,\infty)\f$
 *   -# \f$(-\infty,zmax)\f$
 *
 * The special case when both limits are missing does NOT
 * imply the range \f$(-\infty,\infty)\f$, but the range
 * covered by missing values, including the area outside
 * the area being contoured.
 *
 * Assume the desired 2D area is subdivided into triangles
 * or convex rectangular elements.
 *
 * Assume linear interpolation over the elements is defined
 * so that the value at any midpoint of an edge is precisely
 * the mean of the values at the vertices.
 *
 * Algorithm first step:
 *
 * Each triangle/rectangle is 'filled' according to the values
 * at the vertices. Here filling means outputting a closed
 * path (or closed paths) inside the triangle/rectangle, which
 * cover the desired range exactly. Any edge in the path is marked
 * to be exact, if the values on the edge are precisely either one
 * of the limits of the desired range, and inexact otherwise.
 * Typically the inexact parts are parts of the edges of the
 * triangle/rectangle being examined.
 *
 * The line segments found for the triangle/linesegment are
 * output into a balanced (sorted) binary tree containing at most
 * one copy of each line segment. If a second copy appears (from
 * an adjacent triangle/rectangle), the edge is deleted completely
 * from the tree for being an unnecessary inside edge.
 *
 * The basic data structure required is a "set<edge>", and the
 * basic algorithms needed are find, insert and remove. Note
 * that having a sortable set of elements implies the C++
 * implementation uses a balanced binary tree to implement the set.
 * Currently this probably implies a red-black tree, later on it
 * might imply something else. All that matters for us is that the
 * set is an efficient method to store unique edges, and that
 * it allows us to remove an edge if it is not unique after all.
 *
 * Algorithm second step:
 *
 * The edges in the set are collected into closed polygons
 * by adding one edge at a time into a set of growing polylines.
 *
 * The different cases are:
 *
 *	-# Neither end point of the edge matches any end point of a
 *	   polyline in the set. Thus the edge starts a new polyline.
 *	-# One end point of the edge matches an endpoint of a polyline
 *	   in the set. The edge is appended to the polyline.
 *	-# Both end points match a the end points of a single polyline
 *	   in the set. The polyline is thus closed by the edge.
 * 	-# Both end points match the end points of two different
 *	   polylines in the set. The edge thus appends the two polylines,
 *	   and possibly also closes the polyline.
 *
 * Algorithm result:
 *
 *	A set of closed polygons.
 *
 *
 * Details on subdividing a single triangle/rectangle:
 *
 * A rectangle can be subdivided by dividing each edge into
 * two, and then connecting opposite edges. From the basic
 * assumptions on linear interpolation, the values at the
 * midpoints of the edges are simple means of the values
 * at the vertices, and the value at the center of the rectangle
 * as defined by the intersection point of the lines connecting
 * the opposite edges is the mean of the function values at all
 * 4 vertices.
 *
 * A triangle can be subdivided into 4 subtriangles by connecting
 * the midpoints of all adjacent edges in the triangle. No center
 * point is required, only the centers of the edges are introduced
 * as new vertices.
 *
 * The alternative would be to define the center as a new vertex,
 * and connect it to the corners of the triangle, but on recursion
 * this probably leads to flattened triangles, which may result
 * in longer line segments in one dimension than in the other.
 *
 * Details on 'filling' a single triangle/rectangle:
 *
 *	-#	All vertex values are either completely above
 *		or completely below the desired range. The
 *		triangle/rectangle can be ignored.
 *
 *	-#	All vertex values are completely inside the
 *		desired range. The triangle/rectangle is filled
 *		completely, and each edge is marked as not being
 *		exact.
 *
 *	-#	The remaining case. The triangle/rectangle is to
 *		be filled only partially. Trivial cases can be
 *		handled simply by calculating the intersection
 *		points, and then outputting a single polygon.
 *
 *		As a limiting case, the values at the vertices may
 *		be exactly equalt to the limits of the desired range,
 *		in which case complete edges of the triangle/rectangle
 *		may be output, and marked as exact contours.
 *
 */
// ======================================================================

#pragma once

#include "imagine-config.h"

// Essential includes:

#include "NFmiEdgeTree.h"  // Input edges

#ifndef IMAGINE_WITH_CAIRO
#include "NFmiFillMap.h"  // Fill map generation and rendering
#endif

#include "NFmiDataHints.h"

#include <newbase/NFmiGlobals.h>        // for kFloatMissing
#include <newbase/NFmiGrid.h>           // for grids
#include <newbase/NFmiFastQueryInfo.h>  // for querydata

#include <vector>     // vectors

namespace Imagine
{
/// Class to calculating 2D contour polygons given a data interval.

class NFmiContourTree : public NFmiEdgeTree
{
 public:
  /// Different contouring methods.

  enum NFmiContourInterpolation
  {
    kFmiContourMissingInterpolation,  ///< No interpolation defined
    kFmiContourNearest,               ///< Nearest neighbour interpolation
    kFmiContourLinear,                ///< Linear interpolation
    kFmiContourDiscrete               ///< Discrete interpolation
  };

  /// Convert a name of interpolation method to enumerated type.

  static NFmiContourInterpolation ContourInterpolationValue(const std::string& theName);

  /// Convert interpolation method enum to a name

  static const std::string ContourInterpolationName(NFmiContourInterpolation theInterpolation);

  /// Destructor

  ~NFmiContourTree(void) {}
  /// Constructor.

  /*!
   * The constructor enforces the condition lolimit<hilimit.
   * Equality is not allowed since the resulting contour area
   * would be empty.
   */

  NFmiContourTree(float lolimit,
                  float hilimit,
                  bool loexact = true,
                  bool hiexact = false,
                  bool hasmissing = true,
                  float missing = kFloatMissing)
      : itsLoLimit(lolimit),
        itsHiLimit(hilimit),
        itsLoLimitExact(loexact),
        itsHiLimitExact(hiexact),
        itHasMissingValue(hasmissing),
        itsMissingValue(missing),
        itHasDataLoLimit(false),
        itHasDataHiLimit(false),
        itsDataLoLimit(),
        itsDataHiLimit(),
        itsSubTrianglesOn(true)
  {
  }

  /// Returns the active (read-only) \em low limit of the contoured interval

  float LoLimit(void) const { return itsLoLimit; }
  /// Returns the active (read-only) \em high limit of the contoured interval

  float HiLimit(void) const { return itsHiLimit; }
  /// Returns \c true, if \c itsLoLimit is included in the contoured interval.

  bool LoLimitExact(void) const { return itsLoLimitExact; }
  /// Returns \c true, if \c itsHiLimit is included in the contoured interval.

  bool HiLimitExact(void) const { return itsHiLimitExact; }
  /// Returns \c true, if a \em missing value has been specified.

  bool HasMissingValue(void) const { return itHasMissingValue; }
  /// Returns the missing value used in contouring data.

  float MissingValue(void) const { return itsMissingValue; }
  /// Returns \c true, if the data has a \em low limit for valid data

  bool HasDataLoLimit(void) const { return itHasDataLoLimit; }
  /// Returns \c true, if the data has a \em high limit for valid data

  bool HasDataHiLimit(void) const { return itHasDataHiLimit; }
  /// Returns the \em low limit of valid data, if one has been set.

  float DataLoLimit(void) const { return itsDataLoLimit; }
  /// Returns the \em high limit of valid data, if one has been set.

  float DataHiLimit(void) const { return itsDataHiLimit; }
  /// Sets the value which indicates a \em missing value in contour data.

  void MissingValue(float value)
  {
    itHasMissingValue = true;
    itsMissingValue = value;
  }

  /// Sets the \em low limit for valid data.

  void DataLoLimit(float value)
  {
    itHasDataLoLimit = true;
    itsDataLoLimit = value;
  }

  /// Sets the \em high limit for valid data.

  void DataHiLimit(float value)
  {
    itHasDataHiLimit = true;
    itsDataHiLimit = value;
  }

  /// Sets the subtriangle mode flag
  void SubTriangleMode(bool flag) { itsSubTrianglesOn = flag; }
  // Returns the subtriangle-mode flag
  bool SubTriangleMode() const { return itsSubTrianglesOn; }
  /// Test if a value is valid for contouring
  /*!
   * A value is considered invalid if
   *
   *  -# A missing value has been set, and the value is identical to it.
   *  -# A low limit has been set, and the value is less than the limit.
   *  -# A high limit has been set, and the value is greater than the limit.
   *
   * Otherwise the value is considered valid for contouring.
   */

  bool IsValid(float value)
  {
    if (itHasMissingValue && value == itsMissingValue) return false;
    if (itHasDataLoLimit && value < itsDataLoLimit) return false;
    if (itHasDataHiLimit && value > itsDataHiLimit) return false;
    return true;
  }

  /// Contour a data-matrix using the desired method.

  void Contour(const NFmiDataMatrix<NFmiPoint>& thePts,
               const NFmiDataMatrix<float>& theValues,
               const NFmiContourInterpolation& theInterpolation);

  void Contour(const NFmiDataMatrix<NFmiPoint>& thePts,
               const NFmiDataMatrix<float>& theValues,
               const NFmiDataHints& theHelper,
               const NFmiContourInterpolation& theInterpolation);

  // These assume the coordinates run from 0 to size-1

  void Contour(const NFmiDataMatrix<float>& theValues,
               const NFmiContourInterpolation& theInterpolation);

  void Contour(const NFmiDataMatrix<float>& theValues,
               const NFmiDataHints& theHelper,
               const NFmiContourInterpolation& theInterpolation);

  NFmiPath Path(void) const;

  /// Possible equality conditions of a value with respect to contour limits.

  enum VertexExactness
  {
    kLoLimit,
    kHiLimit,
    kNeither
  };

 private:
  /// Disabled default constructor.

  /*!
   * We always require atleast a contour interval in the constructor.
   */

  NFmiContourTree(void);

  /// Possible locations of a value with respect to contour limits.

  enum VertexInsidedness
  {
    kBelow,
    kInside,
    kAbove
  };

/// A help function definition for vertex insidedness combinations

#define VertexCombo(a, b) (3 * (a) + b)

  /// VertexInsidedness of a value

  /*!
   * Note that whether the value is missing or not is ignored on purpose,
   * it is expected that the validity has been checked before this
   * method is called.
   */

  VertexInsidedness Insidedness(float z) const
  {
    if (z < itsLoLimit && itsLoLimit != kFloatMissing)
      return kBelow;
    else if (z > itsHiLimit && itsHiLimit != kFloatMissing)
      return kAbove;
    else if (z == itsLoLimit && itsLoLimit != kFloatMissing)
      return (itsLoLimitExact ? kInside : kBelow);
    else if (z == itsHiLimit && itsHiLimit != kFloatMissing)
      return (itsHiLimitExact ? kInside : kAbove);
    else
      return kInside;
  }

  /// VertexExactness of a value with respect to contour limits.

  /*!
   * Note that whether the value is missing or not is ignored on purpose,
   * it is expected that the validity has been checked before this
   * method is called.
   */

  VertexExactness Exactness(float z) const
  {
    if (itsLoLimit != kFloatMissing && z == itsLoLimit && itsLoLimitExact)
      return kLoLimit;
    else if (itsHiLimit != kFloatMissing && z == itsHiLimit && itsHiLimitExact)
      return kHiLimit;
    else
      return kNeither;
  }

  //! Test whether we're contouring missing values

  bool ContouringMissing(void) const
  {
    return (LoLimit() == kFloatMissing && HiLimit() == kFloatMissing);
  }

  /// Contour a data-matrix using linear interpolation.

  void ContourLinear(const NFmiDataMatrix<float>& theValues);

  void ContourLinear(const NFmiDataMatrix<float>& theValues, const NFmiDataHints& theHelper);

  void ContourLinear(const NFmiDataMatrix<NFmiPoint>& thePts,
                     const NFmiDataMatrix<float>& theValues);

  void ContourLinear(const NFmiDataMatrix<NFmiPoint>& thePts,
                     const NFmiDataMatrix<float>& theValues,
                     const NFmiDataHints& theHelper);

  /// Contour a data-matrix using nearest neighbour interpolation

  void ContourNearest(const NFmiDataMatrix<float>& theValues);

  void ContourNearest(const NFmiDataMatrix<float>& theValues, const NFmiDataHints& theHelper);

  void ContourNearest(const NFmiDataMatrix<NFmiPoint>& thePts,
                      const NFmiDataMatrix<float>& theValues);

  void ContourNearest(const NFmiDataMatrix<NFmiPoint>& thePts,
                      const NFmiDataMatrix<float>& theValues,
                      const NFmiDataHints& theHelper);

  /// Contour a data-matrix using discrete interpolation

  void ContourDiscrete(const NFmiDataMatrix<float>& theValues);

  void ContourDiscrete(const NFmiDataMatrix<float>& theValues, const NFmiDataHints& theHelper);

  void ContourDiscrete(const NFmiDataMatrix<NFmiPoint>& thePts,
                       const NFmiDataMatrix<float>& theValues);

  void ContourDiscrete(const NFmiDataMatrix<NFmiPoint>& thePts,
                       const NFmiDataMatrix<float>& theValues,
                       const NFmiDataHints& theHelper);

  /// Contour a triangular element using linear interpolation

  void ContourLinear3(
      float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

  // with prechecked values for benefit of ContourLinear4
  void ContourLinear3(float x1,
                      float y1,
                      float z1,
                      VertexInsidedness c1,
                      float x2,
                      float y2,
                      float z2,
                      VertexInsidedness c2,
                      float x3,
                      float y3,
                      float z3,
                      VertexInsidedness c3);

  /// Contour a rectangular element using linear interpolation

  void ContourLinear4(float x1,
                      float y1,
                      float z1,
                      float x2,
                      float y2,
                      float z2,
                      float x3,
                      float y3,
                      float z3,
                      float x4,
                      float y4,
                      float z4);

  /// Contour a triangular element using nearest neighbour interpolation

  void ContourNearest3(
      float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

  /// Contour a rectangular element using nearest neighbour interpolation

  void ContourNearest4(float x1,
                       float y1,
                       float z1,
                       float x2,
                       float y2,
                       float z2,
                       float x3,
                       float y3,
                       float z3,
                       float x4,
                       float y4,
                       float z4);

  /// Contour a triangular element using discrete interpolation

  void ContourDiscrete3(
      float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

  /// Contour a rectangular element using discrete interpolation

  void ContourDiscrete4(float x1,
                        float y1,
                        float z1,
                        float x2,
                        float y2,
                        float z2,
                        float x3,
                        float y3,
                        float z3,
                        float x4,
                        float y4,
                        float z4);

  /// A utility function used by the constructor to check the limits.

  void PrepareLimits(void);

  /// Calculate contour intersections at an edge.

  void IntersectEdge(std::vector<float>& X,
                     std::vector<float>& Y,
                     std::vector<VertexExactness>& B,
                     float x1,
                     float y1,
                     float z1,
                     VertexInsidedness c1,
                     float x2,
                     float y2,
                     float z2,
                     VertexInsidedness c2);

  /// Add polygon formed by given points into the contour polygon.

  void AddEdges(const std::vector<float>& X,
                const std::vector<float>& Y,
                const std::vector<VertexExactness>& B);

  // Data members

  float itsLoLimit;      //!< Low limit of contoured data interval.
  float itsHiLimit;      //!< High limit of contoured data interval.
  bool itsLoLimitExact;  //!< Is low limit included in the interval?
  bool itsHiLimitExact;  //!< Is high limit included in the interval?

  bool itHasMissingValue;  //!< Is there a missing data value?
  float itsMissingValue;   //!< The missing data value.

  bool itHasDataLoLimit;  //!< Is there a low limit to data validity?
  bool itHasDataHiLimit;  //!< Is there a high limit to data validity?
  float itsDataLoLimit;   //!< Low limit to data validity.
  float itsDataHiLimit;   //!< Low limit to data validity.

  bool itsSubTrianglesOn;  //!< True if rectangles subdivide into triangles
};

}  // namespace Imagine

// ----------------------------------------------------------------------

