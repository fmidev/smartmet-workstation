#ifndef SUBMASKEXTRACTOR_H
#define SUBMASKEXTRACTOR_H

#include <calculator/AnalysisSources.h>
#include <calculator/WeatherParameter.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherPeriod.h>
#include <calculator/Acceptor.h>
#include "AreaTools.h"

#include <vector>

#include <newbase/NFmiIndexMask.h>
#include <newbase/NFmiPoint.h>
#include <newbase/NFmiGrid.h>
#include <newbase/NFmiSvgPath.h>

namespace TextGen
{
double GetLocationCoordinates(const AnalysisSources& theSources,
                              const WeatherParameter& theParameter,
                              const WeatherArea& theArea,
                              const WeatherPeriod& thePeriod,
                              const Acceptor& theAcceptor,
                              std::vector<NFmiPoint*>& theResultData);

double ExtractMask(const AnalysisSources& theSources,
                   const WeatherParameter& theParameter,
                   const WeatherArea& theArea,
                   const WeatherPeriod& thePeriod,
                   const Acceptor& theAcceptor,
                   NFmiIndexMask& theResultIndexMask);

const NFmiIndexMask MaskDirection(const NFmiGrid& theGrid,
                                  const WeatherArea& theArea,
                                  const AreaTools::direction_id& theDirectionId);

void PrintLatLon(const AnalysisSources& theSources,
                 const WeatherParameter& theParameter,
                 const NFmiIndexMask& theIndexMask);
}

#endif
