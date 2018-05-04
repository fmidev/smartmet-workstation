// ======================================================================
/*!
 * \file
 * \brief Interface of namespace Imagine::NFmiBezierTools
 */
// ======================================================================

// Note: This namespace contains tools which are mostly useful
// only to the various Bezier fitting namespaces.

#pragma once

#include <list>
#include <vector>

class NFmiPoint;

namespace Imagine
{
template <class T>
class NFmiCounter;
class NFmiPath;

namespace NFmiBezierTools
{
typedef std::list<std::pair<NFmiPath, bool> > Segments;
typedef std::vector<NFmiPath> NFmiPaths;
typedef std::list<NFmiPath> PathList;

bool IsClosed(const NFmiPath& thePath);

const NFmiCounter<NFmiPoint> VertexCounts(const NFmiPaths& thePaths);

const Segments SplitSegments(const NFmiPath& thePath);

const PathList SplitPath(const NFmiPath& thePath);

const PathList SplitPath(const NFmiPath& thePath, const NFmiCounter<NFmiPoint>& theCounts);

double BezierLength(const NFmiPath& thePath, double theRelativeAccuracy);

}  // namespace NFmiBezierTools
}  // namespace Imagine


// ======================================================================
