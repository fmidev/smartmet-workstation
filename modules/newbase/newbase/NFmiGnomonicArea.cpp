// ======================================================================
/*!
 * \file NFmiGnomonicArea.cpp
 * \brief Implementation of class NFmiGnomonicArea
 */
// ======================================================================
/*!
 * \class NFmiGnomonicArea
 *
 * Gnomonic projection. Maps geodetic coordinates (degrees) to rectangular
 * gnomonic xy coordinates (meters) and vice versa.
 *
 * Projection is based on refs
 *
 *    -# Peter Richardus, Ron K.Adler: "Map Projections", North-Holland, 2nd printing 1974.
 *    -# Navigation Facility Reference Manual rev. 2.0 3/93
 *    -# "An Album of Map Projections" USGS Paper 1453
 *
 * \see NFmiAzimuthal, NFmiStereographicArea and NFMiEquidistArea
 *
 * How to convert the geodetic coordinates into rectangular LOCAL polar gnomonic coordinates ?
 *
 * \code
 *	CRect rekti;
 *	GetClientRect(rekti);
 *
 *	double lon0,lat0,lon1,lat1,centralLon,centralLat,trueLat;
 *
 *	lon0 = ...;
 *	lat0 = ...;
 *
 *	lon1 = ...;
 *	lat1 = ...;
 *
 *  Central longitude for Helsinki.
 *	centralLon = 25.0;
 *
 *  Central latitude for POLAR gnomonic projection.
 *	centralLat = 90.0;
 *
 *  true latitude for Helsinki
 *	trueLat = 60.0;
 *
 *	NFmiPoint bottomLeftLatLon2(lon0,lat0);
 *	NFmiPoint topRightLatLon2(lon1,lat1);
 *
 *	NFmiGnomonicArea gnomo(bottomLeftLatLon2
 *		                          ,topRightLatLon2
 *										  ,centralLon
 *										  ,NFmiPoint(0.,0.)
 *										  ,NFmiPoint(rekti.right,rekti.bottom)
 *										  ,centralLat
 *										  ,trueLat);
 *
 *	NFmiPoint gnomoXY =  gnomo.ToXY(bottomLeftLatLon2);
 * \endcode
 *
 * How to convert the rectangular LOCAL gnomonic coordinates back into geodetic coordinates ?
 *
 * \code
 * NFmiPoint gnomoLatLon = gnomo.ToLatLon(gnomoXY);
 * \endcode
 */
// ======================================================================

#include "NFmiGnomonicArea.h"
#include <cmath>
#include <iomanip>

using namespace std;

// ----------------------------------------------------------------------
/*!
 * Defalt constructor
 */
// ----------------------------------------------------------------------

NFmiGnomonicArea::NFmiGnomonicArea() : NFmiAzimuthalArea() {}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param theGnomonicArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiGnomonicArea::NFmiGnomonicArea(const NFmiGnomonicArea &theGnomonicArea)

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
 * \param theCentralLatitude Undocumented
 * \param theTrueLatitude Undocumented
 */
// ----------------------------------------------------------------------

NFmiGnomonicArea::NFmiGnomonicArea(const NFmiPoint &theBottomLeftLatLon,
                                   const NFmiPoint &theTopRightLatLon,
                                   const double theCentralLongitude,
                                   const NFmiPoint &theTopLeftXY,
                                   const NFmiPoint &theBottomRightXY,
                                   const double theCentralLatitude,
                                   const double theTrueLatitude,
                                   bool usePacificView)
    : NFmiAzimuthalArea(theBottomLeftLatLon,
                        theTopRightLatLon,
                        theCentralLongitude,
                        theTopLeftXY,
                        theBottomRightXY,
                        theCentralLatitude,
                        theTrueLatitude,
                        usePacificView)
{
  Init();
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

NFmiGnomonicArea::NFmiGnomonicArea(const NFmiPoint &theBottomLeftLatLon,
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
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);

  itsBottomLeftWorldXY =
      LatLonToWorldXY(NFmiPoint(theBottomLeftLatLon.X(), theBottomLeftLatLon.Y()));
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

NFmiGnomonicArea::NFmiGnomonicArea(const double theRadialRange,
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
      (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2000/EL

  itsWorldRect = NFmiRect(NFmiPoint(-itsRadialRange, -itsRadialRange),
                          NFmiPoint(itsRadialRange, itsRadialRange));

  Init(true);
  // 28.8.2001/Marko&Esa itsWorldRect on laskettu sellaisilla argumenteilla tässä,
  // mitkä eivät ole dataosia, joten sitä ei saa laskea Init:issä uudestaan
}

// ----------------------------------------------------------------------
/*!
 * \param delta Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGnomonicArea::K(const double delta) const
{
  double D;

  D = DistanceFromPerspectivePointToCenterOfEarth();
  return kRearth * (D + kRearth) / (D + (kRearth * delta));
}

// ----------------------------------------------------------------------
/*!
 * \param xyDistance Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGnomonicArea::CalcDelta(const double xyDistance) const
{
  // Calculates the delta angle for gnomonic projection.
  // See details in ref. [1] pp. 58-62.
  // return atan(xyDistance/(itsTrueLatitude.Sin()*kRearth));
  return atan(xyDistance / kRearth);  // 11.5.98/EL
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGnomonicArea::DistanceFromPerspectivePointToCenterOfEarth() const
{
  // Distance (in world-coordinate meters) for gnomonic projection.
  // See details in ref. [1] pp. 58-62.
  return 0.0;
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 * \todo Return an boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiGnomonicArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                    const NFmiPoint &theTopRightLatLon,
                                    bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fixedPointData =
        NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiGnomonicArea(fixedPointData.itsBottomLeftLatlon,
                                fixedPointData.itsTopRightLatlon,
                                itsCentralLongitude,
                                TopLeft(),
                                BottomRight(),
                                itsCentralLatitude.Value(),
                                itsTrueLatitude.Value(),
                                fixedPointData.fIsPacific);
  }
  else
    return new NFmiGnomonicArea(theBottomLeftLatLon,
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
 * \todo Return and boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiGnomonicArea::Clone() const { return new NFmiGnomonicArea(*this); }
// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theArea The other object being copied
 * \return The object assigned to
 * \todo Since NFmiAzimuthalArea::operator= is unknown, self assignment
 *       should be protected
 */
// ----------------------------------------------------------------------

NFmiGnomonicArea &NFmiGnomonicArea::operator=(const NFmiGnomonicArea &theArea) = default;

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the objects are equivalent
 */
// ----------------------------------------------------------------------

bool NFmiGnomonicArea::operator==(const NFmiGnomonicArea &theArea) const
{
  return NFmiAzimuthalArea::operator==(static_cast<const NFmiAzimuthalArea &>(theArea));
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the objects are not equivalent
 */
// ----------------------------------------------------------------------

bool NFmiGnomonicArea::operator!=(const NFmiGnomonicArea &theArea) const
{
  return !(*this == theArea);
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the objects are equivalent
 */
// ----------------------------------------------------------------------

bool NFmiGnomonicArea::operator==(const NFmiArea &theArea) const
{
  return NFmiAzimuthalArea::operator==(theArea);
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the objects are not equivalent
 */
// ----------------------------------------------------------------------

bool NFmiGnomonicArea::operator!=(const NFmiArea &theArea) const { return !(*this == theArea); }
// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiGnomonicArea::Write(std::ostream &file) const
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

std::istream &NFmiGnomonicArea::Read(std::istream &file)
{
  NFmiAzimuthalArea::Read(file);

  // Init(); // 26.10.99/EL: Siirretty emoluokkaan
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect Undocumented
 */
// ----------------------------------------------------------------------

void NFmiGnomonicArea::Init(bool fKeepWorldRect)
{
  if (!fKeepWorldRect)
  {
    itsTrueLatScaleFactor =
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth * itsTrueLatitude.Sin()) /
        (DistanceFromPerspectivePointToCenterOfEarth() + kRearth);  // 17.1.2000/EL

    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));
  }

  NFmiAzimuthalArea::Init(fKeepWorldRect);
}

const std::string NFmiGnomonicArea::AreaStr() const
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
 * PROJCS["FMI_Gnomonic",
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  PROJECTION["Gnomonic"],
 *  PARAMETER["latitude_of_origin",lat_0],
 *  PARAMETER["central_meridian",lon_0],
 *  PARAMETER["false_easting",x_0],
 *  PARAMETER["false_northing",y_0],
 *  UNIT["Metre",1.0]]
 */
// ----------------------------------------------------------------------

const std::string NFmiGnomonicArea::WKT() const
{
  std::ostringstream ret;
  ret << std::setprecision(16) << R"(PROJCS["FMI_Gnomonic",)"
      << R"(GEOGCS["FMI_Sphere",)"
      << R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],)"
      << R"(PRIMEM["Greenwich",0],)"
      << R"(UNIT["Degree",0.0174532925199433]],)"
      << R"(PROJECTION["Gnomonic"],)"
      << R"(PARAMETER["latitude_of_origin",)" << itsCentralLatitude.Value() << "],"
      << R"(PARAMETER["central_meridian",)" << itsCentralLongitude << "],"
      << R"(UNIT["Metre",1.0]])";
  return ret.str();
}

// ======================================================================
