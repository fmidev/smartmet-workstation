// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace ClimatologyTools
 */
// ======================================================================
/*!
 * \namespace TextGen::ClimatologyTools
 *
 * \brief Utilities to determine the season of the given date
 *
 *
 */
// ======================================================================

#include "ClimatologyTools.h"
#include <calculator/WeatherSource.h>
#include <calculator/Settings.h>

#include <newbase/NFmiQueryData.h>
#include <newbase/NFmiFastQueryInfo.h>
#include <boost/shared_ptr.hpp>

namespace TextGen
{
namespace ClimatologyTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Returns a climatology period of the data
 *
 * \param thePeriod Analysis period
 * \param theDataName The name of the data file
 * \param theSources Analysis sources
 *
 * \return The climatology period
 */
// ----------------------------------------------------------------------

WeatherPeriod getClimatologyPeriod(const TextGen::WeatherPeriod& thePeriod,
                                   const std::string& theDataName,
                                   const TextGen::AnalysisSources& theSources)
{
  const std::string datavar = theDataName + "_climatology";
  const std::string dataname = Settings::require_string(datavar);
  boost::shared_ptr<WeatherSource> wsource = theSources.getWeatherSource();
  boost::shared_ptr<NFmiQueryData> qd = wsource->data(dataname);
  NFmiFastQueryInfo qi = NFmiFastQueryInfo(qd.get());
  qi.FirstTime();
  int year = qi.ValidTime().GetYear();
  TextGenPosixTime t1 = thePeriod.localStartTime();
  TextGenPosixTime t2 = thePeriod.localEndTime();
  t1.SetYear(year);
  t2.SetYear(year);
  return WeatherPeriod(t1, t2);
}

}  // namespace ClimatologyTools
}  // namespace TextGen
// ======================================================================
