// ======================================================================
/*!
 * \file NFmiStereographicArea.cpp
 * \brief Implementation of class NFmiStereographicArea
 */
// ======================================================================
/*!
 * \class NFmiStereographicArea
 *
 * Stereographic projection. Maps geodetic coordinates (in degrees) to rectangular
 * stereographic xy coordinates (in meters) and vice versa.
 *
 *	Projection is based on refs
 *
 *    -# Peter Richardus, Ron K.Adler: "Map Projections", North-Holland, 2nd printing 1974.
 *    -# Navigation Facility Reference Manual rev. 2.0 3/93
 *    -# "An Album of Map Projections" USGS Paper 1453
 *
 * \see NFmiGnomonicArea and NFmiEquidistArea
 *
 *
 * How to convert the geodetic coordinates into rectangular LOCAL polar stereographic coordinates ?
 *
 * \code
 *	CRect rekti;
 *	GetClientRect(rekti);
 *
 *	double lon0,lat0,lon1,lat1,centerLon,centerLat,trueLat;
 *
 *	lon0 = ...;
 *	lat0 = ...;
 *
 *	lon1 = ...;
 *	lat1 = ...;
 *
 * // Central longitude for Helsinki.
 *	centerLon = 25.0;
 *
 * // Central latitude for POLAR stereographic projection.
 *	centerLat = 90.0;
 *
 * // True latitude for Helsinki
 *	trueLat = 60.0;
 *
 *
 *	NFmiPoint bottomLeftLatLon2(lon0,lat0);
 *	NFmiPoint topRightLatLon2(lon1,lat1);
 *
 *	NFmistereographicArea stereo(bottomLeftLatLon2,
 *		                         topRightLatLon2,
 *								 centerLon,
 *							     NFmiPoint(0.,0.),
 *								 NFmiPoint(rekti.right,rekti.bottom),
 *								 centralLat,
 *								 trueLat);
 *
 *	NFmiPoint stereoXY =  stereo.ToXY(bottomLeftLatLon2);
 * \endcode
 *
 * How to convert the rectangular LOCAL stereographic coordinates back into geodetic coordinates ?
 *
 * \code
 * NFmiPoint stereoLatLon = stereo.ToLatLon(stereoXY);
 * \endcode
 */
// ======================================================================

#include "NFmiStereographicArea.h"
#include <cmath>
#include <iomanip>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea() = default;
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theStereographicArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea(const NFmiStereographicArea &theStereographicArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theCentralLongitude Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCenterLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea(const NFmiPoint &theBottomLeftLatLon,
                                             const NFmiPoint &theTopRightLatLon,
                                             const double theCentralLongitude,
                                             const NFmiPoint &theTopLeftXY,
                                             const NFmiPoint &theBottomRightXY,
                                             const double theCenterLatitude,
                                             const double theTrueLatitude,
                                             bool usePacificView)
    : NFmiAzimuthalArea(theBottomLeftLatLon,
                        theTopRightLatLon,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCenterLatitude,
                        theTrueLatitude,
                        usePacificView)

{
  Init();
}

/*!
* Constructor
*
* \param theCenterLatLon Undocumented
* \param theRadialRangeInMeters Undocumented
* \param theTopLeftXY Undocumented
* \param theBottomRightXY Undocumented
*/
NFmiStereographicArea::NFmiStereographicArea(double theRadialRangeInMeters,
                                             const NFmiPoint &theCenterLatLon,
                                             const NFmiPoint &theTopLeftXY,
                                             const NFmiPoint &theBottomRightXY)
    : NFmiAzimuthalArea(theRadialRangeInMeters, theCenterLatLon, theTopLeftXY, theBottomRightXY)
{
  // Muodostaa projektioalueen rajaamalla "world-xy"-tasossa 'theRadialRangeInMeters'-säteiselle
  // ympyrälle
  // "bounding-boxin", jonka keskipisteenä on maantiet. piste 'theCenterLatLon'

  itsTrueLatScaleFactor =
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);
  NFmiPoint centerWorldXY = LatLonToWorldXY(NFmiPoint(theCenterLatLon.X(), theCenterLatLon.Y()));
  itsBottomLeftWorldXY = NFmiPoint(centerWorldXY.X() - theRadialRangeInMeters,
                                   centerWorldXY.Y() - theRadialRangeInMeters);
  itsWorldRect = NFmiRect(
      itsBottomLeftWorldXY,
      itsBottomLeftWorldXY + NFmiPoint(2 * theRadialRangeInMeters, 2 * theRadialRangeInMeters));

  Init(true);
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theWidthInMeters Undocumented
 * \param theHeightInMeters Undocumented
 * \param theCentralLongitude Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCentralLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea(const NFmiPoint &theBottomLeftLatLon,
                                             const double theWidthInMeters,
                                             const double theHeightInMeters,
                                             const double theCentralLongitude,
                                             const NFmiPoint &theTopLeftXY,
                                             const NFmiPoint &theBottomRightXY,
                                             const double theCentralLatitude,
                                             const double theTrueLatitude)
    : NFmiAzimuthalArea(theBottomLeftLatLon,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatitude,
                        theTrueLatitude)
{
  itsTrueLatScaleFactor =
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2001/EL

  itsBottomLeftWorldXY = LatLonToWorldXY(theBottomLeftLatLon);
  itsWorldRect = NFmiRect(itsBottomLeftWorldXY,
                          itsBottomLeftWorldXY + NFmiPoint(theWidthInMeters, theHeightInMeters));

  Init(true);
  // 28.8.2001/Marko&Esa itsWorldRect on laskettu sellaisilla argumenteilla tässä,
  // mitkä eivät ole dataosia, joten sitä ei saa laskea Init:issä uudestaan
}

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theRadialRange Undocumented
 * \param theCentralLongitude Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 * \param theCentralLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiStereographicArea::NFmiStereographicArea(const double theRadialRange,
                                             const double theCentralLongitude,
                                             const NFmiPoint &theTopLeftXY,
                                             const NFmiPoint &theBottomRightXY,
                                             const double theCentralLatitude,
                                             const double theTrueLatitude)
    : NFmiAzimuthalArea(theRadialRange,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatitude,
                        theTrueLatitude)
{
  // Purpose: to create a square bounding the circle of radius theRadialRange

  itsTrueLatScaleFactor =
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2001/EL

  itsWorldRect = NFmiRect(NFmiPoint(-itsRadialRange, -itsRadialRange),
                          NFmiPoint(itsRadialRange, itsRadialRange));
  NFmiAzimuthalArea::Init(true);

  // 28.8.2001/Marko&Esa itsWorldRect on laskettu sellaisilla argumenteilla
  // tässä, mitkä eivät ole dataosia, joten sitä ei saa laskea Init:issä uudestaan
}

// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiStereographicArea::Init(bool fKeepWorldRect)
{
  if (!fKeepWorldRect)
  {
    itsTrueLatScaleFactor =
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2001/EL

    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));
  }
  NFmiAzimuthalArea::Init();
}

// ----------------------------------------------------------------------
/*!
 * \param delta Undocumented
 * \result Undocumented
 */
// ----------------------------------------------------------------------

double NFmiStereographicArea::K(const double delta) const
{
  double D;

  D = DistanceFromPerspectivePointToCenterOfEarth();
  if ((D + (kRearth * delta)) != 0)
    return kRearth * (D + kRearth) / (D + (kRearth * delta));
  else
    return kFloatMissing;
}

// ----------------------------------------------------------------------
/*!
 * \param xyDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiStereographicArea::CalcDelta(const double xyDistance) const
{
  // Calculates the delta angle for stereographic projection.
  // See details in ref. [2] p. 13.

  return 2.0 * atan(xyDistance / (2.0 * kRearth));
  // 11.5.98/EL: delta is always computed for tangential plane only --> itsTrueLatitude.Sin() == 1
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiStereographicArea::DistanceFromPerspectivePointToCenterOfEarth() const
{
  // Distance (in world-coordinate meters) for stereographic projection.
  // See details in ref. [2] p. 13.

  return kRearth;
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiStereographicArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                         const NFmiPoint &theTopRightLatLon,
                                         bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fixedPointData =
        NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiStereographicArea(fixedPointData.itsBottomLeftLatlon,
                                     fixedPointData.itsTopRightLatlon,
                                     itsCentralLongitude,
                                     TopLeft(),
                                     BottomRight(),
                                     itsCentralLatitude.Value(),
                                     itsTrueLatitude.Value(),
                                     fixedPointData.fIsPacific);
  }
  else
    return new NFmiStereographicArea(theBottomLeftLatLon,
                                     theTopRightLatLon,
                                     itsCentralLongitude,
                                     TopLeft(),
                                     BottomRight(),
                                     itsCentralLatitude.Value(),
                                     itsTrueLatitude.Value(),
                                     PacificView());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiStereographicArea::Clone() const { return new NFmiStereographicArea(*this); }
// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theArea The other object being copied
 * \return The object assigned to
 * \todo Since NFmiAzimuthalArea::operator= is unknown, must protect
 *       from self assignment
 */
// ----------------------------------------------------------------------

NFmiStereographicArea &NFmiStereographicArea::operator=(const NFmiStereographicArea &theArea) =
    default;

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiStereographicArea::operator==(const NFmiStereographicArea &theArea) const
{
  return NFmiAzimuthalArea::operator==(static_cast<const NFmiAzimuthalArea &>(theArea));
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiStereographicArea::operator!=(const NFmiStereographicArea &theArea) const
{
  return !(*this == theArea);
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiStereographicArea::operator==(const NFmiArea &theArea) const
{
  return NFmiAzimuthalArea::operator==(theArea);
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The other object being compared to
 * \return True, if the objects are not equal
 */
// ----------------------------------------------------------------------

bool NFmiStereographicArea::operator!=(const NFmiArea &theArea) const
{
  return !(*this == theArea);
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiStereographicArea::Write(std::ostream &file) const
{
  NFmiAzimuthalArea::Write(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * Read new object contents from the given input stream
 *
 * \param file The input stream to read from
 * \return The input stream read from
 */
// ----------------------------------------------------------------------

std::istream &NFmiStereographicArea::Read(std::istream &file)
{
  NFmiAzimuthalArea::Read(file);
  return file;
}

NFmiArea *NFmiStereographicArea::CreateNewArea(const NFmiRect &theRect) const
{
  NFmiPoint bottomLeft(ToLatLon(theRect.BottomLeft()));
  NFmiPoint topRight(ToLatLon(theRect.TopRight()));
  NFmiArea *area = new NFmiStereographicArea(
      bottomLeft, topRight, itsCentralLongitude, TopLeft(), BottomRight());
  return area;
}

const std::string NFmiStereographicArea::AreaStr() const
{
  std::ostringstream out;
  out << "stereographic," << CentralLongitude() << ',' << CentralLatitude() << ','
      << itsTrueLatitude.Value() << ':' << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y()
      << ',' << TopRightLatLon().X() << ',' << TopRightLatLon().Y();
  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return Well Known Text representation of the GCS
 *
 * Geneerinen:
 *
 * PROJCS["FMI_Stereographic",
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  PROJECTION["Stereographic"],
 *  PARAMETER["latitude_of_origin",lat_0],
 *  PARAMETER["central_meridian",lon_0],
 *  PARAMETER["false_easting",x_0],
 *  PARAMETER["false_northing",y_0],
 *  UNIT["Metre",1.0]]
 *
 * Polaaristereograafinen:
 *
 * PROJCS["FMI_Polar_Stereographic",
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  PROJECTION["Polar_Stereographic"],
 *  PARAMETER["latitude_of_origin",lat_ts],
 *  PARAMETER["central_meridian",lon],
 *  PARAMETER["false_easting",x_0],
 *  PARAMETER["false_northing",y_0],
 *  UNIT["Metre",1.0]]
 */
// ----------------------------------------------------------------------

const std::string NFmiStereographicArea::WKT() const
{
  std::ostringstream ret;

  if (itsCentralLatitude.Value() != 90)
  {
    ret << std::setprecision(16) << R"(PROJCS["FMI_Stereographic",)"
        << R"(GEOGCS["FMI_Sphere",)"
        << R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],)"
        << R"(PRIMEM["Greenwich",0],)"
        << R"(UNIT["Degree",0.0174532925199433]],)"
        << R"(PROJECTION["Stereographic"],)"
        << R"(PARAMETER["latitude_of_origin",)" << itsCentralLatitude.Value() << "],"
        << R"(PARAMETER["central_meridian",)" << itsCentralLongitude << "],"
        << R"(UNIT["Metre",1.0]])";
  }
  else
  {
    ret << std::setprecision(16) << R"(PROJCS["FMI_Polar_Stereographic",)"
        << R"(GEOGCS["FMI_Sphere",)"
        << R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],)"
        << R"(PRIMEM["Greenwich",0],)"
        << R"(UNIT["Degree",0.0174532925199433]],)"
        << R"(PROJECTION["Polar_Stereographic"],)"
        << R"(PARAMETER["latitude_of_origin",)" << itsTrueLatitude.Value() << "],"
        << R"(PARAMETER["central_meridian",)" << itsCentralLongitude << "],"
        << R"(UNIT["Metre",1.0]])";
  }
  return ret.str();
}

// ======================================================================
