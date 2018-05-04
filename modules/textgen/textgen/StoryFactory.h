// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::StoryFactory
 */
// ======================================================================

#ifndef TEXTGEN_STORYFACTORY_H
#define TEXTGEN_STORYFACTORY_H

#include <string>

class TextGenPosixTime;

namespace TextGen
{
class AnalysisSources;
class WeatherArea;
class WeatherPeriod;
}

namespace TextGen
{
class Paragraph;

namespace StoryFactory
{
Paragraph create(const TextGenPosixTime& theForecastTime,
                 const TextGen::AnalysisSources& theSources,
                 const TextGen::WeatherArea& theArea,
                 const TextGen::WeatherPeriod& thePeriod,
                 const std::string& theName,
                 const std::string& theVariable);

}  // namespace StoryFactory
}  // namespace TextGen

#endif  // TEXTGEN_STORYFACTORY_H

// ======================================================================
