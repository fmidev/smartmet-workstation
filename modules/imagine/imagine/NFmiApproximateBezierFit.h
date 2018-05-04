// ======================================================================
/*!
 * \file
 * \brief Interface of namespace Imagine::NFmiApproximateBezierFit
 */
// ======================================================================

#pragma once

#include "NFmiBezierTools.h"

namespace Imagine
{
class NFmiPath;

namespace NFmiApproximateBezierFit
{
const NFmiPath Fit(const NFmiPath& thePath, double theMaxError);

typedef NFmiBezierTools::NFmiPaths NFmiPaths;
const NFmiPaths Fit(const NFmiPaths& thePaths, double theMaxError);

}  // namespace NFmiApproximateBezierFit
}  // namespace Imagine


// ======================================================================
