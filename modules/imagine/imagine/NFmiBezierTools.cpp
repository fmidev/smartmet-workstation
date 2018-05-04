// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace Imagine::NFmiBezierTools
 */
// ======================================================================
/*!
 * \namespace Imagine::NFmiBezierTools
 *
 * \brief Utility subroutines for various Bezier calculations
 *
 * This namespace is not intended to be used outside the library,
 * instead here are some common utility subroutines for various
 * Bezier fitting functions such as those in
 * Imagine::NFmiCardinalBezierFit and Imagine::NFmiApproximateBezierFit.
 *
 */
// ======================================================================

#include "NFmiBezierTools.h"
#include "NFmiCounter.h"
#include "NFmiPath.h"

#include <newbase/NFmiGeoTools.h>

#include <stdexcept>
#include <utility>
#include <vector>

namespace Imagine
{
namespace NFmiBezierTools
{
namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Utility subroutine for splitting path based on counts
 */
// ----------------------------------------------------------------------

void AppendSplits(PathList& theList, const NFmiPath& thePath, const std::vector<long>& theCounts)
{
  NFmiPath outpath;
  const NFmiPathData& path = thePath.Elements();
  const unsigned long n = path.size();
  for (unsigned long i = 0; i < n; i++)
  {
    outpath.Add(path[i]);

    if (i > 0 && i < n - 1 && (theCounts[i - 1] < theCounts[i] || theCounts[i + 1] < theCounts[i]))
    {
      theList.push_back(outpath);
      outpath.Clear();
    }
    // prepend a leading moveto to all new path segments
    if (i > 0 && outpath.Empty() && path[i].op != Imagine::kFmiMoveTo)
      outpath.MoveTo(path[i].x, path[i].y);
  }

  if (!outpath.Empty()) theList.push_back(outpath);
}

// ----------------------------------------------------------------------
/*!
 * \brief Split a Bezier
 *
 * We assume the path data starts with a moveto and then
 *
 *  - for a size-3 data is followed by 2 conicto commands
 *  - for a size-4 data is followed by 3 cubicto commands
 *
 * All other sizes are error conditions
 *
 * \param thePath The path to split
 * \return Pair of splitted paths
 */
// ----------------------------------------------------------------------

std::pair<NFmiPath, NFmiPath> BezierSplit(const NFmiPath& thePath)
{
  const NFmiPathData& path = thePath.Elements();

  if (path.size() == 3)
  {
    const double x1 = path[0].x;
    const double y1 = path[0].y;
    double cx = path[1].x;
    double cy = path[1].y;
    const double x2 = path[2].x;
    const double y2 = path[2].y;

    const double cx1 = (x1 + cx) / 2.0;
    const double cy1 = (y1 + cy) / 2.0;
    const double cx2 = (x2 + cx) / 2.0;
    const double cy2 = (y2 + cy) / 2.0;
    cx = (cx1 + cx2) / 2.0;
    cy = (cy1 + cy2) / 2.0;

    NFmiPath left;
    left.MoveTo(x1, y1);
    left.ConicTo(cx1, cy1);
    left.ConicTo(cx, cy);

    NFmiPath right;
    right.MoveTo(cx, cy);
    right.ConicTo(cx2, cy2);
    right.ConicTo(x2, y2);

    return std::make_pair(left, right);
  }
  else if (path.size() == 4)
  {
    const double x1 = path[0].x;
    const double y1 = path[0].y;
    double cx1 = path[1].x;
    double cy1 = path[1].y;
    double cx2 = path[2].x;
    double cy2 = path[2].y;
    const double x2 = path[3].x;
    const double y2 = path[3].y;

    double cx = (cx1 + cx2) / 2.0;
    double cy = (cy1 + cy2) / 2.0;
    cx1 = (x1 + cx1) / 2.0;
    cy1 = (y1 + cy1) / 2.0;
    cx2 = (x2 + cx2) / 2.0;
    cy2 = (y2 + cy2) / 2.0;
    const double cx12 = (cx1 + cx) / 2.0;
    const double cy12 = (cy1 + cy) / 2.0;
    const double cx21 = (cx2 + cx) / 2.0;
    const double cy21 = (cy2 + cy) / 2.0;
    cx = (cx12 + cx21) / 2.0;
    cy = (cy12 + cy21) / 2.0;

    NFmiPath left;
    left.MoveTo(x1, y1);
    left.CubicTo(cx1, cy1);
    left.CubicTo(cx12, cy12);
    left.CubicTo(cx, cy);

    NFmiPath right;
    right.MoveTo(cx, cy);
    right.CubicTo(cx21, cy21);
    right.CubicTo(cx2, cy2);
    right.CubicTo(x2, y2);

    return std::make_pair(left, right);
  }
  else
    throw std::runtime_error("BezierSplit does not support arbitrary Bezier degrees");
}

// ----------------------------------------------------------------------
/*!
 * \brief Recursive Bezier length calculation
 */
// ----------------------------------------------------------------------

double BezierLengthRecursion(const NFmiPath& thePath, double theRelativeAccuracy)
{
  const int n = thePath.Size() - 1;
  double arclength = 0;
  for (int i = 0; i < n; i++)
    arclength += NFmiGeoTools::Distance(thePath.Elements()[i].x,
                                        thePath.Elements()[i].y,
                                        thePath.Elements()[i + 1].x,
                                        thePath.Elements()[i + 1].y);

  const double chordlength = NFmiGeoTools::Distance(thePath.Elements().front().x,
                                                    thePath.Elements().front().y,
                                                    thePath.Elements().back().x,
                                                    thePath.Elements().back().y);

  const double len = (2 * chordlength + (n - 1) * arclength) / (n + 1);
  const double err = arclength - chordlength;

  if (len == 0 || err / len <= theRelativeAccuracy)
  {
    return len;
  }
  else
  {
    std::pair<NFmiPath, NFmiPath> parts = BezierSplit(thePath);
    double l1 = BezierLengthRecursion(parts.first, 2 * theRelativeAccuracy);
    double l2 = BezierLengthRecursion(parts.second, 2 * theRelativeAccuracy);
    return l1 + l2;
  }
}

}  // namespace anonymous

// ----------------------------------------------------------------------
/*!
 * \brief Test if the given regular segment is closed
 *
 * A regular segment is a path starting with a moveto and followed
 * by lineto elements only.
 *
 * \param thePath The path to test
 * \return True if the segment is closed
 */
// ----------------------------------------------------------------------

bool IsClosed(const NFmiPath& thePath)
{
  if (thePath.Size() < 2) return false;

  const NFmiPathElement& first = thePath.Elements().front();
  const NFmiPathElement& last = thePath.Elements().back();

  return (first.x == last.x && first.y == last.y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Count the occurrances of each point in paths
 *
 * Throws if any path contains conic or cubic elements.
 *
 * \param thePaths The paths
 * \return The counts
 */
// ----------------------------------------------------------------------

const NFmiCounter<NFmiPoint> VertexCounts(const NFmiPaths& thePaths)
{
  NFmiCounter<NFmiPoint> counts;

  for (NFmiPaths::const_iterator pt = thePaths.begin(); pt != thePaths.end(); ++pt)
  {
    const NFmiPathData& path = pt->Elements();

    // end coordinate of previous moveto
    NFmiPoint previous_moveto(kFloatMissing, kFloatMissing);

    for (NFmiPathData::const_iterator it = path.begin(); it != path.end(); ++it)
    {
      NFmiPoint p(it->x, it->y);
      switch (it->op)
      {
        case kFmiMoveTo:
          counts.Add(p);
          previous_moveto = p;
          break;
        case kFmiLineTo:
        case kFmiGhostLineTo:
          // ignore moves which close a subpath
          if (p != previous_moveto) counts.Add(p);
          break;
        case kFmiCubicTo:
        case kFmiConicTo:
          throw std::runtime_error(
              "Cubic and conic elements not supported in multiple path Bezier fitting");
      }
    }
  }

  return counts;
}

// ----------------------------------------------------------------------
/*!
 * \brief Split the given path into regular line and other segments
 *
 * A regular line segment is considered to be one with
 * a leading moveto command followed by a number of lineto
 * commands, atleast two of them. Zero length lineto commands
 * will be automatically removed to avoid problems during
 * Bezier fitting.
 *
 * \param thePath The path to be splitted
 * \return List of subpaths
 */
// ----------------------------------------------------------------------

const Segments SplitSegments(const NFmiPath& thePath)
{
  Segments out;

  bool isregular = false;
  NFmiPath outpath;

  for (NFmiPathData::const_iterator it = thePath.Elements().begin(); it != thePath.Elements().end();
       ++it)
  {
    // current outpath must be flushed if
    // a) operator is moveto
    // b) type changes from regular to nonregular

    bool flush =
        (!outpath.Empty() && (it->op == kFmiMoveTo || (it->op == kFmiLineTo && !isregular)));

    if (flush)
    {
      // must have sufficiently many lineto cmds to be regular
      if (outpath.Size() <= 3) isregular = false;
      // flush
      out.push_back(std::make_pair(outpath, isregular));
      outpath.Clear();
    }

    // add the element, but remove zero-length line segments
    switch (it->op)
    {
      case kFmiMoveTo:
      case kFmiConicTo:
      case kFmiCubicTo:
        outpath.Add(*it);
        break;
      case kFmiGhostLineTo:
      case kFmiLineTo:
      {
        if (outpath.Empty())
          outpath.Add(*it);
        else if (outpath.Elements().back().x != it->x || outpath.Elements().back().y != it->y)
          outpath.Add(*it);
      }
    }

    // lineto cannot start a new regular segment!
    if (outpath.Size() == 1) isregular = (it->op == kFmiMoveTo);
  }

  if (!outpath.Empty()) out.push_back(std::make_pair(outpath, isregular));

  return out;
}

// ----------------------------------------------------------------------
/*!
 * \brief Split the given path at moveto points
 *
 * The basic idea is that once this split has been done, it is
 * easier to process closed subsegments.
 *
 * \param thePath The path to be splitted
 * \return List of subpaths
 */
// ----------------------------------------------------------------------

const PathList SplitPath(const NFmiPath& thePath)
{
  PathList out;

  NFmiPath outpath;
  for (NFmiPathData::const_iterator it = thePath.Elements().begin(); it != thePath.Elements().end();
       ++it)
  {
    if (it->op == kFmiMoveTo && !outpath.Empty())
    {
      out.push_back(outpath);
      outpath.Clear();
    }
    outpath.Add(*it);
  }

  if (!outpath.Empty()) out.push_back(outpath);

  return out;
}

// ----------------------------------------------------------------------
/*!
 * \brief Split the given regular path into unique use segments
 *
 * A regular line segment is considered to be one with
 * a leading moveto command followed by a number of lineto
 * commands, atleast two of them.
 *
 * The vertex counts indicate how many times a given point
 * is used in some collection of paths. This information
 * can then be used to split the paths into segments
 * which are common to at most two paths at a time.
 *
 *
 * \param thePath The path to be splitted
 * \param theCounts The counts
 * \return List of subpaths
 */
// ----------------------------------------------------------------------

const PathList SplitPath(const NFmiPath& thePath, const NFmiCounter<NFmiPoint>& theCounts)
{
  PathList out;

  // we process all individual connected segments separately
  // to detect closed subsegments more easily

  PathList input = SplitPath(thePath);

  for (PathList::const_iterator it = input.begin(); it != input.end(); ++it)
  {
    const bool isclosed = IsClosed(*it);
    const NFmiPathData& path = it->Elements();

    // establish the counts for each element

    std::vector<long> counts;
    counts.reserve(path.size());

    for (NFmiPathData::const_iterator jt = path.begin(); jt != path.end(); ++jt)
    {
      const NFmiPoint point(jt->x, jt->y);
      const long count = theCounts.Count(point);
      counts.push_back(count);
    }

    // Split the path at points where count is greater than
    // the count at an adjacent point. Note that if the path is closed,
    // we usually do NOT want the path to be split at the closure,
    // but only at the real split points. To accomplish this, we rotate
    // the path so that it starts at the first split point.

    if (!isclosed)
      AppendSplits(out, *it, counts);
    else
    {
      const unsigned long n = path.size();
      unsigned long i;
      for (i = 0; i < n; i++)
      {
        const size_t c0 = (i > 0 ? counts[i - 1] : counts[n - 2]);
        const size_t c1 = counts[i];
        const size_t c2 = (i + 1 < n ? counts[i + 1] : counts[1]);
        if (c0 < c1 || c2 < c1) break;
      }
      // Now if 0<i<n we found a split point to rotate to
      if (i > 0 && i < n)
      {
        NFmiPath tmppath;
        std::vector<long> tmpcounts;
        unsigned long j;
        tmppath.MoveTo(path[i].x, path[i].y);
        tmpcounts.push_back(counts[i]);
        for (j = i + 1; j < n; j++)
        {
          tmppath.Add(path[j]);
          tmpcounts.push_back(counts[j]);
        }
        for (j = 1; j <= i; j++)
        {
          tmppath.Add(path[j]);
          tmpcounts.push_back(counts[j]);
        }
        AppendSplits(out, tmppath, tmpcounts);
      }
      else
        AppendSplits(out, *it, counts);
    }
  }

  return out;
}

// ----------------------------------------------------------------------
/*!
 * \brief Approximate the length of a Bezier segment
 *
 * We assume the path data starts with a moveto and then
 *
 *  - for a size-2 data is followed by a lineto
 *  - for a size-3 data is followed by 2 conicto commands
 *  - for a size-4 data is followed by 3 cubicto commands
 *
 * \param thePath The Bezier curve
 * \param theRelativeAccuracy The desired relative accuracy
 * \return The estimated length of the Bezier segment
 */
// ----------------------------------------------------------------------

double BezierLength(const NFmiPath& thePath, double theRelativeAccuracy)
{
  switch (thePath.Size())
  {
    case 0:
    case 1:
      return 0;
    case 2:
      return NFmiGeoTools::Distance(thePath.Elements().front().x,
                                    thePath.Elements().front().y,
                                    thePath.Elements().back().x,
                                    thePath.Elements().back().y);
    case 3:
    case 4:
    {
      return BezierLengthRecursion(thePath, theRelativeAccuracy);
    }
    default:
      throw std::runtime_error(
          "NFmiBezierTools::BezierLength not implemented for high degree basis");
  }
}

}  // namespace NFmiBezierTools

}  // namespace Imagine
