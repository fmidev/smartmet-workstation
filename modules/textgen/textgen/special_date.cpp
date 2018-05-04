// ======================================================================
/*!
 * \file
 * \brief Implementation of method TextGen::SpecialStory::date
 */
// ======================================================================

#include "SpecialStory.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include <calculator/Settings.h>
#include "Text.h"
#include "WeatherTime.h"
#include "TimePeriod.h"

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

#include <cstdio>
#include <fstream>
#include <sstream>

#ifdef UNIX
#include <unistd.h>
#endif

using namespace TextGen;
using namespace std;
using namespace boost::locale::as;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Generate story on text
 *
 * Variables:
 *
 *     value = "Text to be inserted into paragraph as is."
 *     value = @filename
 *     value = @filename.php
 *
 * Product specific variables are possible:
 *
 *     value::en_html = "<underline>Text to be underlined.</underline>"
 *
 * \return The story
 */
// ----------------------------------------------------------------------

Paragraph SpecialStory::date() const
{
  MessageLogger log("SpecialStory::date");

  Paragraph paragraph;

  paragraph << TimePeriod(itsPeriod);

  return paragraph;
}

}  // namespace TextGen

// ======================================================================
