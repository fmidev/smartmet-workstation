// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace NFmiAreaTools
 */
// ======================================================================
/*!
 * \namespace NFmiAreaTools
 *
 * \brief Various utility functions to handle NFmiArea objects
 *
 */
// ======================================================================

#include "NFmiAreaTools.h"
#include "NFmiArea.h"
#include <algorithm>

// Local utility functions

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Update bounding box based on given point
 *
 * \param thePoint The point to add to the bounding box
 * \param theMinLon The variable in which to store the minimum longitude
 * \param theMinLat The variable in which to store the minimum latitude
 * \param theMaxLon The variable in which to store the maximum longitude
 * \param theMaxLat The variable in which to store the maximum latitude
 */
// ----------------------------------------------------------------------

void update_bbox(const NFmiPoint& thePoint,
                 double& theMinLon,
                 double& theMinLat,
                 double& theMaxLon,
                 double& theMaxLat)
{
  theMinLon = std::min(theMinLon, thePoint.X());
  theMinLat = std::min(theMinLat, thePoint.Y());
  theMaxLon = std::max(theMaxLon, thePoint.X());
  theMaxLat = std::max(theMaxLat, thePoint.Y());
}

}  // namespace anonymous

namespace NFmiAreaTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Find geographic bounding box for given area
 *
 * The bounding box is found by traversing the edges of the area
 * and converting the coordinates to geographic ones for extrema
 * calculations.
 *
 * \param theArea The area
 * \param theMinLon The variable in which to store the minimum longitude
 * \param theMinLat The variable in which to store the minimum latitude
 * \param theMaxLon The variable in which to store the maximum longitude
 * \param theMaxLat The variable in which to store the maximum latitude
 */
// ----------------------------------------------------------------------

void LatLonBoundingBox(const NFmiArea& theArea,
                       double& theMinLon,
                       double& theMinLat,
                       double& theMaxLon,
                       double& theMaxLat)
{
  // Good initial values are obtained from the corners

  theMinLon = theArea.TopLeftLatLon().X();
  theMinLat = theArea.TopLeftLatLon().Y();
  theMaxLon = theMinLon;
  theMaxLat = theMinLat;

  const unsigned int divisions = 500;

  // Go through the top edge

  const double left = theArea.Left();
  const double right = theArea.Right();
  const double bottom = theArea.Bottom();
  const double top = theArea.Top();
  const double width = right - left;
  const double height = bottom - top;

  // Go through the top edge

  unsigned int i = 0;
  for (i = 0; i <= divisions; i++)
  {
    NFmiPoint xy(left + width * i / divisions, top);
    NFmiPoint latlon(theArea.ToLatLon(xy));
    update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
  }

  // Go through the bottom edge

  for (i = 0; i <= divisions; i++)
  {
    NFmiPoint xy(left + width * i / divisions, bottom);
    NFmiPoint latlon(theArea.ToLatLon(xy));
    update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
  }

  // Go through the left edge

  for (i = 0; i <= divisions; i++)
  {
    NFmiPoint xy(left, top + height * i / divisions);
    NFmiPoint latlon(theArea.ToLatLon(xy));
    update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
  }

  // Go through the right edge

  for (i = 0; i <= divisions; i++)
  {
    NFmiPoint xy(right, top + height * i / divisions);
    NFmiPoint latlon(theArea.ToLatLon(xy));
    update_bbox(latlon, theMinLon, theMinLat, theMaxLon, theMaxLat);
  }
}

}  // namespace NFmiAreaTools
