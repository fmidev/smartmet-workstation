// ======================================================================
/*!
 * \file NFmiPKJArea.cpp
 * \brief Implementation of class NFmiPKJArea
 */
// ======================================================================
/*!
 * \class NFmiPKJArea
 *
 * Implements transformation from rectangular PKJ (PerusKoordinaattiJärjestelmä)
 * coordinates into geodetic coordinates and vice versa for the Finnish
 * National Grid Coordinate System KKJ (KartastoKoordinaattiJärjestelmä).
 *
 * The rectangular PKJ coordinates are located on KKJ zones 1-4 with easting
 * and northing in meters.
 *
 * Projection used is Gauss-Krueger projection with Hayford's International
 * Reference ellipsoid.
 *
 * RECTANGULAR LOCAL COORDINATES are relative unitless cartesian XY-coordinates.
 * The range for these coordinates will be derived directly from the size of the
 * local rectangle used. The upper left corner XY-point and the opposite lower right
 * corner XY-point will define the LOCAL RECTANGLE.
 *
 * RECTANGULAR WORLD COORDINATES are cartesian XY-coordinates measured in meters
 * on "true world terrain". The upper left corner XY-point and the opposite lower right
 * corner XY-point (in meters) will define the WORLD RECTANGLE.
 *
 * GEODETIC COORDINATES are latitude-longitude coordinates in degrees.
 *
 * \see NFmiKKJArea and NFmiYKJArea
 *
 * INPUT PARAMETERS/ARGUMENT LISTS
 *
 *	theBottomLeftLatLon = lower left corner point of the rectangle in GEODETIC
 *								 latitude-longitude coordinates
 *
 * theTopRightLatLon =	 upper right corner point of the rectangle in GEODETIC
 *								 latitude-longitude coordinates for
 *the
 *rectangle
 *
 * theTopLeftXY = upper left corner of the rectangle in rectangular LOCAL coordinates
 *
 * theBottomRightXY = lower right corner of the rectangle in rectangular LOCAL coordinates
 *
 * thePKJArea = PKJArea object to be copied
 *
 *
 * NOTES
 *
 * The first constructor is meant to be used when both of the geodetic corner points
 * are known, whereas the second constructor is used when the upper right geodetic
 * corner point is unknown but the metric dimensions of the world rectangle are known.
 *
 * EXAMPLES
 *
 * How to convert the rectangular WORLD PKJ coordinates (in meters) into geodetic
 * coordinates ?
 *
 * \code
 *	NFmiPoint bottomLeftLatLon = ....;
 * 	NFmiPKJArea peekoojii(bottomLeftLatLon, 512000., 1300000.);
 *  NFmiPoint PKJLatLon, PKJxy;
 * \endcode
 *
 * Finally, this is how to convert rectangular WORLD PKJ coordinates into
 * geodetic coordinates:
 *
 * \code
 * PKJLatLon = peekoojii.WorldXYToLatLon(PKJxy);
 * \endcode
 *
 * How to convert the geodetic coordinates back into rectangular WORLD PKJ coordinates
 * (in meters) ?
 *
 * \code
 * PKJxy = peekoojii.LatLonToWorldXY(PKJLatLon);
 * \endcode
 *
 * How to convert the rectangular LOCAL PKJ coordinates into geodetic
 * coordinates ?
 *
 * \code
 * NFmiPoint PKJxy = ...;
 * PKJLatLon = peekoojii.ToLatLon(PKJxy);
 * \endcode
 *
 * How to convert the geodetic coordinates back into rectangular LOCAL PKJ coordinates ?
 *
 * \code
 * PKJxy = peekoojii.ToXY(PKJLatLon);
 * \endcode
 *
 */
// ======================================================================

#include "NFmiPKJArea.h"
#include <cmath>
#include <cstdlib>
#include <stdexcept>

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiPKJArea::NFmiPKJArea() : NFmiKKJArea() {}
// ----------------------------------------------------------------------
/*!
 * Copy constructor
 *
 * \param thePKJArea The other object being copied
 */
// ----------------------------------------------------------------------

NFmiPKJArea::NFmiPKJArea(const NFmiPKJArea &thePKJArea) = default;
// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiPKJArea::NFmiPKJArea(const NFmiPoint &theBottomLeftLatLon,
                         const NFmiPoint &theTopRightLatLon,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY,
                         bool usePacificView)
    : NFmiKKJArea(
          theBottomLeftLatLon, theTopRightLatLon, theTopLeftXY, theBottomRightXY, usePacificView)
{
  itsWorldRect =
      NFmiRect(CornerWorldXY(NFmiPoint(theBottomLeftLatLon.X(), theBottomLeftLatLon.Y())),
               CornerWorldXY(NFmiPoint(theTopRightLatLon.X(), theTopRightLatLon.Y())));

  Init(true);
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theWidthInMeters Undocumented
 * \param theHeightInMeters Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiPKJArea::NFmiPKJArea(const NFmiPoint &theBottomLeftLatLon,
                         const double theWidthInMeters,
                         const double theHeightInMeters,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY)
    : NFmiKKJArea(theBottomLeftLatLon, theTopLeftXY, theBottomRightXY)
{
  itsWorldRect =
      NFmiRect(CornerWorldXY(NFmiPoint(theBottomLeftLatLon.X(), theBottomLeftLatLon.Y())),
               CornerWorldXY(NFmiPoint(theBottomLeftLatLon.X(), theBottomLeftLatLon.Y())) +
                   NFmiPoint(theWidthInMeters, theHeightInMeters));

  Init(true);

  // 28.8.2001/Marko&Esa itsWorldRect on laskettu sellaisilla argumenteilla tässä, mitkä eivät
  // ole dataosia, joten sitä ei saa laskea Init:issä uudestaan
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftWorldXY Undocumented (in meters)
 * \param theTopRightWorldXY Undocumented (in meters)
 * \param isWorldXYInMeters Undocumented, unused
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiPKJArea::NFmiPKJArea(const NFmiPoint &theBottomLeftWorldXY,
                         const NFmiPoint &theTopRightWorldXY,
                         const bool & /* isWorldXYInMeters */,
                         const NFmiPoint &theTopLeftXY,
                         const NFmiPoint &theBottomRightXY)
    : NFmiKKJArea(theTopLeftXY, theBottomRightXY)
{
  itsWorldRect = NFmiRect(theBottomLeftWorldXY, theTopRightWorldXY);

  Init(true);
  // 28.8.2001/Marko&Esa itsWorldRect on laskettu sellaisilla argumenteilla
  // tässä, mitkä eivät ole dataosia, joten sitä ei saa laskea Init:issä
  // uudestaan // 13.11.98/EL lisäsi
}

// ----------------------------------------------------------------------
/*!
 * \param fKeepWorldRect
 */
// ----------------------------------------------------------------------

void NFmiPKJArea::Init(bool fKeepWorldRect)
{
  if (!fKeepWorldRect)
    itsWorldRect =
        NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon),
                 LatLonToWorldXY(itsTopRightLatLon));  // 28.8.2001/Marko&Esa Lisätty laskuihin.

  itsXScaleFactor = Width() / itsWorldRect.Width();
  itsYScaleFactor = Height() / itsWorldRect.Height();

  itsTopRightLatLon = TopRightLatLon();
  itsBottomLeftLatLon = BottomLeftLatLon();
  NFmiKKJArea::Init(fKeepWorldRect);
}

// ----------------------------------------------------------------------
/*!
 * Returns the world rectangular XY-point in meters corresponding the input
 * geodetic latitude-longitude point theLatLonPoint.
 *
 * \param theLatLonPoint The geodetic point
 * \return The respective PKJ rectangle corner point in meters
 */
// ----------------------------------------------------------------------

NFmiPoint NFmiPKJArea::CornerWorldXY(const NFmiPoint &theLatLonPoint) const
{
  // Calculate world coordinates (meters) for PKJ rectangle corner point.
  return LatLonToWorldXY(theLatLonPoint);
}

// ----------------------------------------------------------------------
/*!
 * Returns the KKJ zone number for PKJ. Zone number is related to the input
 * longitude. Longitude must be given in degrees.
 *
 * \param lon The longitude in degrees.
 * \return The KKJ zone number for PKJ
 */
// ----------------------------------------------------------------------

int NFmiPKJArea::ZoneNumberByLongitude(const double lon) const
{
  // For PKJ, determine the zone number by longitude.
  return 1 + int((lon - 19.5) / 3.);
}

// ----------------------------------------------------------------------
/*!
 * Returns the KKJ zone number for PKJ. Zone number is related to the input
 * easting. Easting must be given in meters.
 *
 * \param easting Easting in meters
 * \return The KKJ zone number for PKJ
 */
// ----------------------------------------------------------------------

int NFmiPKJArea::ZoneNumberByEasting(const double easting) const
{
  // For PKJ, determine the zone number by easting.

  if ((1000000. <= easting) && (easting <= 4999999.))
  {
    // Return the 1st digit of the easting value as zone number
    return (int(easting / 1000000.));
  }

  return (-1);
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 * \todo Should return boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiPKJArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                               const NFmiPoint &theTopRightLatLon,
                               bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fixedPointData =
        NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiPKJArea(fixedPointData.itsBottomLeftLatlon,
                           fixedPointData.itsTopRightLatlon,
                           TopLeft(),
                           BottomRight(),
                           fixedPointData.fIsPacific);
  }
  else
    return new NFmiPKJArea(
        theBottomLeftLatLon, theTopRightLatLon, TopLeft(), BottomRight(), PacificView());
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 * \todo Should return boost::shared_ptr instead
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiPKJArea::Clone() const { return new NFmiPKJArea(*this); }
// ----------------------------------------------------------------------
/*!
 * Assignment operator
 *
 * \param theArea The other object being copied
 * \return The object assigned to
 * \todo Should protect from self-assignment due to inheritance
 */
// ----------------------------------------------------------------------

NFmiPKJArea &NFmiPKJArea::operator=(const NFmiPKJArea &theArea) = default;

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The object being compared to
 * \return True, if the projections are equal
 */
// ----------------------------------------------------------------------

bool NFmiPKJArea::operator==(const NFmiPKJArea &theArea) const
{
  return NFmiKKJArea::operator==(static_cast<const NFmiKKJArea &>(theArea));
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison
 *
 * \param theArea Object being compared to
 * \return True, if the objects are different
 */
// ----------------------------------------------------------------------

bool NFmiPKJArea::operator!=(const NFmiPKJArea &theArea) const { return !(*this == theArea); }
// ----------------------------------------------------------------------
/*!
 * Equality comparison with a generic area
 *
 * \param theArea The object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiPKJArea::operator==(const NFmiArea &theArea) const
{
  return NFmiKKJArea::operator==(static_cast<const NFmiArea &>(theArea));
}

// ----------------------------------------------------------------------
/*!
 * Inequality comparison with a generic area
 *
 * \param theArea The object being compared to
 * \return True, if the objects are equal
 */
// ----------------------------------------------------------------------

bool NFmiPKJArea::operator!=(const NFmiArea &theArea) const
{
  return NFmiKKJArea::operator!=(static_cast<const NFmiArea &>(theArea));
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiPKJArea::Write(std::ostream &file) const
{
  NFmiKKJArea::Write(file);
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

std::istream &NFmiPKJArea::Read(std::istream &file)
{
  NFmiKKJArea::Read(file);
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return Well Known Text representation of the GCS
 *
 * Newbasen PKJ näyttäisi olevan sama asia kuin geneerinen KKJ.
 * Erityisesti siis YKJ:ta ei voida esittää valitsemalla vyöhykettä
 * 3 konstruktorissa. Näin ollen heitetään poikkeus.
 */
// ----------------------------------------------------------------------

const std::string NFmiPKJArea::WKT() const
{
  throw std::runtime_error("WKT not available for generic PKJ projections");
}

// ======================================================================
