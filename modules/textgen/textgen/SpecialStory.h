// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::SpecialStory
 */
// ======================================================================

#ifndef TEXTGEN_SPECIALSTORY_H
#define TEXTGEN_SPECIALSTORY_H

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

class SpecialStory : public Story
{
 public:
  virtual ~SpecialStory();
  SpecialStory(const TextGenPosixTime& theForecastTime,
               const TextGen::AnalysisSources& theSources,
               const TextGen::WeatherArea& theArea,
               const TextGen::WeatherPeriod& thePeriod,
               const std::string& theVariable);

  static bool hasStory(const std::string& theName);
  virtual Paragraph makeStory(const std::string& theName) const;

 private:
  Paragraph none() const;
  Paragraph text() const;
  Paragraph date() const;
#if 0
	Paragraph table() const;
#endif

  SpecialStory();
  SpecialStory(const SpecialStory& theStory);
  SpecialStory& operator=(const SpecialStory& theStory);

  const TextGenPosixTime itsForecastTime;
  const TextGen::AnalysisSources& itsSources;
  const TextGen::WeatherArea& itsArea;
  const TextGen::WeatherPeriod& itsPeriod;
  const std::string itsVar;

};  // class SpecialStory
}

#endif  // TEXTGEN_SPECIALSTORY_H

// ======================================================================
