// ======================================================================
/*!
 * \file
 * \brief Interface of namespace Imagine::NFmiCardinalBezierFit
 */
// ======================================================================

#pragma once

#include "NFmiBezierTools.h"

namespace Imagine
{
class NFmiPath;

namespace NFmiCardinalBezierFit
{
const NFmiPath Fit(const NFmiPath& thePath, double theSmoothness);

typedef NFmiBezierTools::NFmiPaths NFmiPaths;
const NFmiPaths Fit(const NFmiPaths& thePaths, double theMaxError);

}  // namespace NFmiCardinalBezierFit
}  // namespace Imagine


// ======================================================================
