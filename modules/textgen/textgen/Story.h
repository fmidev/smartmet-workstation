// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Story
 */
// ======================================================================

#ifndef TEXTGEN_STORY_H
#define TEXTGEN_STORY_H

#include <calculator/AnalysisSources.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherPeriod.h>

namespace TextGen
{
class Paragraph;

class Story
{
 public:
  Story() {}
  virtual ~Story() {}
  virtual Paragraph makeStory(const std::string& theName) const = 0;

};  // class Story
}

#endif  // TEXTGEN_STORY_H

// ======================================================================
