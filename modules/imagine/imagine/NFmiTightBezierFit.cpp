// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace Imagine::NFmiTightBezierFit
 */
// ======================================================================
/*!
 * \namespace Imagine::NFmiTightBezierFit
 *
 * \brief Calculating a tight approximate bezier fit
 *
 * The idea is to use the approximate Bezier fit as the main fitting
 * routine. However, we preprocess the path to be fitted to consist
 * of short line segments only. Hence the resulting approximate fit
 * must follow the polyline closely.
 */
// ======================================================================

#include "NFmiTightBezierFit.h"
#include "NFmiApproximateBezierFit.h"
#include "NFmiBezierTools.h"
#include "NFmiCounter.h"
#include "NFmiPath.h"

#include <newbase/NFmiGeoTools.h>

#include <list>
#include <vector>

using namespace std;

namespace Imagine
{
namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Subdivide a line segment to end of path
 *
 * \param thePath The path to append to
 * \param theOper The line operator to output
 * \param theX1 The line start position
 * \param theY1 The line start position
 * \param theX2 The line end position
 * \param theY2 The line end position
 * \param theMaxLength The maximum allowed line length
 */
// ----------------------------------------------------------------------

void SubdivideLine(NFmiPath& thePath,
                   NFmiPathOperation theOper,
                   double theX1,
                   double theY1,
                   double theX2,
                   double theY2,
                   double theMaxLength)
{
  const double len = NFmiGeoTools::Distance(theX1, theY1, theX2, theY2);
  if (len <= theMaxLength || theMaxLength <= 0)
  {
    thePath.Add(theOper, theX2, theY2);
  }
  else
  {
    const double midX = (theX1 + theX2) / 2;
    const double midY = (theY1 + theY2) / 2;
    SubdivideLine(thePath, theOper, theX1, theY1, midX, midY, theMaxLength);
    SubdivideLine(thePath, theOper, midX, midY, theX2, theY2, theMaxLength);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Subdivide any too long line segment in the path
 *
 * \param thePath The path to be processed
 * \param theMaxLength The maximum allowed line length
 * \return The processed path
 */
// ----------------------------------------------------------------------

NFmiPath SubdividePath(const NFmiPath& thePath, double theMaxLength)
{
  // safety check
  if (theMaxLength <= 0) return thePath;

  NFmiPath out;

  double lastx = 0;
  double lasty = 0;
  for (NFmiPathData::const_iterator it = thePath.Elements().begin(); it != thePath.Elements().end();
       ++it)
  {
    switch (it->op)
    {
      case kFmiMoveTo:
      case kFmiConicTo:
      case kFmiCubicTo:
        out.Add(*it);
        break;
      case kFmiGhostLineTo:
      case kFmiLineTo:
        SubdivideLine(out, it->op, lastx, lasty, it->x, it->y, theMaxLength);
        break;
    }
    lastx = it->x;
    lasty = it->y;
  }

  return out;
}
}

namespace NFmiTightBezierFit
{
// ----------------------------------------------------------------------
/*!
 * \brief Calculate a Bezier approximation
 *
 * The given tolerance determines how closely the approximation
 * must follow the original points. The larger the tolerance,
 * the smoother the approximation is likely to be (with less
 * control points in the path).
 *
 * \param thePath The path to approximate
 * \param theMaxError The maximum Euclidian distance error
 * \return The converted path
 */
// ----------------------------------------------------------------------

const NFmiPath Fit(const NFmiPath& thePath, double theMaxError)
{
  NFmiPath subpath = SubdividePath(thePath, theMaxError);
  return NFmiApproximateBezierFit::Fit(subpath, theMaxError);
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
 * \param theMaxError The maximum allowed error
 * \return The fitted paths
 */
// ----------------------------------------------------------------------

const NFmiPaths Fit(const NFmiPaths& thePaths, double theMaxError)
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
      NFmiPath fitpath = Fit(*jt, theMaxError);
      outpath.Add(fitpath);
    }
    outpaths.push_back(outpath);
  }

  return outpaths;
}

}  // namespace NFmiTightBezierFit

}  // namespace Imagine

// ======================================================================
