// ======================================================================
/*!
 * \file
 * \brief Implementation of WindForecast class
 */
// ======================================================================

#include "WeatherStory.h"
#include "CloudinessStory.h"
#include "CloudinessStoryTools.h"
#include "Delimiter.h"
#include <calculator/GridForecaster.h>
#include <calculator/HourPeriodGenerator.h>
#include "MessageLogger.h"
#include "Paragraph.h"
#include "NightAndDayPeriodGenerator.h"
#include "PeriodPhraseFactory.h"
#include <calculator/RangeAcceptor.h>
#include "ValueAcceptor.h"
#include "Sentence.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/TimeTools.h>
#include <calculator/WeatherResult.h>
#include "WeekdayTools.h"
#include <calculator/NullPeriodGenerator.h>
#include <calculator/WeatherPeriodTools.h>
#include "AreaTools.h"
#include <calculator/MathTools.h>
#include "SeasonTools.h"
#include "SubMaskExtractor.h"
#include "IntegerRange.h"
#include "UnitFactory.h"
#include "WindForecast.h"
#include "DebugTextFormatter.h"
#include "WindStoryTools.h"

#include <newbase/NFmiCombinedParam.h>
#include <newbase/NFmiMercatorArea.h>

#include <macgyver/StringConversion.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <vector>
#include <map>
#include <iomanip>
#include <cmath>

using namespace Settings;
using namespace TextGen;
using namespace AreaTools;
using namespace WindStoryTools;
using namespace boost;
using namespace std;

namespace TextGen
{
namespace WindForecastPhrases
{
#define USE_AT_ITS_STRONGEST_PHRASE true
#define DONT_USE_AT_ITS_STRONGEST_PHRASE false
#define USE_ALKAEN_PHRASE true
#define DONT_USE_ALKAEN_PHRASE false

#define TUULI_WORD "tuuli"
#define VAHAN_WORD "vahan"
#define NOPEASTI_WORD "nopeasti"
#define VAHITELLEN_WORD "vahitellen"
#define KOVIMMILLAAN_PHRASE "kovimmillaan"
#define HEIKKENEVAA_WORD "heikkenevaa"
#define VOIMISTUVAA_WORD "voimistuvaa"
#define TUULTA_WORD "tuulta"
#define TUULI_TYYNTYY_PHRASE "tuuli tyyntyy"
#define POHJOISEEN_PHRASE "pohjoiseen"
#define ETELAAN_PHRASE "etelaan"
#define ITAAN_PHRASE "itaan"
#define LANTEEN_PHRASE "lanteen"
#define KOILLISEEN_PHRASE "koilliseen"
#define KAAKKOON_PHRASE "kaakkoon"
#define LOUNAASEEN_PHRASE "lounaaseen"
#define LUOTEESEEN_PHRASE "luoteeseen"
#define POHJOISEN_PUOLELLE_PHRASE "1-puolelle"
#define ETELAN_PUOLELLE_PHRASE "5-puolelle"
#define IDAN_PUOLELLE_PHRASE "3-puolelle"
#define LANNEN_PUOLELLE_PHRASE "7-puolelle"
#define KOILLISEN_PUOLELLE_PHRASE "2-puolelle"
#define KAAKON_PUOLELLE_PHRASE "4-puolelle"
#define LOUNAAN_PUOLELLE_PHRASE "6-puolelle"
#define LUOTEEN_PUOLELLE_PHRASE "8-puolelle"

#define POHJOISEN_JA_KOILLISEN_VALILLE_PHRASE "1- ja 2-valille"
#define IDAN_JA_KOILLISEN_VALILLE_PHRASE "3- ja 2-valille"
#define IDAN_JA_KAAKON_VALILLE_PHRASE "3- ja 4-valille"
#define ETELAN_JA_KAAKON_VALILLE_PHRASE "5- ja 4-valille"
#define ETELAN_JA_LOUNAAN_VALILLE_PHRASE "5- ja 6-valille"
#define LANNEN_JA_LOUNAAN_VALILLE_PHRASE "7- ja 6-valille"
#define LANNEN_JA_LUOTEEN_VALILLE_PHRASE "7- ja 8-valille"
#define POHJOISEN_JA_LUOTEEN_VALILLE_PHRASE "1- ja 8-valille"
#define TUULI_MUUTTUU_VAIHTELEVAKSI_PHRASE "tuuli muuttuu vaihtelevaksi"

#define POHJOINEN_TUULI_P "1-tuulta"
#define POHJOINEN_TUULI "1-tuuli"
#define POHJOISEN_PUOLEINEN_TUULI_P "1-puoleista tuulta"
#define POHJOISEN_PUOLEINEN_TUULI "1-puoleinen tuuli"
#define POHJOINEN_KOILLINEN_TUULI_P "1- ja 2-valista tuulta"
#define POHJOINEN_KOILLINEN_TUULI "1- ja 2-valinen tuuli"
#define KOILLINEN_TUULI_P "2-tuulta"
#define KOILLINEN_TUULI "2-tuuli"
#define KOILLISEN_PUOLEINEN_TUULI_P "2-puoleista tuulta"
#define KOILLISEN_PUOLEINEN_TUULI "2-puoleinen tuuli"
#define ITA_KOILLINEN_TUULI_P "3- ja 2-valista tuulta"
#define ITA_KOILLINEN_TUULI "3- ja 2-valinen tuuli"
#define ITA_TUULI_P "3-tuulta"
#define ITA_TUULI "3-tuuli"
#define IDAN_PUOLEINEN_TUULI_P "3-puoleista tuulta"
#define IDAN_PUOLEINEN_TUULI "3-puoleinen tuuli"
#define ITA_KAAKKO_TUULI_P "3- ja 4-valista tuulta"
#define ITA_KAAKKO_TUULI "3- ja 4-valinen tuuli"
#define KAAKKO_TUULI_P "4-tuulta"
#define KAAKKO_TUULI "4-tuuli"
#define KAAKON_PUOLEINEN_TUULI_P "4-puoleista tuulta"
#define KAAKON_PUOLEINEN_TUULI "4-puoleinen tuuli"
#define ETELA_KAAKKO_TUULI_P "5- ja 4-valista tuulta"
#define ETELA_KAAKKO_TUULI "5- ja 4-valinen tuuli"
#define ETELA_TUULI_P "5-tuulta"
#define ETELA_TUULI "5-tuuli"
#define ETELAN_PUOLEINEN_TUULI_P "5-puoleista tuulta"
#define ETELAN_PUOLEINEN_TUULI "5-puoleinen tuuli"
#define ETELA_LOUNAS_TUULI_P "5- ja 6-valista tuulta"
#define ETELA_LOUNAS_TUULI "5- ja 6-valinen tuuli"
#define LOUNAS_TUULI_P "6-tuulta"
#define LOUNAS_TUULI "6-tuuli"
#define LOUNAAN_PUOLEINEN_TUULI_P "6-puoleista tuulta"
#define LOUNAAN_PUOLEINEN_TUULI "6-puoleinen tuuli"
#define LANSI_LOUNAS_TUULI_P "7- ja 6-valista tuulta"
#define LANSI_LOUNAS_TUULI "7- ja 6-valinen tuuli"
#define LANSI_TUULI_P "7-tuulta"
#define LANSI_TUULI "7-tuuli"
#define LANNEN_PUOLEINEN_TUULI_P "7-puoleista tuulta"
#define LANNEN_PUOLEINEN_TUULI "7-puoleinen tuuli"
#define LANSI_LUODE_TUULI_P "7- ja 8-valista tuulta"
#define LANSI_LUODE_TUULI "7- ja 8-valinen tuuli"
#define LUODE_TUULI_P "8-tuulta"
#define LUODE_TUULI "8-tuuli"
#define LUOTEEN_PUOLEINEN_TUULI_P "8-puoleista tuulta"
#define LUOTEEN_PUOLEINEN_TUULI "8-puoleinen tuuli"
#define POHJOINEN_LUODE_TUULI_P "1- ja 8-valista tuulta"
#define POHJOINEN_LUODE_TUULI "1- ja 8-valinen tuuli"
#define VAIHTELEVA_TUULI_P "suunnaltaan vaihtelevaa tuulta"
#define VAIHTELEVA_TUULI "suunnaltaan vaihteleva tuuli"

#define POHJOISTUULI_ALKAA_VOIMISTUA_JA_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[pohjoistuuli] alkaa voimistua ja muuttuu vaihtelevaksi"
#define POHJOISTUULI_VOIMISTUU_JA_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[pohjoistuuli] voimistuu ja muuttuu vaihtelevaksi"
#define POHJOISTUULI_ALKAA_HEIKETA_JA_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[pohjoistuuli] alkaa heiketa ja muuttuu vaihtelevaksi"
#define POHJOISTUULI_HEIKKENEE_JA_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[pohjoistuuli] heikkenee ja muuttuu vaihtelevaksi"
#define POHJOISTUULI_VOIMISTUU_JA_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[pohjoistuuli] voimistuu ja muuttuu tilapaisesti vaihtelevaksi"
#define POHJOISTUULI_ALKAA_VOIMISTUA_JA_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[pohjoistuuli] alkaa voimistua ja muuttuu tilapaisesti vaihtelevaksi"
#define POHJOISTUULI_HEIKKENEE_JA_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[pohjoistuuli] heikkenee ja muuttuu tilapaisesti vaihtelevaksi"
#define POHJOISTUULI_ALKAA_HEIKETA_JA_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[pohjoistuuli] alkaa heiketa ja muuttuu tilapaisesti vaihtelevaksi"
#define POHJOISTUULI_HEIKKENEE_NOPEASTI_COMPOSITE_PHRASE "[pohjoistuuli] heikkenee [nopeasti]"
#define POHJOISTUULI_VOIMISTUU_NOPEASTI_COMPOSITE_PHRASE "[pohjoistuuli] voimistuu [nopeasti]"

#define PUUSKITTAISTA_ETELATUULTA_COMPOSITE_PHRASE "puuskittaista [etelatuulta]"
#define POHJOISTUULTA_INTERVALLI_MS_JOKA_ALKAA_HEIKETA_COMPOSITE_PHRASE \
  "[pohjoistuulta] [m...n] [metria sekunnissa], joka alkaa heiketa"
#define POHJOISTUULTA_INTERVALLI_MS_JOKA_ALKAA_VOIMISTUA_COMPOSITE_PHRASE \
  "[pohjoistuulta] [m...n] [metria sekunnissa], joka alkaa voimistua"
#define ILTAPAIVALLA_EDELLEEN_HEIKKENEVAA_POHJOISTUULTA_COMPOSITE_PHRASE \
  "[iltapaivalla] edelleen [heikkenevaa] [pohjoistuulta]"
#define ILTAPAIVALLA_POHJOISTUULI_HEIKKENEE_EDELLEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] [pohjoistuuli] heikkenee edelleen"
#define ILTAPAIVALLA_POHJOISTUULI_VOIMISTUU_EDELLEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] [pohjoistuuli] voimistuu edelleen"
#define ILTAPAIVALLA_TUULI_KAANTYY_ETELAAN_JA_VOIMISTUU_EDELLEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli kaantyy [etelaan] ja voimistuu edelleen"
#define ILTAPAIVALLA_TUULI_KAANTYY_ETELAAN_JA_HEIKKENEE_EDELLEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli kaantyy [etelaan] ja heikkenee edelleen"
#define ILTAPAIVALLA_TUULI_VOIMISTUU_JA_KAANTYY_ETELAAN_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli voimistuu ja kaantyy [etelaan]"
#define ILTAPAIVALLA_POHJOISTUULI_HEIKKENEE_JA_KAANTYY_ETELAAN_COMPOSITE_PHRASE \
  "[iltapaivalla] [pohjoistuuli] heikkenee ja kaantyy [etelaan]"
#define ILTAPAIVALLA_TUULI_MUUTTUU_VAIHTELEVAKSI_JA_VOIMISTUU_EDELLEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli muuttuu vaihtelevaksi ja voimistuu edelleen"
#define ILTAPAIVALLA_TUULI_ALKAA_VOIMISTUA_JA_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli alkaa voimistua ja muuttuu vaihtelevaksi"
#define ILTAPAIVALLA_TUULI_VOIMISTUU_JA_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli voimistuu ja muuttuu vaihtelevaksi"
#define ILTAPAIVALLA_TUULI_MUUTTUU_VAIHTELEVAKSI_JA_HEIKKENEE_EDELLEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli muuttuu vaihtelevaksi ja heikkenee edelleen"
#define ILTAPAIVALLA_TUULI_HEIKKENEE_JA_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli heikkenee ja muuttuu vaihtelevaksi"
#define ILTAPAIVALLA_TUULI_ALKAA_HEIKETA_JA_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli alkaa heiketa ja muuttuu vaihtelevaksi"
#define POHJOISTUULTA_INTERVALLI_MS_JOKA_KAANTYY_ILTAPAIVALLA_ETELAAN_COMPOSITE_PHRASE \
  "[pohjoistuulta] [m...n] [metria sekunnissa], joka kaantyy [iltapaivalla] [etelaan]"
#define ILTAPAIVALLA_TUULI_KAANTYY_ETELAAN_COMPOSITE_PHRASE "[iltapaivalla] tuuli kaantyy [etelaan]"
#define ILTAPAIVALLA_TUULI_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli muuttuu vaihtelevaksi"
#define ILTAPAIVALLA_ETELATUULI_HEIKKENEE_NOPEASTI_COMPOSITE_PHRASE \
  "[iltapaivalla] [etelatuuli] heikkenee [nopeasti]"
#define ILTAPAIVALLA_ETELATUULI_VOIMISTUU_NOPEASTI_COMPOSITE_PHRASE \
  "[iltapaivalla] [etelatuuli] voimistuu [nopeasti]"
#define ILTAPAIVALLA_NOPEASTI_HEIKKENEVAA_ETELATUULTA_COMPOSITE_PHRASE \
  "[iltapaivalla] [nopeasti] [heikkenevaa] [etelatuulta]"
#define ILTAPAIVALLA_NOPEASTI_HEIKKENEVAA_ETELATUULTA_JOKA_KAANTYY_POHJOISEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] [nopeasti] [heikkenevaa] [etelatuulta], joka kaantyy [pohjoiseen]"
#define ILTAPAIVALLA_NOPEASTI_HEIKKENEVAA_ETELATUULTA_JOKA_KAANTYY_ILLALLA_POHJOISEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] [nopeasti] [heikkenevaa] [etelatuulta], joka kaantyy [illalla] [pohjoiseen]"
#define ILTAPAIVALLA_NOPEASTI_HEIKKENEVAA_ETELATUULTA_INTERVALLI_MS_JOKA_KAANTYY_ILLALLA_POHJOISEEN_COMPOSITE_PHRASE \
  "[iltap] [nop] [heikken] [tuulta] [m...n] [m sek], joka kaantyy [illalla] [pohj]"
#define ILTAPAIVALLA_ETELATUULTA_COMPOSITE_PHRASE "[iltapaivalla] [etelatuulta]"

#define ILTAPAIVALLA_TUULI_VOIMISTUU_JA_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli voimistuu ja muuttuu tilapaisesti vaihtelevaksi"
#define ILTAPAIVALLA_TUULI_ALKAA_VOIMISTUA_JA_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli alkaa voimistua ja muuttuu tilapaisesti vaihtelevaksi"
#define ILTAPAIVALLA_TUULI_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_JA_VOIMISTUU_EDELLEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli muuttuu tilapaisesti vaihtelevaksi ja voimistuu edelleen"
#define ILTAPAIVALLA_TUULI_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_JA_HEIKKENEE_EDELLEEN_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli muuttuu tilapaisesti vaihtelevaksi ja heikkenee edelleen"
#define ILTAPAIVALLA_TUULI_HEIKKENEE_JA_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli heikkenee ja muuttuu tilapaisesti vaihtelevaksi"
#define ILTAPAIVALLA_TUULI_ALKAA_HEIKETA_JA_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli alkaa heiketa ja muuttuu tilapaisesti vaihtelevaksi"
#define ILTAPAIVALLA_TUULI_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli muuttuu tilapaisesti vaihtelevaksi"

#define ILTAPAIVALLA_ETELATUULI_ALKAA_HEIKETA_NOPEASTI_COMPOSITE_PHRASE \
  "[iltapaivalla] [etelatuuli] alkaa heiketa [nopeasti]"
#define ILTAPAIVALLA_ETELATUULI_ALKAA_VOIMISTUA_NOPEASTI_COMPOSITE_PHRASE \
  "[iltapaivalla] [etelatuuli] alkaa voimistua [nopeasti]"
#define ILTAPAIVALLA_TUULI_ALKAA_VOIMISTUA_JA_KAANTYY_ETELAAN_COMPOSITE_PHRASE \
  "[iltapaivalla] tuuli alkaa voimistua ja kaantyy [etelaan]"
#define ILTAPAIVALLA_POHJOISTUULI_ALKAA_HEIKETA_JA_KAANTYY_ETELAAN_COMPOSITE_PHRASE \
  "[iltapaivalla] [pohjoistuuli] alkaa heiketa ja kaantyy [etelaan]"
#define ILTAPAIVALLA_POHJOISTUULTA_INTERVALLI_MS_JOKA_ALKAA_HEIKETA_NOPEASTI_JA_KAANTYY_ILLALLA_ETELAAN_COMPOSITE_PHRASE \
  "[ip] [p-tuulta] [m...n] [m/s], joka alkaa heiketa [nopeasti] ja kaantyy [illalla] [etelaan]"
#define ILTAPAIVALLA_POHJOISTUULTA_INTERVALLI_MS_JOKA_ALKAA_VOIMISTUA_NOPEASTI_JA_KAANTYY_ILLALLA_ETELAAN_COMPOSITE_PHRASE \
  "[ip] [p-tuulta] [m...n] [m/s], joka alkaa voimistua [nopeasti] ja kaantyy [illalla] [etelaan]"

}  // namespace WindForecastPhrases

using namespace WindForecastPhrases;

part_of_the_day_id get_most_relevant_part_of_the_day_id_narrow(const WeatherPeriod& thePeriod,
                                                               bool theAlkaenPhrase);

part_of_the_day_id get_part_of_the_day_id_wind(const TextGenPosixTime& theTime)
{
  int hour = theTime.GetHour();

  if (hour >= 0 && hour <= 6)
    return AAMUYO;
  else if (hour > 6 && hour <= 9)
    return AAMU;
  else if (hour > 9 && hour < 12)
    return AAMUPAIVA;
  else if (hour >= 12 && hour <= 18)
    return ILTAPAIVA;
  else if (hour > 18 && hour <= 21)
    return ILTA;
  else  // if (hour > 21)
    return ILTAYO;
}

part_of_the_day_id get_part_of_the_day_id_wind(const WeatherPeriod& thePeriod)
{
  return get_most_relevant_part_of_the_day_id_narrow(thePeriod, get_period_length(thePeriod));
}

bool operator==(const TimePhraseInfo& p1, const TimePhraseInfo& p2)
{
  if (abs(p1.starttime.DifferenceInHours(p2.starttime)) <= 2) return true;

  return p1.day_number == p2.day_number && p1.part_of_the_day == p2.part_of_the_day;
}

bool operator!=(const TimePhraseInfo& p1, const TimePhraseInfo& p2)
{
  return ((p1.day_number != p2.day_number || p1.part_of_the_day != p2.part_of_the_day));
}

std::ostream& operator<<(std::ostream& os, const TimePhraseInfo& tpi)
{
  os << tpi.starttime << "..." << tpi.endtime << " -> ";
  switch (tpi.day_number)
  {
    case 1:
      os << "maanantai";
      break;
    case 2:
      os << "tiistai";
      break;
    case 3:
      os << "keskiviikko";
      break;
    case 4:
      os << "torstai";
      break;
    case 5:
      os << "perjantai";
      break;
    case 6:
      os << "lauantai";
      break;
    case 7:
      os << "sunnuntai";
      break;
  }
  os << part_of_the_day_string(tpi.part_of_the_day) << std::endl;

  return os;
}

std::string as_string(const GlyphContainer& gc)
{
  DebugTextFormatter dtf;

  return gc.realize(dtf);
}

float wind_direction_error(const wind_data_item_vector& theWindDataVector,
                           const WeatherArea& theArea,
                           const WeatherPeriod& thePeriod)
{
  unsigned int counter(0);
  float cumulativeWindDirectionError(0.0);

  for (unsigned int i = 0; i < theWindDataVector.size(); i++)
  {
    WindDataItemUnit& item = theWindDataVector[i]->getDataItem(theArea.type());
    if (is_inside(item.thePeriod.localStartTime(), thePeriod))
    {
      cumulativeWindDirectionError += item.theEqualizedWindDirection.error();
      counter++;
    }
  }

  return (counter > 0 ? (cumulativeWindDirectionError / counter) : 0.0);
}

// return true if equalized top wind is weak duuring whole period
bool is_weak_period(const wo_story_params& theParameters, const WeatherPeriod& thePeriod)
{
  BOOST_FOREACH (const WindDataItemsByArea* item, theParameters.theWindDataVector)
  {
    WindDataItemUnit& dataitem = item->getDataItem(theParameters.theArea.type());

    if (is_inside(dataitem.thePeriod.localStartTime(), thePeriod) &&
        dataitem.theEqualizedTopWind.value() > WEAK_WIND_SPEED_UPPER_LIMIT)
      return false;
  }

  return true;
}

// find top wind on period
unsigned int get_peak_wind(const WeatherPeriod& thePeriod, const wo_story_params& theParameters)
{
  unsigned int upper_index = 0;

  bool upper_index_updated(false);
  // iterate timesteps
  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    const WindDataItemUnit& windDataItem =
        (*theParameters.theWindDataVector[i])(theParameters.theArea.type());

    if (is_inside(windDataItem.thePeriod, thePeriod))
    {
      double totalShareAtTimestep = 0.0;
      // iterate shares at this timestep: shares are cumulated
      for (unsigned int k = 0; k < windDataItem.theWindSpeedDistributionTop.size(); k++)
      {
        WeatherResult shareToAdd(windDataItem.theWindSpeedDistributionTop[k].second);
        totalShareAtTimestep += shareToAdd.value();
        if (totalShareAtTimestep >= theParameters.theWindSpeedTopCoverage)
        {
          if (!upper_index_updated || k > static_cast<unsigned int>(upper_index))
          {
            upper_index = k;
            upper_index_updated = true;
          }
          break;
        }
      }
    }
  }

  return upper_index;
}

WeatherResult get_wind_direction_result_at(const wo_story_params& theParameters,
                                           const TextGenPosixTime& pointOfTime,
                                           const string& var)
{
  WeatherResult retval(kFloatMissing, 0.0);

  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    WindDataItemUnit& item =
        theParameters.theWindDataVector[i]->getDataItem(theParameters.theArea.type());
    if (item.thePeriod.localStartTime() == pointOfTime)
    {
      retval = item.theEqualizedWindDirection;
      break;
    }
  }

  return retval;
}

bool is_gusty_wind(const wo_story_params& theParameters,
                   const TextGenPosixTime& pointOfTime,
                   const string& var)
{
  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    WindDataItemUnit& item =
        theParameters.theWindDataVector[i]->getDataItem(theParameters.theArea.type());
    if (item.thePeriod.localStartTime() == pointOfTime)
    {
      return (item.theEqualizedTopWind.value() >= 20.0 &&
              item.theGustSpeed.value() - item.theEqualizedTopWind.value() >=
                  theParameters.theGustyWindTopWindDifference);
    }
  }
  return false;
}

bool is_gusty_wind(const wo_story_params& theParameters,
                   const WeatherPeriod& period,
                   const string& var)
{
  float speedSum(0.0);
  float gustSpeedSum(0.0);
  unsigned int n(0);
  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    WindDataItemUnit& item =
        theParameters.theWindDataVector[i]->getDataItem(theParameters.theArea.type());
    if (is_inside(item.thePeriod.localStartTime(), period))
    {
      speedSum += item.theEqualizedTopWind.value();
      gustSpeedSum += item.theGustSpeed.value();
      n++;
    }
  }

  if (n > 0)
  {
    float avgTopWind(speedSum / n);
    float avgGustSpeed(gustSpeedSum / n);
    return (avgTopWind >= 20.0 &&
            avgGustSpeed - avgTopWind >= theParameters.theGustyWindTopWindDifference);
  }

  return false;
}

// If proposed direction is 'PUOLEINEN' and the previous was in the same direction, change the
// latter to prevent tautology
// for example (LOUNAISTUULI, LOUNAAN PUOLEINEN) -> (LOUNAISTUULI, LÄNNEN JA LOUNAAN VÄLINEN)
WindDirectionInfo negotiateWindDirection(WindDirectionInfo& theProposedWindDirection,
                                         const WindDirectionInfo& thePreviousWindDirection)
{
  if ((theProposedWindDirection.id == POHJOISEN_PUOLEINEN &&
       thePreviousWindDirection.id == POHJOINEN) ||
      (theProposedWindDirection.id == KOILLISEN_PUOLEINEN &&
       thePreviousWindDirection.id == KOILLINEN) ||
      (theProposedWindDirection.id == IDAN_PUOLEINEN && thePreviousWindDirection.id == ITA) ||
      (theProposedWindDirection.id == KAAKON_PUOLEINEN && thePreviousWindDirection.id == KAAKKO) ||
      (theProposedWindDirection.id == ETELAN_PUOLEINEN && thePreviousWindDirection.id == ETELA) ||
      (theProposedWindDirection.id == LOUNAAN_PUOLEINEN && thePreviousWindDirection.id == LOUNAS) ||
      (theProposedWindDirection.id == LANNEN_PUOLEINEN && thePreviousWindDirection.id == LANSI) ||
      (theProposedWindDirection.id == LUOTEEN_PUOLEINEN && thePreviousWindDirection.id == LUODE))
    theProposedWindDirection.id = direction_between_id(theProposedWindDirection.direction.value());

  return theProposedWindDirection;
}

WindDirectionInfo get_wind_direction(const wo_story_params& theParameters,
                                     const TextGenPosixTime& pointOfTime,
                                     const WindDirectionInfo* thePreviousWindDirection = 0)
{
  WeatherResult directionValue(kFloatMissing, 0.0);
  WindDirectionId directionId(MISSING_WIND_DIRECTION_ID);

  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    WindDataItemUnit& item =
        theParameters.theWindDataVector[i]->getDataItem(theParameters.theArea.type());

    if (item.thePeriod.localStartTime() == pointOfTime)
    {
      directionValue = item.theEqualizedWindDirection;
      directionId = wind_direction_id(item.theEqualizedWindDirection,
                                      item.theEqualizedTopWind,
                                      theParameters.theVar,
                                      theParameters.theWindDirectionMinSpeed);
    }
  }

  WindDirectionInfo windDirectionInfo(directionValue, directionId);

  return windDirectionInfo;
}

WindDirectionInfo get_wind_direction(const wo_story_params& theParameters,
                                     const WeatherPeriod& period,
                                     const WindDirectionInfo* thePreviousWindDirection = 0)
{
  float topSpeedSum(0.0);
  float topSpeedStdDevSum(0.0);
  float medianSpeedSum(0.0);
  float medianSpeedStdDevSum(0.0);
  unsigned int n(0);
  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    WindDataItemUnit& item =
        theParameters.theWindDataVector[i]->getDataItem(theParameters.theArea.type());
    if (is_inside(item.thePeriod.localStartTime(), period))
    {
      topSpeedSum += item.theEqualizedTopWind.value();
      topSpeedStdDevSum += item.theEqualizedTopWind.error();
      medianSpeedSum += item.theEqualizedMedianWind.value();
      medianSpeedStdDevSum += item.theEqualizedMedianWind.error();
      n++;
    }
  }
  float avgTopWind(topSpeedSum / n);
  float avgTopWindStdDev(topSpeedStdDevSum / n);
  float avgMedianWind(medianSpeedSum / n);
  float avgMedianWindStdDev(medianSpeedStdDevSum / n);
  WeatherResult topWindSpeed(avgTopWind, avgTopWindStdDev);
  WeatherResult medianWindSpeed(avgMedianWind, avgMedianWindStdDev);

  WeatherResult resultDirection = WindStoryTools::mode_wind_direction(theParameters.theSources,
                                                                      theParameters.theArea,
                                                                      period,
                                                                      medianWindSpeed,
                                                                      topWindSpeed,
                                                                      theParameters.theVar);

  float directionError =
      wind_direction_error(theParameters.theWindDataVector, theParameters.theArea, period);

  if (directionError < resultDirection.error())
    resultDirection = WeatherResult(resultDirection.value(), directionError);

  WindDirectionInfo windDirectionInfo =
      WindDirectionInfo(resultDirection,
                        wind_direction_id(resultDirection,
                                          topWindSpeed,
                                          theParameters.theVar,
                                          theParameters.theWindDirectionMinSpeed));
  return windDirectionInfo;
}

std::vector<WeatherPeriod> get_wind_direction_periods(const wo_story_params& theParameters,
                                                      const WeatherPeriod& thePeriod)
{
  std::vector<WeatherPeriod> ret;

  unsigned int firstIndex(UINT_MAX);
  unsigned int lastIndex(UINT_MAX);

  // find start and end index
  bool periodFound = false;
  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    const WindDataItemUnit& windDataItem =
        theParameters.theWindDataVector[i]->getDataItem(theParameters.theArea.type());

    if (is_inside(windDataItem.thePeriod, thePeriod))
    {
      if (firstIndex == UINT_MAX) firstIndex = i;
      lastIndex = i;
      periodFound = true;
    }
  }

  if (!periodFound) return ret;

  WindDataItemUnit previousWindDataItem =
      theParameters.theWindDataVector[firstIndex]->getDataItem(theParameters.theArea.type());

  WindDirectionInfo previousWD = get_wind_direction(theParameters, previousWindDataItem.thePeriod);

  std::vector<TextGenPosixTime> changeTimes;
  changeTimes.push_back(previousWindDataItem.thePeriod.localStartTime());

  for (unsigned int index = firstIndex + 1; index < lastIndex; index++)
  {
    WindDataItemUnit currentWindDataItem =
        theParameters.theWindDataVector[index]->getDataItem(theParameters.theArea.type());

    WindDirectionInfo currentWD = get_wind_direction(theParameters, currentWindDataItem.thePeriod);

    if (currentWD.id == previousWD.id) continue;
    if (currentWD.id != VAIHTELEVA && previousWD.id != VAIHTELEVA &&
        !wind_direction_differ_enough(
            previousWD.direction, currentWD.direction, theParameters.theWindDirectionThreshold))
      continue;

    changeTimes.push_back(currentWindDataItem.thePeriod.localStartTime());

    previousWindDataItem = currentWindDataItem;
    previousWD = currentWD;
  }

  if (changeTimes.size() == 0) return ret;

  std::vector<WeatherPeriod> directionPeriods;
  WeatherPeriod newPeriod(changeTimes[0], changeTimes[0]);
  WeatherPeriod previousPeriod(changeTimes[0], changeTimes[0]);
  for (unsigned int i = 0; i < changeTimes.size(); i++)
  {
    if (i < changeTimes.size() - 1)
    {
      TextGenPosixTime startTime = changeTimes[i];
      TextGenPosixTime endTime = changeTimes[i + 1];
      endTime.ChangeByHours(-1);
      newPeriod = WeatherPeriod(startTime, endTime);
    }
    else
    {
      TextGenPosixTime startTime = changeTimes[i];
      TextGenPosixTime endTime = thePeriod.localEndTime();
      newPeriod = WeatherPeriod(startTime, endTime);
    }

    // if direction id is the same merge previous and new period
    if (directionPeriods.size() > 0 &&
        get_wind_direction(theParameters, newPeriod).id ==
            get_wind_direction(theParameters, previousPeriod).id)
    {
      directionPeriods.back() =
          WeatherPeriod(previousPeriod.localStartTime(), newPeriod.localEndTime());
    }
    else
    {
      directionPeriods.push_back(newPeriod);
    }

    previousPeriod = directionPeriods.back();
  }

  theParameters.theLog << "** ORIGINAL DIRECTION PERIODS **" << std::endl;

  BOOST_FOREACH (const WeatherPeriod& p, directionPeriods)
  {
    WindDirectionInfo wdi = get_wind_direction(theParameters, p);

    theParameters.theLog << p << " -> " << wind_direction_string(wdi.id) << ", "
                         << wdi.direction.value() << "(" << wdi.direction.error() << ")"
                         << std::endl;
  }

  // remove short periods (max 2 hours) aroud varying wind, except the last period
  std::set<unsigned int> indexesToRemove;
  for (unsigned int i = 1; i < directionPeriods.size() - 1; i++)
  {
    WeatherPeriod previousPeriod = directionPeriods[i - 1];
    WeatherPeriod currentPeriod = directionPeriods[i];
    WeatherPeriod nextPeriod = directionPeriods[i + 1];
    WindDirectionId currentId =
        get_wind_direction(theParameters, currentPeriod.localStartTime()).id;
    if (currentId == VAIHTELEVA)
    {
      if (i != 1 && get_period_length(previousPeriod) <= 2)
      {
        indexesToRemove.insert(i - 1);
      }
      if (i + 1 != directionPeriods.size() - 1 && get_period_length(nextPeriod) <= 2)
      {
        indexesToRemove.insert(i + 1);
      }
    }
  }

  std::vector<WeatherPeriod> cleanedPeriods;
  for (unsigned int i = 0; i < directionPeriods.size(); i++)
  {
    // short period in the beginning is ignored
    if (i == 0 && get_period_length(directionPeriods[i]) < 1) continue;

    if (indexesToRemove.find(i) == indexesToRemove.end())
      cleanedPeriods.push_back(directionPeriods[i]);
  }

  for (unsigned int i = 0; i < cleanedPeriods.size(); i++)
  {
    WeatherPeriod period = cleanedPeriods[i];
    WindDirectionId id = get_wind_direction(theParameters, period).id;

    // dont report short varying wind if it is not the first/last one
    if (i < cleanedPeriods.size() - 1 && id == VAIHTELEVA && get_period_length(period) < 4 &&
        i != 0)
      continue;
    ret.push_back(period);
  }

  return ret;
}

std::string get_wind_event_string(WindEventId theWindEventId)
{
  std::string retval;

  switch (theWindEventId)
  {
    case TUULI_HEIKKENEE:
      retval = "tuuli heikkenee";
      break;
    case TUULI_VOIMISTUU:
      retval = "tuuli voimistuu";
      break;
    case TUULI_TYYNTYY:
      retval = "tuuli tyyntyy";
      break;
    case TUULI_KAANTYY:
      retval = "tuuli kaantyy";
      break;
    case TUULI_KAANTYY_JA_HEIKKENEE:
      retval = "tuuli kaantyy ja heikkenee";
      break;
    case TUULI_KAANTYY_JA_VOIMISTUU:
      retval = "tuuli kaantyy ja voimistuu";
      break;
    case TUULI_KAANTYY_JA_TYYNTYY:
      retval = "tuuli kaantyy ja tyyntyy";
      break;
    case TUULI_MUUTTUU_VAIHTELEVAKSI:
      retval = "tuuli muuttuu vaihtelevaksi";
      break;
    case TUULI_MUUTTUU_VAIHTELEVAKSI_JA_HEIKKENEE:
      retval = "tuuli muuttuu vaihtelevaksi ja heikkenee";
      break;
    case TUULI_MUUTTUU_VAIHTELEVAKSI_JA_VOIMISTUU:
      retval = "tuuli muuttuu vaihtelevaksi ja voimistuu";
      break;
    case TUULI_MUUTTUU_VAIHTELEVAKSI_JA_TYYNTYY:
      retval = "tuuli muuttuu vaihtelevaksi ja tyyntyy";
      break;
    case MISSING_WIND_EVENT:
      retval = "missing wind event";
      break;
    case MISSING_WIND_SPEED_EVENT:
      retval = "missing wind speed event";
      break;
    case MISSING_WIND_DIRECTION_EVENT:
      retval = "missing wind direction event";
      break;
  }

  return retval;
}

Sentence windDirectionSentence(WindDirectionId& theWindDirectionId, bool theBasicForm = false)
{
  Sentence sentence;

  switch (theWindDirectionId)
  {
    case POHJOINEN:
      sentence << (theBasicForm ? POHJOINEN_TUULI : POHJOINEN_TUULI_P);
      break;
    case POHJOISEN_PUOLEINEN:
      sentence << (theBasicForm ? POHJOISEN_PUOLEINEN_TUULI : POHJOISEN_PUOLEINEN_TUULI_P);
      break;
    case POHJOINEN_KOILLINEN:
      sentence << (theBasicForm ? POHJOINEN_KOILLINEN_TUULI : POHJOINEN_KOILLINEN_TUULI_P);
      break;
    case KOILLINEN:
      sentence << (theBasicForm ? KOILLINEN_TUULI : KOILLINEN_TUULI_P);
      break;
    case KOILLISEN_PUOLEINEN:
      sentence << (theBasicForm ? KOILLISEN_PUOLEINEN_TUULI : KOILLISEN_PUOLEINEN_TUULI_P);
      break;
    case ITA_KOILLINEN:
      sentence << (theBasicForm ? ITA_KOILLINEN_TUULI : ITA_KOILLINEN_TUULI_P);
      break;
    case ITA:
      sentence << (theBasicForm ? ITA_TUULI : ITA_TUULI_P);
      break;
    case IDAN_PUOLEINEN:
      sentence << (theBasicForm ? IDAN_PUOLEINEN_TUULI : IDAN_PUOLEINEN_TUULI_P);
      break;
    case ITA_KAAKKO:
      sentence << (theBasicForm ? ITA_KAAKKO_TUULI : ITA_KAAKKO_TUULI_P);
      break;
    case KAAKKO:
      sentence << (theBasicForm ? KAAKKO_TUULI : KAAKKO_TUULI_P);
      break;
    case KAAKON_PUOLEINEN:
      sentence << (theBasicForm ? KAAKON_PUOLEINEN_TUULI : KAAKON_PUOLEINEN_TUULI_P);
      break;
    case ETELA_KAAKKO:
      sentence << (theBasicForm ? ETELA_KAAKKO_TUULI : ETELA_KAAKKO_TUULI_P);
      break;
    case ETELA:
      sentence << (theBasicForm ? ETELA_TUULI : ETELA_TUULI_P);
      break;
    case ETELAN_PUOLEINEN:
      sentence << (theBasicForm ? ETELAN_PUOLEINEN_TUULI : ETELAN_PUOLEINEN_TUULI_P);
      break;
    case ETELA_LOUNAS:
      sentence << (theBasicForm ? ETELA_LOUNAS_TUULI : ETELA_LOUNAS_TUULI_P);
      break;
    case LOUNAS:
      sentence << (theBasicForm ? LOUNAS_TUULI : LOUNAS_TUULI_P);
      break;
    case LOUNAAN_PUOLEINEN:
      sentence << (theBasicForm ? LOUNAAN_PUOLEINEN_TUULI : LOUNAAN_PUOLEINEN_TUULI_P);
      break;
    case LANSI_LOUNAS:
      sentence << (theBasicForm ? LANSI_LOUNAS_TUULI : LANSI_LOUNAS_TUULI_P);
      break;
    case LANSI:
      sentence << (theBasicForm ? LANSI_TUULI : LANSI_TUULI_P);
      break;
    case LANNEN_PUOLEINEN:
      sentence << (theBasicForm ? LANNEN_PUOLEINEN_TUULI : LANNEN_PUOLEINEN_TUULI_P);
      break;
    case LANSI_LUODE:
      sentence << (theBasicForm ? LANSI_LUODE_TUULI : LANSI_LUODE_TUULI_P);
      break;
    case LUODE:
      sentence << (theBasicForm ? LUODE_TUULI : LUODE_TUULI_P);
      break;
    case LUOTEEN_PUOLEINEN:
      sentence << (theBasicForm ? LUOTEEN_PUOLEINEN_TUULI : LUOTEEN_PUOLEINEN_TUULI_P);
      break;
    case POHJOINEN_LUODE:
      sentence << (theBasicForm ? POHJOINEN_LUODE_TUULI : POHJOINEN_LUODE_TUULI_P);
      break;
    case VAIHTELEVA:
    case MISSING_WIND_DIRECTION_ID:
      sentence << (theBasicForm ? VAIHTELEVA_TUULI : VAIHTELEVA_TUULI_P);
      break;
  }

  return sentence;
}

std::string getWindDirectionTurntoString(WindDirectionId theWindDirectionId)
{
  std::string retval;

  switch (theWindDirectionId)
  {
    case POHJOINEN:
      retval = POHJOISEEN_PHRASE;
      break;
    case POHJOISEN_PUOLEINEN:
      retval = POHJOISEN_PUOLELLE_PHRASE;
      break;
    case POHJOINEN_KOILLINEN:
      retval = POHJOISEN_JA_KOILLISEN_VALILLE_PHRASE;
      break;
    case KOILLINEN:
      retval = KOILLISEEN_PHRASE;
      break;
    case KOILLISEN_PUOLEINEN:
      retval = KOILLISEN_PUOLELLE_PHRASE;
      break;
    case ITA_KOILLINEN:
      retval = IDAN_JA_KOILLISEN_VALILLE_PHRASE;
      break;
    case ITA:
      retval = ITAAN_PHRASE;
      break;
    case IDAN_PUOLEINEN:
      retval = IDAN_PUOLELLE_PHRASE;
      break;
    case ITA_KAAKKO:
      retval = IDAN_JA_KAAKON_VALILLE_PHRASE;
      break;
    case KAAKKO:
      retval = KAAKKOON_PHRASE;
      break;
    case KAAKON_PUOLEINEN:
      retval = KAAKON_PUOLELLE_PHRASE;
      break;
    case ETELA_KAAKKO:
      retval = ETELAN_JA_KAAKON_VALILLE_PHRASE;
      break;
    case ETELA:
      retval = ETELAAN_PHRASE;
      break;
    case ETELAN_PUOLEINEN:
      retval = ETELAN_PUOLELLE_PHRASE;
      break;
    case ETELA_LOUNAS:
      retval = ETELAN_JA_LOUNAAN_VALILLE_PHRASE;
      break;
    case LOUNAS:
      retval = LOUNAASEEN_PHRASE;
      break;
    case LOUNAAN_PUOLEINEN:
      retval = LOUNAAN_PUOLELLE_PHRASE;
      break;
    case LANSI_LOUNAS:
      retval = LANNEN_JA_LOUNAAN_VALILLE_PHRASE;
      break;
    case LANSI:
      retval = LANTEEN_PHRASE;
      break;
    case LANNEN_PUOLEINEN:
      retval = LANNEN_PUOLELLE_PHRASE;
      break;
    case LANSI_LUODE:
      retval = LANNEN_JA_LUOTEEN_VALILLE_PHRASE;
      break;
    case LUODE:
      retval = LUOTEESEEN_PHRASE;
      break;
    case LUOTEEN_PUOLEINEN:
      retval = LUOTEEN_PUOLELLE_PHRASE;
      break;
    case POHJOINEN_LUODE:
      retval = POHJOISEN_JA_LUOTEEN_VALILLE_PHRASE;
      break;
    case VAIHTELEVA:
      retval = TUULI_MUUTTUU_VAIHTELEVAKSI_PHRASE;
      break;
    case MISSING_WIND_DIRECTION_ID:
      retval = EMPTY_STRING;
      break;
  }

  return retval;
}

void get_plain_wind_speed_interval(const WeatherPeriod& period,
                                   wo_story_params& theParameter,
                                   float& lowerLimit,
                                   float& upperLimit)
{
  bool firstMatchProcessed(false);

  theParameter.theContextualMaxIntervalSize = theParameter.theMaxIntervalSize;

  float minValue(lowerLimit);
  float topValue(upperLimit);
  float maxValue(upperLimit);
  for (unsigned int i = 0; i < theParameter.theWindDataVector.size(); i++)
  {
    const WindDataItemUnit& windDataItem =
        (*theParameter.theWindDataVector[i])(theParameter.theArea.type());

    if (is_inside(windDataItem.thePeriod, period))
    {
      if (!firstMatchProcessed)
      {
        minValue = windDataItem.theEqualizedMedianWind.value();
        topValue = windDataItem.theEqualizedTopWind.value();
        maxValue = windDataItem.theEqualizedMaxWind.value();

        firstMatchProcessed = true;
      }
      else
      {
        if (windDataItem.theEqualizedMedianWind.value() < minValue)
          minValue = windDataItem.theEqualizedMedianWind.value();
        if (windDataItem.theEqualizedTopWind.value() > topValue)
          topValue = windDataItem.theEqualizedTopWind.value();
        if (windDataItem.theEqualizedMaxWind.value() > maxValue)
          maxValue = windDataItem.theEqualizedMaxWind.value();
      }
    }
  }
  lowerLimit = minValue;
  // if top wind is smaller than configuration value, use maximum wind as upper limit
  if (topValue < theParameter.theWindSpeedWarningThreshold)
  {
    // Annakaisa / Punkka 06.06.2016: pienennetään maksimikoko neljään
    theParameter.theLog << "Top wind speed (" << topValue
                        << ") is smaller than theWindSpeedWarningThreshold ("
                        << theParameter.theWindSpeedWarningThreshold
                        << ") setting interval maximum size to 4." << std::endl;
    theParameter.theContextualMaxIntervalSize = 4;
    upperLimit = maxValue;
  }
  else
  {
    upperLimit = topValue;
  }
}

float get_median_wind(const WeatherPeriod& thePeriod, const wo_story_params& theParameters)
{
  float retval(0.0);
  unsigned int counter(0);

  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    const WindDataItemUnit& windDataItem =
        (*theParameters.theWindDataVector[i])(theParameters.theArea.type());

    if (is_inside(windDataItem.thePeriod, thePeriod))
    {
      retval += windDataItem.theEqualizedMedianWind.value();
      counter++;
    }
  }

  return (counter == 0 ? retval : retval / counter);
}

float get_top_wind(const WeatherPeriod& thePeriod, const wo_story_params& theParameters)
{
  float top_wind(kFloatMissing);

  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    const WindDataItemUnit& windDataItem =
        (*theParameters.theWindDataVector[i])(theParameters.theArea.type());

    if (is_inside(windDataItem.thePeriod, thePeriod))
    {
      if (top_wind == kFloatMissing || windDataItem.theWindSpeedTop.value() > top_wind)
        top_wind = windDataItem.theWindSpeedTop.value();
    }
  }

  return top_wind;
}

void windspeed_distribution_interval(const WeatherPeriod& thePeriod,
                                     const wo_story_params& theParameter,
                                     float coverage,
                                     float& lowerLimit,
                                     float& upperLimit,
                                     unsigned int& mostTypicalWindSpeed)
{
  if (lowerLimit == kFloatMissing || upperLimit == kFloatMissing) return;

  value_distribution_data_vector windSpeedDistributionVector;

  // first calculte the sum of the distribution values for the period for each wind speed
  unsigned int counter(0);
  for (unsigned int i = 0; i < theParameter.theWindDataVector.size(); i++)
  {
    const WindDataItemUnit& windDataItem =
        (*theParameter.theWindDataVector[i])(theParameter.theArea.type());

    if (is_inside(windDataItem.thePeriod, thePeriod))
    {
      if (windSpeedDistributionVector.size() == 0)
      {
        windSpeedDistributionVector = windDataItem.theWindSpeedDistribution;
      }
      else
      {
        for (unsigned int k = 0; k < windDataItem.theWindSpeedDistribution.size(); k++)
        {
          WeatherResult originalShare(windSpeedDistributionVector[k].second);
          WeatherResult shareToAdd(windDataItem.theWindSpeedDistribution[k].second);
          windSpeedDistributionVector[k].second =
              WeatherResult(originalShare.value() + shareToAdd.value(), 0.0);
        }
      }
      counter++;
    }
  }

  mostTypicalWindSpeed = 0;
  // then calculate mean value of the distribution values of the period
  for (unsigned int i = 0; i < windSpeedDistributionVector.size(); i++)
  {
    WeatherResult cumulativeShare(windSpeedDistributionVector[i].second);
    windSpeedDistributionVector[i].second = WeatherResult(cumulativeShare.value() / counter, 0.0);
    // store most typical wind speed category
    if (i > 0 &&
        windSpeedDistributionVector[mostTypicalWindSpeed].second.value() <
            windSpeedDistributionVector[i].second.value())
      mostTypicalWindSpeed = i;
  }

  // Make sure that there is enough values (coverage) inside the interval
  int lower_index = static_cast<int>(round(upperLimit));
  if (lower_index >= static_cast<int>(windSpeedDistributionVector.size()))
    lower_index = windSpeedDistributionVector.size() - 1;

  float sumDistribution = 0.0;
  while (sumDistribution < coverage && lower_index >= 0)
  {
    sumDistribution += windSpeedDistributionVector[lower_index].second.value();
    lower_index--;
  }

  lowerLimit = lower_index;
}

void get_wind_speed_interval(const WeatherPeriod& thePeriod,
                             wo_story_params& theParameter,
                             float& lowerLimit,
                             float& upperLimit)
{
  std::stringstream ss_log;

  ss_log << " wind_speed_interval:\n period: " << thePeriod << std::endl;

  // first get plain interval: smallest median wind on period, highest top wind or maximum wind on
  // period
  float plainLowerLimit = lowerLimit;
  float plainUpperLimit = upperLimit;
  get_plain_wind_speed_interval(thePeriod, theParameter, plainLowerLimit, plainUpperLimit);

  ss_log << " plain interval: " << plainLowerLimit << "..." << plainUpperLimit << std::endl;

  float distributionLowerLimit67 = plainLowerLimit;
  float distributionUpperLimit67 = plainUpperLimit;
  unsigned int mostTypicalWindSpeed = 0;
  // interval must contain 67% of winds
  float intervalCoverage = 67.0;
  windspeed_distribution_interval(thePeriod,
                                  theParameter,
                                  intervalCoverage,
                                  distributionLowerLimit67,
                                  distributionUpperLimit67,
                                  mostTypicalWindSpeed);

  ss_log << " distribution interval 67: " << distributionLowerLimit67 << "..."
         << distributionUpperLimit67 << " most typical: " << mostTypicalWindSpeed << std::endl;

  if (abs(distributionUpperLimit67 - distributionLowerLimit67) >
      theParameter.theContextualMaxIntervalSize)
  {
    float distributionLowerLimit50 = plainLowerLimit;
    float distributionUpperLimit50 = plainUpperLimit;
    // try with 50% of winds
    intervalCoverage = 50.0;
    windspeed_distribution_interval(thePeriod,
                                    theParameter,
                                    intervalCoverage,
                                    distributionLowerLimit50,
                                    distributionUpperLimit50,
                                    mostTypicalWindSpeed);

    ss_log << " distribution interval 50: " << distributionLowerLimit50 << "..."
           << distributionUpperLimit50 << " most typical: " << mostTypicalWindSpeed << std::endl;

    if (distributionLowerLimit50 >= mostTypicalWindSpeed)
      lowerLimit = distributionLowerLimit50;
    else
      lowerLimit = distributionLowerLimit67;
  }
  else
  {
    lowerLimit = distributionLowerLimit67;
  }
  upperLimit = plainUpperLimit;

  // we may cut peak wind based on areal coverage (wind_speed_top_coverage configuration parameter)
  int peakWind = get_peak_wind(thePeriod, theParameter);

  ss_log << " interval before peak check: " << lowerLimit << "..." << upperLimit
         << " peak: " << peakWind << std::endl;

  if (peakWind < upperLimit) upperLimit = peakWind;

  ss_log << " interval after peak check: " << lowerLimit << "..." << upperLimit << std::endl;

  theParameter.theLog << ss_log.str();
}

void get_wind_speed_interval(const TextGenPosixTime& pointOfTime,
                             wo_story_params& theParameters,
                             float& lowerLimit,
                             float& upperLimit)
{
  WeatherPeriod period(pointOfTime, pointOfTime);

  get_wind_speed_interval(period, theParameters, lowerLimit, upperLimit);
}

float get_wind_speed_at(const wo_story_params& theParameters, const TextGenPosixTime& theTime)
{
  WeatherPeriod wp(theTime, theTime);

  float lowerLimit = get_median_wind(wp, theParameters);
  float upperLimit = get_top_wind(wp, theParameters);

  double topWindShare = (theParameters.theWeakTopWind ? theParameters.theWindCalcTopShareWeak
                                                      : theParameters.theWindCalcTopShare);

  double topWindWeight = (topWindShare / 100.0);
  double topMedianWindWeight = 1.0 - topWindWeight;

  return (upperLimit * topWindWeight + lowerLimit * topMedianWindWeight);
}

// return time when wind has changes n% of configured threshold
TextGenPosixTime get_wind_change_point(const wo_story_params& theParameters,
                                       const WeatherPeriod& thePeriod,
                                       float thePerCent)
{
  TextGenPosixTime timeIter(thePeriod.localStartTime());
  timeIter.ChangeByHours(1);

  while (timeIter < thePeriod.localEndTime())
  {
    WeatherPeriod end(timeIter, timeIter);

    float begSpeed = get_wind_speed_at(theParameters, thePeriod.localStartTime());
    float endSpeed = get_wind_speed_at(theParameters, timeIter);

    float difference = abs(endSpeed - begSpeed);

    if (difference >= theParameters.theWindSpeedThreshold * (thePerCent / 100.0)) break;
    timeIter.ChangeByHours(1);
  }

  return timeIter;
}

bool wind_speed_differ_enough(const wo_story_params& theParameters, const WeatherPeriod& thePeriod)
{
  float begSpeed = get_wind_speed_at(theParameters, thePeriod.localStartTime());
  float endSpeed = get_wind_speed_at(theParameters, thePeriod.localEndTime());

  // dont report change when wind is weak
  if (begSpeed < WEAK_WIND_SPEED_UPPER_LIMIT && endSpeed < WEAK_WIND_SPEED_UPPER_LIMIT)
    return false;

  float difference = abs(endSpeed - begSpeed);

  return (difference >= theParameters.theWindSpeedThreshold);
}

bool wind_direction_differ_enough(const WeatherResult theWindDirection1,
                                  const WeatherResult theWindDirection2,
                                  float theWindDirectionThreshold)
{
  float dire1 = theWindDirection1.value();
  float dire2 = theWindDirection2.value();

  float difference = abs(dire2 - dire1);
  if (difference > 180.0)
    difference = abs((dire2 > dire1 ? dire2 - (dire1 + 360) : dire1 - (dire2 + 360)));

  return (difference >= theWindDirectionThreshold);
}

bool wind_direction_differ_enough(const wo_story_params& theParameter,
                                  const WindEventPeriodDataItem& windEventPeriodDataItem)
{
  const WeatherResult& windDirection1(
      windEventPeriodDataItem.thePeriodBeginDataItem.theEqualizedWindDirection);
  const WeatherResult& windDirection2(
      windEventPeriodDataItem.thePeriodEndDataItem.theEqualizedWindDirection);
  const WeatherResult& maximumWind1(
      windEventPeriodDataItem.thePeriodBeginDataItem.theEqualizedTopWind);
  const WeatherResult& maximumWind2(
      windEventPeriodDataItem.thePeriodEndDataItem.theEqualizedTopWind);

  WindDirectionId directionIdBeg = wind_direction_id(
      windDirection1, maximumWind1, theParameter.theVar, theParameter.theWindDirectionMinSpeed);
  WindDirectionId directionIdEnd = wind_direction_id(
      windDirection2, maximumWind2, theParameter.theVar, theParameter.theWindDirectionMinSpeed);

  if (directionIdBeg == directionIdEnd) return false;

  return wind_direction_differ_enough(
      windDirection1, windDirection2, theParameter.theWindDirectionThreshold);
}

bool ascending_order(float direction1, float direction2)
{
  bool retval(false);

  if (direction1 >= 180.0)
  {
    if (direction2 > direction1 || direction2 < direction1 - 180.0) retval = true;
  }
  else
  {
    retval = (direction2 > direction1 && direction2 - direction1 < 180);
  }
  return retval;
}

bool almost_same_direction(WindDirectionId id1, WindDirectionId id2)
{
  if (id1 == id2) return true;
  if ((id1 == POHJOINEN && id2 == POHJOISEN_PUOLEINEN) ||
      (id2 == POHJOINEN && id1 == POHJOISEN_PUOLEINEN))
    return true;
  if ((id1 == KOILLINEN && id2 == KOILLISEN_PUOLEINEN) ||
      (id2 == KOILLINEN && id1 == KOILLISEN_PUOLEINEN))
    return true;
  if ((id1 == ITA && id2 == IDAN_PUOLEINEN) || (id2 == ITA && id1 == IDAN_PUOLEINEN)) return true;
  if ((id1 == KAAKKO && id2 == KAAKON_PUOLEINEN) || (id2 == KAAKKO && id1 == KAAKON_PUOLEINEN))
    return true;
  if ((id1 == ETELA && id2 == ETELAN_PUOLEINEN) || (id2 == ETELA && id1 == ETELAN_PUOLEINEN))
    return true;
  if ((id1 == LOUNAS && id2 == LOUNAAN_PUOLEINEN) || (id2 == LOUNAS && id1 == LOUNAAN_PUOLEINEN))
    return true;
  if ((id1 == LANSI && id2 == LANNEN_PUOLEINEN) || (id2 == LANSI && id1 == LANNEN_PUOLEINEN))
    return true;
  if ((id1 == LUODE && id2 == LUOTEEN_PUOLEINEN) || (id2 == LUODE && id1 == LUOTEEN_PUOLEINEN))
    return true;

  return false;
}

void compose_wind_speed_range(const wo_story_params& theParameters,
                              int& intervalLowerLimit,
                              int& intervalUpperLimit,
                              int& peakWind)
{
  int actualIntervalSize(intervalUpperLimit - intervalLowerLimit);
  std::stringstream ss_log;
  ss_log << std::endl;

  if (actualIntervalSize < theParameters.theMinIntervalSize)
  {
    // Mikael Frisk 07.03.2013: pelkkä 5 m/s on kökkö => näytetään minimi-intervalli
    /*
      sentence << intervalLowerLimit
      << *UnitFactory::create(MetersPerSecond);
    */
    ss_log << " interval size < theMinIntervalSize: " << actualIntervalSize << " <  "
           << theParameters.theMinIntervalSize << " -> use minimum interval size: ";

    intervalUpperLimit = intervalLowerLimit + theParameters.theMinIntervalSize;
    actualIntervalSize = intervalUpperLimit - intervalLowerLimit;

    ss_log << intervalLowerLimit << "..." << intervalUpperLimit << std::endl;
  }

  if (actualIntervalSize > theParameters.theContextualMaxIntervalSize)
  {
    ss_log << " interval size > theContextualMaxIntervalSize: " << actualIntervalSize << " <  "
           << theParameters.theContextualMaxIntervalSize << " -> adapt interval: ";
    // if size of the interval is one more than allowed, we increase lower limit by 1
    // else we use 'kovimmillaan'-phrase
    if (actualIntervalSize == theParameters.theContextualMaxIntervalSize + 1)
    {
      intervalLowerLimit++;
    }
    else
    {
      intervalUpperLimit = intervalLowerLimit + theParameters.theContextualMaxIntervalSize;
    }
    ss_log << intervalLowerLimit << "..." << intervalUpperLimit << std::endl;
  }

  // if there is only 1 m/s difference between peak wind and upper limit, make them equal
  if (peakWind - intervalUpperLimit == 1)
  {
    intervalUpperLimit++;
    // and increase lower limit if necessary
    if (intervalUpperLimit - intervalLowerLimit > theParameters.theContextualMaxIntervalSize)
      intervalLowerLimit++;

    ss_log << " difference between interval upper limit and peak wind is only 1 m/s -> adapt "
              "interval: "
           << intervalLowerLimit << "..." << intervalUpperLimit << std::endl;
  }

  if (ss_log.str().size() > 1) theParameters.theLog << ss_log.str();
}

Sentence compose_wind_speed_sentence(const wo_story_params& theParameters,
                                     int intervalLowerLimit,
                                     int intervalUpperLimit,
                                     int peakWind,
                                     bool theUseAtItsStrongestPhrase)
{
  Sentence sentence;

  if (peakWind > intervalUpperLimit)
  {
    if (theUseAtItsStrongestPhrase)
    {
      sentence << IntegerRange(
                      intervalLowerLimit, intervalUpperLimit, theParameters.theRangeSeparator)
               << *UnitFactory::create(MetersPerSecond);
      sentence << Delimiter(COMMA_PUNCTUATION_MARK) << KOVIMMILLAAN_PHRASE << peakWind
               << *UnitFactory::create(MetersPerSecond);
    }
    else
    {
      sentence << IntegerRange(peakWind - theParameters.theContextualMaxIntervalSize,
                               peakWind,
                               theParameters.theRangeSeparator)
               << *UnitFactory::create(MetersPerSecond);
    }
  }
  else
  {
    sentence << IntegerRange(
                    intervalLowerLimit, intervalUpperLimit, theParameters.theRangeSeparator)
             << *UnitFactory::create(MetersPerSecond);
  }

  return sentence;
}

void handle_period_end(WeatherPeriod& period)
{
  int periodLength = get_period_length(period);
  int endHour = period.localEndTime().GetHour();

  // short period in the end: forecast can not end to 'illalla'/'aamulla', but
  // 'iltapäivällä'/'aamuyöllä'
  if (periodLength < 2 && (endHour == 18 || endHour == 17 || endHour == 6))
  {
    TextGenPosixTime startTime(period.localEndTime());
    startTime.ChangeByHours(-2);
    period = WeatherPeriod(startTime, period.localEndTime());
  }
}

WindForecast::WindForecast(wo_story_params& parameters) : theParameters(parameters) {}
// report direction if it is same as thePreviousTimePhraseInfo
std::vector<Sentence> WindForecast::reportDirectionChanges(
    std::vector<WeatherPeriod>& theDirectionPeriods,
    WindDirectionInfo& thePreviousWindDirection,
    const TimePhraseInfo& thePreviousTimePhraseInfo,
    bool theTuuliBasicForm) const
{
  std::vector<Sentence> sentences;
  unsigned int lastReportedIndex = UINT_MAX;

  theParameters.theLog << "Find out direction changes that happen nearby "
                       << thePreviousTimePhraseInfo << std::endl;

  for (unsigned int i = 0; i < theDirectionPeriods.size(); i++)
  {
    const WeatherPeriod& period = theDirectionPeriods[i];

    if (get_period_length(period) < 2)
    {
      theParameters.theLog << "Short direction period " << period << " skipped." << std::endl;
      continue;
    }

    Sentence timePhrase;
    TimePhraseInfo timePhraseInfo;
    timePhraseInfo.day_number = thePreviousTimePhraseInfo.day_number;
    timePhraseInfo.part_of_the_day = thePreviousTimePhraseInfo.part_of_the_day;

    timePhrase << getTimePhrase(
        period.localStartTime(), timePhraseInfo, get_period_length(period) > 6);

    if (timePhraseInfo == thePreviousTimePhraseInfo)
    {
      lastReportedIndex = i;
      WindDirectionInfo windDirection = get_wind_direction(theParameters, period);

      if (windDirection.id == thePreviousWindDirection.id)
      {
        theParameters.theLog << "Same direction as previous "
                             << wind_direction_string(windDirection.id) << " -> skipped."
                             << std::endl;
        continue;
      }

      theParameters.theLog << "Reporting direction change at " << period << " -> "
                           << wind_direction_string(windDirection.id) << std::endl;

      negotiateWindDirection(windDirection, thePreviousWindDirection);
      sentences.push_back(windDirectionSentence(windDirection.id, theTuuliBasicForm));

      thePreviousWindDirection = windDirection;
    }
  }

  if (lastReportedIndex != UINT_MAX)
  {
    if (lastReportedIndex == theDirectionPeriods.size() - 1)
      theDirectionPeriods.clear();
    else
      theDirectionPeriods.assign(theDirectionPeriods.begin() + lastReportedIndex + 1,
                                 theDirectionPeriods.end());
  }

  // report only last change
  if (sentences.size() > 0) sentences.erase(sentences.begin(), sentences.end() - 1);

  theParameters.theLog << "Reporting wind direction changes #" << sentences.size() << std::endl;

  return sentences;
}

std::vector<Sentence> WindForecast::reportDirectionChanges(
    const WeatherPeriod& thePeriod,
    std::vector<WeatherPeriod>& theDirectionPeriods,
    WindDirectionInfo& thePreviousWindDirection,
    TimePhraseInfo& thePreviousTimePhraseInfo,
    bool lastPeriod,
    bool voimistuvaa) const
{
  std::vector<Sentence> sentences;

  unsigned int lastReportedIndex = UINT_MAX;
  TextGenPosixTime previousPeriodStartTime;
  TimePhraseInfo timePhraseInfo;
  timePhraseInfo.day_number = thePreviousTimePhraseInfo.day_number;
  timePhraseInfo.part_of_the_day = thePreviousTimePhraseInfo.part_of_the_day;

  theParameters.theLog << "Reporting wind direction changes during period " << thePeriod << ", "
                       << theDirectionPeriods.size() << std::endl;

  for (unsigned int i = 0; i < theDirectionPeriods.size(); i++)
  {
    const WeatherPeriod& period = theDirectionPeriods[i];

    if (get_period_length(period) < 2)
    {
      theParameters.theLog << "Short direction period " << period << " skipped." << std::endl;
      continue;
    }

    if (is_inside(period.localStartTime(), thePeriod))
    {
      lastReportedIndex = i;

      WindDirectionInfo windDirection = get_wind_direction(theParameters, period);

      if (windDirection.id == thePreviousWindDirection.id)
      {
        theParameters.theLog << "Same direction as previous "
                             << wind_direction_string(windDirection.id) << " -> skipped."
                             << std::endl;
        continue;
      }

      Sentence timePhrase;

      Sentence sentence;

      timePhraseInfo.day_number = thePreviousTimePhraseInfo.day_number;
      timePhraseInfo.part_of_the_day = thePreviousTimePhraseInfo.part_of_the_day;

      if (windDirection.id == VAIHTELEVA)
      {
        timePhrase << getTimePhrase(
            period.localStartTime(), timePhraseInfo, get_period_length(period) >= 6);

        sentence << timePhrase << VAIHTELEVA_TUULI_P;
      }
      else
      {
        if (get_period_length(period) >= 6)
          timePhrase << getTimePhrase(
              period.localStartTime(), timePhraseInfo, get_period_length(period) >= 6);
        else
          timePhrase << getTimePhrase(period, timePhraseInfo, get_period_length(period) >= 6);

        negotiateWindDirection(windDirection, thePreviousWindDirection);
        /*
if (thePreviousWindDirection.id == VAIHTELEVA && voimistuvaa)
  sentence << ILTAPAIVALLA_NOPEASTI_HEIKKENEVAA_ETELATUULTA_COMPOSITE_PHRASE << timePhrase
           << EMPTY_STRING << VOIMISTUVAA_WORD << windDirectionSentence(windDirection.id);
else
        */
        sentence << ILTAPAIVALLA_ETELATUULTA_COMPOSITE_PHRASE << timePhrase
                 << windDirectionSentence(windDirection.id);
      }

      theParameters.theLog << "Reporting wind direction change on period " << period << std::endl;

      // if wind direction changes rapidly on the period report only the the latest direction
      if (i > 0 && sentences.size() > 0 &&
          period.localStartTime().DifferenceInHours(previousPeriodStartTime) < 8 &&
          timePhraseInfo == thePreviousTimePhraseInfo)
        sentences[sentences.size() - 1] = sentence;
      else
        sentences.push_back(sentence);

      thePreviousTimePhraseInfo = timePhraseInfo;
      //      WeatherResult windDirectionValue = get_wind_direction_at(theParameters, period);
      thePreviousWindDirection = get_wind_direction(theParameters, period);
      previousPeriodStartTime = period.localStartTime();
    }
  }

  if (lastReportedIndex != UINT_MAX)
  {
    if (lastReportedIndex == theDirectionPeriods.size() - 1)
      theDirectionPeriods.clear();
    else
      theDirectionPeriods.assign(theDirectionPeriods.begin() + lastReportedIndex + 1,
                                 theDirectionPeriods.end());
  }

  // report only last change
  //  if (sentences.size() > 0) sentences.erase(sentences.begin(), sentences.end() - 1);

  theParameters.theLog << " Number of direction changes " << sentences.size() << std::endl;

  return sentences;
}

std::vector<Sentence> WindForecast::constructWindSentence(
    const WindEventPeriodDataItem* windSpeedItem,
    const WindEventPeriodDataItem* nextWindSpeedItem,
    std::vector<WeatherPeriod> windDirectionReportingPeriods,
    WindDirectionInfo& thePreviousWindDirection,
    TimePhraseInfo& thePreviousTimePhraseInfo,
    bool firstSentence) const
{
  std::vector<Sentence> ret;

  const WeatherPeriod& windSpeedEventPeriod = windSpeedItem->thePeriod;

  // we dont report speed and direction changes on weak period
  if (is_weak_period(theParameters, windSpeedEventPeriod))
  {
    if (!firstSentence)
    {
      theParameters.theLog << "Wind is weak on period " << windSpeedEventPeriod
                           << " -> period is not reported" << std::endl;
    }
    else
    {
      theParameters.theLog << "Wind is weak on period " << windSpeedEventPeriod
                           << " -> reporting the whole period at once!" << std::endl;
      Sentence sentence;
      WindDirectionInfo windDirection = get_wind_direction(theParameters, windSpeedEventPeriod);
      negotiateWindDirection(windDirection, thePreviousWindDirection);
      sentence << windDirectionSentence(windDirection.id)
               << windSpeedIntervalSentence(windSpeedItem->thePeriod, USE_AT_ITS_STRONGEST_PHRASE);

      thePreviousWindDirection = windDirection;
      ret.push_back(sentence);
    }
    return ret;
  }

  WindEventId windSpeedEventId = windSpeedItem->theWindEvent;

  bool lastPeriod =
      theParameters.theForecastPeriod.localEndTime() == windSpeedEventPeriod.localEndTime();

  // get direction changes during whole period
  std::vector<WeatherPeriod> windDirectionChangePeriods;
  TextGenPosixTime directionReportingStartTime = windSpeedEventPeriod.localStartTime();
  TextGenPosixTime directionReportingEndTime = windSpeedEventPeriod.localEndTime();
  if (windSpeedEventId == MISSING_WIND_SPEED_EVENT && !lastPeriod &&
      get_period_length(windSpeedEventPeriod) > 2)
  {
    // if MISSING_WIND_SPEED_EVENT and not last period dont report direction change near end
    directionReportingEndTime.ChangeByHours(-2);
  }
  else if (windSpeedEventId != MISSING_WIND_SPEED_EVENT)
  {
    directionReportingStartTime.ChangeByHours(-2);
  }

  BOOST_FOREACH (const WeatherPeriod& period, windDirectionReportingPeriods)
  {
    if (period.localStartTime() >= directionReportingStartTime &&
        period.localStartTime() <= directionReportingEndTime)
      windDirectionChangePeriods.push_back(period);
  }

  switch (windSpeedEventId)
  {
    case MISSING_WIND_SPEED_EVENT:
    {
      // first period must be reported
      if (firstSentence && windDirectionChangePeriods.empty())
        windDirectionChangePeriods.push_back(windSpeedEventPeriod);

      theParameters.theLog << "Processing MISSING_WIND_SPEED_EVENT at " << windSpeedEventPeriod
                           << ". " << windDirectionChangePeriods.size()
                           << " direction changes during the period." << std::endl;

      TimePhraseInfo timePhraseInfo;

      std::vector<DirectionSentenceInfo> directionSentences;
      WindDirectionInfo previousWindDirection = thePreviousWindDirection;

      for (std::vector<WeatherPeriod>::const_iterator it = windDirectionChangePeriods.begin();
           it != windDirectionChangePeriods.end();
           it++)
      {
        DirectionSentenceInfo directionSentenceInfo;
        std::vector<WeatherPeriod>::const_iterator iterNext = it;
        iterNext++;
        bool lastDirectionChangeOnCurrentPeriod = (iterNext == windDirectionChangePeriods.end());

        WeatherPeriod directionChangePeriod = *it;

        if (!firstSentence &&
            windSpeedEventPeriod.localStartTime() == directionChangePeriod.localStartTime() &&
            get_period_length(directionChangePeriod) > 0)
        {
          TextGenPosixTime startTime = directionChangePeriod.localStartTime();
          startTime.ChangeByHours(1);
          directionChangePeriod = WeatherPeriod(startTime, directionChangePeriod.localEndTime());
        }

        if (lastPeriod && lastDirectionChangeOnCurrentPeriod)
          handle_period_end(directionChangePeriod);

        WindDirectionInfo currentWindDirection =
            get_wind_direction(theParameters, directionChangePeriod);

        if (currentWindDirection.id == previousWindDirection.id)
        {
          theParameters.theLog << "Direction '" << wind_direction_string(currentWindDirection.id)
                               << "' at period " << directionChangePeriod
                               << " is the same as previous period -> skipping" << std::endl;
          continue;
        }

        bool shortPeriod = get_period_length(directionChangePeriod) < 2;

        // dont report short period except in the beginning
        if (shortPeriod && (!firstSentence || (firstSentence && !directionSentences.empty())))
        {
          theParameters.theLog << "Short period " << directionChangePeriod << " in the middle ("
                               << wind_direction_string(currentWindDirection.id) << ") -> skipping"
                               << std::endl;
          continue;
        }

        if (firstSentence && directionSentences.empty())
        {
          negotiateWindDirection(currentWindDirection, thePreviousWindDirection);
          directionSentenceInfo.sentence
              << windDirectionSentence(currentWindDirection.id)
              << windSpeedIntervalSentence(windSpeedItem->thePeriod, USE_AT_ITS_STRONGEST_PHRASE);

          previousWindDirection = currentWindDirection;

          theParameters.theLog << "Reporting direction and interval at start: speed period is "
                               << windSpeedItem->thePeriod << ", direction period is "
                               << directionChangePeriod << std::endl;
        }
        else
        {
          bool useAlkaenPhrase = get_period_length(directionChangePeriod) >= 6;

          theParameters.theLog << "Reporting direction change for period " << directionChangePeriod
                               << std::endl;

          if (currentWindDirection.id == VAIHTELEVA)
          {
            bool temporaryChange = get_period_length(directionChangePeriod) <= 5;

            // dont report temporary change if it is not last change
            if (temporaryChange && !lastDirectionChangeOnCurrentPeriod)
            {
              theParameters.theLog << "Temporary VAIHTELEVA period (not last direction change) "
                                   << directionChangePeriod << " -> skipping" << std::endl;
              continue;
            }

            if (temporaryChange && !lastPeriod)
              directionSentenceInfo.sentence
                  << ILTAPAIVALLA_TUULI_MUUTTUU_TILAPAISESTI_VAIHTELEVAKSI_COMPOSITE_PHRASE;
            else
              directionSentenceInfo.sentence
                  << ILTAPAIVALLA_TUULI_MUUTTUU_VAIHTELEVAKSI_COMPOSITE_PHRASE;

            directionSentenceInfo.sentence
                << getTimePhrase(directionChangePeriod.localStartTime(), timePhraseInfo, false);
            theParameters.theLog << "CASE1:  " << as_string(directionSentenceInfo.sentence)
                                 << std::endl;
          }
          else
          {
            // after varying wind use this form
            if (previousWindDirection.id == VAIHTELEVA)
            {
              negotiateWindDirection(currentWindDirection, thePreviousWindDirection);
              directionSentenceInfo.sentence
                  << ILTAPAIVALLA_ETELATUULTA_COMPOSITE_PHRASE
                  << getTimePhrase(directionChangePeriod, timePhraseInfo, useAlkaenPhrase)
                  << windDirectionSentence(currentWindDirection.id);
              theParameters.theLog << "CASE2: " << as_string(directionSentenceInfo.sentence)
                                   << std::endl;
            }
            else
            {
              // dont report directon change for eample pohjoinen -> pohjoisen puoleinen except when
              // it is long term change
              if (almost_same_direction(currentWindDirection.id, previousWindDirection.id) &&
                  !useAlkaenPhrase)
                continue;

              negotiateWindDirection(currentWindDirection, thePreviousWindDirection);
              // to get some variation in phrases
              if (lastDirectionChangeOnCurrentPeriod ||
                  almost_same_direction(currentWindDirection.id, previousWindDirection.id))
                directionSentenceInfo.sentence
                    << ILTAPAIVALLA_ETELATUULTA_COMPOSITE_PHRASE
                    << getTimePhrase(directionChangePeriod, timePhraseInfo, useAlkaenPhrase)
                    << windDirectionSentence(currentWindDirection.id);
              else
                directionSentenceInfo.sentence
                    << ILTAPAIVALLA_TUULI_KAANTYY_ETELAAN_COMPOSITE_PHRASE
                    << getTimePhrase(directionChangePeriod, timePhraseInfo, false)
                    << getWindDirectionTurntoString(currentWindDirection.id);
              theParameters.theLog << "CASE3: " << as_string(directionSentenceInfo.sentence) << ""
                                   << std::endl;
            }
          }
        }

        directionSentenceInfo.startTime = directionChangePeriod.localStartTime();
        directionSentenceInfo.endTime = directionChangePeriod.localEndTime();
        directionSentenceInfo.timePhraseInfo = timePhraseInfo;
        directionSentenceInfo.windDirectionInfo = currentWindDirection;

        // first sentence: two direction changes are reported
        if (firstSentence && directionSentences.size() < 2)
        {
          directionSentences.push_back(directionSentenceInfo);
        }
        else if (directionSentences.back().timePhraseInfo == directionSentenceInfo.timePhraseInfo)
        {
          // if direction changes at the same part of the day, report the direction that lasts
          // longer time
          DirectionSentenceInfo previousDirectionSentenceInfo = directionSentences.back();
          WeatherPeriod previousPeriod(previousDirectionSentenceInfo.startTime,
                                       previousDirectionSentenceInfo.endTime);
          WeatherPeriod currentPeriod(directionSentenceInfo.startTime,
                                      directionSentenceInfo.endTime);
          if (get_period_length(previousPeriod) < get_period_length(currentPeriod))
          {
            directionSentences.back() = directionSentenceInfo;
          }
        }
        else
        {
          directionSentences.push_back(directionSentenceInfo);
        }

        previousWindDirection = currentWindDirection;
      }

      theParameters.theLog << "Final: directions reported at period : " << windSpeedEventPeriod
                           << std::endl;
      BOOST_FOREACH (const DirectionSentenceInfo& item, directionSentences)
      {
        theParameters.theLog << item.startTime << "..." << item.endTime << " -> "
                             << wind_direction_string(item.windDirectionInfo.id) << std::endl;
        ret.push_back(item.sentence);
        thePreviousTimePhraseInfo = item.timePhraseInfo;
      }

      if (directionSentences.size() > 0)
        thePreviousWindDirection = directionSentences.back().windDirectionInfo;

      theParameters.theLog << "Reporting " << ret.size() << " direction changes on period "
                           << windSpeedEventPeriod << std::endl;
    }
    break;
    case TUULI_VOIMISTUU:
    case TUULI_HEIKKENEE:
    {
      bool tuuliVoimistuu = (windSpeedEventId == TUULI_VOIMISTUU);
      bool useAtItsStrongestPhrase = (tuuliVoimistuu ? true : false);

      // get point of times/periods to report wind speed diring sthrenghtening/weakening period
      std::vector<WeatherPeriod> windSpeedReportingPeriods =
          getWindSpeedReportingPeriods(*windSpeedItem, firstSentence);

      if (windSpeedReportingPeriods.size() == 0)
      {
        // we shouldnt newe be here, but just in case
        TextGenPosixTime sTime = windSpeedEventPeriod.localStartTime();
        TextGenPosixTime eTime = windSpeedEventPeriod.localEndTime();
        if (get_period_length(windSpeedEventPeriod) > 3)
        {
          sTime = eTime;
          sTime.ChangeByHours(-3);
        }
        WeatherPeriod p(sTime, eTime);
        windSpeedReportingPeriods.push_back(p);
        theParameters.theLog << "No wind speed reporting periods found -> reporting wind speed at "
                                "the end of the period: "
                             << p << std::endl;
      }
      else if (!tuuliVoimistuu && windSpeedReportingPeriods.size() > 1)
      {
        // if TUULI_HEIKKENEE on last period and it is longer than 3h,
        // report speed only on only three last hours
        WeatherPeriod& lp = windSpeedReportingPeriods[windSpeedReportingPeriods.size() - 1];
        if (get_period_length(lp) > 3)
        {
          // three last hours
          TextGenPosixTime st = lp.localEndTime();
          st.ChangeByHours(-3);
          windSpeedReportingPeriods[windSpeedReportingPeriods.size() - 1] =
              WeatherPeriod(st, lp.localEndTime());
        }
      }

      theParameters.theLog << "Processing " << (tuuliVoimistuu ? "TUULI_VOIMISTUU event at "
                                                               : "TUULI_HEIKKENEE event at ")
                           << windSpeedEventPeriod
                           << (firstSentence ? " - first sentence" : " not first sentence") << " ("
                           << windSpeedReportingPeriods.size() << " reporting times)" << std::endl;

      bool smallChange = false;
      bool gradualChange = false;
      bool fastChange = false;
      std::string changeAttributeStr = EMPTY_STRING;
      if (!getWindSpeedChangeAttribute(
              windSpeedEventPeriod, changeAttributeStr, smallChange, gradualChange, fastChange))
        return ret;

      // if tuuli heikkenee dont use vähän attribute
      if (!tuuliVoimistuu && changeAttributeStr == VAHAN_WORD) changeAttributeStr = EMPTY_STRING;

      Sentence sentence;

      // iterate reporting points
      for (unsigned int i = 0; i < windSpeedReportingPeriods.size(); i++)
      {
        WeatherPeriod period = windSpeedReportingPeriods[i];
        WindDirectionInfo windDirection;

        theParameters.theLog << "Reporting "
                             << (tuuliVoimistuu ? "strenghtening wind at " : "weakening wind at ")
                             << period << std::endl;
        bool nextWindSpeedEventMissing =
            (nextWindSpeedItem && nextWindSpeedItem->theWindEvent == MISSING_WIND_SPEED_EVENT);

        // if no wind speed change is happening on next period report wind speed for that period
        // here too
        if (i == windSpeedReportingPeriods.size() - 1 && nextWindSpeedEventMissing)
        {
          TextGenPosixTime directionPeriodEndTime = nextWindSpeedItem->thePeriod.localEndTime();
          // direction period starts two hours before speed period
          directionPeriodEndTime.ChangeByHours(-2);

          BOOST_FOREACH (const WeatherPeriod& reportPeriod, windDirectionReportingPeriods)
            if (reportPeriod.localStartTime() >= period.localEndTime() &&
                reportPeriod.localStartTime() < directionPeriodEndTime)
            {
              windDirectionChangePeriods.push_back(reportPeriod);
            }

          period =
              WeatherPeriod(period.localStartTime(), nextWindSpeedItem->thePeriod.localEndTime());

          theParameters.theLog << "Reporting also next missing period " << period << std::endl;
        }

        Sentence speedIntervalSentence(windSpeedIntervalSentence(period, useAtItsStrongestPhrase));

        bool startOfTheStory = (firstSentence && i == 0);

        TextGenPosixTime prePeriodStart = windSpeedEventPeriod.localStartTime();
        TextGenPosixTime prePeriodEnd = period.localStartTime();
        // in the beginning there is no previous period
        if (startOfTheStory)
        {
          prePeriodStart.ChangeByHours(-1);
          prePeriodEnd.ChangeByHours(-1);
        }
        else if (prePeriodStart < prePeriodEnd)
        {
          prePeriodStart.ChangeByHours(1);
        }

        WeatherPeriod prePeriod(prePeriodStart, prePeriodEnd);

        Sentence timePhrase;
        TimePhraseInfo timePhraseInfo;
        timePhraseInfo.day_number = thePreviousTimePhraseInfo.day_number;
        timePhraseInfo.part_of_the_day = thePreviousTimePhraseInfo.part_of_the_day;

        WeatherPeriod speedChangePeriod = period;
        bool addInterval = true;

        // first wind speed reporting point
        if (i == 0)
        {
          if (firstSentence)
            timePhrase << EMPTY_STRING;
          else
          {
            speedChangePeriod =
                WeatherPeriod(windSpeedItem->thePeriod.localStartTime(), period.localEndTime());

            // if speed change period length > 6, the change is reported to start
            // when half of the required wind speed has happened
            WeatherPeriod reportedSpeedChangePeriod(windSpeedEventPeriod);

            if (period.localStartTime().DifferenceInHours(speedChangePeriod.localStartTime()) > 6)
            {
              // get point of time when speed has changed 50 % of the configured threshold
              TextGenPosixTime sTime = get_wind_change_point(
                  theParameters,
                  WeatherPeriod(speedChangePeriod.localStartTime(), period.localStartTime()),
                  50.0);
              reportedSpeedChangePeriod = WeatherPeriod(sTime, windSpeedEventPeriod.localEndTime());
              // change attributes may have changed because period has changed
              getWindSpeedChangeAttribute(reportedSpeedChangePeriod,
                                          changeAttributeStr,
                                          smallChange,
                                          gradualChange,
                                          fastChange);

              // if tuuli heikkenee dont use vähän attribute
              if (!tuuliVoimistuu && changeAttributeStr == VAHAN_WORD)
                changeAttributeStr = EMPTY_STRING;

              // report here if wind direction has changed
              TextGenPosixTime directionChangeStartTime(speedChangePeriod.localStartTime());

              // report direction changes if they dont happen on the same part of the day as
              // reported speed change
              if (sTime.DifferenceInHours(directionChangeStartTime) > 2 &&
                  get_part_of_the_day_id_wind(sTime) !=
                      get_part_of_the_day_id_wind(directionChangeStartTime))
              {
                sTime.ChangeByHours(-2);
                // start two hours before start
                if (!firstSentence) directionChangeStartTime.ChangeByHours(-2);
                WeatherPeriod prePeriod(directionChangeStartTime, sTime);
                std::vector<Sentence> directionChangeSentences =
                    reportDirectionChanges(prePeriod,
                                           windDirectionChangePeriods,
                                           thePreviousWindDirection,
                                           timePhraseInfo,
                                           lastPeriod,
                                           tuuliVoimistuu);

                if (directionChangeSentences.size() > 0)
                {
                  thePreviousTimePhraseInfo = timePhraseInfo;
                  theParameters.theLog
                      << "Direction changes (during speed changes) before interval: " << std::endl;
                }

                Sentence directionChangeSentence;
                BOOST_FOREACH (const Sentence& s, directionChangeSentences)
                {
                  if (!directionChangeSentence.empty())
                    directionChangeSentence << Delimiter(COMMA_PUNCTUATION_MARK);
                  directionChangeSentence << s;
                  theParameters.theLog << as_string(s) << std::endl;
                }
                if (!directionChangeSentence.empty()) ret.push_back(directionChangeSentence);
              }
            }

            timePhrase << getTimePhrase(reportedSpeedChangePeriod,
                                        timePhraseInfo,
                                        get_period_length(windSpeedEventPeriod) >= 6);
          }

          bool tuuliBasicForm = false;
          if (changeAttributeStr == EMPTY_STRING && thePreviousWindDirection.id == VAIHTELEVA)
          {
            if (tuuliVoimistuu)
              sentence << ILTAPAIVALLA_ETELATUULI_ALKAA_VOIMISTUA_NOPEASTI_COMPOSITE_PHRASE;
            else
              sentence << ILTAPAIVALLA_ETELATUULI_ALKAA_HEIKETA_NOPEASTI_COMPOSITE_PHRASE;
            sentence << timePhrase;
            thePreviousTimePhraseInfo = timePhraseInfo;
            tuuliBasicForm = true;
          }
          else
          {
            if (windSpeedItem->theSuccessiveEventFlag)
              sentence << ILTAPAIVALLA_EDELLEEN_HEIKKENEVAA_POHJOISTUULTA_COMPOSITE_PHRASE
                       << timePhrase << (tuuliVoimistuu ? VOIMISTUVAA_WORD : HEIKKENEVAA_WORD);
            else
              sentence << ILTAPAIVALLA_NOPEASTI_HEIKKENEVAA_ETELATUULTA_COMPOSITE_PHRASE
                       << timePhrase << changeAttributeStr
                       << (tuuliVoimistuu ? VOIMISTUVAA_WORD : HEIKKENEVAA_WORD);

            thePreviousTimePhraseInfo = timePhraseInfo;
          }

          // if first sentence in the forecast
          if (firstSentence)
          {
            // handle just first period
            WeatherPeriod& directionPeriod = windDirectionChangePeriods[0];
            windDirection = get_wind_direction(theParameters, directionPeriod);

            Sentence firstSentence;

            negotiateWindDirection(windDirection, thePreviousWindDirection);
            if (windDirection.id == VAIHTELEVA)
            {
              sentence.clear();

              // varying wind can not weaken or strenghten
              sentence << ILTAPAIVALLA_ETELATUULTA_COMPOSITE_PHRASE << timePhrase
                       << windDirectionSentence(windDirection.id);
            }
            else
            {
              sentence << windDirectionSentence(windDirection.id);
            }
            thePreviousWindDirection = windDirection;
            windDirectionChangePeriods.assign(windDirectionChangePeriods.begin() + 1,
                                              windDirectionChangePeriods.end());
          }
          else
          {
            part_of_the_day_id speedDayPartId =
                get_part_of_the_day_id_wind(speedChangePeriod.localStartTime());

            // handle special cases
            bool windDirectionAdded = false;
            // if sentence starts 'aamuyolla'/'aamuyosta alkaen' (and the actual period
            // starts
            // couple of hours before), then direction change can not happen at 'keskiyo'
            if (windDirectionChangePeriods.size() > 0)
            {
              const WeatherPeriod& directionPeriod = windDirectionChangePeriods[0];
              if (get_period_length(directionPeriod) > 2 &&
                  timePhraseInfo.part_of_the_day == AAMUYO &&
                  (speedDayPartId == ILTAYO || speedDayPartId == KESKIYO))
              {
                part_of_the_day_id directionDayPart =
                    get_part_of_the_day_id_wind(directionPeriod.localStartTime());

                if (directionDayPart == KESKIYO)
                {
                  windDirection = get_wind_direction(theParameters, directionPeriod);
                  if (windDirection.id == VAIHTELEVA && !tuuliVoimistuu)
                  {
                    sentence.clear();
                    sentence << ILTAPAIVALLA_ETELATUULTA_COMPOSITE_PHRASE << timePhrase;
                    addInterval = wind_speed_differ_enough(theParameters, speedChangePeriod);
                  }
                  negotiateWindDirection(windDirection, thePreviousWindDirection);
                  sentence << windDirectionSentence(windDirection.id, tuuliBasicForm);

                  thePreviousWindDirection = windDirection;
                  windDirectionChangePeriods.assign(windDirectionChangePeriods.begin() + 1,
                                                    windDirectionChangePeriods.end());
                  windDirectionAdded = true;
                }
              }
            }  // if(windDirectionChangePeriods.size() > 0)
            if (!windDirectionAdded && !sentence.empty())
            {
              // report direction change if it happens on the same period
              std::vector<Sentence> directionChangeSentences =
                  reportDirectionChanges(windDirectionChangePeriods,
                                         thePreviousWindDirection,
                                         thePreviousTimePhraseInfo,
                                         tuuliBasicForm);

              if (directionChangeSentences.size() > 0)
              {
                thePreviousTimePhraseInfo = timePhraseInfo;
                theParameters.theLog << "Direction changes #1 (during speed change): " << std::endl;
              }

              BOOST_FOREACH (const Sentence& s, directionChangeSentences)
              {
                sentence << s;
                theParameters.theLog << as_string(s) << std::endl;
              }
              if (directionChangeSentences.size() == 0)
              {
                sentence << (tuuliBasicForm ? TUULI_WORD : TUULTA_WORD);
              }
            }
          }
        }  // if (i == 0) // first reporting point

        if (startOfTheStory)
        {
          if (windSpeedReportingPeriods.size() > 1 && windDirection.id != VAIHTELEVA)
            sentence << Delimiter(COMMA_PUNCTUATION_MARK) << ALUKSI_WORD;
          if (!tuuliVoimistuu)
          {
            speedIntervalSentence.clear();
            // take lower limit from start and upper limit from whole period, since
            // highest speed is not necessarily in the beginning
            speedIntervalSentence << windSpeedIntervalSentence(
                period, windSpeedEventPeriod, DONT_USE_AT_ITS_STRONGEST_PHRASE);
          }
          theParameters.theLog << "Start of the story: " << period << std::endl;
        }
        else
        {
          theParameters.theLog << "Direction changes #2: " << prePeriod << std::endl;

          // report here if wind direction has changed
          std::vector<Sentence> directionChangeSentences =
              reportDirectionChanges(prePeriod,
                                     windDirectionChangePeriods,
                                     thePreviousWindDirection,
                                     timePhraseInfo,
                                     lastPeriod,
                                     tuuliVoimistuu);

          if (directionChangeSentences.size() > 0)
          {
            thePreviousTimePhraseInfo = timePhraseInfo;
            theParameters.theLog << "Direction changes2 (during speed changes) before interval: "
                                 << std::endl;
          }

          BOOST_FOREACH (const Sentence& s, directionChangeSentences)
          {
            if (!sentence.empty()) sentence << Delimiter(COMMA_PUNCTUATION_MARK);
            sentence << s;
            theParameters.theLog << as_string(s) << std::endl;
          }

          // if the previous change was to varying wind, just report
          // interval till next change
          if (directionChangeSentences.size() > 0 && thePreviousWindDirection.id == VAIHTELEVA)
          {
            speedIntervalSentence.clear();
            WeatherPeriod intervalPeriod(thePreviousTimePhraseInfo.starttime,
                                         period.localEndTime());
            speedIntervalSentence << windSpeedIntervalSentence(intervalPeriod,
                                                               useAtItsStrongestPhrase);

            theParameters.theLog << "Because wind has changed to varying, report just interval at "
                                 << intervalPeriod << " -> " << as_string(speedIntervalSentence)
                                 << std::endl;
            sentence << speedIntervalSentence;
            continue;
          }
        }
        if (!startOfTheStory)
        {
          timePhrase.clear();
          theParameters.theLog << "Not start of the story: " << period << std::endl;
          //

          if ((period.localStartTime().GetHour() == period.localEndTime().GetHour() &&
               period.localStartTime().GetHour() ==
                   theParameters.theForecastPeriod.localEndTime().GetHour()))
          {
            TextGenPosixTime t = period.localStartTime();
            t.ChangeByHours(-1);
            WeatherPeriod p(t, t);
            timePhrase << getTimePhrase(p, timePhraseInfo, get_period_length(period) >= 6);
          }
          else
            timePhrase << getTimePhrase(period, timePhraseInfo, get_period_length(period) >= 6);
        }

        if (addInterval)
        {
          theParameters.theLog << "Add interval: " << period << std::endl;
          if (!startOfTheStory)
          {
            theParameters.theLog << "Not start of the story 2: " << thePreviousTimePhraseInfo
                                 << " vs. " << timePhraseInfo << std::endl;
            if (thePreviousTimePhraseInfo != timePhraseInfo)
            {
              theParameters.theLog << "thePreviousTimePhraseInfo != timePhraseInfo: " << std::endl;
              bool directionChangeHappened = false;
              part_of_the_day_id id_period = get_part_of_the_day_id_narrow(period, true);
              theParameters.theLog << "period: " << period << " -> "
                                   << part_of_the_day_string(id_period) << std::endl;

              // if direction changes same time as interval, report it here
              theParameters.theLog << "Direction changes #3: " << period << std::endl;

              TextGenPosixTime periodEndTime = period.localStartTime();
              while (get_part_of_the_day_id_narrow(
                         WeatherPeriod(period.localStartTime(), periodEndTime), true) == id_period)
                periodEndTime.ChangeByHours(1);

              if (periodEndTime.GetHour() != period.localStartTime().GetHour())
                periodEndTime.ChangeByHours(-1);

              id_period = get_part_of_the_day_id_narrow(
                  WeatherPeriod(period.localStartTime(), periodEndTime), true);

              int startHour = -1;
              int endHour = -1;
              get_part_of_the_day(id_period, startHour, endHour);

              // increase end time as long as end hour is reached: direction can change on same
              // period
              while (periodEndTime.GetHour() != endHour)
                periodEndTime.ChangeByHours(1);

              theParameters.theLog << "Actual direction change period: " << period.localStartTime()
                                   << "..." << periodEndTime << " -> "
                                   << part_of_the_day_string(id_period) << std::endl;

              theParameters.theLog
                  << "thePreviousTimePhraseInfo.part_of_the_day: "
                  << part_of_the_day_string(thePreviousTimePhraseInfo.part_of_the_day) << std::endl;

              // if wind direction changes in the same part of the day as
              // reported wind speed change, report it at the same time
              TextGenPosixTime periodStartTime = period.localStartTime();
              periodStartTime.ChangeByHours(-1);
              WeatherPeriod windDirectionChangePeriod(periodStartTime, periodEndTime);
              TimePhraseInfo tpi = thePreviousTimePhraseInfo;
              std::vector<Sentence> directionChangeSentences =
                  reportDirectionChanges(windDirectionChangePeriod,
                                         windDirectionChangePeriods,
                                         thePreviousWindDirection,
                                         tpi,
                                         lastPeriod,
                                         tuuliVoimistuu);

              if (directionChangeSentences.size() > 0)
              {
                thePreviousTimePhraseInfo = tpi;
                theParameters.theLog << "Direction changes (during speed "
                                        "changes) same time as interval: "
                                     << std::endl;
              }

              BOOST_FOREACH (const Sentence& s, directionChangeSentences)
              {
                if (!sentence.empty()) sentence << Delimiter(COMMA_PUNCTUATION_MARK);
                sentence << s;
                theParameters.theLog << as_string(s) << std::endl;
              }
              directionChangeHappened = (directionChangeSentences.size() > 0);

              // if there was no direction change at the same time report time here
              if (!directionChangeHappened && timePhraseInfo != thePreviousTimePhraseInfo)
              {
                sentence << Delimiter(COMMA_PUNCTUATION_MARK) << timePhrase;
                thePreviousTimePhraseInfo = timePhraseInfo;
              }
            }
          }

          sentence << speedIntervalSentence;

          if (i == 0 && firstSentence && windDirection.id == VAIHTELEVA)
          {
            ret.push_back(sentence);
            sentence.clear();
          }
        }
        timePhrase.clear();
      }  // reporting periods

      if (!sentence.empty())
      {
        theParameters.theLog << "Wind speed change sentence: " << std::endl
                             << as_string(sentence) << std::endl;
        ret.push_back(sentence);
      }
      // in the end report rest of wind direction changes during this wind speed change period
      BOOST_FOREACH (const WeatherPeriod& period, windDirectionChangePeriods)
      {
        if (get_period_length(period) <= 2 ||
            windSpeedEventPeriod.localEndTime() == period.localStartTime())
          continue;

        Sentence timePhrase;
        TimePhraseInfo timePhraseInfo;
        timePhraseInfo.day_number = thePreviousTimePhraseInfo.day_number;
        timePhraseInfo.part_of_the_day = thePreviousTimePhraseInfo.part_of_the_day;

        timePhrase << getTimePhrase(period, timePhraseInfo, get_period_length(period) >= 6);

        WindDirectionInfo windDirection = get_wind_direction(theParameters, period);

        if (windDirection.id != thePreviousWindDirection.id)
        {
          Sentence sentence;

          negotiateWindDirection(windDirection, thePreviousWindDirection);
          /*
          if(windDirection.id == thePreviousWindDirection.id ||
                 )
                continue;
          */
          sentence << timePhrase << windDirectionSentence(windDirection.id);

          theParameters.theLog
              << "Wind direction change reported in the end of wind speed change period "
              << windSpeedEventPeriod << ", " << period << ": " << std::endl
              << as_string(sentence) << std::endl;

          ret.push_back(sentence);

          thePreviousTimePhraseInfo = timePhraseInfo;
          thePreviousWindDirection = windDirection;
        }
      }
    }
    break;
    default:
      break;
  }

  return ret;
}

Paragraph WindForecast::getWindStory(const WeatherPeriod& thePeriod) const
{
  Paragraph paragraph;
  std::vector<Sentence> sentences;

  WindDirectionInfo previouslyReportedWindDirection;

  std::vector<WeatherPeriod> windDirectionPeriods =
      get_wind_direction_periods(theParameters, thePeriod);

  theParameters.theLog << "*** WIND DIRECTION REPORTING PERIODS ***" << std::endl;

  BOOST_FOREACH (const WeatherPeriod& period, windDirectionPeriods)
    theParameters.theLog << period << " - "
                         << wind_direction_string(get_wind_direction(theParameters, period).id)
                         << std::endl;

  theParameters.theLog << "*** WIND SPEED REPORTING PERIODS ***" << std::endl;

  BOOST_FOREACH (const WindEventPeriodDataItem* item, theParameters.theWindSpeedEventPeriodVector)
    theParameters.theLog << item->thePeriod << " - " << get_wind_event_string(item->theWindEvent)
                         << std::endl;

  TimePhraseInfo previouslyReportedTimePhrase;
  WindDirectionInfo previousWindDirection;

  // iterate wind speed events and report speed and direction changes
  for (unsigned int i = 0; i < theParameters.theWindSpeedEventPeriodVector.size(); i++)
  {
    bool firstPeriod(i == 0);
    const WindEventPeriodDataItem* windSpeedEventPeriodDataItem =
        theParameters.theWindSpeedEventPeriodVector[i];

    const WindEventPeriodDataItem* nextWindSpeedEventPeriodDataItem =
        (i < theParameters.theWindSpeedEventPeriodVector.size() - 1
             ? theParameters.theWindSpeedEventPeriodVector[i + 1]
             : 0);

    // dont report 1-hour long period in the end
    if (i == theParameters.theWindSpeedEventPeriodVector.size() - 1 &&
        get_period_length(windSpeedEventPeriodDataItem->thePeriod) == 1)
      continue;

    std::vector<Sentence> retval = constructWindSentence(windSpeedEventPeriodDataItem,
                                                         nextWindSpeedEventPeriodDataItem,
                                                         windDirectionPeriods,
                                                         previouslyReportedWindDirection,
                                                         previouslyReportedTimePhrase,
                                                         firstPeriod);
    //    previouslyReportedTimePhrase.part_of_the_day = MISSING_PART_OF_THE_DAY_ID;

    sentences.insert(sentences.end(), retval.begin(), retval.end());

    // missing period reported already
    if (nextWindSpeedEventPeriodDataItem &&
        nextWindSpeedEventPeriodDataItem->theWindEvent == MISSING_WIND_SPEED_EVENT)
      i++;

    WeatherPeriod previousDirectionPeriod(windSpeedEventPeriodDataItem->thePeriod.localEndTime(),
                                          windSpeedEventPeriodDataItem->thePeriod.localEndTime());
    previousWindDirection = get_wind_direction(theParameters, previousDirectionPeriod);
  }

  BOOST_FOREACH (const Sentence& s, sentences)
    paragraph << s;

  return paragraph;
}

Sentence WindForecast::getTimePhrase(const TextGenPosixTime& theTime,
                                     TimePhraseInfo& timePhraseInfo,
                                     bool useAlkaenPhrase) const
{
  return getTimePhrase(WeatherPeriod(theTime, theTime), timePhraseInfo, useAlkaenPhrase);
}

Sentence WindForecast::getTimePhrase(const WeatherPeriod& thePeriod,
                                     TimePhraseInfo& timePhraseInfo,
                                     bool useAlkaenPhrase) const
{
  Sentence sentence;
  Sentence timePhrase;
  bool specifyDay = false;
  WeatherPeriod actualPeriod = thePeriod;
  int forecastPeriodLength = get_period_length(theParameters.theForecastPeriod);
  part_of_the_day_id previousPartOfTheDay = timePhraseInfo.part_of_the_day;

  if (get_period_length(thePeriod) == 6 && thePeriod.localStartTime().GetHour() == 12 &&
      thePeriod.localEndTime().GetHour() == 18)
    useAlkaenPhrase = false;

  if (get_period_length(thePeriod) == 0)
  {
    timePhraseInfo.part_of_the_day = get_part_of_the_day_id_wind(thePeriod.localStartTime());
    specifyDay = ((forecastPeriodLength > 6 &&
                   theParameters.theForecastPeriod.localStartTime().GetWeekday() !=
                       actualPeriod.localStartTime().GetWeekday()) &&
                  ((timePhraseInfo.day_number != actualPeriod.localStartTime().GetWeekday())));

    std::string plainTimePhrase =
        get_time_phrase_from_id(timePhraseInfo.part_of_the_day, theParameters.theVar, false);
    timePhrase << parse_time_phrase(
        thePeriod.localStartTime().GetWeekday(), specifyDay, plainTimePhrase);

    theParameters.theAlkaenPhraseUsed = false;
  }
  else
  {
    std::string dayPhasePhrase;
    theParameters.theAlkaenPhraseUsed = (get_period_length(thePeriod) >= 6 && useAlkaenPhrase &&
                                         !fit_into_narrow_day_part(thePeriod));

    // try to prevent tautology, like "iltapäivällä"... "iltapäivästä alkaen"
    if (timePhraseInfo.part_of_the_day == get_most_relevant_part_of_the_day_id_narrow(
                                              thePeriod, theParameters.theAlkaenPhraseUsed) &&
        timePhraseInfo.day_number != MISSING_PART_OF_THE_DAY_ID && get_period_length(thePeriod) > 4)
    {
      TextGenPosixTime startTime(thePeriod.localStartTime());
      startTime.ChangeByHours(2);
      WeatherPeriod shortenedPeriod(startTime, thePeriod.localEndTime());
      theParameters.theAlkaenPhraseUsed =
          (get_period_length(shortenedPeriod) >= 6 && useAlkaenPhrase &&
           !fit_into_narrow_day_part(shortenedPeriod));

      actualPeriod = shortenedPeriod;
    }

    specifyDay = ((forecastPeriodLength > 6 &&
                   theParameters.theForecastPeriod.localStartTime().GetWeekday() !=
                       actualPeriod.localStartTime().GetWeekday()) &&
                  ((timePhraseInfo.day_number != actualPeriod.localStartTime().GetWeekday())));

    timePhrase << get_time_phrase_large(actualPeriod,
                                        specifyDay,
                                        theParameters.theVar,
                                        dayPhasePhrase,
                                        useAlkaenPhrase,
                                        timePhraseInfo.part_of_the_day);
  }

  if (timePhraseInfo.part_of_the_day == ILTA &&
      get_part_of_the_day_id_wind(theParameters.theForecastPeriod.localEndTime()) == ILTA &&
      abs(actualPeriod.localStartTime().DifferenceInHours(
          theParameters.theForecastPeriod.localEndTime())) < 6)
  {
    // if the whole forecast ends in the evening, the last phrase can not be 'illalla', but it must
    // be 'iltapaivalla'
    timePhraseInfo.part_of_the_day = ILTAPAIVA;
    timePhrase.clear();
    timePhrase << get_time_phrase_from_id(
        timePhraseInfo.part_of_the_day, theParameters.theVar, useAlkaenPhrase);
  }
  else if (timePhraseInfo.part_of_the_day == AAMU &&
           get_part_of_the_day_id_wind(theParameters.theForecastPeriod.localEndTime()) == AAMU &&
           abs(actualPeriod.localStartTime().DifferenceInHours(
               theParameters.theForecastPeriod.localEndTime())) < 6)
  {
    // if the whole forecast ends in the morning, the last phrase can not be 'aamulla', but it must
    // be 'aamuyolla'
    timePhraseInfo.part_of_the_day = AAMUYO;
    timePhrase.clear();
    timePhrase << get_time_phrase_from_id(
        timePhraseInfo.part_of_the_day, theParameters.theVar, useAlkaenPhrase);
  }

  timePhraseInfo.starttime = actualPeriod.localStartTime();
  timePhraseInfo.endtime = actualPeriod.localEndTime();
  timePhraseInfo.day_number = actualPeriod.localStartTime().GetWeekday();

  std::string tps = as_string(timePhrase);

  boost::algorithm::to_lower(tps);
  // remove dot and line feed
  tps.erase(tps.end() - 2, tps.end());

  if (tps.find("aamuyo") != std::string::npos && timePhraseInfo.part_of_the_day != AAMUYO)
  {
    if (previousPartOfTheDay == ILTAYO)
    {
      short lastDayNumberOfPeriod = thePeriod.localStartTime().GetWeekday();

      short newDayNumber = (timePhraseInfo.day_number == 7 ? 1 : timePhraseInfo.day_number + 1);

      if (newDayNumber <= lastDayNumberOfPeriod &&
          !(lastDayNumberOfPeriod == 7 && newDayNumber == 1))
        timePhraseInfo.day_number = newDayNumber;
    }

    if (!isdigit(tps[0]) && specifyDay)
      tps.insert(0, (Fmi::to_string(timePhraseInfo.day_number) + "-"));
    timePhraseInfo.part_of_the_day = AAMUYO;
  }
  if (tps.find("aamuyolla ja aamulla") != std::string::npos)
  {
    tps.replace(tps.find("aamuyolla ja aamulla"), 20, "aamuyolla");
    timePhraseInfo.part_of_the_day = AAMUYO;
  }

  if (tps.find("keskiyo") != std::string::npos)
  {
    // replace keskiyo with aamuyo
    tps.replace(tps.find("keskiyo"), 7, "aamuyo");
    if (forecastPeriodLength > 6 && !isdigit(tps[0]))
    {
      timePhraseInfo.day_number = thePeriod.localEndTime().GetWeekday();
      tps.insert(0, (Fmi::to_string(timePhraseInfo.day_number) + "-"));
    }
    timePhraseInfo.part_of_the_day = AAMUYO;
  }

  sentence << tps;

  /*
  std::cout << "timePhrase: " << thePeriod << " -> " << actualPeriod << " -> " << tps << " --> "
            << timePhraseInfo << std::endl;
   */
  return sentence;
}

vector<WeatherPeriod> WindForecast::getWindSpeedReportingPeriods(
    const WindEventPeriodDataItem& eventPeriodDataItem, bool firstSentenceInTheStory) const
{
  vector<WeatherPeriod> resultVector;

  WeatherPeriod speedEventPeriod(eventPeriodDataItem.theWindSpeedChangePeriod);

  theParameters.theLog << "Find out wind speed reporting points/period at period "
                       << speedEventPeriod << std::endl;

  // find the point of time when max wind has changed theParameters.theWindSpeedThreshold from
  // the previous, take the period from there to the end of the speedEventPeriod
  unsigned int begIndex = 0;
  unsigned int endIndex = 0;

  do
  {
    const WindDataItemUnit& windDataItem =
        (*theParameters.theWindDataVector[begIndex])(theParameters.theArea.type());
    if (is_inside(windDataItem.thePeriod.localStartTime(), speedEventPeriod) ||
        begIndex == theParameters.theWindDataVector.size() - 1)
      break;
    begIndex++;
  } while (true);

  if (begIndex < theParameters.theWindDataVector.size())
  {
    vector<unsigned int> reportingIndexes;
    float previousTopWind(kFloatMissing);
    TextGenPosixTime previousTime;
    for (unsigned int i = begIndex; i < theParameters.theWindDataVector.size(); i++)
    {
      const WindDataItemUnit& windDataItem =
          (*theParameters.theWindDataVector[i])(theParameters.theArea.type());

      // when outside period break the loop
      if (!is_inside(windDataItem.thePeriod.localStartTime(), speedEventPeriod))
      {
        // we must have at least one reporting point
        if ((firstSentenceInTheStory && reportingIndexes.size() == 1) ||
            reportingIndexes.size() == 0)
        {
          const WindDataItemUnit& previousWindDataItem =
              (*theParameters.theWindDataVector[i - 1])(theParameters.theArea.type());
          reportingIndexes.push_back(i - 1);
          previousTopWind = previousWindDataItem.theEqualizedTopWind.value();
          previousTime = previousWindDataItem.thePeriod.localStartTime();
          theParameters.theLog << "Reporting wind speed (case C) at "
                               << previousWindDataItem.thePeriod << std::endl;
        }
        break;
      }

      // windDataItem.theEqualizedTopWind.value() > WEAK_WIND_SPEED_UPPER_LIMIT &&
      // reportingIndexes.size() == 0
      if (i == begIndex)
      {
        // speed at the beginning of the period is reported
        if (firstSentenceInTheStory)
        {
          reportingIndexes.push_back(i);
          theParameters.theLog << "Reporting wind speed (case A) at " << windDataItem.thePeriod
                               << std::endl;
        }
        previousTopWind = windDataItem.theEqualizedTopWind.value();
        previousTime = windDataItem.thePeriod.localStartTime();
      }
      else
      {
        // add reporting point when speed has changed enough
        if (windDataItem.theEqualizedTopWind.value() > WEAK_WIND_SPEED_UPPER_LIMIT ||
            previousTopWind > WEAK_WIND_SPEED_UPPER_LIMIT)
        {
          WeatherPeriod p(previousTime, windDataItem.thePeriod.localStartTime());
          if (wind_speed_differ_enough(theParameters, p))
          {
            reportingIndexes.push_back(i);
            previousTime = windDataItem.thePeriod.localStartTime();
            theParameters.theLog << "Reporting wind speed (case B) at " << windDataItem.thePeriod
                                 << std::endl;
          }
        }
      }
      endIndex = i;
    }  //

    // last
    const WindDataItemUnit& windDataItemLast =
        (*theParameters.theWindDataVector[endIndex])(theParameters.theArea.type());

    for (unsigned int i = 0; i < reportingIndexes.size(); i++)
    {
      unsigned int index = reportingIndexes[i];

      const WindDataItemUnit& windDataItem =
          (*theParameters.theWindDataVector[index])(theParameters.theArea.type());

      if (reportingIndexes.size() == 1)
      {
        // report wind speed at whole remaining increasing/decreasing period,
        // because there is only one reporting point
        WeatherPeriod remainingPeriod(windDataItem.thePeriod.localStartTime(),
                                      windDataItemLast.thePeriod.localEndTime());
        resultVector.push_back(remainingPeriod);
        theParameters.theLog << "Only one reporting point, extend the period to the end "
                             << remainingPeriod << std::endl;
      }
      else if (i < reportingIndexes.size() - 1)
      {
        // report wind speed at certain point of time during increasing/decreasing period
        resultVector.push_back(windDataItem.thePeriod);
        theParameters.theLog << "Report speed at certain point of time " << windDataItem.thePeriod
                             << std::endl;
      }
      else
      {
        // handle last reporting period when there are many reporting points:
        // last reporting point is at the end of the increasing/decreasing period
        // except when period ends in the evening or in the morning, in that case
        // last reporting period starts three hours before increasing/decreasing period ends
        // so that last time phrase in e.g 'iltaan asti'-forecast is 'iltapäivällä'
        TextGenPosixTime startTime(windDataItemLast.thePeriod.localStartTime());
        TextGenPosixTime endTime(windDataItemLast.thePeriod.localEndTime());
        if (endTime.GetHour() == 18 || endTime.GetHour() == 17 || endTime.GetHour() == 6)
        {
          startTime.ChangeByHours(-3);
          if (startTime < eventPeriodDataItem.thePeriod.localStartTime())
            startTime = eventPeriodDataItem.thePeriod.localStartTime();
        }
        WeatherPeriod lastReportingPeriod(startTime, endTime);

        resultVector.push_back(lastReportingPeriod);

        theParameters.theLog << "Last reporting period, special handling " << lastReportingPeriod
                             << std::endl;
      }
    }
  }

  // check that there is no more than three reporting periods:
  // e.g. aluksi 5-7 m/s, iltapäivällä 9-13 m/s, illasta alkaen 11-15 m/s, ylimmillään 19 m/s
  if (resultVector.size() > 3)
  {
    TextGenPosixTime lastPeriodStartTime = resultVector[2].localStartTime();
    TextGenPosixTime lastPeriodEndTime = resultVector.back().localEndTime();
    resultVector[2] = WeatherPeriod(lastPeriodStartTime, lastPeriodEndTime);

    while (resultVector.size() > 3)
      resultVector.pop_back();
  }

  vector<WeatherPeriod> retVector;

  // if there are wind speed changes on the same part of the day report only last one
  for (unsigned int i = 0; i < resultVector.size(); i++)
  {
    if (retVector.size() < (firstSentenceInTheStory ? 2 : 1))
    {
      retVector.push_back(resultVector[i]);
      continue;
    }

    const WeatherPeriod period1 = retVector.back();
    const WeatherPeriod period2 = resultVector[i];

    part_of_the_day_id id1 = get_part_of_the_day_id_wind(period1.localStartTime());
    part_of_the_day_id id2 = get_part_of_the_day_id_wind(period2.localStartTime());

    /*
std::cout << "period1,period2,is1,id2: " << period1 << ", " << period2 << ", "
          << part_of_the_day_string(id1) << ", " << part_of_the_day_string(id2) << std::endl;
    */

    // if part of the day is the same or difference between reporting points is less than three
    // hours, report only te latter point
    if ((id1 == id2 &&
         abs(period1.localStartTime().DifferenceInHours(period2.localStartTime())) < 8) ||
        abs(period1.localStartTime().DifferenceInHours(period2.localStartTime())) < 3)
    {
      retVector.back() = resultVector[i];
      theParameters.theLog
          << "Several wind speed changes on the same part of the day -> report only last one: "
          << resultVector[i] << std::endl;
    }
    else
      retVector.push_back(resultVector[i]);
  }

  return retVector;
}

bool WindForecast::getWindSpeedChangeAttribute(const WeatherPeriod& changePeriod,
                                               std::string& phraseStr,
                                               bool& smallChange,
                                               bool& gradualChange,
                                               bool& fastChange) const
{
  float begLowerLimit(kFloatMissing), begUpperLimit(kFloatMissing), endLowerLimit(kFloatMissing),
      endUpperLimit(kFloatMissing);

  smallChange = gradualChange = fastChange = false;

  get_wind_speed_interval(
      changePeriod.localStartTime(), theParameters, begLowerLimit, begUpperLimit);
  get_wind_speed_interval(changePeriod.localEndTime(), theParameters, endLowerLimit, endUpperLimit);

  if (begLowerLimit == kFloatMissing || begUpperLimit == kFloatMissing ||
      endLowerLimit == kFloatMissing || endUpperLimit == kFloatMissing)
  {
    theParameters.theLog << "Error: failed to get wind speed change parameters for period "
                         << changePeriod << ": " << begLowerLimit << "..." << begUpperLimit
                         << " and " << endLowerLimit << "..." << endUpperLimit << std::endl;
    return false;
  }

  begLowerLimit = round(begLowerLimit);
  begUpperLimit = round(begUpperLimit);
  endLowerLimit = round(endLowerLimit);
  endUpperLimit = round(endUpperLimit);

  float changeThreshold(5.0);
  int periodLength(get_period_length(changePeriod));

  phraseStr = EMPTY_STRING;
  if (abs(endUpperLimit - begUpperLimit) <= changeThreshold * 0.5)
  {
    phraseStr = VAHAN_WORD;
    smallChange = true;
  }
  else if (abs(endUpperLimit - begUpperLimit) >= changeThreshold)
  {
    if (periodLength <= 6 && endUpperLimit >= NAVAKKA_UPPER_LIMIT)
    {
      phraseStr = NOPEASTI_WORD;
      fastChange = true;
    }
    else if (periodLength >= 10)
    {
      phraseStr = VAHITELLEN_WORD;
      gradualChange = true;
    }
  }

  return true;
}

Sentence WindForecast::speedRangeSentence(const WeatherPeriod& thePeriod,
                                          const WeatherResult& theTopSpeed,
                                          const WeatherResult& theMedianSpeed,
                                          const string& theVariable,
                                          bool theUseAtItsStrongestPhrase) const
{
  using Settings::optional_int;

  Sentence sentence;

  float intervalLowerLimit(theMedianSpeed.value());
  float intervalUpperLimit(theTopSpeed.value());

  get_wind_speed_interval(thePeriod, theParameters, intervalLowerLimit, intervalUpperLimit);

  int lowerLimit = round(intervalLowerLimit);
  int upperLimit = round(intervalUpperLimit);

  theParameters.theLog << " rounded interval: " << lowerLimit << "..." << upperLimit << std::endl;

  int peakWind = intervalUpperLimit;

  compose_wind_speed_range(theParameters, lowerLimit, upperLimit, peakWind);

  theParameters.theLog << " final interval: " << lowerLimit << "..." << upperLimit
                       << " peak: " << peakWind << std::endl;

  return compose_wind_speed_sentence(
      theParameters, lowerLimit, upperLimit, peakWind, theUseAtItsStrongestPhrase);
}

Sentence WindForecast::windSpeedIntervalSentence(const WeatherPeriod& thePeriodLowerLimit,
                                                 const WeatherPeriod& thePeriodUpperLimit,
                                                 bool theUseAtItsStrongestPhrase /* = true*/) const
{
  float intervalLowerLimit(-kFloatMissing);
  float intervalUpperLimit(kFloatMissing);

  // get upper limit
  get_wind_speed_interval(
      thePeriodUpperLimit, theParameters, intervalLowerLimit, intervalUpperLimit);

  int upperLimit = round(intervalUpperLimit);
  int peakWind = upperLimit;

  // get lower limit
  get_wind_speed_interval(
      thePeriodLowerLimit, theParameters, intervalLowerLimit, intervalUpperLimit);
  int lowerLimit = round(intervalLowerLimit);

  compose_wind_speed_range(theParameters, lowerLimit, upperLimit, peakWind);

  return compose_wind_speed_sentence(
      theParameters, lowerLimit, upperLimit, peakWind, theUseAtItsStrongestPhrase);
}

Sentence WindForecast::windSpeedIntervalSentence(const WeatherPeriod& thePeriod,
                                                 bool theUseAtItsStrongestPhrase /* = true*/) const
{
  Sentence sentence;

  WeatherResult lowerLimit(kFloatMissing, 0.0);
  WeatherResult upperLimit(kFloatMissing, 0.0);

  sentence << speedRangeSentence(
      thePeriod, upperLimit, lowerLimit, theParameters.theVar, theUseAtItsStrongestPhrase);
  return sentence;
}

}  // namespace TextGen
