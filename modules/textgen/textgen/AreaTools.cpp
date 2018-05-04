// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace SeasonTools
 */
// ======================================================================
/*!
 * \namespace TextGen::SeasonTools
 *
 * \brief Utilities to determine the season of the given date
 *
 *
 */
// ======================================================================

#include "AreaTools.h"
#include "LandMaskSource.h"
#include <calculator/RegularMaskSource.h>
#include <calculator/ParameterAnalyzer.h>
#include <calculator/Settings.h>
#include <calculator/WeatherDataType.h>
#include "SubMaskExtractor.h"

#include <newbase/NFmiSvgTools.h>
#include <newbase/NFmiIndexMask.h>
#include <newbase/NFmiSvgPath.h>
#include <newbase/NFmiFastQueryInfo.h>

typedef TextGen::RegularMaskSource::mask_type mask_type;

using namespace std;
using namespace boost;

namespace TextGen
{
namespace AreaTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Tests if the are1 is part of are2
 *
 * \param theWeatherSource The source for weather data
 * \param theWeatherArea1 The weather area 1
 * \param theWeatherArea2 The weather area 2
 *
 * \return True if area 1 is part of area 2 false otherwise
 */
// ----------------------------------------------------------------------

bool isPartOfArea(const UserWeatherSource& theWeatherSource,
                  const WeatherArea& theWeatherArea1,
                  const WeatherArea& theWeatherArea2)
{
  if (theWeatherArea1.isPoint())
  {
    return theWeatherArea2.path().IsInside(theWeatherArea1.point());
  }
  else
  {
    LandMaskSource area1LandMaskSource(theWeatherArea1);

    mask_type mask = area1LandMaskSource.mask(theWeatherArea2, "data", theWeatherSource);

    int size_of_mask = mask->size();

    return size_of_mask > 0;
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Tests if the are1 is part of are2
 *
 * \param theWeatherArea1 The weather area 1
 * \param theArea2SvgFile The svg-file, that defines area 2
 * \param theQueryData The query data
 *
 * \return True if area 1 is part of area 2 false otherwise
 */
// ----------------------------------------------------------------------

bool isPartOfArea(const WeatherArea& theWeatherArea1,
                  const std::string& theArea2SvgFile,
                  const std::string& theQueryData)
{
  boost::shared_ptr<NFmiQueryData> sqd(new NFmiQueryData(theQueryData));

  UserWeatherSource theWeatherSource;
  theWeatherSource.insert("data", sqd);

  WeatherArea theWeatherArea2(theArea2SvgFile);

  return isPartOfArea(theWeatherSource, theWeatherArea1, theWeatherArea2);
}

bool isPartOfArea(const WeatherArea& theWeatherArea,
                  const AnalysisSources& theSources,
                  const WeatherParameter& theParameter,
                  const NFmiIndexMask& theIndexMask)
{
  std::string parameterName("");
  std::string dataName("");

  ParameterAnalyzer::getParameterStrings(theParameter, parameterName, dataName);
  const string default_forecast = Settings::optional_string("textgen::default_forecast", "");
  const string datavar = dataName + '_' + "forecast";
  const string dataname = Settings::optional_string(datavar, default_forecast);

  boost::shared_ptr<WeatherSource> wsource = theSources.getWeatherSource();
  boost::shared_ptr<NFmiQueryData> qd = wsource->data(dataname);
  NFmiFastQueryInfo theQI = NFmiFastQueryInfo(qd.get());

  for (NFmiIndexMask::const_iterator it = theIndexMask.begin(); it != theIndexMask.end(); ++it)
  {
    NFmiPoint point(theQI.LatLon(*it).X(), theQI.LatLon(*it).Y());
    if (!theWeatherArea.path().IsInside(point)) return false;
  }

  return true;
}

NFmiPoint getArealDistribution(const AnalysisSources& theSources,
                               const WeatherParameter& theParameter,
                               const WeatherArea& theArea,
                               const WeatherPeriod& thePeriod,
                               const Acceptor& theAcceptor,
                               WeatherResult& theNortEastShare,
                               WeatherResult& theSouthEastShare,
                               WeatherResult& theSouthWestShare,
                               WeatherResult& theNortWestShare)
{
  NFmiPoint retval(kFloatMissing, kFloatMissing);
  double latSum(0.0);
  double lonSum(0.0);

  std::string parameterName("");
  std::string dataName("");

  ParameterAnalyzer::getParameterStrings(theParameter, parameterName, dataName);
  const string default_forecast = Settings::optional_string("textgen::default_forecast", "");
  const string datavar = dataName + '_' + "forecast";
  const string dataname = Settings::optional_string(datavar, default_forecast);

  boost::shared_ptr<WeatherSource> wsource = theSources.getWeatherSource();
  boost::shared_ptr<NFmiQueryData> qd = wsource->data(dataname);
  NFmiFastQueryInfo theQI = NFmiFastQueryInfo(qd.get());

  NFmiIndexMask indexMask;

  ExtractMask(theSources, theParameter, theArea, thePeriod, theAcceptor, indexMask);

  vector<NFmiPoint*> latitudeLongitudeCoordinates;
  for (NFmiIndexMask::const_iterator it = indexMask.begin(); it != indexMask.end(); ++it)
  {
    lonSum += theQI.LatLon(*it).X();
    latSum += theQI.LatLon(*it).Y();
    latitudeLongitudeCoordinates.push_back(new NFmiPoint(theQI.LatLon(*it)));
  }

  if (!latitudeLongitudeCoordinates.empty())
  {
    map<direction_id, double> arealDistribution;
    getArealDistribution(latitudeLongitudeCoordinates, arealDistribution);
    theNortEastShare = WeatherResult(arealDistribution[NORTHEAST], 0);
    theSouthEastShare = WeatherResult(arealDistribution[SOUTHEAST], 0);
    theSouthWestShare = WeatherResult(arealDistribution[SOUTHWEST], 0);
    theNortWestShare = WeatherResult(arealDistribution[NORTHWEST], 0);
    latitudeLongitudeCoordinates.clear();
    retval.X(lonSum / indexMask.size());
    retval.Y(latSum / indexMask.size());
  }
  else
  {
    theNortEastShare = WeatherResult(0.0, 0);
    theSouthEastShare = WeatherResult(0.0, 0);
    theSouthWestShare = WeatherResult(0.0, 0);
    theNortWestShare = WeatherResult(0.0, 0);
  }

  return retval;
}

void getArealDistribution(const vector<NFmiPoint*>& thePointVector,
                          map<direction_id, double>& theResultData)
{
  Rect area(thePointVector);

  theResultData.insert(make_pair(NORTH, 0.0));
  theResultData.insert(make_pair(SOUTH, 0.0));
  theResultData.insert(make_pair(EAST, 0.0));
  theResultData.insert(make_pair(WEST, 0.0));
  theResultData.insert(make_pair(NORTHEAST, 0.0));
  theResultData.insert(make_pair(SOUTHEAST, 0.0));
  theResultData.insert(make_pair(SOUTHWEST, 0.0));
  theResultData.insert(make_pair(NORTHWEST, 0.0));
  theResultData.insert(make_pair(NO_DIRECTION, 0.0));

  unsigned int count = thePointVector.size();

  for (unsigned int i = 0; i < count; i++)
  {
    NFmiPoint thePoint(*(thePointVector[i]));
    theResultData[area.getHalfDirection(thePoint)] += 1.0;
  }

  theResultData[NORTH] += (theResultData[NORTHEAST] + theResultData[NORTHWEST]);
  theResultData[SOUTH] += (theResultData[SOUTHEAST] + theResultData[SOUTHWEST]);
  theResultData[EAST] += (theResultData[NORTHEAST] + theResultData[SOUTHEAST]);
  theResultData[WEST] += (theResultData[NORTHWEST] + theResultData[SOUTHWEST]);

  theResultData[NORTH] = (theResultData[NORTH] / count) * 100;
  theResultData[SOUTH] = (theResultData[SOUTH] / count) * 100;
  theResultData[EAST] = (theResultData[EAST] / count) * 100;
  theResultData[WEST] = (theResultData[WEST] / count) * 100;
  theResultData[NORTHEAST] = (theResultData[NORTHEAST] / count) * 100;
  theResultData[SOUTHEAST] = (theResultData[SOUTHEAST] / count) * 100;
  theResultData[SOUTHWEST] = (theResultData[SOUTHWEST] / count) * 100;
  theResultData[NORTHWEST] = (theResultData[NORTHWEST] / count) * 100;
  theResultData[NO_DIRECTION] = (theResultData[NO_DIRECTION] / count) * 100;
}

// returns direction of theSecondaryRect inside thePrimaryRect
// if theSecondaryRect is outside, even if only partly, thePrimaryRect NO_DIRECTION is returned
direction_id getDirection(const Rect& thePrimaryRect, const Rect& theSecondaryRect)
{
  if (!thePrimaryRect.contains(theSecondaryRect)) return NO_DIRECTION;

  NFmiPoint topLeftPrimary = thePrimaryRect.getTopLeft();
  NFmiPoint bottomRightPrimary = thePrimaryRect.getBottomRight();

  double centerCoordinateYPrimary =
      bottomRightPrimary.Y() + ((topLeftPrimary.Y() - bottomRightPrimary.Y()) / 2.0);
  double centerCoordinateXPrimary =
      topLeftPrimary.X() + ((bottomRightPrimary.X() - topLeftPrimary.X()) / 2.0);

  Rect northRect(topLeftPrimary, NFmiPoint(bottomRightPrimary.X(), centerCoordinateYPrimary));
  Rect southRect(NFmiPoint(topLeftPrimary.X(), centerCoordinateYPrimary), bottomRightPrimary);
  Rect eastRect(NFmiPoint(centerCoordinateXPrimary, topLeftPrimary.Y()), bottomRightPrimary);
  Rect westRect(topLeftPrimary, NFmiPoint(centerCoordinateXPrimary, bottomRightPrimary.Y()));

  if (northRect.contains(theSecondaryRect))
  {
    if (eastRect.contains(theSecondaryRect))
      return NORTHEAST;
    else if (westRect.contains(theSecondaryRect))
      return NORTHWEST;
    else
      return NORTH;
  }
  else if (southRect.contains(theSecondaryRect))
  {
    if (eastRect.contains(theSecondaryRect))
      return SOUTHEAST;
    else if (westRect.contains(theSecondaryRect))
      return SOUTHWEST;
    else
      return SOUTH;
  }
  else if (eastRect.contains(theSecondaryRect))
  {
    return EAST;
  }
  else if (westRect.contains(theSecondaryRect))
  {
    return WEST;
  }

  return NO_DIRECTION;
}

std::string getDirectionString(const direction_id& theDirectionId)
{
  std::string retval("void");

  switch (theDirectionId)
  {
    case NORTH:
      retval = "north";
      break;
    case SOUTH:
      retval = "south";
      break;
    case EAST:
      retval = "east";
      break;
    case WEST:
      retval = "west";
      break;
    case NORTHEAST:
      retval = "northeast";
      break;
    case SOUTHEAST:
      retval = "southeast";
      break;
    case SOUTHWEST:
      retval = "southwest";
      break;
    case NORTHWEST:
      retval = "northwest";
      break;
    case NO_DIRECTION:
      retval = "no direction";
      break;
  }

  return retval;
}

Rect::Rect(const vector<NFmiPoint*>& thePointVector)
{
  double xMin = 0.0, yMin = 0.0, xMax = 0.0, yMax = 0.0;
  for (unsigned int i = 0; i < thePointVector.size(); i++)
  {
    if (i == 0)
    {
      xMin = xMax = thePointVector.at(i)->X();
      yMin = yMax = thePointVector.at(i)->Y();
      continue;
    }
    if (xMin > thePointVector.at(i)->X()) xMin = thePointVector.at(i)->X();
    if (xMax < thePointVector.at(i)->X()) xMax = thePointVector.at(i)->X();
    if (yMin > thePointVector.at(i)->Y()) yMin = thePointVector.at(i)->Y();
    if (yMax < thePointVector.at(i)->Y()) yMax = thePointVector.at(i)->Y();
  }
  m_topLeft.X(xMin);
  m_topLeft.Y(yMax);
  m_bottomRight.X(xMax);
  m_bottomRight.Y(yMin);
}

Rect::Rect(const double& topLeftX,
           const double& topLeftY,
           const double& bottomRightX,
           const double& bottomRightY)
{
  m_topLeft.X(topLeftX);
  m_topLeft.Y(topLeftY);
  m_bottomRight.X(bottomRightX);
  m_bottomRight.Y(bottomRightY);
}

Rect::Rect(const WeatherArea& theWeatherArea)
{
  double xMin = 0, yMin = 0, xMax = 0, yMax = 0;

  NFmiSvgTools::BoundingBox(theWeatherArea.path(), xMin, yMin, xMax, yMax);

  m_topLeft.X(xMin);
  m_topLeft.Y(yMax);
  m_bottomRight.X(xMax);
  m_bottomRight.Y(yMin);
}

Rect::Rect(const AnalysisSources& theSources,
           const WeatherParameter& theParameter,
           const NFmiIndexMask& theIndexMask)
{
  if (theIndexMask.size() == 0)
  {
    m_topLeft.X(0);
    m_topLeft.Y(0);
    m_bottomRight.X(0);
    m_bottomRight.Y(0);
    return;
  }

  std::string parameterName;
  std::string dataName;

  ParameterAnalyzer::getParameterStrings(theParameter, parameterName, dataName);
  const string default_forecast = Settings::optional_string("textgen::default_forecast", "");
  const string datavar =
      dataName + '_' + "forecast";  // ParameterAnalyzer::getDataTypeName(TextGen::Forecast);
  const string dataname = Settings::optional_string(datavar, default_forecast);

  boost::shared_ptr<WeatherSource> wsource = theSources.getWeatherSource();
  boost::shared_ptr<NFmiQueryData> qd = wsource->data(dataname);
  NFmiFastQueryInfo theQI = NFmiFastQueryInfo(qd.get());

  double lon_min = 180.0;
  double lat_min = 90.0;
  double lon_max = 0.0;
  double lat_max = 0.0;

  for (NFmiIndexMask::const_iterator it = theIndexMask.begin(); it != theIndexMask.end(); ++it)
  {
    NFmiPoint point(theQI.LatLon(*it));
    if (lon_min > point.X()) lon_min = point.X();
    if (lon_max < point.X()) lon_max = point.X();
    if (lat_min > point.Y()) lat_min = point.Y();
    if (lat_max < point.Y()) lat_max = point.Y();
  }
  m_topLeft.X(lon_min);
  m_topLeft.Y(lat_max);
  m_bottomRight.X(lon_max);
  m_bottomRight.Y(lat_min);
}

Rect Rect::boundingRect(const Rect& theRect) const
{
  double minX = theRect.m_topLeft.X() < m_topLeft.X() ? theRect.m_topLeft.X() : m_topLeft.X();
  double maxX =
      theRect.m_bottomRight.X() > m_bottomRight.X() ? theRect.m_bottomRight.X() : m_bottomRight.X();
  double minY =
      theRect.m_bottomRight.Y() < m_bottomRight.Y() ? theRect.m_bottomRight.Y() : m_bottomRight.Y();
  double maxY = theRect.m_topLeft.Y() > m_topLeft.Y() ? theRect.m_topLeft.Y() : m_topLeft.Y();

  Rect retval(NFmiPoint(minX, maxY), NFmiPoint(maxX, minY));

  return retval;
}

Rect Rect::intersection(const Rect& theRect) const
{
  direction_id dirId = TextGen::AreaTools::getDirection(*this, theRect);

  Rect retval;

  if (dirId == NO_DIRECTION)
  {
    if (this->contains(theRect))  // inside, but on different directions
    {
      retval = theRect;
    }
    else
    {
      if (theRect.contains(*this))
      {
        retval = *this;
      }
      else  // intersects partially
      {
        double minX = theRect.m_topLeft.X() > m_topLeft.X() ? theRect.m_topLeft.X() : m_topLeft.X();

        double maxX = theRect.m_bottomRight.X() < m_bottomRight.X() ? theRect.m_bottomRight.X()
                                                                    : m_bottomRight.X();
        double minY = theRect.m_bottomRight.Y() > m_bottomRight.Y() ? theRect.m_bottomRight.Y()
                                                                    : m_bottomRight.Y();
        double maxY = theRect.m_topLeft.Y() < m_topLeft.Y() ? theRect.m_topLeft.Y() : m_topLeft.Y();

        retval.setTopLeft(NFmiPoint(minX, maxY));
        retval.setBottomRight(NFmiPoint(maxX, minY));
      }
    }
  }
  else
  {
    retval = theRect;
  }

  return retval;
}

double Rect::size() const
{
  double width = m_bottomRight.X() - m_topLeft.X();
  double height = m_topLeft.Y() - m_bottomRight.Y();

  return width * height;
}

double Rect::intersectionPercentage(const Rect& theRect) const
{
  Rect intersectionRect = intersection(theRect);

  return (size() == 0.0 ? 0.0 : (intersectionRect.size() / size()) * 100);
}

Rect Rect::subRect(const direction_id& theDirectionId) const
{
  Rect retval;

  double centerCoordinateY = m_bottomRight.Y() + ((m_topLeft.Y() - m_bottomRight.Y()) / 2.0);
  double centerCoordinateX = m_topLeft.X() + ((m_bottomRight.X() - m_topLeft.X()) / 2.0);

  switch (theDirectionId)
  {
    case NORTH:
    {
      retval.m_topLeft = m_topLeft;
      retval.m_bottomRight.X(m_bottomRight.X());
      retval.m_bottomRight.Y(centerCoordinateY);
    }
    break;
    case SOUTH:
    {
      retval.m_topLeft.X(m_topLeft.X());
      retval.m_topLeft.Y(centerCoordinateY);
      retval.m_bottomRight = m_bottomRight;
    }
    break;
    case EAST:
    {
      retval.m_topLeft.X(centerCoordinateX);
      retval.m_topLeft.Y(m_topLeft.Y());
      retval.m_bottomRight = m_bottomRight;
    }
    break;
    case WEST:
    {
      retval.m_topLeft = m_topLeft;
      retval.m_bottomRight.X(centerCoordinateX);
      retval.m_bottomRight.Y(m_bottomRight.Y());
    }
    break;
    case NORTHEAST:
    {
      retval.m_topLeft.X(centerCoordinateX);
      retval.m_topLeft.Y(m_topLeft.Y());
      retval.m_bottomRight.X(m_bottomRight.X());
      retval.m_bottomRight.Y(centerCoordinateY);
    }
    break;
    case SOUTHEAST:
    {
      retval.m_topLeft.X(centerCoordinateX);
      retval.m_topLeft.Y(centerCoordinateY);
      retval.m_bottomRight = m_bottomRight;
    }
    break;
    case SOUTHWEST:
    {
      retval.m_topLeft.X(m_topLeft.X());
      retval.m_topLeft.Y(centerCoordinateY);
      retval.m_bottomRight.X(centerCoordinateX);
      retval.m_bottomRight.Y(m_bottomRight.Y());
    }
    break;
    case NORTHWEST:
    {
      retval.m_topLeft = m_topLeft;
      retval.m_bottomRight.X(centerCoordinateX);
      retval.m_bottomRight.Y(centerCoordinateY);
    }
    break;
    case NO_DIRECTION:
      break;
  }
  return retval;
}

bool Rect::contains(const Rect& theRect) const
{
  if (size() == 0 || theRect.size() == 0) return false;

  return (m_topLeft.X() <= theRect.m_topLeft.X() &&
          m_bottomRight.X() >= theRect.m_bottomRight.X() &&
          m_topLeft.Y() >= theRect.m_topLeft.Y() && m_bottomRight.Y() <= theRect.m_bottomRight.Y());
}

bool Rect::contains(const NFmiPoint& thePoint) const
{
  return (m_topLeft.X() <= thePoint.X() && m_bottomRight.X() >= thePoint.X() &&
          m_topLeft.Y() >= thePoint.Y() && m_bottomRight.Y() <= thePoint.Y());
}

direction_id Rect::getHalfDirection(const NFmiPoint& thePoint) const
{
  if (!contains(thePoint)) return NO_DIRECTION;

  direction_id retval = NO_DIRECTION;

  if (subRect(NORTH).contains(thePoint))
  {
    if (subRect(NORTHEAST).contains(thePoint))
      retval = NORTHEAST;
    else
      retval = NORTHWEST;
  }
  else
  {
    if (subRect(SOUTHEAST).contains(thePoint))
      retval = SOUTHEAST;
    else
      retval = SOUTHWEST;
  }

  return retval;
}

NFmiPoint Rect::getCenter() const
{
  double xCoord = m_topLeft.X() + (m_bottomRight.X() - m_topLeft.X()) / 2.0;
  double yCoord = m_bottomRight.Y() + (m_topLeft.Y() - m_bottomRight.Y()) / 2.0;

  return NFmiPoint(xCoord, yCoord);
}

}  // namespace AreaTools
}  // namespace TextGen
// ======================================================================
