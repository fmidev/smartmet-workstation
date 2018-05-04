#include <calculator/RegularFunctionAnalyzer.h>

#include <calculator/ParameterAnalyzer.h>
#include <calculator/QueryDataIntegrator.h>
#include <calculator/CalculatorFactory.h>
#include <calculator/MaskSource.h>
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/WeatherResult.h>
#include <calculator/WeatherSource.h>
#include <calculator/QueryDataTools.h>
#include "SubMaskExtractor.h"

#include <newbase/NFmiNearTree.h>
#include <newbase/NFmiEnumConverter.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiQueryData.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <newbase/NFmiIndexMaskSource.h>
#include <calculator/TextGenPosixTime.h>
#include <newbase/NFmiMetMath.h>
#include <newbase/NFmiSvgTools.h>
#include <cassert>

#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <sstream>

using namespace std;
using namespace boost;

// ----------------------------------------------------------------------
/*!
 * \brief Integrate over space
 *
 * Integrate over space with current time, parameter and level.
 *
 * It is assumed that the index mask was generated from a grid
 * equivalent to the one in the query data.
 *
 * \param theQI The query info
 * \param theIndexMask The mask containing the space indices
 * \param theSpaceCalculator The modifier for accumulating the result
 * \return The accumulation result
 */
// ----------------------------------------------------------------------

namespace
{
NFmiEnumConverter converter;
}  // namespace anonymous

namespace TextGen
{
double GetLocationCoordinates(const AnalysisSources& theSources,
                              const WeatherParameter& theParameter,
                              const WeatherArea& theArea,
                              const WeatherPeriod& thePeriod,
                              const Acceptor& theAcceptor,
                              vector<NFmiPoint*>& theResultData)
{
  double retval = 0.0;

  std::string parameterName;
  std::string dataName;

  ParameterAnalyzer::getParameterStrings(theParameter, parameterName, dataName);
  const string default_forecast = Settings::optional_string("textgen::default_forecast", "");
  const string datavar = dataName + '_' + data_type_name(Forecast);
  const string dataname = Settings::optional_string(datavar, default_forecast);

  // Get the data into use

  boost::shared_ptr<WeatherSource> wsource = theSources.getWeatherSource();
  boost::shared_ptr<NFmiQueryData> qd = wsource->data(dataname);
  NFmiFastQueryInfo theQI = NFmiFastQueryInfo(qd.get());

  FmiParameterName param = FmiParameterName(converter.ToEnum(parameterName));
  if (param == kFmiBadParameter)
    throw TextGenError("Parameter " + parameterName + " is not defined in newbase");

  if (!theQI.Param(param)) throw TextGenError(parameterName + " is not available in " + dataname);

  if (!theArea.isPoint())
  {
    MaskSource::mask_type theIndexMask;

    switch (theArea.type())
    {
      case WeatherArea::Full:
        theIndexMask = theSources.getMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Land:
        theIndexMask = theSources.getLandMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Coast:
        theIndexMask = theSources.getCoastMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Inland:
        theIndexMask = theSources.getInlandMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Northern:
        theIndexMask = theSources.getNorthernMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Southern:
        theIndexMask = theSources.getSouthernMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Eastern:
        theIndexMask = theSources.getEasternMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Western:
        theIndexMask = theSources.getWesternMaskSource()->mask(theArea, dataname, *wsource);
        break;
    }

    if (theIndexMask->empty()) return 0;

    unsigned long startindex, endindex;

    if (!QueryDataTools::findIndices(
            theQI, thePeriod.utcStartTime(), thePeriod.utcEndTime(), startindex, endindex))
    {
      return 0;
    }

    for (NFmiIndexMask::const_iterator it = theIndexMask->begin(); it != theIndexMask->end(); ++it)
    {
      theQI.TimeIndex(startindex);

      do
      {
        // possible -1 is handled by IndexFloatValue
        const unsigned long idx =
            theQI.Index(theQI.ParamIndex(), *it, theQI.LevelIndex(), theQI.TimeIndex());
        const float tmp = theQI.GetFloatValue(idx);

        if (theAcceptor.accept(tmp))
        {
          theResultData.push_back(new NFmiPoint(theQI.LatLon(*it)));
          retval += tmp;
        }
      } while (theQI.NextTime() && theQI.TimeIndex() < endindex);
    }
  }

  return retval;
}

double ExtractMask(const AnalysisSources& theSources,
                   const WeatherParameter& theParameter,
                   const WeatherArea& theArea,
                   const WeatherPeriod& thePeriod,
                   const Acceptor& theAcceptor,
                   NFmiIndexMask& theResultIndexMask)
{
  double retval = 0.0;

  std::string parameterName;
  std::string dataName;

  ParameterAnalyzer::getParameterStrings(theParameter, parameterName, dataName);
  const string default_forecast = Settings::optional_string("textgen::default_forecast", "");
  const string datavar = dataName + '_' + data_type_name(Forecast);
  const string dataname = Settings::optional_string(datavar, default_forecast);

  // Get the data into use

  boost::shared_ptr<WeatherSource> wsource = theSources.getWeatherSource();
  boost::shared_ptr<NFmiQueryData> qd = wsource->data(dataname);
  NFmiFastQueryInfo theQI = NFmiFastQueryInfo(qd.get());

  FmiParameterName param = FmiParameterName(converter.ToEnum(parameterName));
  if (param == kFmiBadParameter)
    throw TextGenError("Parameter " + parameterName + " is not defined in newbase");

  if (!theQI.Param(param)) throw TextGenError(parameterName + " is not available in " + dataname);

  if (!theArea.isPoint())
  {
    MaskSource::mask_type theIndexMask;

    switch (theArea.type())
    {
      case WeatherArea::Full:
        theIndexMask = theSources.getMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Land:
        theIndexMask = theSources.getLandMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Coast:
        theIndexMask = theSources.getCoastMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Inland:
        theIndexMask = theSources.getInlandMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Northern:
        theIndexMask = theSources.getNorthernMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Southern:
        theIndexMask = theSources.getSouthernMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Eastern:
        theIndexMask = theSources.getEasternMaskSource()->mask(theArea, dataname, *wsource);
        break;
      case WeatherArea::Western:
        theIndexMask = theSources.getWesternMaskSource()->mask(theArea, dataname, *wsource);
        break;
    }

    if (theIndexMask->empty()) return 0;

    unsigned long startindex, endindex;

    if (!QueryDataTools::findIndices(
            theQI, thePeriod.utcStartTime(), thePeriod.utcEndTime(), startindex, endindex))
      return 0;

    for (NFmiIndexMask::const_iterator it = theIndexMask->begin(); it != theIndexMask->end(); ++it)
    {
      theQI.TimeIndex(startindex);

      do
      {
        // possible -1 is handled by IndexFloatValue
        const unsigned long idx =
            theQI.Index(theQI.ParamIndex(), *it, theQI.LevelIndex(), theQI.TimeIndex());
        const float tmp = theQI.GetFloatValue(idx);

        if (theAcceptor.accept(tmp))
        {
          theResultIndexMask.insert(*it);
          retval += tmp;
        }
      } while (theQI.NextTime() && theQI.TimeIndex() < endindex);
    }
  }

  return retval;
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert a line into the given NFmiNearTree
 *
 * This is used when inserting a NFmiSvgPath into the NFmiNearTree
 * with some fixed resolution.
 *
 * \param theTree The tree to insert the data into
 * \param theStart The starting X-coordinate
 * \param theEnd The end X-coordinate
 * \param theResolution The maximum allowed edge distance
 */
// ----------------------------------------------------------------------

void Insert(NFmiNearTree<NFmiPoint>& theTree,
            const NFmiPoint& theStart,
            const NFmiPoint& theEnd,
            double theResolution)
{
  // Safety against infinite recursion
  if (theResolution <= 0)
  {
    theTree.Insert(theStart);
    theTree.Insert(theEnd);
  }
  else
  {
    // if edge length is small enough, stop recursion
    const double dist = theStart.Distance(theEnd);
    if (dist <= theResolution)
    {
      theTree.Insert(theStart);
      theTree.Insert(theEnd);
    }
    else
    {
      // subdivide and recurse
      NFmiPoint mid((theStart.X() + theEnd.X()) / 2, (theStart.Y() + theEnd.Y()) / 2);
      Insert(theTree, theStart, mid, theResolution);
      Insert(theTree, theEnd, mid, theResolution);
    }
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Insert the SVG path into the given NFmiNearTree
 *
 * The purpose here is to provide means for calculating
 * the distance of some point from the NFmiSvgPath. The user
 * is expected to provide some suitable resolution for
 * subdividing too long edges into more vertices.
 *
 * \param theTree The tree to insert the data into
 * \param thePath The path to insert
 * \param theResolution The maximum allowed edge distance
 */
// ----------------------------------------------------------------------

void Insert(NFmiNearTree<NFmiPoint>& theTree, const NFmiSvgPath& thePath, double theResolution)
{
  if (thePath.empty()) return;

  NFmiPoint firstPoint(thePath.front().itsX, thePath.front().itsY);

  NFmiPoint lastPoint(0, 0);

  for (NFmiSvgPath::const_iterator it = thePath.begin(); it != thePath.end(); ++it)
  {
    switch (it->itsType)
    {
      case NFmiSvgPath::kElementMoveto:
        lastPoint.Set(it->itsX, it->itsY);
        firstPoint = lastPoint;
        break;
      case NFmiSvgPath::kElementClosePath:
      {
        Insert(theTree, lastPoint, firstPoint, theResolution);
        lastPoint = firstPoint;
        break;
      }
      case NFmiSvgPath::kElementLineto:
      {
        NFmiPoint nextPoint(it->itsX, it->itsY);
        Insert(theTree, lastPoint, nextPoint, theResolution);
        lastPoint = nextPoint;
        break;
      }
      case NFmiSvgPath::kElementNotValid:
        return;
    }
  }
}

const NFmiIndexMask MaskDirection(const NFmiGrid& theGrid,
                                  const WeatherArea& theArea,
                                  const AreaTools::direction_id& theDirectionId)
{
  NFmiIndexMask mask;

  const NFmiSvgPath svgPath = theArea.path();

  const double resolution_factor = 1.0 / 4;

  // Establish grid resolution

  const double dx = theGrid.Area()->WorldXYWidth() / theGrid.XNumber();
  const double dy = theGrid.Area()->WorldXYHeight() / theGrid.YNumber();

  // Fast lookup tree for distance calculations

  NFmiSvgPath projectedPath(svgPath);
  NFmiSvgTools::LatLonToWorldXY(projectedPath, *theGrid.Area());
  NFmiNearTree<NFmiPoint> tree;
  Insert(tree, projectedPath, FmiMin(dx, dy) * resolution_factor);

  //
  double theXmin, theYmin, theXmax, theYmax;

  NFmiSvgTools::BoundingBox(svgPath, theXmin, theYmin, theXmax, theYmax);

  double latitudeDivisionLine =
      (theArea.latitudeDivisionLineSet() ? theArea.getLatitudeDivisionLine()
                                         : theYmin + ((theYmax - theYmin) / 2.0));

  double longitudeDivisionLine =
      (theArea.longitudeDivisionLineSet() ? theArea.getLongitudeDivisionLine()
                                          : theXmin + ((theXmax - theXmin) / 2.0));

  // Non-optimal solution loops through the entire grid

  const unsigned long nx = theGrid.XNumber();
  const unsigned long ny = theGrid.YNumber();

  if (!svgPath.empty())
  {
    for (unsigned long j = 0; j < ny; j++)
      for (unsigned long i = 0; i < nx; i++)
      {
        const unsigned long idx = j * nx + i;
        const NFmiPoint p = theGrid.LatLon(idx);

        // const NFmiPoint xy = theGrid.GridToWorldXY(i,j);

        if (NFmiSvgTools::IsInside(svgPath, p))
        {
          bool insert = false;
          switch (theDirectionId)
          {
            case AreaTools::NORTH:
              insert = p.Y() >= latitudeDivisionLine;
              break;
            case AreaTools::SOUTH:
              insert = p.Y() < latitudeDivisionLine;
              break;
            case AreaTools::EAST:
              insert = p.X() >= longitudeDivisionLine;
              break;
            case AreaTools::WEST:
              insert = p.X() < longitudeDivisionLine;
              break;
            case AreaTools::NORTHEAST:
              insert = p.Y() >= latitudeDivisionLine && p.X() >= longitudeDivisionLine;
              break;
            case AreaTools::SOUTHEAST:
              insert = p.Y() < latitudeDivisionLine && p.X() >= longitudeDivisionLine;
              break;
            case AreaTools::NORTHWEST:
              insert = p.Y() >= latitudeDivisionLine && p.X() < longitudeDivisionLine;
              break;
            case AreaTools::SOUTHWEST:
              insert = p.Y() < latitudeDivisionLine && p.X() < longitudeDivisionLine;
              break;
            default:
              insert = false;
              break;
          }

          if (insert) mask.insert(idx);
        }
      }
  }

  return mask;
}

void PrintLatLon(const AnalysisSources& theSources,
                 const WeatherParameter& theParameter,
                 const NFmiIndexMask& theIndexMask)
{
  std::string parameterName;
  std::string dataName;

  ParameterAnalyzer::getParameterStrings(theParameter, parameterName, dataName);
  const string default_forecast = Settings::optional_string("textgen::default_forecast", "");
  const string datavar = dataName + '_' + data_type_name(Forecast);
  const string dataname = Settings::optional_string(datavar, default_forecast);

  boost::shared_ptr<WeatherSource> wsource = theSources.getWeatherSource();
  boost::shared_ptr<NFmiQueryData> qd = wsource->data(dataname);
  NFmiFastQueryInfo theQI = NFmiFastQueryInfo(qd.get());

  for (NFmiIndexMask::const_iterator it = theIndexMask.begin(); it != theIndexMask.end(); ++it)
  {
    cout << theQI.LatLon(*it);
  }
}
}  // namespace
