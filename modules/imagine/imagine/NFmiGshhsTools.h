// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiGshhsTools
 */
// ======================================================================
/*!
 * \namespace NFmiGshhsTools
 *
 * \brief Various functions for manipulating GSHHS files
 *
 * GSHHS files are global shoreline databases used for example
 * in the Generic Mapping Tools (GMT).
 *
 * This namespace provides tools mainly for reading GSHHS files.
 * The highest accuracy file is so large that the full file
 * is never read entirely, only the desired portions.
 *
 * Note that the reader only understands the binary gshhs*.b files,
 * it does not understand the newer *.cdf files.
 *
 */
// ======================================================================

#pragma once

#include <string>

namespace Imagine
{
class NFmiPath;

namespace NFmiGshhsTools
{
const NFmiPath ReadPath(const std::string& theFilename,
                        double theMinLongitude,
                        double theMinLatitude,
                        double theMaxLongitude,
                        double theMaxLatitude,
                        double theMinArea = -1);

}  // namespace NFmiGshhsTools

}  // namespace Imagine


// ======================================================================
