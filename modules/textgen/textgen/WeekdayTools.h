// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::WeekdayTools
 */
// ----------------------------------------------------------------------

#ifndef TEXTGEN_WEEKDAYTOOLS_H
#define TEXTGEN_WEEKDAYTOOLS_H

#include <string>

#include <calculator/WeatherHistory.h>

class TextGenPosixTime;

namespace TextGen
{
namespace WeekdayTools
{
std::string on_weekday(const TextGenPosixTime& theTime);
std::string on_weekday_time(const TextGenPosixTime& theTime);
std::string night_against_weekday(const TextGenPosixTime& theTime);
std::string until_weekday_morning(const TextGenPosixTime& theTime);
std::string until_weekday_evening(const TextGenPosixTime& theTime);
std::string until_weekday_time(const TextGenPosixTime& theTime);
std::string from_weekday(const TextGenPosixTime& theTime);
std::string from_weekday_morning(const TextGenPosixTime& theTime);
std::string from_weekday_evening(const TextGenPosixTime& theTime);
std::string from_weekday_time(const TextGenPosixTime& theTime);
std::string on_weekday_morning(const TextGenPosixTime& theTime);
std::string on_weekday_forenoon(const TextGenPosixTime& theTime);
std::string on_weekday_afternoon(const TextGenPosixTime& theTime);
std::string on_weekday_evening(const TextGenPosixTime& theTime);

std::string on_weekday(const TextGenPosixTime& theTime, TextGen::WeatherHistory& theHistory);
std::string on_weekday_time(const TextGenPosixTime& theTime, TextGen::WeatherHistory& theHistory);
std::string night_against_weekday(const TextGenPosixTime& theTime,
                                  TextGen::WeatherHistory& theHistory);
std::string until_weekday_morning(const TextGenPosixTime& theTime,
                                  TextGen::WeatherHistory& theHistory);
std::string until_weekday_evening(const TextGenPosixTime& theTime,
                                  TextGen::WeatherHistory& theHistory);
std::string until_weekday_time(const TextGenPosixTime& theTime,
                               TextGen::WeatherHistory& theHistory);
std::string from_weekday(const TextGenPosixTime& theTime, TextGen::WeatherHistory& theHistory);
std::string from_weekday_morning(const TextGenPosixTime& theTime,
                                 TextGen::WeatherHistory& theHistory);
std::string from_weekday_evening(const TextGenPosixTime& theTime,
                                 TextGen::WeatherHistory& theHistory);
std::string from_weekday_time(const TextGenPosixTime& theTime, TextGen::WeatherHistory& theHistory);
std::string on_weekday_morning(const TextGenPosixTime& theTime,
                               TextGen::WeatherHistory& theHistory);
std::string on_weekday_forenoon(const TextGenPosixTime& theTime,
                                TextGen::WeatherHistory& theHistory);
std::string on_weekday_afternoon(const TextGenPosixTime& theTime,
                                 TextGen::WeatherHistory& theHistory);
std::string on_weekday_evening(const TextGenPosixTime& theTime,
                               TextGen::WeatherHistory& theHistory);
std::string get_time_phrase(const TextGenPosixTime& theTime,
                            const std::string theNewPhrase,
                            TextGen::WeatherHistory& theHistory);

}  // namespace WeekdayTools
}  // namespace TextGen

#endif  // TEXTGEN_WEEKDAYTOOLS_H

// ======================================================================
