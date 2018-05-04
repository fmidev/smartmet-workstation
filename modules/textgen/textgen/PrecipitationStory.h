// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::PrecipitationStory
 */
// ======================================================================

#ifndef TEXTGEN_PRECIPITATIONSTORY_H
#define TEXTGEN_PRECIPITATIONSTORY_H

#include "Story.h"

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

class PrecipitationStory : public Story
{
 public:
  virtual ~PrecipitationStory();
  PrecipitationStory(const TextGenPosixTime& theForecastTime,
                     const TextGen::AnalysisSources& theSources,
                     const TextGen::WeatherArea& theArea,
                     const TextGen::WeatherPeriod& thePeriod,
                     const std::string& theVariable);

  static bool hasStory(const std::string& theName);
  virtual Paragraph makeStory(const std::string& theName) const;

 private:
  Paragraph total() const;
  Paragraph total_day() const;
  Paragraph range() const;
  Paragraph classification() const;
  Paragraph pop_twodays() const;
  Paragraph pop_days() const;
  Paragraph pop_max() const;
  Paragraph sums() const;
  Paragraph daily_sums() const;

  PrecipitationStory();
  PrecipitationStory(const PrecipitationStory& theStory);
  PrecipitationStory& operator=(const PrecipitationStory& theStory);

  const TextGenPosixTime itsForecastTime;
  const TextGen::AnalysisSources& itsSources;
  const TextGen::WeatherArea& itsArea;
  const TextGen::WeatherPeriod& itsPeriod;
  const std::string itsVar;

};  // class PrecipitationStory
}

#endif  // TEXTGEN_PRECIPITATIONSTORY_H

// ======================================================================
