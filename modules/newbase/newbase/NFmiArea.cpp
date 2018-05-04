// ======================================================================
/*!
 * \file NFmiArea.cpp
 * \brief Implementation of class NFmiArea
 */
// ======================================================================
/*!
 * \class NFmiArea
 *
 * Undocumented
 */
// ======================================================================

#include "NFmiArea.h"
#include "NFmiAreaFactory.h"
#include <boost/functional/hash.hpp>

// ----------------------------------------------------------------------
/*!
 * \bug Turha argumentti konstruktorille
 */
// ----------------------------------------------------------------------

void NFmiArea::Init(bool /* fKeepWorldRect */) { CheckForPacificView(); }
// ----------------------------------------------------------------------
/*!
 * \param newArea Undocumented
 */
// ----------------------------------------------------------------------

void NFmiArea::SetXYArea(const NFmiRect &newArea)
{
  Place(newArea.TopLeft());
  Size(newArea.Size());
  Init();
}

static void FixPacificLongitude(NFmiPoint &lonLat)
{
  if (lonLat.X() < 0)
  {
    NFmiLongitude lon(lonLat.X(), true);
    lonLat.X(lon.Value());
  }
}

#if 0
static void FixAtlanticLongitude(NFmiPoint &lonLat)
{
    if(lonLat.X() > 180)
    {
        NFmiLongitude lon(lonLat.X(), false);
        lonLat.X(lon.Value());
    }
}
#endif

// ----------------------------------------------------------------------
/*!
 * \param theArea Undocumented
 */
// ----------------------------------------------------------------------

const NFmiRect NFmiArea::XYArea(const NFmiArea *theArea) const
{
  if (PacificView() && theArea->PacificView() == false)
  {
    NFmiPoint topLeftLatlon = theArea->ToLatLon(theArea->TopLeft());
    ::FixPacificLongitude(topLeftLatlon);
    NFmiPoint bottomRightLatlon = theArea->ToLatLon(theArea->BottomRight());
    ::FixPacificLongitude(bottomRightLatlon);

    NFmiPoint topLeft(ToXY(topLeftLatlon));
    NFmiPoint bottomRight(ToXY(bottomRightLatlon));
    NFmiRect rect(topLeft, bottomRight);
    return rect;
  }
  else if (PacificView() == false && theArea->PacificView())
  {
    std::unique_ptr<NFmiArea> pacificAreaFromThis(DoForcePacificFix());

    NFmiPoint topLeft(pacificAreaFromThis->ToXY(theArea->ToLatLon(theArea->TopLeft())));
    NFmiPoint bottomRight(pacificAreaFromThis->ToXY(theArea->ToLatLon(theArea->BottomRight())));
    NFmiRect rect(topLeft, bottomRight);
    return rect;
  }
  else
  {
    NFmiPoint topLeft(ToXY(theArea->ToLatLon(theArea->TopLeft())));
    NFmiPoint bottomRight(ToXY(theArea->ToLatLon(theArea->BottomRight())));
    NFmiRect rect(topLeft, bottomRight);
    return rect;
  }
}

// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::ostream &NFmiArea::Write(std::ostream &file) const
{
  file << itsXYRectArea;
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \param file Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

std::istream &NFmiArea::Read(std::istream &file)
{
  file >> itsXYRectArea;
  return file;
}

// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::WorldXYSize() const { return WorldRect().Size(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::WorldXYPlace() const { return WorldRect().Place(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::WorldXYWidth() const { return WorldRect().Width(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::WorldXYHeight() const { return WorldRect().Height(); }
// ----------------------------------------------------------------------
/*!
 * \return Undocumented
 */
// ----------------------------------------------------------------------

double NFmiArea::WorldXYAspectRatio() const { return WorldXYWidth() / WorldXYHeight(); }
// ----------------------------------------------------------------------
/*!
 * Creates a new area from the current one by altering the corner points only.
 * Previously this method required the new area to be inside the original
 * one, but I failed to see the point in restricing the functionality - Mika
 *
 * \param theBottomLeftLatLon Undocumented
 * \param theTopRightLatLon Undocumented
 * \return Undocumented
 *
 * \todo Should return an boost::shared_ptr
 * \todo Remove the unnecessary cast in the last return statement
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::CreateNewArea(const NFmiPoint &theBottomLeftLatLon,
                                  const NFmiPoint &theTopRightLatLon) const
{
  return NewArea(theBottomLeftLatLon, theTopRightLatLon);
}

// ----------------------------------------------------------------------
/*!
 *
 * Creates a new sub-area inside the current "mother" area.
 * The new area is defined by the input local rectangle 'theRect'
 *
 * \note
 *   - the input rectangle defines a local XY coordinate area only, NOT the metric
 *     XY world rectangle
 *   - the sub-area MUST fit completely inside the current local area
 *   - the sub-area gets all the projection-specific properties but its dimensions
 *     from its "mother" area. For example, the orientation stays the same.
 *
 * \param theRect Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::CreateNewArea(const NFmiRect &theRect) const
{
  NFmiPoint newBottomLeftXY = theRect.BottomLeft();
  NFmiPoint newTopRightXY = theRect.TopRight();

  NFmiPoint newBottomLeftLatLon = ToLatLon(newBottomLeftXY);
  NFmiPoint newTopRightLatLon = ToLatLon(newTopRightXY);

  NFmiArea *newArea = NewArea(newBottomLeftLatLon, newTopRightLatLon);
  return newArea;
}

// ----------------------------------------------------------------------
/*!
 * Creates a new area with its aspect ratio defined by the input parameters
 *
 * \note
 *    - the new area MAY OR MAY NOT completely reside inside the current local area
 *    - the new area gets all the projection-specific properties but its dimensions
 *      from its "mother" area. For example, the orientation stays the same.
 *
 * \param theNewAspectRatioXperY Undocumented
 * \param theFixedPoint Undocumented
 * \param fShrinkArea Undocumented
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::CreateNewArea(double theNewAspectRatioXperY,
                                  FmiDirection theFixedPoint,
                                  bool fShrinkArea)
{
  double originalAspectRatio = WorldXYAspectRatio();

  bool keepWidth;

  if (fShrinkArea)
  {
    // The new area will be "shrunk" to completely fit inside the current area
    if ((theNewAspectRatioXperY < originalAspectRatio))
      keepWidth = false;  // Maintain height, compute width
    else
      keepWidth = true;  // Maintain width, compute height
  }
  else
  {
    // The new area will in part grow out of the current area
    if ((theNewAspectRatioXperY < originalAspectRatio))
      keepWidth = true;  // Maintain width, compute height
    else
      keepWidth = false;  // Maintain height, compute width
  }

  // Create copy of the original "world rectangle" to be freely modified
  NFmiRect newWorldRect = WorldRect();

  // REDIMENSIONING OF THE WORLD RECTANGLE
  //----------------------------------------

  if (!newWorldRect.AdjustAspectRatio(theNewAspectRatioXperY, keepWidth, theFixedPoint))
    return nullptr;

  // Create a new area with the new aspect ratio
  NFmiArea *newArea =
      NewArea(WorldXYToLatLon(newWorldRect.TopLeft()), WorldXYToLatLon(newWorldRect.BottomRight()));

  // Return the re-dimensioned copy of the original area
  return newArea;
}

// ----------------------------------------------------------------------
/*!
 * Creates a new sub-area inside the current "mother" area.
 * The new area is defined by the input metric rectangle 'theWorldRect'
 *
 * \note
 *   - the input rectangle defines a metric XY world rectangle
 *   - the sub-area MUST fit completely inside the current local area
 *   - the sub-area gets all the projection-specific properties but its dimensions
 *   - from its "mother" area. For example, the orientation stays the same.
 *
 * \param theWorldRect
 * \return Undocumented
 */
// ----------------------------------------------------------------------

NFmiArea *NFmiArea::CreateNewAreaByWorldRect(const NFmiRect &theWorldRect)
{
  NFmiPoint newBottomLeftXY = theWorldRect.BottomLeft();
  NFmiPoint newTopRightXY = theWorldRect.TopRight();

  NFmiPoint newBottomLeftLatLon = WorldXYToLatLon(newBottomLeftXY);
  NFmiPoint newTopRightLatLon = WorldXYToLatLon(newTopRightXY);

  if (!IsInside(newBottomLeftLatLon) || !IsInside(newTopRightLatLon)) return nullptr;

  auto *newArea = static_cast<NFmiArea *>(NewArea(newBottomLeftLatLon, newTopRightLatLon));

  if (!IsInside(*newArea)) return nullptr;

  return newArea;
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

const NFmiAngle NFmiArea::TrueNorthAzimuth(const NFmiPoint &theLatLonPoint,
                                           double theLatitudeEpsilon) const
{
  using namespace std;

  NFmiPoint xyWorldPoint = LatLonToWorldXY(theLatLonPoint);
  NFmiPoint latLonIncr =
      NFmiPoint(0., theLatitudeEpsilon);  // Arbitrary small latitude increment in degrees

  // Move up toward geo-north along the meridian of the input point
  NFmiPoint xyDistanceAlongMeridian = LatLonToWorldXY(theLatLonPoint + latLonIncr) - xyWorldPoint;

  // Get the angle between 'xyDistanceAlongMeridian.X()' and map "up" direction Y-axis
  if (xyDistanceAlongMeridian.Y() == 0.)
    return xyDistanceAlongMeridian.X() > 0.
               ? NFmiAngle(90.)
               : NFmiAngle(
                     270.);  // Azimuth is exactly east 90 degrees or west 270 degrees, respectively

  return NFmiAngle(FmiDeg(atan2(xyDistanceAlongMeridian.X(), xyDistanceAlongMeridian.Y())));
}

// ----------------------------------------------------------------------
/*!
 * \brief Return center latlon coordinate
 */
// ----------------------------------------------------------------------

const NFmiPoint NFmiArea::CenterLatLon() const
{
  NFmiPoint bl = BottomLeft();
  NFmiPoint tr = TopRight();

  NFmiPoint center(0.5 * (bl.X() + tr.X()), 0.5 * (bl.Y() + tr.Y()));

  return ToLatLon(center);
}

bool NFmiArea::IsPacificLongitude(double theLongitude)
{
  if (theLongitude > 180 && theLongitude <= 360)
    return true;
  else
    return false;
}

void NFmiArea::CheckForPacificView()
{
  fPacificView = NFmiArea::IsPacificView(BottomLeftLatLon(), TopRightLatLon());
}

// 1. Tarkistaa kattaako annetut pisteet Atlantic vai Pacific alueen
// 2. Jos Pacific, korjaa longitudet pacific:eiksi
// 3. Palauta originaali/korjatut pisteet pair:issa
PacificPointFixerData NFmiArea::PacificPointFixer(const NFmiPoint &theBottomLeftLatlon,
                                                  const NFmiPoint &theTopRightLatlon)
{
  bool usePacificView = NFmiArea::IsPacificView(theBottomLeftLatlon, theTopRightLatlon);
  if (usePacificView)
  {
    NFmiPoint bottomLeftLatLon = theBottomLeftLatlon;
    NFmiPoint topRightLatLon = theTopRightLatlon;
    NFmiAreaFactory::DoPossiblePacificFix(bottomLeftLatLon, topRightLatLon, usePacificView);
    return PacificPointFixerData(bottomLeftLatLon, topRightLatLon, usePacificView);
  }
  else
    return PacificPointFixerData(theBottomLeftLatlon, theTopRightLatlon, usePacificView);
}

bool NFmiArea::IsPacificView(const NFmiPoint &bottomleftLatlon, const NFmiPoint &toprightLatlon)
{
  // Obvious case
  if (bottomleftLatlon.X() >= 0 && toprightLatlon.X() < 0) return true;
  // 0...360 coordinate system is used
  if (IsPacificLongitude(bottomleftLatlon.X()) || IsPacificLongitude(toprightLatlon.X()))
    return true;
  return false;
}

double NFmiArea::FixLongitude(double theLon) const
{
  if (!fPacificView)
  {
    if (theLon > 180.00000001)  // Tämä ei voi olla tasan 180, koska SmartMet maailman rajaviivat
                                // shapessa (maps\\shapes\\ne_10m_admin_0_countries) ja niiden
                                // piirto imagine-kirjastolla menee jossain kohdissa sekaisin
      // reunoilla, koska siellä on käytetty vähän yli 180-pituuspiirin
      // meneviä arvoja Tyynenmeren 180 asteen pituuspiirin reunoilla
      return theLon - 360;
    else
      return theLon;
  }
  else if (theLon < 0)
    return theLon + 360;
  else
    return theLon;
}

NFmiArea *NFmiArea::DoPossiblePacificFix() const
{
  // On olemassa pari erikoistapausta, mitkä halutaan eri areoissa korjata, että alueet toimisivat
  // paremmin newbase:ssa.
  if (fPacificView)
  {
    bool usedPacificViewState = fPacificView;
    NFmiPoint bottomleft = BottomLeftLatLon();
    NFmiPoint topright = TopRightLatLon();
    bool createNewArea =
        NFmiAreaFactory::DoPossiblePacificFix(bottomleft, topright, usedPacificViewState);

    if (createNewArea)
    {
      NFmiArea *newArea = NewArea(bottomleft, topright);
      if (newArea)
      {
        newArea->PacificView(usedPacificViewState);
        return newArea;
      }
    }
  }
  return nullptr;
}

NFmiArea *NFmiArea::DoForcePacificFix() const
{
  // Joskus on pakko muuttaa atlantic-area pacific tyyppiseksi vaikka väkisin
  if (!fPacificView)
  {
    NFmiPoint bottomleftLatlon = BottomLeftLatLon();
    ::FixPacificLongitude(bottomleftLatlon);
    NFmiPoint toprightLatlon = TopRightLatLon();
    ::FixPacificLongitude(toprightLatlon);

    NFmiArea *newArea = NewArea(bottomleftLatlon, toprightLatlon, false);
    if (newArea)
    {
      newArea->PacificView(true);
      return newArea;
    }
  }
  return nullptr;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return hash value for the base NFmiArea components
 */
// ----------------------------------------------------------------------

std::size_t NFmiArea::HashValue() const
{
  std::size_t hash = itsXYRectArea.HashValue();
  boost::hash_combine(hash, boost::hash_value(fPacificView));
  boost::hash_combine(hash, boost::hash_value(WKT()));
  return hash;
}
