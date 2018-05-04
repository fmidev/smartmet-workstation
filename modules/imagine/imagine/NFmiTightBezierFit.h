// ======================================================================
/*!
 * \file
 * \brief Interface of namespace Imagine::NFmiTightBezierFit
 */
// ======================================================================

#pragma once

#include "NFmiBezierTools.h"

namespace Imagine
{
class NFmiPath;

namespace NFmiTightBezierFit
{
const NFmiPath Fit(const NFmiPath& thePath, double theMaxError);

typedef NFmiBezierTools::NFmiPaths NFmiPaths;
const NFmiPaths Fit(const NFmiPaths& thePaths, double theMaxError);

}  // namespace NFmiTightBezierFit
}  // namespace Imagine


// ======================================================================
