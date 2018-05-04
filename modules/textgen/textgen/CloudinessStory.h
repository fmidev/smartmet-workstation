// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::CloudinessStory
 */
// ======================================================================

#ifndef TEXTGEN_CLOUDINESSSTORY_H
#define TEXTGEN_CLOUDINESSSTORY_H

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

class CloudinessStory : public Story
{
 public:
  virtual ~CloudinessStory();
  CloudinessStory(const TextGenPosixTime& theForecastTime,
                  const TextGen::AnalysisSources& theSources,
                  const TextGen::WeatherArea& theArea,
                  const TextGen::WeatherPeriod& thePeriod,
                  const std::string& theVariable);

  static bool hasStory(const std::string& theName);
  virtual Paragraph makeStory(const std::string& theName) const;

 private:
  const Paragraph overview(void) const;

  CloudinessStory();
  CloudinessStory(const CloudinessStory& theStory);
  CloudinessStory& operator=(const CloudinessStory& theStory);

  const TextGenPosixTime itsForecastTime;
  const TextGen::AnalysisSources& itsSources;
  const TextGen::WeatherArea& itsArea;
  const TextGen::WeatherPeriod& itsPeriod;
  const std::string itsVar;

};  // class CloudinessStory
}

#endif  // TEXTGEN_CLOUDINESSSTORY_H

// ======================================================================
