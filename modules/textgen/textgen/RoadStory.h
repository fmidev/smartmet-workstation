// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::RoadStory
 */
// ======================================================================

#ifndef TEXTGEN_ROADSTORY_H
#define TEXTGEN_ROADSTORY_H

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

class RoadStory : public Story
{
 public:
  virtual ~RoadStory();
  RoadStory(const TextGenPosixTime& theForecastTime,
            const TextGen::AnalysisSources& theSources,
            const TextGen::WeatherArea& theArea,
            const TextGen::WeatherPeriod& thePeriod,
            const std::string& theVariable);

  static bool hasStory(const std::string& theName);
  virtual Paragraph makeStory(const std::string& theName) const;

 private:
  Paragraph daynightranges() const;
  Paragraph condition_overview() const;
  Paragraph warning_overview() const;
  Paragraph condition_shortview() const;
  Paragraph shortrange() const;
  Paragraph warning_shortview() const;

  RoadStory();
  RoadStory(const RoadStory& theStory);
  RoadStory& operator=(const RoadStory& theStory);

  const TextGenPosixTime itsForecastTime;
  const TextGen::AnalysisSources& itsSources;
  const TextGen::WeatherArea& itsArea;
  const TextGen::WeatherPeriod& itsPeriod;
  const std::string itsVar;

};  // class RoadStory
}

#endif  // TEXTGEN_ROADSTORY_H

// ======================================================================
