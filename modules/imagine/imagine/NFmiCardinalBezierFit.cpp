// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace Imagine::NFmiCardinalBezierFit
 */
// ======================================================================
/*!
 * \namespace Imagine::NFmiCardinalBezierFit
 *
 * \brief Calculating a cardinal Bezier curve fit
 *
 */
// ======================================================================

#include "NFmiCardinalBezierFit.h"
#include "NFmiCounter.h"
#include "NFmiBezierTools.h"
#include "NFmiPath.h"

#include <list>

using namespace std;

namespace Imagine
{
namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Estimate control points for vertices.
 *
 * This function calculates cubic Bezier curve control points between
 * vertices (x1,y1) and (x2,y2) given the previous vertex (x0,y0)
 * and the following vertex (x3,y3). The given smoothness factor
 * determines how closely the Bezier curve follows a polyline.
 *
 * The calculated control points are returned into the given
 * variables passed by reference.
 *
 * Special cases:
 *  -# x0==x1 && y0==y1 should work fine, is untested
 *  -# x1==x2 && y2==y3 should work fine, is untested (sharp corner?)
 *  -# x2==x3 && y2==y3 should work fine, is untested
 *
 * \param smooth_value The smoothness factor in range 0-1
 * \param x0 Vertex 0 x-coordinate
 * \param y0 Vertex 0 y-coordinate
 * \param x1 Vertex 1 x-coordinate
 * \param y1 Vertex 1 y-coordinate
 * \param x2 Vertex 2 x-coordinate
 * \param y2 Vertex 2 y-coordinate
 * \param x3 Vertex 3 x-coordinate
 * \param y3 Vertex 3 y-coordinate
 * \param cx1 Control point 1 x-coordinate (returned via reference)
 * \param cy1 Control point 1 y-coordinate (returned via reference)
 * \param cx2 Control point 2 x-coordinate (returned via reference)
 * \param cy2 Control point 2 y-coordinate (returned via reference)
 */
// ----------------------------------------------------------------------

void CubicControlPoints(double smooth_value,
                        double x0,
                        double y0,
                        double x1,
                        double y1,
                        double x2,
                        double y2,
                        double x3,
                        double y3,
                        double& cx1,
                        double& cy1,
                        double& cx2,
                        double& cy2)
{
  // calculate the centers for each of the three edges
  const double xc1 = (x0 + x1) / 2.0;
  const double yc1 = (y0 + y1) / 2.0;
  const double xc2 = (x1 + x2) / 2.0;
  const double yc2 = (y1 + y2) / 2.0;
  const double xc3 = (x2 + x3) / 2.0;
  const double yc3 = (y2 + y3) / 2.0;

  const double len1 = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
  const double len2 = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
  const double len3 = sqrt((x3 - x2) * (x3 - x2) + (y3 - y2) * (y3 - y2));

  const double k1 = len1 / (len1 + len2);
  const double k2 = len2 / (len2 + len3);

  const double xm1 = xc1 + (xc2 - xc1) * k1;
  const double ym1 = yc1 + (yc2 - yc1) * k1;

  const double xm2 = xc2 + (xc3 - xc2) * k2;
  const double ym2 = yc2 + (yc3 - yc2) * k2;

  // Resulting control points. Here smooth_value is mentioned
  // above coefficient K whose value should be in range [0...1].
  cx1 = xm1 + (xc2 - xm1) * smooth_value + x1 - xm1;
  cy1 = ym1 + (yc2 - ym1) * smooth_value + y1 - ym1;

  cx2 = xm2 + (xc2 - xm2) * smooth_value + x2 - xm2;
  cy2 = ym2 + (yc2 - ym2) * smooth_value + y2 - ym2;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate cardinal bezier for regular path segment
 *
 * A regular path segment starts with a moveto, is followed by
 * several lineto commands and may or may not be closed.
 *
 * The algorithm is to generate the Bezier for each line segment in
 * turn. If the curve is closed, we use wrap-around to generate
 * the surrounding vertices for the first and last segments. If the
 * curve is not closed, we use the first and last points as the
 * next points also. This may not be the best possible approach,
 * but it works.
 *
 * \param thePath The path to cardinalize
 * \param theSmoothness The smoothness factor
 * \return The cardinalized path
 */
// ----------------------------------------------------------------------

NFmiPath SimpleFit(const NFmiPath& thePath, double theSmoothness)
{
  using namespace NFmiBezierTools;

  if (thePath.Empty()) return thePath;

  const NFmiPathData& path = thePath.Elements();
  const bool isclosed = IsClosed(thePath);

  NFmiPath outpath;
  outpath.Add(path.front());

  const unsigned int n = path.size();

  double cx1, cy1, cx2, cy2;

  for (unsigned int i = 0; i < n - 1; i++)
  {
    const unsigned int p0 = (i != 0 ? i - 1 : (isclosed ? n - 2 : 0));
    const unsigned int p1 = i;
    const unsigned int p2 = i + 1;
    const unsigned int p3 = (i + 2 < n ? i + 2 : (isclosed ? 1 : n - 1));

    CubicControlPoints(theSmoothness,
                       path[p0].x,
                       path[p0].y,
                       path[p1].x,
                       path[p1].y,
                       path[p2].x,
                       path[p2].y,
                       path[p3].x,
                       path[p3].y,
                       cx1,
                       cy1,
                       cx2,
                       cy2);

    outpath.CubicTo(cx1, cy1);
    outpath.CubicTo(cx2, cy2);
    outpath.CubicTo(path[p2].x, path[p2].y);
  }

  return outpath;
}

}  // namespace anonymous

namespace NFmiCardinalBezierFit
{
// ----------------------------------------------------------------------
/*!
 * \brief Calculate cardinal Bezier approximation
 *
 * All real line segments long enough will be converted
 * to cubic bezier line segments. The conversion is cardinal,
 * that is, the original points are on the output path.
 *
 * The smoothness factor determines how strongly the approximation
 * approaches a line.
 *
 * The algorithm is based on that in the AGG graphics library.
 * The original path is split into parts containing regular
 * line segments and other segments. The regular line segments
 * are then converted, the other segments are preserved as is.
 *
 * \param thePath The path to approximate
 * \param theSmoothness The smoothness in range 0-1
 * \return The converted path
 *
 */
// ----------------------------------------------------------------------

const NFmiPath Fit(const NFmiPath& thePath, double theSmoothness)
{
  using namespace NFmiBezierTools;

  NFmiPath outpath;
  Segments segments = SplitSegments(thePath);
  for (Segments::const_iterator it = segments.begin(); it != segments.end(); ++it)
  {
    if (it->second)
      outpath.Add(SimpleFit(it->first, theSmoothness));
    else
      outpath.Add(it->first);
  }

  return outpath;
}

// ----------------------------------------------------------------------
/*!
 * \brief Calculate multiple Bezier approximations
 *
 * Note that the different paths may have common subsegments
 * which must be fitted identically, otherwise for example
 * contour plots may show gaps.
 *
 * This function works only for flattened paths, conic
 * or cubic elements are not allowed in the input.
 *
 * The algorithm is:
 *  -# Calculate how many times each point occurs, taking
 *     care not to calculate closed subpath joinpoints twice
 *  -# Split each path to subsegments based on where the
 *     count of each endpoint and adjacent points reveals
 *     a join point between another path.
 *  -# Establish unique order for all the subsegments. For
 *     closed subsegments we must establish a unique starting
 *     point.
 *  -# Fit each subsegment separately.
 *  -# Join the subsegments back
 *
 * \param thePaths Vector of paths to fit
 * \param theSmoothness The smoothness
 * \return The fitted paths
 */
// ----------------------------------------------------------------------

const NFmiPaths Fit(const NFmiPaths& thePaths, double theSmoothness)
{
  using namespace NFmiBezierTools;

  // Calculate the points

  NFmiCounter<NFmiPoint> counts = VertexCounts(thePaths);

  NFmiPaths outpaths;
  for (NFmiPaths::const_iterator it = thePaths.begin(); it != thePaths.end(); ++it)
  {
    PathList pathlist = SplitPath(*it, counts);
    NFmiPath outpath;
    for (PathList::const_iterator jt = pathlist.begin(); jt != pathlist.end(); ++jt)
    {
      outpath.Add(Fit(*jt, theSmoothness));
    }
    outpaths.push_back(outpath);
  }

  return thePaths;
}

}  // namespace NFmiCardinalBezierFit

}  // namespace Imagine

// ======================================================================
