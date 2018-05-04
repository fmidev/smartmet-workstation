// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::ForestStory::forestfirewarning_county
 */
// ======================================================================

#include "ForestStory.h"
#include "FireWarnings.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include "Sentence.h"
#include <calculator/Settings.h>

using namespace TextGen;
using namespace boost;
using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on forest fire warning
 *
 * \return The generated paragraph
 */
// ----------------------------------------------------------------------

Paragraph ForestStory::forestfirewarning_county() const
{
  MessageLogger log("ForestStory::forestfirewarning_county");

  Paragraph paragraph;

  // Establish area name and respective area code

  const string& areaname = itsArea.name();
  const int areacode = Settings::require_int("qdtext::forestfirewarning::areacodes::" + areaname);

  log << "Area code for " << areaname << ' ' << " is " << areacode << endl;

  // Read the file containing the warnings

  const string datadir = Settings::require_string("qdtext::forestfirewarning::directory");

  try
  {
    FireWarnings warnings(datadir, itsForecastTime);

    Sentence sentence;
    switch (warnings.state(areacode))
    {
      case FireWarnings::Undefined:
        log << "Warning: warning state for given area is undefined!" << endl
            << "Returning an empty story" << endl;
        break;
      case FireWarnings::None:
      case FireWarnings::GrassFireWarning:
        sentence << "metsapalovaroitus ei ole voimassa";
        paragraph << sentence;
        break;
      case FireWarnings::FireWarning:
        sentence << "metsapalovaroitus on voimassa";
        paragraph << sentence;
        break;
    }
  }
  catch (...)
  {
    log << "Failed to read forest fire warning data from directory '" << datadir << "'" << endl
        << "Returning an empty story" << endl;
  }

  log << paragraph;
  return paragraph;
}

}  // namespace TextGen

// ======================================================================
