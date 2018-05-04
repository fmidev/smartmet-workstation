// ======================================================================
/*!
 * \file NFmiRotatedLatLonArea.cpp
 * \brief Implementation of class NFmiRotatedLatLonArea
 */
// ======================================================================
/*!
 * \class NFmiRotatedLatLonArea
 *
 * Undocumented
 *
 */
// ======================================================================

#include "NFmiRotatedLatLonArea.h"
#include <algorithm>
#include <iomanip>

using namespace std;

namespace
{
const double kOneRad = 57.29577951;
const double kDegToRad = 0.017453292;

// ----------------------------------------------------------------------
/*!
 * \brief Convert latlon to rotated latlon
 *
 * \param theLatLonPoint The coordinates to convert
 * \param thePole The south pole coordinates
 * \return The rotated coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint torotlatlon(const NFmiPoint &theLatLonPoint,
                            const NFmiPoint &thePole,
                            bool usePacificView = false)
{
  // This methode is copied from the Hirlam-subroutine GEOROT
  // Also functions, if only longitude of the pole is rotated, but is prevented

  if (thePole.Y() == -90.) return theLatLonPoint;

  NFmiLatitude YPole = NFmiLatitude(thePole.Y());
  NFmiLongitude XPole = NFmiLongitude(thePole.X(), usePacificView);
  double SinYPole = sin((YPole.Value() + 90.) * kDegToRad);
  double CosYPole = cos((YPole.Value() + 90.) * kDegToRad);

  double SinXReg = NFmiLongitude(theLatLonPoint.X() - XPole.Value(), usePacificView).Sin();
  double CosXReg = NFmiLongitude(theLatLonPoint.X() - XPole.Value(), usePacificView).Cos();
  double SinYReg = NFmiLatitude(theLatLonPoint.Y()).Sin();
  double CosYReg = NFmiLatitude(theLatLonPoint.Y()).Cos();
  double SinYRot = CosYPole * SinYReg - SinYPole * CosYReg * CosXReg;
  SinYRot = std::min(std::max(SinYRot, -1.), 1.);
  NFmiLatitude YRot = NFmiLatitude(asin(SinYRot) * kOneRad);

  double CosYRot = YRot.Cos();
  double CosXRot = (CosYPole * CosYReg * CosXReg + SinYPole * SinYReg) / CosYRot;
  CosXRot = std::min(std::max(CosXRot, -1.), 1.);
  double SinXRot = CosYReg * SinXReg / CosYRot;
  NFmiLongitude XRot = NFmiLongitude(acos(CosXRot) * kOneRad, usePacificView);
  if (SinXRot < 0.)
    XRot.SetValue(-XRot.Value());
  else
    XRot.SetValue(XRot.Value());

  return NFmiPoint(XRot.Value(), YRot.Value());
}

// ----------------------------------------------------------------------
/*!
 * \brief Convert latlon to rotated latlon
 *
 * \param theRotLatLonPoint The coordinates to convert
 * \param thePole The south pole coordinates
 * \return The latlon coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint toreglatlon(const NFmiPoint &theRotLatLonPoint,
                            const NFmiPoint &thePole,
                            bool usePacificView = false)
{
  // This method is copied from the Hirlam-subroutine GEOROT
  // Also functions, if only longitude of the pole is rotated, but is prevented

  if (thePole.Y() == -90.) return theRotLatLonPoint;

  NFmiLatitude YPole = NFmiLatitude(thePole.Y());
  NFmiLongitude XPole = NFmiLongitude(thePole.X(), usePacificView);
  double SinYPole = sin((YPole.Value() + 90.) * kDegToRad);
  double CosYPole = cos((YPole.Value() + 90.) * kDegToRad);

  double SinXRot = NFmiLongitude(theRotLatLonPoint.X(), usePacificView).Sin();
  double CosXRot = NFmiLongitude(theRotLatLonPoint.X(), usePacificView).Cos();
  double SinYRot = NFmiLatitude(theRotLatLonPoint.Y()).Sin();
  double CosYRot = NFmiLatitude(theRotLatLonPoint.Y()).Cos();
  double SinYReg = CosYPole * SinYRot + SinYPole * CosYRot * CosXRot;
  SinYReg = std::min(std::max(SinYReg, -1.0), 1.0);

  NFmiLatitude YReg = NFmiLatitude(asin(SinYReg) * kOneRad);

  double CosYReg = YReg.Cos();
  double CosXReg = (CosYPole * CosYRot * CosXRot - SinYPole * SinYRot) / CosYReg;
  CosXReg = std::min(std::max(CosXReg, -1.0), 1.0);
  double SinXReg = CosYRot * SinXRot / CosYReg;

  double XRegVal = acos(CosXReg) * kOneRad;
  if (SinXReg < 0.) XRegVal = -XRegVal;
  XRegVal = XRegVal + XPole.Value();
  NFmiLongitude XReg = NFmiLongitude(XRegVal, usePacificView);

  return NFmiPoint(XReg.Value(), YReg.Value());
}
}

// ----------------------------------------------------------------------
/*!
 * Void constructor
 */
// ----------------------------------------------------------------------

NFmiRotatedLatLonArea::NFmiRotatedLatLonArea() : NFmiLatLonArea(), itsSouthernPole() {}
NFmiRotatedLatLonArea::NFmiRotatedLatLonArea(const NFmiRotatedLatLonArea &theRotatedLatLonArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * Constructor
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \param theSouthernPole Undocumented
 * \param theTopLeftXY Undocumented
 * \param theBottomRightXY Undocumented
 */
// ----------------------------------------------------------------------

NFmiRotatedLatLonArea::NFmiRotatedLatLonArea(const NFmiPoint &theBottomLeftLatLon,
                                             const NFmiPoint &theTopRightLatLon,
                                             const NFmiPoint &theSouthernPole,
                                             const NFmiPoint &theTopLeftXY,
                                             const NFmiPoint &theBottomRightXY,
                                             const bool initiallyRotated,
                                             bool usePacificView)
    : NFmiLatLonArea(
          initiallyRotated ? theBottomLeftLatLon
                           : torotlatlon(theBottomLeftLatLon, theSouthernPole),
          initiallyRotated ? theTopRightLatLon : torotlatlon(theTopRightLatLon, theSouthernPole),
          theTopLeftXY,
          theBottomRightXY,
          usePacificView),
      itsSouthernPole(theSouthernPole)
{
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiRotatedLatLonArea::Clone() const { return new NFmiRotatedLatLonArea(*this); }
// ----------------------------------------------------------------------
/*!
 * \param theXYPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiRotatedLatLonArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  return ToRegLatLon(NFmiLatLonArea::ToLatLon(theXYPoint));
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiRotatedLatLonArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  return NFmiLatLonArea::ToXY(ToRotLatLon(theLatLonPoint));
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

bool NFmiRotatedLatLonArea::IsInside(const NFmiPoint &theLatLonPoint) const
{
  return NFmiLatLonArea::IsInside(ToRotLatLon(theLatLonPoint));
}

// ----------------------------------------------------------------------
/*!
 * \param theLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiRotatedLatLonArea::ToRotLatLon(const NFmiPoint &theLatLonPoint) const
{
  return torotlatlon(theLatLonPoint, itsSouthernPole);
}

// ----------------------------------------------------------------------
/*!
 * \param theRotLatLonPoint Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiRotatedLatLonArea::ToRegLatLon(const NFmiPoint &theRotLatLonPoint) const
{
  return toreglatlon(theRotLatLonPoint, itsSouthernPole);
}

// ----------------------------------------------------------------------
/*!
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiRotatedLatLonArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                         const NFmiPoint &theTopRightLatLon,
                                         bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fixedPointData =
        NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiRotatedLatLonArea(fixedPointData.itsBottomLeftLatlon,
                                     fixedPointData.itsTopRightLatlon,
                                     itsSouthernPole,
                                     TopLeft(),
                                     BottomRight(),
                                     fixedPointData.fIsPacific);
  }
  else
    return new NFmiRotatedLatLonArea(theBottomLeftLatLon,
                                     theTopRightLatLon,
                                     itsSouthernPole,
                                     TopLeft(),
                                     BottomRight(),
                                     PacificView());
}

// ----------------------------------------------------------------------
/*!
 * Write the object to the given output stream
 *
 * \param file The output stream to write to
 * \return The output stream written to
 */
// ----------------------------------------------------------------------

std::ostream &NFmiRotatedLatLonArea::Write(std::ostream &file) const
{
  file.precision(12);
  NFmiLatLonArea::Write(file);
  file << itsSouthernPole;
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

std::istream &NFmiRotatedLatLonArea::Read(std::istream &file)
{
  NFmiLatLonArea::Read(file);
  file >> itsSouthernPole;
  return file;
}

const std::string NFmiRotatedLatLonArea::AreaStr() const
{
  std::ostringstream out;
  out << "rotlatlon," << SouthernPole().Y() << ',' << SouthernPole().X() << ':'
      << BottomLeftLatLon().X() << ',' << BottomLeftLatLon().Y() << ',' << TopRightLatLon().X()
      << ',' << TopRightLatLon().Y();
  return out.str();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return Well Known Text representation of the GCS
 *  GEOGCS["FMI_Sphere",
 *         DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],
 *         PRIMEM["Greenwich",0],
 *         UNIT["Degree",0.0174532925199433]],
 *  PARAMETER["latitude_of_origin",lat_0],
 *  PARAMETER["central_meridian",lon_0],
 *  UNIT["Metre",1.0]
 */
// ----------------------------------------------------------------------

const std::string NFmiRotatedLatLonArea::WKT() const
{
  std::ostringstream ret;
  ret << std::setprecision(16) << R"(GEOGCS["FMI_Sphere",)"
      << R"(DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],)"
      << R"(PRIMEM["Greenwich",0],)"
      << R"(UNIT["Degree",0.0174532925199433]],)"
      << R"(PARAMETER["latitude_of_origin",)" << itsSouthernPole.Y() << "],"
      << R"(PARAMETER["central_meridian",)" << itsSouthernPole.X() << "]";
  return ret.str();
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison with an area derived from NFmiLatLonArea.
 *
 * \param theArea The other area being compared to
 * \return True, if the NFmiRotatedLatLonArea parts are equivalent
 * \todo Use static_cast instead of C-style cast
 */
// ----------------------------------------------------------------------

bool NFmiRotatedLatLonArea::operator==(const NFmiArea &theArea) const
{
  return *this == static_cast<const NFmiRotatedLatLonArea &>(theArea);
}

// ----------------------------------------------------------------------
/*!
 * Equality comparison
 *
 * \param theArea The other area being compared to
 * \return True, if the areas are equivalent
 */
// ----------------------------------------------------------------------

bool NFmiRotatedLatLonArea::operator==(const NFmiRotatedLatLonArea &theArea) const
{
  if ((itsSouthernPole == theArea.itsSouthernPole) && NFmiLatLonArea::operator==(theArea))
    return true;

  return false;
}

// ----------------------------------------------------------------------
/*!
 * Measured from the input point, returns the azimuth angle between
 * the true geodetic north direction and the "up" Y-axis of the map
 * area rectangle. Azimuth angle runs 0..360 degrees clockwise, with
 * north zero degrees.
 *
 * \param theLatLonPoint Undocumented
 * \param theLatitudeEpsilon Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiAngle NFmiRotatedLatLonArea::TrueNorthAzimuth(const NFmiPoint &theLatLonPoint,
                                                        double theLatitudeEpsilon) const
{
  using namespace std;

  NFmiPoint xyWorldPoint = LatLonToWorldXY(ToRotLatLon(theLatLonPoint));
  NFmiPoint latLonIncr =
      NFmiPoint(0., theLatitudeEpsilon);  // Arbitrary small latitude increment in degrees

  // Move up toward geo-north along the meridian of the input point
  NFmiPoint xyDistanceAlongMeridian =
      LatLonToWorldXY(ToRotLatLon(theLatLonPoint + latLonIncr)) - xyWorldPoint;

  // Get the angle between 'xyDistanceAlongMeridian.X()' and map "up" direction Y-axis
  if (xyDistanceAlongMeridian.Y() == 0.)
    return xyDistanceAlongMeridian.X() > 0.
               ? NFmiAngle(90.)
               : NFmiAngle(
                     270.);  // Azimuth is exactly east 90 degrees or west 270 degrees, respectively

  return NFmiAngle(FmiDeg(atan2(xyDistanceAlongMeridian.X(), xyDistanceAlongMeridian.Y())));
}

// ======================================================================
