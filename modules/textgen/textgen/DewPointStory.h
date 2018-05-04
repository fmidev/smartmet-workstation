// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::DewPointStory
 */
// ======================================================================

#ifndef TEXTGEN_DEWPOINTSTORY_H
#define TEXTGEN_DEWPOINTSTORY_H

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

class DewPointStory : public Story
{
 public:
  virtual ~DewPointStory();
  DewPointStory(const TextGenPosixTime& theForecastTime,
                const TextGen::AnalysisSources& theSources,
                const TextGen::WeatherArea& theArea,
                const TextGen::WeatherPeriod& thePeriod,
                const std::string& theVariable);

  static bool hasStory(const std::string& theName);
  virtual Paragraph makeStory(const std::string& theName) const;

 private:
  Paragraph range() const;

  DewPointStory();
  DewPointStory(const DewPointStory& theStory);
  DewPointStory& operator=(const DewPointStory& theStory);

  const TextGenPosixTime itsForecastTime;
  const TextGen::AnalysisSources& itsSources;
  const TextGen::WeatherArea& itsArea;
  const TextGen::WeatherPeriod& itsPeriod;
  const std::string itsVar;

};  // class DewPointStory
}

#endif  // TEXTGEN_DEWPOINTSTORY_H

// ======================================================================
