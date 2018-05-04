// ======================================================================
/*!
 * \file NFmiGdalArea.cpp
 * \brief Implementation of class NFmiGdalArea
 */
// ======================================================================

#ifdef UNIX

#include "NFmiGdalArea.h"
#include "NFmiString.h"
#include "NFmiStringTools.h"
#include <boost/math/constants/constants.hpp>
#include <gdal/ogr_spatialref.h>
#include <cmath>
#include <iomanip>

using namespace std;

// See also NFmiLatLonArea::WKT()
std::string fmiwkt =
    R"(GEOGCS["FMI_Sphere",DATUM["FMI_2007",SPHEROID["FMI_Sphere",6371220,0]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]])";

// ----------------------------------------------------------------------
/*!
 * \brief Destructor does nothing special
 */
// ----------------------------------------------------------------------

NFmiGdalArea::~NFmiGdalArea() = default;
// ----------------------------------------------------------------------
/*!
 * \brief Default constructor
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea()
    : NFmiArea(),
      itsDatum("WGS84"),
      itsDescription(),
      itsWKT(),
      itsBottomLeftLatLon(),
      itsTopRightLatLon(),
      itsWorldRect(),
      itsSpatialReference(),
      itsLatLonToWorldXYTransformation(),
      itsWorldXYToLatLonTransformation()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Copy constructor
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const NFmiGdalArea &theArea)

    = default;

// ----------------------------------------------------------------------
/*!
 * \brief Assignment operator
 */
// ----------------------------------------------------------------------

NFmiGdalArea &NFmiGdalArea::operator=(const NFmiGdalArea &theArea) = default;

// ----------------------------------------------------------------------
/*!
 * \brief Construct from user input
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const std::string &theDatum,
                           const std::string &theDescription,
                           const NFmiPoint &theBottomLeftLatLon,
                           const NFmiPoint &theTopRightLatLon,
                           const NFmiPoint &theTopLeftXY,
                           const NFmiPoint &theBottomRightXY,
                           bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsDatum(theDatum),
      itsDescription(theDescription),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsWorldRect()
{
  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from user input
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const std::string &theDatum,
                           const OGRSpatialReference &theCRS,
                           const NFmiPoint &theBottomLeftLatLon,
                           const NFmiPoint &theTopRightLatLon,
                           const NFmiPoint &theTopLeftXY,
                           const NFmiPoint &theBottomRightXY,
                           bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsDatum(theDatum),
      itsDescription(),
      itsBottomLeftLatLon(theBottomLeftLatLon),
      itsTopRightLatLon(theTopRightLatLon),
      itsWorldRect(),
      itsSpatialReference(new OGRSpatialReference(theCRS))
{
  // Guess a good value for itsDescription

  const char *auth = theCRS.GetAuthorityName(nullptr);
  if (auth != nullptr)
  {
    itsDescription = std::string(auth);
    const char *code = theCRS.GetAuthorityCode(nullptr);
    if (code != nullptr) itsDescription += ":" + std::string(code);
  }
  else
  {
    char *out;
    theCRS.exportToWkt(&out);
    itsDescription = out;
    OGRFree(out);
  }

  init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from bounding box
 */
// ----------------------------------------------------------------------

NFmiGdalArea::NFmiGdalArea(const std::string &theDatum,
                           const OGRSpatialReference &theCRS,
                           double theXmin,
                           double theYmin,
                           double theXmax,
                           double theYmax,
                           const NFmiPoint &theTopLeftXY,
                           const NFmiPoint &theBottomRightXY,
                           bool usePacificView)
    : NFmiArea(theTopLeftXY, theBottomRightXY, usePacificView),
      itsDatum(theDatum),
      itsDescription(),
      itsBottomLeftLatLon(),
      itsTopRightLatLon(),
      itsWorldRect(NFmiPoint(theXmin, theYmin), NFmiPoint(theXmax, theYmax)),
      itsSpatialReference(new OGRSpatialReference(theCRS))
{
  // Guess a good value for itsDescription

  const char *auth = theCRS.GetAuthorityName(nullptr);
  if (auth != nullptr)
  {
    itsDescription = std::string(auth);
    const char *code = theCRS.GetAuthorityCode(nullptr);
    if (code != nullptr) itsDescription += ":" + std::string(code);
  }
  else
  {
    char *out;
    theCRS.exportToWkt(&out);
    itsDescription = out;
    OGRFree(out);
  }

  // The needed spatial references

  if (!itsSpatialReference)
  {
    itsSpatialReference.reset(new OGRSpatialReference);
    OGRErr err = itsSpatialReference->SetFromUserInput(itsDescription.c_str());
    if (err != OGRERR_NONE)
      throw std::runtime_error("GDAL does not understand projection spec: '" + itsDescription +
                               "'");
  }

  OGRErr err;
  boost::shared_ptr<OGRSpatialReference> datum(new OGRSpatialReference);
  if (itsDatum == "FMI")
    err = datum->SetFromUserInput(fmiwkt.c_str());
  else
    err = datum->SetFromUserInput(itsDatum.c_str());

  if (err != OGRERR_NONE) throw std::runtime_error("Failed to set datum: '" + itsDatum + "'");

  // The needed coordinate transformations

  itsWorldXYToLatLonTransformation.reset(
      OGRCreateCoordinateTransformation(itsSpatialReference.get(), datum.get()));
  if (!itsWorldXYToLatLonTransformation)
    throw std::runtime_error("Projection spec unsable with the chosen datum (" + itsDatum + "): '" +
                             itsDescription + "'");

  itsLatLonToWorldXYTransformation.reset(
      OGRCreateCoordinateTransformation(datum.get(), itsSpatialReference.get()));
  if (!itsLatLonToWorldXYTransformation)
    throw std::runtime_error("Projection spec unsable with the chosen datum (" + itsDatum + "): '" +
                             itsDescription + "'");

  // Bottom left and top right coordinates - needed only for geographic projections (Width()
  // calculations)
  // The same data could be extracted from the WorldXYRect too though - and these variables
  // would not be needed.

  double x1 = theXmin;
  double y1 = theYmin;
  double x2 = theXmax;
  double y2 = theYmax;
  itsWorldXYToLatLonTransformation->Transform(1, &x1, &y1);
  itsWorldXYToLatLonTransformation->Transform(1, &x2, &y2);
  itsBottomLeftLatLon = NFmiPoint(x1, y1);
  itsTopRightLatLon = NFmiPoint(x2, y2);

  // Initialize itsWKT
  char *out;
  itsSpatialReference->exportToWkt(&out);
  itsWKT = out;
  OGRFree(out);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return class ID
 */
// ----------------------------------------------------------------------

unsigned long NFmiGdalArea::ClassId() const { return kNFmiGdalArea; }
// ----------------------------------------------------------------------
/*!
 * \brief Return class name
 */
// ----------------------------------------------------------------------

const char *NFmiGdalArea::ClassName() const { return "kNFmiGdalArea"; }
// ----------------------------------------------------------------------
/*!
 * \brief Return a clone
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiGdalArea::Clone() const { return new NFmiGdalArea(*this); }
// ----------------------------------------------------------------------
/*!
 * \brief Area descriptor
 */
// ----------------------------------------------------------------------

const std::string NFmiGdalArea::AreaStr() const { return itsDescription; }
// ----------------------------------------------------------------------
/*!
 * \brief Datum
 */
// ----------------------------------------------------------------------

const std::string &NFmiGdalArea::Datum() const { return itsDatum; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the WKT description
 */
// ----------------------------------------------------------------------

const std::string NFmiGdalArea::WKT() const { return itsWKT; }
// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator==(const NFmiGdalArea &theArea) const
{
  return (itsBottomLeftLatLon == theArea.itsBottomLeftLatLon &&
          itsTopRightLatLon == theArea.itsTopRightLatLon && itsWorldRect == theArea.itsWorldRect &&
          itsDescription == theArea.itsDescription && itsDatum == theArea.itsDatum);
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator!=(const NFmiGdalArea &theArea) const { return !(*this == theArea); }
// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator==(const NFmiArea &theArea) const
{
  return *this == static_cast<const NFmiGdalArea &>(theArea);
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison
 */
// ----------------------------------------------------------------------

bool NFmiGdalArea::operator!=(const NFmiArea &theArea) const { return !(*this == theArea); }
// ----------------------------------------------------------------------
/*!
 * \brief Write the projection definition to a file
 */
// ----------------------------------------------------------------------

std::ostream &NFmiGdalArea::Write(std::ostream &file) const
{
  NFmiString tmp1 = itsDatum;
  NFmiString tmp2 = itsDescription;

  NFmiArea::Write(file);
  file << itsBottomLeftLatLon << itsTopRightLatLon << tmp1 << tmp2;

  return file;
}

// ----------------------------------------------------------------------
/*!
 * \brief Read new projection definition from the input stream
 */
// ----------------------------------------------------------------------

std::istream &NFmiGdalArea::Read(std::istream &file)
{
  NFmiString tmp1;
  NFmiString tmp2;

  NFmiArea::Read(file);
  file >> itsBottomLeftLatLon >> itsTopRightLatLon >> tmp1 >> tmp2;

  itsDatum = tmp1.CharPtr();
  itsDescription = tmp2.CharPtr();

  init();

  return file;
}

// ----------------------------------------------------------------------
/*!
 * \brief XY coordinate to LatLon
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::ToLatLon(const NFmiPoint &theXYPoint) const
{
  double xscale = Width() / itsWorldRect.Width();
  double yscale = Height() / itsWorldRect.Height();

  double worldx = itsWorldRect.Left() + (theXYPoint.X() - Left()) / xscale;
  double worldy = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / yscale;

  return WorldXYToLatLon(NFmiPoint(worldx, worldy));
}

// ----------------------------------------------------------------------
/*!
 * \brief LatLon to XY-coordinate
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::ToXY(const NFmiPoint &theLatLonPoint) const
{
  NFmiPoint latlon(FixLongitude(theLatLonPoint.X()), theLatLonPoint.Y());
  NFmiPoint worldxy = LatLonToWorldXY(latlon);

  double xscale = Width() / itsWorldRect.Width();
  double yscale = Height() / itsWorldRect.Height();

  double x = Left() + xscale * (worldxy.X() - itsWorldRect.Left());
  double y = Top() + yscale * (itsWorldRect.Bottom() - worldxy.Y());

  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief XY-coordinate to World coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::XYToWorldXY(const NFmiPoint &theXYPoint) const
{
  double xscale = Width() / itsWorldRect.Width();
  double yscale = Height() / itsWorldRect.Height();

  double worldx = itsWorldRect.Left() + (theXYPoint.X() - Left()) / xscale;
  double worldy = itsWorldRect.Bottom() - (theXYPoint.Y() - Top()) / yscale;

  return NFmiPoint(worldx, worldy);
}

// ----------------------------------------------------------------------
/*!
 * \brief World coordinates to LatLon
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::WorldXYToLatLon(const NFmiPoint &theXYPoint) const
{
  if (!itsWorldXYToLatLonTransformation)
    throw std::runtime_error("Trying to use an uninitialized GDAL area");
  double x = theXYPoint.X();
  double y = theXYPoint.Y();
  if (!itsWorldXYToLatLonTransformation->Transform(1, &x, &y))
    return NFmiPoint(kFloatMissing, kFloatMissing);
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief LatLon to world coordinates
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiGdalArea::LatLonToWorldXY(const NFmiPoint &theLatLonPoint) const
{
  if (!itsLatLonToWorldXYTransformation)
    throw std::runtime_error("Trying to use an uninitialized GDAL area");
  double x = theLatLonPoint.X();
  double y = theLatLonPoint.Y();
  if (!itsLatLonToWorldXYTransformation->Transform(1, &x, &y))
    return NFmiPoint(kFloatMissing, kFloatMissing);
  return NFmiPoint(x, y);
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the world rectangle
 */
// ----------------------------------------------------------------------

const NFmiRect NFmiGdalArea::WorldRect() const { return itsWorldRect; }
// ----------------------------------------------------------------------
/*!
 * \brief
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiGdalArea::NewArea(const NFmiPoint &theBottomLeftLatLon,
                                const NFmiPoint &theTopRightLatLon,
                                bool allowPacificFix) const
{
  if (allowPacificFix)
  {
    PacificPointFixerData fix = NFmiArea::PacificPointFixer(theBottomLeftLatLon, theTopRightLatLon);
    return new NFmiGdalArea(itsDatum,
                            itsDescription,
                            fix.itsBottomLeftLatlon,
                            fix.itsTopRightLatlon,
                            TopLeft(),
                            BottomRight(),
                            fix.fIsPacific);
  }
  else
    return new NFmiGdalArea(itsDatum,
                            itsDescription,
                            theBottomLeftLatLon,
                            theTopRightLatLon,
                            TopLeft(),
                            BottomRight(),
                            PacificView());
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize the projection transformation objects
 */
// ----------------------------------------------------------------------

void NFmiGdalArea::init()
{
  // The needed spatial references

  if (!itsSpatialReference)
  {
    itsSpatialReference.reset(new OGRSpatialReference);
    OGRErr err = itsSpatialReference->SetFromUserInput(itsDescription.c_str());
    if (err != OGRERR_NONE)
      throw std::runtime_error("GDAL does not understand projection spec: '" + itsDescription +
                               "'");
  }

  OGRErr err;
  boost::shared_ptr<OGRSpatialReference> datum(new OGRSpatialReference);
  if (itsDatum == "FMI")
    err = datum->SetFromUserInput(fmiwkt.c_str());
  else
    err = datum->SetFromUserInput(itsDatum.c_str());

  if (err != OGRERR_NONE) throw std::runtime_error("Failed to set datum: '" + itsDatum + "'");

  // The needed coordinate transformations

  itsWorldXYToLatLonTransformation.reset(
      OGRCreateCoordinateTransformation(itsSpatialReference.get(), datum.get()));
  if (!itsWorldXYToLatLonTransformation)
    throw std::runtime_error("Projection spec unsable with the chosen datum (" + itsDatum + "): '" +
                             itsDescription + "'");

  itsLatLonToWorldXYTransformation.reset(
      OGRCreateCoordinateTransformation(datum.get(), itsSpatialReference.get()));
  if (!itsLatLonToWorldXYTransformation)
    throw std::runtime_error("Projection spec unsable with the chosen datum (" + itsDatum + "): '" +
                             itsDescription + "'");

  // The needed world XY rectangle

  itsWorldRect = NFmiRect(LatLonToWorldXY(itsBottomLeftLatLon), LatLonToWorldXY(itsTopRightLatLon));

  // Initialize itsWKT
  char *out;
  itsSpatialReference->exportToWkt(&out);
  itsWKT = out;
  OGRFree(out);
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGdalArea::WorldXYWidth() const
{
  if (!itsSpatialReference->IsGeographic())
    return WorldRect().Width();
  else
  {
    double pi = boost::math::constants::pi<double>();
    double circumference = 2 * pi * 6371220;
    double dlon = itsTopRightLatLon.X() - itsBottomLeftLatLon.X();
    if (dlon < 0) dlon += 360;
    double clat = 0.5 * (itsBottomLeftLatLon.Y() + itsTopRightLatLon.Y());
    return dlon / 360 * circumference * cos(clat * pi / 180);
  }
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiGdalArea::WorldXYHeight() const
{
  if (!itsSpatialReference->IsGeographic())
    return WorldRect().Height();
  else
  {
    double pi = boost::math::constants::pi<double>();
    double circumference = 2 * pi * 6371220;
    double dlat = itsTopRightLatLon.Y() - itsBottomLeftLatLon.Y();
    return dlat / 360.0 * circumference;  // angle -> meters
  }
}

#endif  // UNIX

// ======================================================================
