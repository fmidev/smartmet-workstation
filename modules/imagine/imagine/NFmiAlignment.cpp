/*!
 * \file NFmiAlignment.cpp
 *
 * \b History:
 *
 * \li 30.09.2001, Mika Heiskanen\par
 * Implemented
 *
 */

#include "NFmiAlignment.h"

using namespace std;

namespace Imagine
{
/*!
 * Convert an alignment name to a alignment enum.
 * Returns kFmiAlignmentMissing for an unrecognized name.
 */

NFmiAlignment AlignmentValue(const string& theName)
{
  if (theName == "Center")
    return kFmiAlignCenter;
  else if (theName == "NorthWest")
    return kFmiAlignNorthWest;
  else if (theName == "North")
    return kFmiAlignNorth;
  else if (theName == "NortEast")
    return kFmiAlignNorthEast;
  else if (theName == "East")
    return kFmiAlignEast;
  else if (theName == "SouthEast")
    return kFmiAlignSouthEast;
  else if (theName == "South")
    return kFmiAlignSouth;
  else if (theName == "SouthWest")
    return kFmiAlignSouthWest;
  else if (theName == "West")
    return kFmiAlignWest;
  else
    return kFmiAlignMissing;
}

/*!
 * Convert an alignment enum to a alignment name.
 * Returns a "Unknown" if the name is unknown.
 */

const string AlignmentName(const NFmiAlignment theAlignment)
{
  switch (theAlignment)
  {
    case kFmiAlignCenter:
      return string("Center");
    case kFmiAlignNorthWest:
      return string("NorthWest");
    case kFmiAlignNorth:
      return string("North");
    case kFmiAlignNorthEast:
      return string("NorthEast");
    case kFmiAlignEast:
      return string("East");
    case kFmiAlignSouthEast:
      return string("SouthEast");
    case kFmiAlignSouth:
      return string("South");
    case kFmiAlignSouthWest:
      return string("SouthWest");
    case kFmiAlignWest:
      return string("West");
    default:
      return string("Unknown");
  }
}

/*!
 * Return the X-position in range 0-1 for the given alignment.
 * The value can then be used in linear interpolation to weight
 * original values as in formula
 * \f$x' = \lambda x_1 + (1-\lambda) x_2\f$
 * where \f$\lambda\f$ is the alignment factor.
 */

double XAlignmentFactor(NFmiAlignment theAlignment)
{
  switch (theAlignment)
  {
    case kFmiAlignNorthEast:
    case kFmiAlignEast:
    case kFmiAlignSouthEast:
      return 1.0;
    case kFmiAlignCenter:
    case kFmiAlignNorth:
    case kFmiAlignSouth:
      return 0.5;
    default:
      return 0.0;
  }
}

/*!
 * Return the Y-position in range 0-1 for the given alignment.
 * The value can then be used in linear interpolation to weight
 * original values as in formula
 * \f$y' = \lambda y_1 + (1-\lambda) y_2\f$
 * where \f$\lambda\f$ is the alignment factor.
 */

double YAlignmentFactor(NFmiAlignment theAlignment)
{
  switch (theAlignment)
  {
    case kFmiAlignSouthWest:
    case kFmiAlignSouthEast:
    case kFmiAlignSouth:
      return 1.0;
    case kFmiAlignEast:
    case kFmiAlignCenter:
    case kFmiAlignWest:
      return 0.5;
    default:
      return 0.0;
  }
}

}  // namespace Imagine

// ======================================================================
