// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::WeatherArea
 */
// ======================================================================
/*!
 * \class TextGen::WeatherArea
 *
 * \brief Representation of an area to be analyzed
 *
 * The WeatherArea class represents either a point location or
 * a polygonal area. The area is described by a generic path.
 *
 * Instances are generally created from textual descriptions
 * of the form
 *
 *    -# \c filename - a path to a SVG polygon
 *    -# \c filename:radius - a path to a SVG polygon plus expansion radius
 *    -# \c location - a location name
 *    -# \c location:radius - a location name plus expansion radius
 *    -# \c lon,lat - location coordinates
 *    -# \c lon,lat:radius - location coordinates plus expansion radius
 *
 * Additionally the area may be given a name for text generation
 * purposes.
 *
 * For convenience a searchpath textgen::mappath will be used for the SVG polygons,
 * and a .svg suffix will be tested if necessary.
 *
 * Usage examples:
 *
 * \code
 * WeatherArea area(specifications);
 *
 * if(area.isPoint())
 * {
 *   NFmiPoint lonlat = area.point();
 *   ...
 * }
 * else
 * {
 *   NFmiSvgPath path = area.path();
 *   float radius = area.radius();
 *   ...
 * }
 * \endcode
 *
 * Note that the path may consist of a single moveto followed by
 * a closepath, but in that case the radius is always positive to
 * indicate a circular area around the moveto point.
 *
 * Each WeatherArea may also be marked to be a subarea of the
 * actual specified area. For example, one may specify that
 * the actual area is the coastal area within the area polygon.
 * It is then up to the user to utilize this information, for
 * example by choosing to use CoastMaskSource instead of RegularMaskSource.
 *
 * Implementation note:
 *
 * Each WeatherArea will contain a sort key based on the
 * arguments given in the constructor. This allows one to place
 * the objects into an associative container.
 *
 */
// ======================================================================

#include "WeatherArea.h"
#include "LocationSource.h"
#include "Settings.h"
#include "TextGenError.h"
#include <newbase/NFmiFileSystem.h>
#include <newbase/NFmiStringTools.h>
#include <fstream>

using namespace std;

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Create a path for a point
 *
 * Note that the closepath is required so that NFmiIndexMask
 * will calculate distances from the edges (a null edge here)
 * properly.
 */
// ----------------------------------------------------------------------

void make_point_path(NFmiSvgPath& thePath, const NFmiPoint& thePoint)
{
  NFmiSvgPath::Element element1(NFmiSvgPath::kElementMoveto, thePoint.X(), thePoint.Y());
  NFmiSvgPath::Element element2(NFmiSvgPath::kElementClosePath, 0, 0);
  thePath.push_back(element1);
  thePath.push_back(element2);
}
}

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * Construct from text description and name. The allowed descriptions
 * are of the forms
 *
 * \param theSpecs The area specification
 * \param theName The name of the area
 */
// ----------------------------------------------------------------------

WeatherArea::WeatherArea(const std::string& theSpecs, const std::string& theName)
    : itsType(Full),
      itsPointFlag(),
      itsNamedFlag(true),
      itsName(theName),
      itsPoint(kFloatMissing, kFloatMissing),
      itsPolygon(new NFmiSvgPath()),
      itsRadius(0),
      itsSortKey(theName + '=' + theSpecs),
      itsLatitude(90.0),
      itsLongitude(180.0),
      itsBooleanParameters(0x0),
      itsHistory()
{
  parse_specs(theSpecs);
  set_boolean_parameters();
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * Construct from text description. The allowed descriptions are of the
 * forms. The WeatherArea will not have a name, even though some
 * specs might imply one.
 *
 * \param theSpecs The area specification
 */
// ----------------------------------------------------------------------

WeatherArea::WeatherArea(const std::string& theSpecs)
    : itsType(Full),
      itsPointFlag(),
      itsNamedFlag(false),
      itsName(),
      itsPoint(kFloatMissing, kFloatMissing),
      itsPolygon(new NFmiSvgPath()),
      itsRadius(0),
      itsSortKey(theSpecs),
      itsLatitude(90.0),
      itsLongitude(180.0),
      itsBooleanParameters(0x0),
      itsHistory()
{
  parse_specs(theSpecs);
  set_boolean_parameters();
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * Construct from explicit coordinates.
 *
 * \param thePoint The coordinate of the weather point
 * \param theRadius The expansion radius
 */
// ----------------------------------------------------------------------

WeatherArea::WeatherArea(const NFmiPoint& thePoint, float theRadius)
    : itsType(Full),
      itsPointFlag(theRadius == 0),
      itsNamedFlag(false),
      itsName(),
      itsPoint(thePoint),
      itsPolygon(new NFmiSvgPath()),
      itsRadius(theRadius),
      itsSortKey(NFmiStringTools::Convert(thePoint.X()) + ',' +
                 NFmiStringTools::Convert(thePoint.Y()) +
                 (theRadius == 0 ? "" : ":" + NFmiStringTools::Convert(theRadius))),
      itsLatitude(90.0),
      itsLongitude(180.0),
      itsBooleanParameters(0x0),
      itsHistory()
{
  if (theRadius < 0) throw TextGenError("A weather point cannot have a negative expansion radius");
  if (theRadius > 0) make_point_path(*itsPolygon, thePoint);
  set_boolean_parameters();
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * Construct named point from explicit coordinates
 *
 * \param thePoint The coordinate of the weather point
 * \param theName The name of the coordinate point
 * \param theRadius The expansion radius
 */
// ----------------------------------------------------------------------

WeatherArea::WeatherArea(const NFmiPoint& thePoint, const std::string& theName, float theRadius)
    : itsType(Full),
      itsPointFlag(theRadius == 0),
      itsNamedFlag(true),
      itsName(theName),
      itsPoint(thePoint),
      itsPolygon(new NFmiSvgPath()),
      itsRadius(theRadius),
      itsSortKey(theName + '=' + NFmiStringTools::Convert(thePoint.X()) + ',' +
                 NFmiStringTools::Convert(thePoint.Y()) +
                 (theRadius == 0 ? "" : ":" + NFmiStringTools::Convert(theRadius))),
      itsLatitude(90.0),
      itsLongitude(180.0),
      itsBooleanParameters(0x0),
      itsHistory()
{
  if (theRadius < 0) throw TextGenError("A weather point cannot have a negative expansion radius");
  if (theRadius > 0) make_point_path(*itsPolygon, thePoint);
  set_boolean_parameters();
}

WeatherArea::WeatherArea(const WeatherArea& theArea)
{
  itsType = theArea.itsType;
  itsPointFlag = theArea.itsPointFlag;
  itsNamedFlag = theArea.itsNamedFlag;
  itsName = theArea.itsName;
  itsPoint = theArea.itsPoint;
  itsPolygon = theArea.itsPolygon;
  itsRadius = theArea.itsRadius;
  itsSortKey = theArea.itsSortKey;
  itsLatitude = theArea.itsLatitude;
  itsLongitude = theArea.itsLongitude;
  itsBooleanParameters = theArea.itsBooleanParameters;
  itsHistory = theArea.itsHistory;
}

WeatherArea::WeatherArea(const NFmiSvgPath& theSvgPath, const std::string& theName)
    : itsType(Full),
      itsPointFlag(),
      itsNamedFlag(true),
      itsName(theName),
      itsPoint(kFloatMissing, kFloatMissing),
      itsPolygon(new NFmiSvgPath(theSvgPath)),
      itsRadius(0),
      itsSortKey(theName),
      itsLatitude(90.0),
      itsLongitude(180.0),
      itsBooleanParameters(0x0),
      itsHistory()
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the area is really just a point
 *
 * \return True if the area is just a point
 */
// ----------------------------------------------------------------------

bool WeatherArea::isPoint() const { return itsPointFlag; }
// ----------------------------------------------------------------------
/*!
 * \brief Test if the area has a name
 *
 * \return True if the area has a name
 */
// ----------------------------------------------------------------------

bool WeatherArea::isNamed() const { return itsNamedFlag; }
// ----------------------------------------------------------------------
/*!
 * \brief Name accessor
 *
 * \return The area name
 */
// ----------------------------------------------------------------------

const std::string& WeatherArea::name() const
{
  if (itsNamedFlag) return itsName;
  throw TextGenError("Trying to access name of unnamed weather area");
}

// ----------------------------------------------------------------------
/*!
 * \brief Coordinate accessor
 *
 * \return The coordinate
 */
// ----------------------------------------------------------------------

const NFmiPoint& WeatherArea::point() const
{
  if (itsPointFlag) return itsPoint;
  throw TextGenError("Trying to access coordinate of polygonal weather area");
}

// ----------------------------------------------------------------------
/*!
 * \brief Path accessor
 *
 * \return The path
 */
// ----------------------------------------------------------------------

const NFmiSvgPath& WeatherArea::path() const
{
  if (itsPointFlag) throw TextGenError("Trying to access path of a point");
  if (itsPolygon.get() == 0) throw TextGenError("Internal polygon allocation error in WeatherArea");
  return *itsPolygon;
}

// ----------------------------------------------------------------------
/*!
 * \brief Radius accessor
 *
 * \return The radius
 */
// ----------------------------------------------------------------------

float WeatherArea::radius() const { return itsRadius; }
// ----------------------------------------------------------------------
/*!
 * \brief Type accessor
 *
 * \return The type of the desired area
 */
// ----------------------------------------------------------------------

WeatherArea::Type WeatherArea::type() const { return itsType; }
// ----------------------------------------------------------------------
/*!
 * \brief Change the type of the area
 *
 * \param theType The desired area subtype
 */
// ----------------------------------------------------------------------

void WeatherArea::type(Type theType) { itsType = theType; }
// ----------------------------------------------------------------------
/*!
 * \brief Parse a text description of a WeatherArea
 *
 * The allowed descriptions are of the forms
 *
 *    -# \c filename - a path to a SVG polygon
 *    -# \c filename:radius - a path to a SVG polygon plus expansion radius
 *    -# \c location - a location name
 *    -# \c location:radius - a location name plus expansion radius
 *    -# \c lon,lat - location coordinates
 *    -# \c lon,lat:radius - location coordinates plus expansion radius
 *
 */
// ----------------------------------------------------------------------

void WeatherArea::parse_specs(const std::string& theSpecs)
{
  if (theSpecs.empty())
    throw TextGenError("Trying to construct WeatherArea from empty string description");

  vector<string> words = NFmiStringTools::Split(theSpecs, ":");
  if (words.size() > 2)
    throw TextGenError("Too many ':' characters in WeatherArea specification '" + theSpecs + "'");

  // Parse the radius part

  if (words.size() == 2)
  {
    try
    {
      itsRadius = NFmiStringTools::Convert<float>(words[1]);
    }
    catch (...)
    {
      throw TextGenError("Expecting a valid radius after the ':' character in WeatherArea '" +
                         theSpecs + "'");
    }
  }

  // Parse the location part

  const string& spec = words[0];

  const string searchpath = Settings::optional_string("textgen::mappath", ".");

  string filename = spec;
  if (NFmiFileSystem::FileExists(filename))
  {
  }
  else if (NFmiFileSystem::FileExists(filename = spec + ".svg"))
  {
  }
  else if (NFmiFileSystem::FileExists(filename = searchpath + '/' + spec))
  {
  }
  else if (NFmiFileSystem::FileExists(filename = searchpath + '/' + spec + ".svg"))
  {
  }

  if (NFmiFileSystem::FileExists(filename))
  {
    itsPointFlag = false;

    ifstream in(filename.c_str(), ios::in);
    if (!in) throw TextGenError("Could not open map file '" + filename + "' for reading");
    in >> *itsPolygon;
    in.close();

    if (itsPolygon->empty())
      throw TextGenError("Map file '" + filename + "' does not contain an acceptable SVG path");
    return;
  }

  // Not a polygon - must be pointlike then

  if (itsRadius < 0)
    throw TextGenError("Location '" + spec + "' cannot have negative expansion radius");

  if (LocationSource::instance().hasCoordinates(spec))
  {
    itsPoint = LocationSource::instance().coordinates(spec);
  }
  else
  {
    try
    {
      vector<float> tmp = NFmiStringTools::Split<vector<float> >(spec);
      itsPoint = NFmiPoint(tmp[0], tmp[1]);
    }
    catch (...)
    {
      throw TextGenError("Location '" + spec + "' has no known coordinates");
    }
  }

  itsPointFlag = (itsRadius == 0);
  if (!itsPointFlag) make_point_path(*itsPolygon, itsPoint);
}

// ----------------------------------------------------------------------
/*!
 * \brief Lexical less-than comparison for TextGen::WeatherArea
 *
 * This is implemented solely for the benefit of putting WeatherArea
 * objects into standard associative containers. For example, MaskSource
 * objects need a std::map mapping a WeatherArea to a NFmiIndexMask
 * or a NFmiIndexMaskSource object.
 *
 * \param theOther The area to compare with
 * \return True if \c this is lexicographically less than theOther
 */
// ----------------------------------------------------------------------

bool WeatherArea::operator<(const WeatherArea& theOther) const
{
  return itsSortKey < theOther.itsSortKey;
}

// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison for TextGen::WeatherArea
 *
 * \param theOther The area to compare with
 * \return True if \c this is lexicographically less than theOther
 */
// ----------------------------------------------------------------------

bool WeatherArea::operator==(const WeatherArea& theOther) const
{
  return itsSortKey == theOther.itsSortKey;
}

// ----------------------------------------------------------------------
/*!
 * \brief Inequality comparison for TextGen::WeatherArea
 *
 * \param theOther The area to compare with
 * \return True if \c this is lexicographically less than theOther
 */
// ----------------------------------------------------------------------

bool WeatherArea::operator!=(const WeatherArea& theOther) const
{
  return itsSortKey != theOther.itsSortKey;
}

// ----------------------------------------------------------------------
/*!
 * \brief Equality comparison for TextGen::WeatherArea
 *
 * \param theOther The area to compare with
 * \return True if \c this is lexicographically less than theOther
 */
// ----------------------------------------------------------------------

bool WeatherArea::identicalArea(const WeatherArea& theOther) const
{
  return (itsType == theOther.itsType && itsPointFlag == theOther.itsPointFlag &&
          itsNamedFlag == theOther.itsNamedFlag && itsName == theOther.itsName &&
          itsPoint == theOther.itsPoint && itsPolygon == theOther.itsPolygon &&
          itsRadius == theOther.itsRadius && itsSortKey == theOther.itsSortKey &&
          itsLatitude == theOther.itsLatitude && itsLongitude == theOther.itsLongitude);
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the east-west division line for the area.
 *
 * If the area is split vertically this value is used: eastern part is
 * east of and including the given longitude and western part is
 * west of the given longitude.
 *
 * \param theLongitude The longitude to be used when the area is split to eastern and western part
 */
// ----------------------------------------------------------------------
void WeatherArea::setLongitudeDivisionLine(const double& theLongitude)
{
  itsLongitude = theLongitude;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the north-south division line for the area.
 *
 * If the area is split horizontally this value is used: northern part is
 * north of and including the given latitude and southern part is
 * south of the given latitude.
 *
 * \param theLatitude The latitude to be used when the area is split to northern and southern part
 */
// ----------------------------------------------------------------------
void WeatherArea::setLatitudeDivisionLine(const double& theLatitude) { itsLatitude = theLatitude; }
bool WeatherArea::booleanParameterValue(const ParameterId& parameterId)
{
  bool retval(false);

  string parameterName("");

  switch (parameterId)
  {
    case WeatherArea::Marine:
      parameterName = "marine";
      break;
    case WeatherArea::Mountain:
      parameterName = "mountain";
      break;
    case WeatherArea::Island:
      parameterName = "island";
      break;
  }

  if (!parameterName.empty())
  {
    if (Settings::isset("qdtext::areas::" + parameterName + "::" + itsName))
      retval = Settings::require_bool("qdtext::areas::" + parameterName + "::" + itsName);
    else
      retval = Settings::optional_bool("qdtext::areas::" + parameterName + "::default", false);
  }

  return retval;
}

void WeatherArea::set_boolean_parameters()
{
  itsBooleanParameters |= (booleanParameterValue(Marine) ? Marine : 0x0);
  itsBooleanParameters |= (booleanParameterValue(Mountain) ? Mountain : 0x0);
  itsBooleanParameters |= (booleanParameterValue(Island) ? Island : 0x0);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns true if the corresponding boolean parameter is true, false otherwise.
 *
 * \param parameterId The parameter of which we are interested in
 */
// ----------------------------------------------------------------------
bool WeatherArea::booleanParameter(const ParameterId& parameterId) const
{
  return (itsBooleanParameters & parameterId);
}

// ----------------------------------------------------------------------
/*!
 * \brief Returns true if the area is sea area, false otherwise
 *
 */
// ----------------------------------------------------------------------
bool WeatherArea::isMarine() const { return (itsBooleanParameters & Marine); }
// ----------------------------------------------------------------------
/*!
 * \brief Returns true if the area is mountain area, false otherwise
 *
 */
// ----------------------------------------------------------------------
bool WeatherArea::isMountain() const { return (itsBooleanParameters & Mountain); }
// ----------------------------------------------------------------------
/*!
 * \brief Returns true if the area is island, false otherwise
 *
 */
// ----------------------------------------------------------------------
bool WeatherArea::isIsland() const { return (itsBooleanParameters & Island); }
}  // namespace TextGen

// ======================================================================
