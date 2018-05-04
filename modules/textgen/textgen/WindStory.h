// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::WindStory
 */
// ======================================================================

#ifndef TEXTGEN_WINDSTORY_H
#define TEXTGEN_WINDSTORY_H

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

class WindStory : public Story
{
 public:
  virtual ~WindStory();
  WindStory(const TextGenPosixTime& theForecastTime,
            const TextGen::AnalysisSources& theSources,
            const TextGen::WeatherArea& theArea,
            const TextGen::WeatherPeriod& thePeriod,
            const std::string& theVariable);

  static bool hasStory(const std::string& theName);
  virtual Paragraph makeStory(const std::string& theName) const;

 private:
  WindStory();
  WindStory(const WindStory& theStory);
  WindStory& operator=(const WindStory& theStory);

  Paragraph simple_overview() const;
  Paragraph overview() const;
  Paragraph daily_ranges() const;
  Paragraph range() const;
  Paragraph anomaly() const;

  const TextGenPosixTime itsForecastTime;
  const TextGen::AnalysisSources& itsSources;
  const TextGen::WeatherArea& itsArea;
  const TextGen::WeatherPeriod& itsPeriod;
  const std::string itsVar;

};  // class WindStory
}

#endif  // TEXTGEN_WINDSTORY_H

// ======================================================================
