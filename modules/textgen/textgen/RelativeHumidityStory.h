// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::RelativeHumidityStory
 */
// ======================================================================

#ifndef TEXTGEN_RELATIVEHUMIDITYSTORY_H
#define TEXTGEN_RELATIVEHUMIDITYSTORY_H

#include "Story.h"
#include <string>

namespace TextGen
{
class AnalysisSources;
class WeatherArea;
class WeatherPeriod;
}

class TextGenPosixTime;

namespace TextGen
{
class Paragraph;

class RelativeHumidityStory : public Story
{
 public:
  virtual ~RelativeHumidityStory();
  RelativeHumidityStory(const TextGenPosixTime& theForecastTime,
                        const TextGen::AnalysisSources& theSources,
                        const TextGen::WeatherArea& theArea,
                        const TextGen::WeatherPeriod& thePeriod,
                        const std::string& theVariable);

  static bool hasStory(const std::string& theName);
  virtual Paragraph makeStory(const std::string& theName) const;

 private:
  Paragraph lowest() const;
  Paragraph day() const;
  Paragraph range() const;

  RelativeHumidityStory();
  RelativeHumidityStory(const RelativeHumidityStory& theStory);
  RelativeHumidityStory& operator=(const RelativeHumidityStory& theStory);

  const TextGenPosixTime itsForecastTime;
  const TextGen::AnalysisSources& itsSources;
  const TextGen::WeatherArea& itsArea;
  const TextGen::WeatherPeriod& itsPeriod;
  const std::string itsVar;

};  // class RelativeHumidityStory
}

#endif  // TEXTGEN_RELATIVEHUMIDITYSTORY_H

// ======================================================================
