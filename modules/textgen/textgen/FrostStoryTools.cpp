// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::FrostStoryTools
 */
// ======================================================================
/*!
 * \namespace TextGen::FrostStoryTools
 *
 * \brief Common utilities for FrostStory methods
 *
 */
// ======================================================================

#include "FrostStoryTools.h"
#include "Integer.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include "UnitFactory.h"
#include <calculator/WeatherPeriod.h>
#include "WeekdayTools.h"

using TextGen::WeatherPeriod;

namespace TextGen
{
namespace FrostStoryTools
{
// ----------------------------------------------------------------------
/*!
 * \brief Return true if one is allowed to report on frost
 */
// ----------------------------------------------------------------------

bool is_frost_season() { return Settings::require_bool("textgen::frostseason"); }
// ----------------------------------------------------------------------
/*!
 * \brief Return sentence for severe frost
 *
 * \param thePeriod The night period
 * \param theProbability The probability
 * \return The sentence
 */
// ----------------------------------------------------------------------

Sentence severe_frost_sentence(const WeatherPeriod& thePeriod, int theProbability)
{
  Sentence sentence;
  sentence << "ankaran hallan todennakoisyys"
           << "on" << WeekdayTools::night_against_weekday(thePeriod.localEndTime())
           << Integer(theProbability) << *UnitFactory::create(Percent);
  return sentence;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return sentence for frost
 *
 * \param thePeriod The night period
 * \param theProbability The probability
 * \return The sentence
 */
// ----------------------------------------------------------------------

Sentence frost_sentence(const WeatherPeriod& thePeriod, int theProbability)
{
  Sentence sentence;
  sentence << "hallan todennakoisyys"
           << "on" << WeekdayTools::night_against_weekday(thePeriod.localEndTime())
           << Integer(theProbability) << *UnitFactory::create(Percent);
  return sentence;
}

}  // namespace FrostStoryTools
}  // namespace TextGen

// ======================================================================
