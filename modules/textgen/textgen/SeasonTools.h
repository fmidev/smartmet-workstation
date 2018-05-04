
// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::SeasonTools
 */
// ======================================================================

#ifndef TEXTGEN_SEASONTOOLS_H
#define TEXTGEN_SEASONTOOLS_H

#include <ctime>
#include <string>

class TextGenPosixTime;

namespace TextGen
{
class WeatherPeriod;
class WeatherResult;
class AnalysisSources;
class WeatherArea;
class WeatherPeriodGenerator;

namespace SeasonTools
{
enum forecast_season_id
{
  SUMMER_SEASON,
  WINTER_SEASON,
  UNDEFINED_SEASON
};

bool isWinter(const TextGenPosixTime& theDate, const std::string& theVar);
bool isSpring(const TextGenPosixTime& theDate, const std::string& theVar);
bool isSummer(const TextGenPosixTime& theDate, const std::string& theVar);
bool isAutumn(const TextGenPosixTime& theDate, const std::string& theVar);

bool isWinterHalf(const TextGenPosixTime& theDate, const std::string& theVar);
bool isSummerHalf(const TextGenPosixTime& theDate, const std::string& theVar);

float growing_season_percentage(const WeatherArea& theArea,
                                const AnalysisSources& theSources,
                                const WeatherPeriod& thePeriod,
                                const std::string& theVariable);

bool growing_season_going_on(const WeatherArea& theArea,
                             const AnalysisSources& theSources,
                             const WeatherPeriod& thePeriod,
                             const std::string theVariable);

forecast_season_id get_forecast_season(const WeatherArea& theArea,
                                       const AnalysisSources& theSources,
                                       const WeatherPeriod& thePeriod,
                                       const std::string theVariable);

}  // namespace SeasonTools
}  // namespace TextGen

#endif  // TEXTGEN_SEASONTOOLS_H

// ======================================================================
