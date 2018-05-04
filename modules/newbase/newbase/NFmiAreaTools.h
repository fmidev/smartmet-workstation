// ======================================================================
/*!
 * \file
 * \brief Interface of namespace NFmiAreaTools
 */
// ======================================================================

#pragma once

class NFmiArea;

namespace NFmiAreaTools
{
void LatLonBoundingBox(const NFmiArea& theArea,
                       double& theMinLon,
                       double& theMinLat,
                       double& theMaxLon,
                       double& theMaxLat);
}

// ======================================================================
