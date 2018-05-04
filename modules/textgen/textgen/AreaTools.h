// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::AreaTools
 */
// ======================================================================

#ifndef TEXTGEN_AREATOOLS_H
#define TEXTGEN_AREATOOLS_H

#include <calculator/Acceptor.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherPeriod.h>
#include <calculator/WeatherResult.h>
#include <calculator/AnalysisSources.h>
#include <calculator/WeatherParameter.h>
#include <calculator/UserWeatherSource.h>

#include <newbase/NFmiIndexMask.h>
#include <newbase/NFmiPoint.h>

#include <string>
#include <map>
#include <vector>

namespace TextGen
{
namespace AreaTools
{
enum forecast_area_id
{
  NO_AREA = 0x0,
  COASTAL_AREA = 0x1,
  INLAND_AREA = 0x2,
  FULL_AREA = 0x4
};

enum direction_id
{
  NORTH,
  SOUTH,
  EAST,
  WEST,
  NORTHEAST,
  SOUTHEAST,
  SOUTHWEST,
  NORTHWEST,
  NO_DIRECTION
};

bool isPartOfArea(const UserWeatherSource& theWeatherSource,
                  const WeatherArea& theWeatherArea1,
                  const WeatherArea& theWeatherArea2);

bool isPartOfArea(const WeatherArea& theWeatherArea1,
                  const std::string& theArea2SvgFile,
                  const std::string theQueryData);

bool isPartOfArea(const WeatherArea& theWeatherArea,
                  const AnalysisSources& theSources,
                  const WeatherParameter& theParameter,
                  const NFmiIndexMask& theIndexMask);

class Rect
{
 public:
  Rect() : m_topLeft(NFmiPoint(0.0, 0.0)), m_bottomRight(NFmiPoint(0.0, 0.0)) {}
  Rect(const Rect& theRect) : m_topLeft(theRect.m_topLeft), m_bottomRight(theRect.m_bottomRight) {}
  Rect(const double& topLeftX,
       const double& topLeftY,
       const double& bottomRightX,
       const double& bottomRightY);
  Rect(const std::vector<NFmiPoint*>& thePointVector);
  Rect(const NFmiPoint& topLeft, const NFmiPoint& bottomRight)
      : m_topLeft(topLeft), m_bottomRight(bottomRight)
  {
  }
  Rect(const WeatherArea& theWeatherArea);
  Rect(const AnalysisSources& theSources,
       const WeatherParameter& theParameter,
       const NFmiIndexMask& theIndexMask);

  float getHeightWidthRatio()
  {
    return ((m_topLeft.Y() - m_bottomRight.Y()) / (m_bottomRight.X() - m_topLeft.X()));
  }
  NFmiPoint getCenter() const;
  NFmiPoint getTopLeft() const { return m_topLeft; }
  NFmiPoint getBottomRight() const { return m_bottomRight; }
  NFmiPoint getBottomLeft() const { return NFmiPoint(m_topLeft.X(), m_bottomRight.Y()); }
  NFmiPoint getTopRight() const { return NFmiPoint(m_bottomRight.X(), m_topLeft.Y()); }
  void setTopLeft(const NFmiPoint& topLeft) { m_topLeft = topLeft; }
  void setBottomRight(const NFmiPoint& bottomRight) { m_bottomRight = bottomRight; }
  bool contains(const Rect& theRect) const;

  operator std::string()
  {
    std::string retval;

    double topLeftX = m_topLeft.X();
    double topLeftY = m_topLeft.Y();
    double bottomRightX = m_bottomRight.X();
    double bottomRightY = m_bottomRight.Y();

    std::ostringstream o;
    o << topLeftX << " " << topLeftY << ", " << bottomRightX << " " << bottomRightY;

    retval = "Rect(" + o.str() + ")";

    return retval;
  }

  Rect boundingRect(const Rect& theRect) const;
  Rect intersection(const Rect& theRect) const;
  double intersectionPercentage(const Rect& theRect) const;
  Rect subRect(const direction_id& theDirectionId) const;
  bool contains(const NFmiPoint& thePoint) const;
  double size() const;
  direction_id getHalfDirection(const NFmiPoint& thePoint) const;

 private:
  NFmiPoint m_topLeft;
  NFmiPoint m_bottomRight;
};

direction_id getDirection(const Rect& thePrimaryRect, const Rect& theSecondaryRect);

NFmiPoint getArealDistribution(const AnalysisSources& theSources,
                               const WeatherParameter& theParameter,
                               const WeatherArea& theArea,
                               const WeatherPeriod& thePeriod,
                               const Acceptor& theAcceptor,
                               WeatherResult& theNortEastShare,
                               WeatherResult& theSouthEastShare,
                               WeatherResult& theSouthWestShare,
                               WeatherResult& theNortWestShare);

void getArealDistribution(const std::vector<NFmiPoint*>& thePointVector,
                          std::map<direction_id, double>& theResultData);

std::string getDirectionString(const direction_id& theDirectionId);

}  // namespace AreaTools
}  // namespace TextGen

#endif  // TEXTGEN_AREATOOLS_H

// ======================================================================
