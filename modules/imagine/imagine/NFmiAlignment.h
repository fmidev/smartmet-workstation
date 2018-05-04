// ======================================================================
/*!
 * \file NFmiAlignment.h
 * \brief Graphical alignment information.
 *
 * This file defines an enumeration of all the 9 possible ways to
 * align something on the corners, edges or center of a rectangular
 * area. Methods are provided to convert between textual descriptions
 * of the placements and the enumerated values, and to obtain linear
 * interpolation coefficients for X- and Y-directions.
 *
 * \b History:
 *
 * \li 30.09.2001, Mika Heiskanen\par
 * Implemented
 *
 */
// ======================================================================

#pragma once

#include <string>

namespace Imagine
{
//! Enumeration of possible alignments.

enum NFmiAlignment
{
  kFmiAlignMissing,
  kFmiAlignCenter,
  kFmiAlignNorthWest,
  kFmiAlignNorth,
  kFmiAlignNorthEast,
  kFmiAlignEast,
  kFmiAlignSouthEast,
  kFmiAlignSouth,
  kFmiAlignSouthWest,
  kFmiAlignWest
};

//! Convert string to enumerated value

NFmiAlignment AlignmentValue(const std::string& theName);

//! Convert enumerated value to string
const std::string AlignmentName(const NFmiAlignment theAlignment);

//! The linear interpolation coefficient for X-direction in range 0-1.
double XAlignmentFactor(NFmiAlignment theAlignment);

//! The linear interpolation coefficient for Y-direction in range 0-1.
double YAlignmentFactor(NFmiAlignment theAlignment);

}  // namespace Imagine


// ======================================================================
