// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::TextGenerator
 */
// ======================================================================
/*!
 * \class TextGen::TextGenerator
 *
 * \brief The main text generator driver
 *
 */
// ======================================================================

#include "TextGenerator.h"
#include <calculator/AnalysisSources.h>
#include "CoastMaskSource.h"
#include "Document.h"
#include "Header.h"
#include "HeaderFactory.h"
#include <calculator/HourPeriodGenerator.h>
#include "InlandMaskSource.h"
#include "LandMaskSource.h"
#include <calculator/LatestWeatherSource.h>
#include <calculator/MaskSource.h>
#include "MessageLogger.h"
#include "NullMaskSource.h"
#include "Paragraph.h"
#include <calculator/RegularMaskSource.h>
#include "SectionTag.h"
#include <calculator/Settings.h>
#include "StoryFactory.h"
#include "StoryTag.h"
#include <calculator/WeatherArea.h>
#include <calculator/WeatherPeriod.h>
#include "WeatherPeriodFactory.h"
#include "NorthernMaskSource.h"
#include "SouthernMaskSource.h"
#include "EasternMaskSource.h"
#include "WesternMaskSource.h"

#include <newbase/NFmiStringTools.h>
#include <calculator/TextGenPosixTime.h>

using namespace TextGen;
using namespace std;
using namespace boost;

namespace TextGen
{
namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate contents from given contents list
 *
 * \param theContents The string with content variables
 * \param theVar The control variable prefix
 * \param theForecastTime The forecast time
 * \param theSources The analysis sources
 * \param theArea The weather area
 * \param thePeriod The weather period
 * \return A paragraph
 */
// ----------------------------------------------------------------------

const Paragraph make_contents(const string& theContents,
                              const string& theVar,
                              const TextGenPosixTime& theForecastTime,
                              const AnalysisSources& theSources,
                              const WeatherArea& theArea,
                              const WeatherPeriod& thePeriod)
{
  const vector<string> contents = NFmiStringTools::Split(theContents);

  Paragraph paragraph;

  for (vector<string>::const_iterator iter = contents.begin(); iter != contents.end(); ++iter)
  {
    const string storyvar = theVar + "::story::" + *iter;

    paragraph << StoryTag(storyvar, true);
    Paragraph p =
        StoryFactory::create(theForecastTime, theSources, theArea, thePeriod, *iter, storyvar);
    paragraph << p;
    paragraph << StoryTag(storyvar, false);
  }

  return paragraph;
}

}  // namespace anonymous

// ----------------------------------------------------------------------
/*!
 * \brief Implementation hiding pimple
 */
// ----------------------------------------------------------------------

class TextGenerator::Pimple
{
 public:
  Pimple();
  Pimple(const WeatherArea& theLandMaskArea, const WeatherArea& theCoastMaskArea);

  AnalysisSources itsSources;
  TextGenPosixTime itsForecastTime;

};  // class Pimple

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * The constructor initializes reasonable defaults for all
 * the analysis sources components.
 */
// ----------------------------------------------------------------------

TextGenerator::Pimple::Pimple()
{
  boost::shared_ptr<WeatherSource> weathersource(new LatestWeatherSource());
  itsSources.setWeatherSource(weathersource);

  typedef boost::shared_ptr<MaskSource> mask_source;

  mask_source masksource(new RegularMaskSource());
  itsSources.setMaskSource(masksource);

  // land mask is optional - by default the area is land so the source can be shared
  const string land_name = Settings::optional_string("textgen::mask::land", "");
  if (land_name.empty())
    itsSources.setLandMaskSource(masksource);
  else
    itsSources.setLandMaskSource(mask_source(new LandMaskSource(WeatherArea(land_name))));

  // coast mask is optional - by default there is no coast
  const string coast_name = Settings::optional_string("textgen::mask::coast", "");
  if (coast_name.empty())
  {
    mask_source nullsource(new NullMaskSource);
    itsSources.setCoastMaskSource(nullsource);
    itsSources.setInlandMaskSource(nullsource);
  }
  else
  {
    const WeatherArea coast(coast_name);
    itsSources.setCoastMaskSource(mask_source(new CoastMaskSource(coast)));
    itsSources.setInlandMaskSource(mask_source(new InlandMaskSource(coast)));
  }

  // mask sources for split areas
  NFmiPoint point(0.0, 0.0);
  WeatherArea theArea(point);

  itsSources.setNorthernMaskSource(boost::shared_ptr<MaskSource>(new NorthernMaskSource(theArea)));
  itsSources.setSouthernMaskSource(boost::shared_ptr<MaskSource>(new SouthernMaskSource(theArea)));
  itsSources.setEasternMaskSource(boost::shared_ptr<MaskSource>(new EasternMaskSource(theArea)));
  itsSources.setWesternMaskSource(boost::shared_ptr<MaskSource>(new WesternMaskSource(theArea)));
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * The constructor initializes reasonable defaults for all
 * the analysis sources components. Inland and coastal masks are provided as parameter
 */
// ----------------------------------------------------------------------

TextGenerator::Pimple::Pimple(const WeatherArea& theLandMaskArea,
                              const WeatherArea& theCoastMaskArea)
{
  boost::shared_ptr<WeatherSource> weathersource(new LatestWeatherSource());
  itsSources.setWeatherSource(weathersource);

  typedef boost::shared_ptr<MaskSource> mask_source;

  mask_source masksource(new RegularMaskSource());
  itsSources.setMaskSource(masksource);

  if (!theLandMaskArea.isPoint() && theLandMaskArea.path().size() > 0)
    itsSources.setLandMaskSource(mask_source(new LandMaskSource(theLandMaskArea)));
  else
    itsSources.setLandMaskSource(masksource);

  if (!theCoastMaskArea.isPoint() && theCoastMaskArea.path().size() > 0)
  {
    itsSources.setCoastMaskSource(mask_source(new CoastMaskSource(theCoastMaskArea)));
    itsSources.setInlandMaskSource(mask_source(new InlandMaskSource(theCoastMaskArea)));
  }
  else
  {
    mask_source nullsource(new NullMaskSource);
    itsSources.setCoastMaskSource(nullsource);
    itsSources.setInlandMaskSource(nullsource);
  }

  // mask sources for split areas
  NFmiPoint point(0.0, 0.0);
  WeatherArea theArea(point);

  itsSources.setNorthernMaskSource(boost::shared_ptr<MaskSource>(new NorthernMaskSource(theArea)));
  itsSources.setSouthernMaskSource(boost::shared_ptr<MaskSource>(new SouthernMaskSource(theArea)));
  itsSources.setEasternMaskSource(boost::shared_ptr<MaskSource>(new EasternMaskSource(theArea)));
  itsSources.setWesternMaskSource(boost::shared_ptr<MaskSource>(new WesternMaskSource(theArea)));
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
TextGenerator::TextGenerator(const WeatherArea& theLandMaskArea,
                             const WeatherArea& theCoastMaskArea)
    : itsPimple(new Pimple(theLandMaskArea, theCoastMaskArea))
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 */
// ----------------------------------------------------------------------

TextGenerator::TextGenerator() : itsPimple(new Pimple()) {}
// ----------------------------------------------------------------------
/*!
 * \brief Return the forecast time
 *
 * \return The forecast time
 */
// ----------------------------------------------------------------------

const TextGenPosixTime& TextGenerator::time() const { return itsPimple->itsForecastTime; }
// ----------------------------------------------------------------------
/*!
 * \brief Set a new forecast time
 *
 * This is mostly used to override the current time as taken from
 * the CPU clock. This is useful for both debugging and regression
 * testing.
 *
 * \param theForecastTime The time to use as the forecast time
 */
// ----------------------------------------------------------------------

void TextGenerator::time(const TextGenPosixTime& theForecastTime)
{
  itsPimple->itsForecastTime = theForecastTime;
}

// ----------------------------------------------------------------------
/*!
 * \brief Generate the text
 *
 * The algorithm consists of the following steps:
 *
 * -# Parse textgen::paragraphs for paragraph names
 * -# Initialize output document
 * -# For each paragraph name
 *    -# Generate period from textgen::name::period
 *    -# Generate header from textgen::name::header, if it exists
 *    -# Initialize output paragraph
 *    -# Append the header to the document
 *    -# If there are no subperiods
 *       -# For each story name in textgen::name::content
 *           -# Generate the story
 *           -# Append the story to the output paragraph
 *       -# Append the paragraph to the document
 *    -# Else for each subperiod
 *       -# For each story name in textgen::name::dayX::content
 *           -# Generate the story
 *           -# Append the story to the output paragraph
 * -# Return the document
 *
 * \param theArea The weather area
 *
 */
// ----------------------------------------------------------------------

Document TextGenerator::generate(const WeatherArea& theArea) const
{
  MessageLogger log("TextGenerator::generate");

  const vector<string> paragraphs =
      NFmiStringTools::Split(Settings::require_string("textgen::sections"));

  Document doc;
  for (vector<string>::const_iterator it = paragraphs.begin(); it != paragraphs.end(); ++it)
  {
    doc << SectionTag("textgen::" + *it, true);

    const string periodvar = "textgen::" + *it + "::period";
    const WeatherPeriod period =
        WeatherPeriodFactory::create(itsPimple->itsForecastTime, periodvar);

    const string headervar = "textgen::" + *it + "::header";

    log << "TextGenerator::generate periodvar " << periodvar << endl
        << "TextGenerator::generate headervar " << headervar << endl
        << "TextGenerator::generate period : " << period.localStartTime() << endl
        << " -  " << period.localEndTime() << endl;

    Header header = HeaderFactory::create(theArea, period, headervar);
    if (!header.empty()) doc << header;

    const bool subs = Settings::optional_bool("textgen::" + *it + "::subperiods", false);

    if (!subs)
    {
      const string contents = Settings::require("textgen::" + *it + "::content");
      log << "TextGenerator::generate contents " << contents << endl;
      doc << make_contents(contents,
                           "textgen::" + *it,
                           itsPimple->itsForecastTime,
                           itsPimple->itsSources,
                           theArea,
                           period);
    }
    else
    {
      // Generate subparagraphs for each day
      HourPeriodGenerator generator(period, "textgen::" + *it + "::subperiod::day");

      const string defaultvar = "textgen::" + *it;

      for (HourPeriodGenerator::size_type day = 1; day <= generator.size(); day++)
      {
        const WeatherPeriod subperiod = generator.period(day);

        log << "TextGenerator::generate subperiod: " << subperiod.localStartTime() << " - "
            << subperiod.localEndTime() << endl;

        const string dayvar = defaultvar + "::day" + NFmiStringTools::Convert(day);

        const bool hasday = Settings::isset(dayvar + "::content");

        doc << make_contents(hasday ? Settings::require_string(dayvar + "::content")
                                    : Settings::require_string(defaultvar + "::content"),
                             hasday ? dayvar : defaultvar,
                             itsPimple->itsForecastTime,
                             itsPimple->itsSources,
                             theArea,
                             subperiod);
      }
    }
    doc << SectionTag("textgen::" + *it, false);
  }
  return doc;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set a new forecast data
 *
 * \param theSources new data
 */
// ----------------------------------------------------------------------
void TextGenerator::sources(const AnalysisSources& theSources)
{
  itsPimple->itsSources = theSources;
}

}  // namespace TextGen

// ======================================================================
