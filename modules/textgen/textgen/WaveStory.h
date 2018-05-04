// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::WaveStory
 */
// ======================================================================

#ifndef TEXTGEN_WAVESTORY_H
#define TEXTGEN_WAVESTORY_H

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

class WaveStory : public Story
{
 public:
  virtual ~WaveStory();
  WaveStory(const TextGenPosixTime& theForecastTime,
            const TextGen::AnalysisSources& theSources,
            const TextGen::WeatherArea& theArea,
            const TextGen::WeatherPeriod& thePeriod,
            const std::string& theVariable);

  static bool hasStory(const std::string& theName);
  virtual Paragraph makeStory(const std::string& theName) const;

 private:
  WaveStory();
  WaveStory(const WaveStory& theStory);
  WaveStory& operator=(const WaveStory& theStory);

  Paragraph range() const;

  const TextGenPosixTime itsForecastTime;
  const TextGen::AnalysisSources& itsSources;
  const TextGen::WeatherArea& itsArea;
  const TextGen::WeatherPeriod& itsPeriod;
  const std::string itsVar;

};  // class WaveStory
}

#endif  // TEXTGEN_WAVESTORY_H

// ======================================================================
