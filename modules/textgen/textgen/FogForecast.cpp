// ======================================================================
/*!
 * \file
 * \brief Implementation of FogForecast class
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
#include "PrecipitationPeriodTools.h"
#include "PrecipitationStoryTools.h"
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
#include "FogForecast.h"

#include <newbase/NFmiCombinedParam.h>
#include <newbase/NFmiMercatorArea.h>

#include <boost/lexical_cast.hpp>
#include <vector>
#include <map>

namespace TextGen
{
using namespace Settings;
using namespace TextGen;
using namespace AreaTools;
using namespace boost;
using namespace std;

#define TIME_PLACE_INPLACES_FOG_COMPOSITE_PHRASE "[huomenna] [sisamaassa] [paikoin] sumua"
#define TIME_PLACE_FOG_COMPOSITE_PHRASE "[huomenna] [sisamaassa] sumua"
#define PLACE_INPLACES_FOG_COMPOSITE_PHRASE "[sisamaassa] [paikoin] sumua"
#define PLACE_FOG_COMPOSITE_PHRASE "[sisamaassa] sumua"
#define TIME_INPLACES_FOG_COMPOSITE_PHRASE "[huomenna] [paikoin] sumua"
#define TIME_FOG_COMPOSITE_PHRASE "[huomenna] sumua"
#define INPLACES_FOG_COMPOSITE_PHRASE "[paikoin] sumua"

#define TIME_PLACE_INPLACES_FOG_DENSE_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] [paikoin] sumua, joka voi olla sakeaa"
#define TIME_PLACE_FOG_DENSE_COMPOSITE_PHRASE "[huomenna] [sisamaassa] sumua, joka voi olla sakeaa"
#define PLACE_INPLACES_FOG_DENSE_COMPOSITE_PHRASE \
  "[sisamaassa] [paikoin] sumua, joka voi olla sakeaa"
#define PLACE_FOG_DENSE_COMPOSITE_PHRASE "[sisamaassa] sumua, joka voi olla sakeaa"
#define TIME_INPLACES_FOG_DENSE_COMPOSITE_PHRASE "[huomenna] [paikoin] sumua, joka voi olla sakeaa"
#define TIME_FOG_DENSE_COMPOSITE_PHRASE "[huomenna] sumua, joka voi olla sakeaa"
#define INPLACES_FOG_DENSE_COMPOSITE_PHRASE "[paikoin] sumua, joka voi olla sakeaa"

/*
std::ostream& operator<<(std::ostream & theOutput,
                                                 const FogIntensityDataItem&
theFogIntensityDataItem)
{
      theOutput << "moderate fog=" << theFogIntensityDataItem.moderateFogExtent << " ";
      theOutput << "dense fog=" << theFogIntensityDataItem.denseFogExtent << " ";
      return theOutput;
}

std::ostream& operator<<(std::ostream & theOutput,
                                                 const CloudinessDataItem& theCloudinessDataItem)
{
      if(theCloudinessDataItem.theCoastalData)
        {
              theOutput << "  Coastal" << endl;
              theOutput << *theCloudinessDataItem.theCoastalData;
        }
      if(theCloudinessDataItem.theInlandData)
        {
              theOutput << "  Inland" << endl;
              theOutput << *theCloudinessDataItem.theInlandData;
        }
      if(theCloudinessDataItem.theFullData)
        {
              theOutput << "  Full area" << endl;
              theOutput << *theCloudinessDataItem.theFullData;
        }
      return theOutput;
}
*/

pair<WeatherPeriod, FogIntensityDataItem> get_fog_wp(const TextGenPosixTime& theStartTime,
                                                     const TextGenPosixTime& theEndTime,
                                                     const float& theModerateFogExtent,
                                                     const float& theDenseFogExtent)
{
  return make_pair(WeatherPeriod(theStartTime, theEndTime),
                   FogIntensityDataItem(theModerateFogExtent, theDenseFogExtent));
}

pair<WeatherPeriod, fog_type_id> get_fog_type_wp(const TextGenPosixTime& theStartTime,
                                                 const TextGenPosixTime& theEndTime,
                                                 const fog_type_id& theFogTypeId)
{
  return make_pair(WeatherPeriod(theStartTime, theEndTime), theFogTypeId);
}

const char* get_fog_type_string(const fog_type_id& theFogTypeId)
{
  const char* retval = "";

  switch (theFogTypeId)
  {
    case FOG:
      retval = "sumua";
      break;
    case FOG_POSSIBLY_DENSE:
      retval = "sumua, joka voi olla sakeaa";
      break;
    case FOG_IN_SOME_PLACES:
      retval = "paikoin sumua";
      break;
    case FOG_IN_SOME_PLACES_POSSIBLY_DENSE:
      retval = "paikoin sumua, joka voi olla sakeaa";
      break;
    case FOG_IN_MANY_PLACES:
      retval = "monin paikoin sumua";
      break;
    case FOG_IN_MANY_PLACES_POSSIBLY_DENSE:
      retval = "monin paikoin sumua, joka voi olla sakeaa";
      break;
    default:
      retval = "ei sumua";
      break;
  }

  return retval;
}

fog_type_id get_fog_type(const float& theModerateFog, const float& theDenseFog)
{
  float totalFog = theModerateFog + theDenseFog;

  if (totalFog < IN_SOME_PLACES_LOWER_LIMIT_FOG || theModerateFog == kFloatMissing ||
      theDenseFog == kFloatMissing)
  {
    return NO_FOG;
  }
  else if (totalFog >= IN_SOME_PLACES_LOWER_LIMIT_FOG && totalFog <= IN_SOME_PLACES_UPPER_LIMIT)
  {
    if (theDenseFog < IN_SOME_PLACES_LOWER_LIMIT_FOG)
      return FOG_IN_SOME_PLACES;
    else
      return FOG_IN_SOME_PLACES_POSSIBLY_DENSE;
  }
  else if (totalFog > IN_MANY_PLACES_LOWER_LIMIT && totalFog < IN_MANY_PLACES_UPPER_LIMIT)
  {
    if (theDenseFog < IN_SOME_PLACES_LOWER_LIMIT_FOG)
      return FOG_IN_MANY_PLACES;
    else
      return FOG_IN_MANY_PLACES_POSSIBLY_DENSE;
  }
  else
  {
    if (theDenseFog < IN_SOME_PLACES_LOWER_LIMIT_FOG)
      return FOG;
    else
      return FOG_POSSIBLY_DENSE;
  }
}

FogForecast::FogForecast(wf_story_params& parameters)
    : theParameters(parameters),
      theCoastalModerateFogData(0),
      theInlandModerateFogData(0),
      theFullAreaModerateFogData(0),
      theCoastalDenseFogData(0),
      theInlandDenseFogData(0),
      theFullAreaDenseFogData(0)

{
  if (theParameters.theForecastArea & FULL_AREA)
  {
    theFullAreaModerateFogData =
        ((*theParameters.theCompleteData[FULL_AREA])[FOG_INTENSITY_MODERATE_DATA]);
    theFullAreaDenseFogData =
        ((*theParameters.theCompleteData[FULL_AREA])[FOG_INTENSITY_DENSE_DATA]);
  }
  if (theParameters.theForecastArea & COASTAL_AREA)
  {
    theCoastalModerateFogData =
        ((*theParameters.theCompleteData[COASTAL_AREA])[FOG_INTENSITY_MODERATE_DATA]);
    theCoastalDenseFogData =
        ((*theParameters.theCompleteData[COASTAL_AREA])[FOG_INTENSITY_DENSE_DATA]);
  }
  if (theParameters.theForecastArea & INLAND_AREA)
  {
    theInlandModerateFogData =
        ((*theParameters.theCompleteData[INLAND_AREA])[FOG_INTENSITY_MODERATE_DATA]);
    theInlandDenseFogData =
        ((*theParameters.theCompleteData[INLAND_AREA])[FOG_INTENSITY_DENSE_DATA]);
  }

  findOutFogPeriods();
  findOutFogTypePeriods();
}

void FogForecast::findOutFogPeriods(const weather_result_data_item_vector* theModerateFogData,
                                    const weather_result_data_item_vector* theDenseFogData,
                                    fog_period_vector& theFogPeriods)
{
  if (theModerateFogData && theDenseFogData && theModerateFogData->size() > 0)
  {
    TextGenPosixTime fogPeriodStartTime(theModerateFogData->at(0)->thePeriod.localStartTime());

    for (unsigned int i = 1; i < theModerateFogData->size(); i++)
    {
      if (i == theModerateFogData->size() - 1)
      {
        if ((theModerateFogData->at(i)->theResult.value() !=
                 theModerateFogData->at(i - 1)->theResult.value() ||
             theDenseFogData->at(i)->theResult.value() !=
                 theDenseFogData->at(i - 1)->theResult.value()))
        {
          theFogPeriods.push_back(
              get_fog_wp(fogPeriodStartTime,
                         theModerateFogData->at(i - 1)->thePeriod.localEndTime(),
                         theModerateFogData->at(i - 1)->theResult.value(),
                         theDenseFogData->at(i - 1)->theResult.value()));
          theFogPeriods.push_back(get_fog_wp(theModerateFogData->at(i)->thePeriod.localStartTime(),
                                             theModerateFogData->at(i)->thePeriod.localEndTime(),
                                             theModerateFogData->at(i)->theResult.value(),
                                             theDenseFogData->at(i)->theResult.value()));
        }
        else
        {
          theFogPeriods.push_back(get_fog_wp(fogPeriodStartTime,
                                             theModerateFogData->at(i)->thePeriod.localEndTime(),
                                             theModerateFogData->at(i)->theResult.value(),
                                             theDenseFogData->at(i)->theResult.value()));
        }
      }
      else
      {
        if ((theModerateFogData->at(i)->theResult.value() !=
                 theModerateFogData->at(i - 1)->theResult.value() ||
             theDenseFogData->at(i)->theResult.value() !=
                 theDenseFogData->at(i - 1)->theResult.value()))
        {
          theFogPeriods.push_back(
              get_fog_wp(fogPeriodStartTime,
                         theModerateFogData->at(i - 1)->thePeriod.localEndTime(),
                         theModerateFogData->at(i - 1)->theResult.value(),
                         theDenseFogData->at(i - 1)->theResult.value()));
          fogPeriodStartTime = theModerateFogData->at(i)->thePeriod.localStartTime();
        }
      }
    }
  }
}

void FogForecast::findOutFogPeriods()
{
  findOutFogPeriods(theCoastalModerateFogData, theCoastalDenseFogData, theCoastalFog);
  findOutFogPeriods(theInlandModerateFogData, theInlandDenseFogData, theInlandFog);
  findOutFogPeriods(theFullAreaModerateFogData, theFullAreaDenseFogData, theFullAreaFog);
}

void FogForecast::findOutFogTypePeriods(const fog_period_vector& theFogPeriods,
                                        fog_type_period_vector& theFogTypePeriods)
{
  if (theFogPeriods.size() == 0) return;

  TextGenPosixTime fogPeriodStartTime(theFogPeriods.at(0).first.localStartTime());
  fog_type_id previousFogType = get_fog_type(theFogPeriods.at(0).second.theModerateFogExtent,
                                             theFogPeriods.at(0).second.theDenseFogExtent);
  for (unsigned int i = 1; i < theFogPeriods.size(); i++)
  {
    fog_type_id currentFogType = get_fog_type(theFogPeriods.at(i).second.theModerateFogExtent,
                                              theFogPeriods.at(i).second.theDenseFogExtent);

    if (i == theFogPeriods.size() - 1)
    {
      if (previousFogType != currentFogType)
      {
        if (previousFogType != NO_FOG)
          theFogTypePeriods.push_back(get_fog_type_wp(
              fogPeriodStartTime, theFogPeriods.at(i - 1).first.localEndTime(), previousFogType));
        if (currentFogType != NO_FOG)
          theFogTypePeriods.push_back(get_fog_type_wp(theFogPeriods.at(i).first.localStartTime(),
                                                      theFogPeriods.at(i).first.localEndTime(),
                                                      currentFogType));
      }
      else
      {
        if (currentFogType != NO_FOG)
          theFogTypePeriods.push_back(get_fog_type_wp(
              fogPeriodStartTime, theFogPeriods.at(i).first.localEndTime(), currentFogType));
      }
    }
    else
    {
      if (previousFogType != currentFogType)
      {
        if (previousFogType != NO_FOG)
          theFogTypePeriods.push_back(get_fog_type_wp(
              fogPeriodStartTime, theFogPeriods.at(i - 1).first.localEndTime(), previousFogType));
        fogPeriodStartTime = theFogPeriods.at(i).first.localStartTime();
        previousFogType = currentFogType;
      }
    }
  }
}

void FogForecast::findOutFogTypePeriods()
{
  findOutFogTypePeriods(theCoastalFog, theCoastalFogType);
  findOutFogTypePeriods(theInlandFog, theInlandFogType);
  findOutFogTypePeriods(theFullAreaFog, theFullAreaFogType);
}

void FogForecast::printOutFogPeriods(std::ostream& theOutput,
                                     const fog_period_vector& theFogPeriods) const
{
  for (unsigned int i = 0; i < theFogPeriods.size(); i++)
  {
    WeatherPeriod period(theFogPeriods.at(i).first.localStartTime(),
                         theFogPeriods.at(i).first.localEndTime());

    float moderateFog(theFogPeriods.at(i).second.theModerateFogExtent);
    float denseFog(theFogPeriods.at(i).second.theDenseFogExtent);

    theOutput << period.localStartTime() << "..." << period.localEndTime()
              << ": moderate=" << moderateFog << " dense=" << denseFog << endl;
  }
}

void FogForecast::printOutFogData(std::ostream& theOutput,
                                  const std::string& theLinePrefix,
                                  const weather_result_data_item_vector& theFogData) const
{
  for (unsigned int i = 0; i < theFogData.size(); i++)
  {
    WeatherPeriod period(theFogData.at(i)->thePeriod.localStartTime(),
                         theFogData.at(i)->thePeriod.localEndTime());

    theOutput << theFogData.at(i)->thePeriod.localStartTime() << "..."
              << theFogData.at(i)->thePeriod.localEndTime() << ": " << theLinePrefix << "="
              << theFogData.at(i)->theResult.value() << endl;
  }
}

void FogForecast::printOutFogData(std::ostream& theOutput) const
{
  theOutput << "** FOG DATA **" << endl;
  bool fogDataExists = false;

  if (theCoastalModerateFogData && theCoastalModerateFogData->size() > 0)
  {
    theOutput << "** Coastal moderate (1) fog data **" << endl;
    printOutFogData(theOutput, "moderate", *theCoastalModerateFogData);
    fogDataExists = true;
  }
  if (theCoastalDenseFogData && theCoastalDenseFogData->size() > 0)
  {
    theOutput << "** Coastal dense (2) fog data **" << endl;
    printOutFogData(theOutput, "dense", *theCoastalDenseFogData);
    fogDataExists = true;
  }

  if (theInlandModerateFogData && theInlandModerateFogData->size() > 0)
  {
    theOutput << "** Inland moderate (1) fog data **" << endl;
    printOutFogData(theOutput, "moderate", *theInlandModerateFogData);
    fogDataExists = true;
  }
  if (theInlandDenseFogData && theInlandDenseFogData->size() > 0)
  {
    theOutput << "** Inland dense (2) fog data **" << endl;
    printOutFogData(theOutput, "dense", *theInlandDenseFogData);
    fogDataExists = true;
  }

  if (theFullAreaModerateFogData && theFullAreaModerateFogData->size() > 0)
  {
    theOutput << "** Full area moderate (1) fog data **" << endl;
    printOutFogData(theOutput, "moderate", *theFullAreaModerateFogData);
    fogDataExists = true;
  }
  if (theFullAreaDenseFogData && theFullAreaDenseFogData->size() > 0)
  {
    theOutput << "** Full dense (2) fog data **" << endl;
    printOutFogData(theOutput, "dense", *theFullAreaDenseFogData);
    fogDataExists = true;
  }

  if (!fogDataExists) theOutput << "No Fog on this forecast period!" << endl;
}

void FogForecast::printOutFogPeriods(std::ostream& theOutput) const
{
  theOutput << "** FOG PERIODS **" << endl;

  if (theCoastalFog.size() > 0)
  {
    theOutput << "Coastal fog: " << endl;
    printOutFogPeriods(theOutput, theCoastalFog);
  }
  if (theInlandFog.size() > 0)
  {
    theOutput << "Inland fog: " << endl;
    printOutFogPeriods(theOutput, theInlandFog);
  }
  if (theFullAreaFog.size() > 0)
  {
    theOutput << "Full area fog: " << endl;
    printOutFogPeriods(theOutput, theFullAreaFog);
  }
}

void FogForecast::printOutFogTypePeriods(std::ostream& theOutput,
                                         const fog_type_period_vector& theFogTypePeriods) const
{
  for (unsigned int i = 0; i < theFogTypePeriods.size(); i++)
  {
    WeatherPeriod period(theFogTypePeriods.at(i).first.localStartTime(),
                         theFogTypePeriods.at(i).first.localEndTime());

    theOutput << period.localStartTime() << "..." << period.localEndTime() << ": "
              << get_fog_type_string(theFogTypePeriods.at(i).second) << endl;
  }
}

void FogForecast::printOutFogTypePeriods(std::ostream& theOutput) const
{
  theOutput << "** FOG TYPE PERIODS **" << endl;

  if (theCoastalFog.size() > 0)
  {
    theOutput << "Coastal fog types: " << endl;
    printOutFogTypePeriods(theOutput, theCoastalFogType);
  }
  if (theInlandFog.size() > 0)
  {
    theOutput << "Inland fog types: " << endl;
    printOutFogTypePeriods(theOutput, theInlandFogType);
  }
  if (theFullAreaFog.size() > 0)
  {
    theOutput << "Full area fog types: " << endl;
    printOutFogTypePeriods(theOutput, theFullAreaFogType);
  }
}

float FogForecast::getMean(const fog_period_vector& theFogPeriods,
                           const WeatherPeriod& theWeatherPeriod) const
{
  float sum(0.0);
  unsigned int count(0);

  for (unsigned int i = 0; i < theFogPeriods.size(); i++)
  {
    float totalFog = theFogPeriods.at(i).second.theModerateFogExtent +
                     theFogPeriods.at(i).second.theDenseFogExtent;
    if (theFogPeriods.at(i).first.localStartTime() >= theWeatherPeriod.localStartTime() &&
        theFogPeriods.at(i).first.localStartTime() <= theWeatherPeriod.localEndTime() &&
        theFogPeriods.at(i).first.localEndTime() >= theWeatherPeriod.localStartTime() &&
        theFogPeriods.at(i).first.localEndTime() <= theWeatherPeriod.localEndTime() && totalFog > 0)
    {
      sum += totalFog;
      count++;
    }
  }

  return (count > 0 ? sum / count : 0);
}

Sentence FogForecast::getFogPhrase(const fog_type_id& theFogTypeId) const
{
  Sentence sentence;

  switch (theFogTypeId)
  {
    case FOG:
      sentence << SUMUA_WORD;
      break;
    case FOG_POSSIBLY_DENSE:
      sentence << SUMUA_WORD << Delimiter(",") << JOKA_VOI_OLLA_SAKEAA_PHRASE;
      break;
    case FOG_IN_SOME_PLACES:
      sentence << PAIKOIN_WORD << SUMUA_WORD;
      break;
    case FOG_IN_SOME_PLACES_POSSIBLY_DENSE:
      sentence << PAIKOIN_WORD << SUMUA_WORD << Delimiter(",") << JOKA_VOI_OLLA_SAKEAA_PHRASE;
      break;
    case FOG_IN_MANY_PLACES:
      sentence << MONIN_PAIKOIN_WORD << SUMUA_WORD;
      break;
    case FOG_IN_MANY_PLACES_POSSIBLY_DENSE:
      sentence << MONIN_PAIKOIN_WORD << SUMUA_WORD << Delimiter(",") << JOKA_VOI_OLLA_SAKEAA_PHRASE;
      break;
    default:
      break;
  }
  return sentence;
}

WeatherPeriod FogForecast::getActualFogPeriod(const WeatherPeriod& theForecastPeriod,
                                              const WeatherPeriod& theFogPeriod,
                                              bool& theFogPeriodOkFlag) const
{
  int start_year(0), start_month(0), start_day(0), start_hour(0);
  int end_year(0), end_month(0), end_day(0), end_hour(0);
  theParameters.theLog << "getActualFogPeriod ";
  theFogPeriodOkFlag = false;

  if (is_inside(theFogPeriod.localStartTime(), theForecastPeriod) &&
      !is_inside(theFogPeriod.localEndTime(), theForecastPeriod))
  {
    start_year = theFogPeriod.localStartTime().GetYear();
    start_month = theFogPeriod.localStartTime().GetMonth();
    start_day = theFogPeriod.localStartTime().GetDay();
    start_hour = theFogPeriod.localStartTime().GetHour();
    end_year = theForecastPeriod.localEndTime().GetYear();
    end_month = theForecastPeriod.localEndTime().GetMonth();
    end_day = theForecastPeriod.localEndTime().GetDay();
    end_hour = theForecastPeriod.localEndTime().GetHour();
    theFogPeriodOkFlag = true;
  }
  else if (is_inside(theFogPeriod.localEndTime(), theForecastPeriod) &&
           !is_inside(theFogPeriod.localStartTime(), theForecastPeriod))
  {
    start_year = theForecastPeriod.localStartTime().GetYear();
    start_month = theForecastPeriod.localStartTime().GetMonth();
    start_day = theForecastPeriod.localStartTime().GetDay();
    start_hour = theForecastPeriod.localStartTime().GetHour();
    end_year = theFogPeriod.localEndTime().GetYear();
    end_month = theFogPeriod.localEndTime().GetMonth();
    end_day = theFogPeriod.localEndTime().GetDay();
    end_hour = theFogPeriod.localEndTime().GetHour();
    theFogPeriodOkFlag = true;
  }
  else if (is_inside(theFogPeriod.localEndTime(), theForecastPeriod) &&
           is_inside(theFogPeriod.localStartTime(), theForecastPeriod))
  {
    start_year = theFogPeriod.localStartTime().GetYear();
    start_month = theFogPeriod.localStartTime().GetMonth();
    start_day = theFogPeriod.localStartTime().GetDay();
    start_hour = theFogPeriod.localStartTime().GetHour();
    end_year = theFogPeriod.localEndTime().GetYear();
    end_month = theFogPeriod.localEndTime().GetMonth();
    end_day = theFogPeriod.localEndTime().GetDay();
    end_hour = theFogPeriod.localEndTime().GetHour();
    theFogPeriodOkFlag = true;
  }
  else if (theFogPeriod.localStartTime() <= theForecastPeriod.localStartTime() &&
           theFogPeriod.localEndTime() >= theForecastPeriod.localEndTime())
  {
    start_year = theForecastPeriod.localStartTime().GetYear();
    start_month = theForecastPeriod.localStartTime().GetMonth();
    start_day = theForecastPeriod.localStartTime().GetDay();
    start_hour = theForecastPeriod.localStartTime().GetHour();
    end_year = theForecastPeriod.localEndTime().GetYear();
    end_month = theForecastPeriod.localEndTime().GetMonth();
    end_day = theForecastPeriod.localEndTime().GetDay();
    end_hour = theForecastPeriod.localEndTime().GetHour();
    theFogPeriodOkFlag = true;
  }

  if (theFogPeriodOkFlag)
  {
    TextGenPosixTime startTime(start_year, start_month, start_day, start_hour);
    TextGenPosixTime endTime(end_year, end_month, end_day, end_hour);

    WeatherPeriod theResultFogPeriod(startTime, endTime);

    theParameters.theLog << "result: ";
    theParameters.theLog << startTime;
    theParameters.theLog << "...";
    theParameters.theLog << endTime << endl;

    return theResultFogPeriod;
  }

  return theFogPeriod;
}

bool FogForecast::getFogPeriodAndId(const WeatherPeriod& theForecastPeriod,
                                    const fog_type_period_vector& theFogTypePeriods,
                                    WeatherPeriod& theResultPeriod,
                                    fog_type_id& theFogTypeId) const
{
  bool fogPeriodOk(false);

  if (theFogTypePeriods.size() == 1)
  {
    WeatherPeriod actualFogPeriod(
        getActualFogPeriod(theForecastPeriod, theFogTypePeriods.at(0).first, fogPeriodOk));
    if (fogPeriodOk)
    {
      if (!(actualFogPeriod.localEndTime().DifferenceInHours(actualFogPeriod.localStartTime()) ==
                1 &&
            (actualFogPeriod.localStartTime() == theForecastPeriod.localStartTime() ||
             actualFogPeriod.localEndTime() == theForecastPeriod.localEndTime())))
      {
        theResultPeriod = actualFogPeriod;
        theFogTypeId = theFogTypePeriods.at(0).second;
        return true;
      }
    }
  }
  else
  {
    float forecastPeriodLength(
        theForecastPeriod.localEndTime().DifferenceInHours(theForecastPeriod.localStartTime()));
    int longestFogPeriodIndex(-1);
    int firstPeriodIndex(-1);
    int lastPeriodIndex(-1);
    int fogIdSum(0);
    int fogPeriodCount(0);
    map<unsigned int, unsigned int> encounteredFogTypes;

    // Merge close periods. If one long fog-period use that and ignore the small ones,
    // otherwise theFogTypeId is weighted average of all fog-periods and the
    // returned fog-period encompasses all small periods.
    for (unsigned int i = 0; i < theFogTypePeriods.size(); i++)
    {
      WeatherPeriod actualFogPeriod(
          getActualFogPeriod(theForecastPeriod, theFogTypePeriods.at(i).first, fogPeriodOk));

      if (!fogPeriodOk) continue;

      int actualPeriodLength(
          actualFogPeriod.localEndTime().DifferenceInHours(actualFogPeriod.localStartTime()));

      if (actualFogPeriod.localEndTime().DifferenceInHours(actualFogPeriod.localStartTime()) == 1 &&
          (actualFogPeriod.localStartTime() == theForecastPeriod.localStartTime() ||
           actualFogPeriod.localEndTime() == theForecastPeriod.localEndTime()))
        continue;

      if (longestFogPeriodIndex == -1)
      {
        longestFogPeriodIndex = i;
        firstPeriodIndex = i;
      }
      else
      {
        WeatherPeriod longestPeriod(theFogTypePeriods.at(longestFogPeriodIndex).first);

        if (actualPeriodLength >
            longestPeriod.localEndTime().DifferenceInHours(longestPeriod.localStartTime()))
        {
          longestFogPeriodIndex = i;
        }
      }

      lastPeriodIndex = i;
      fogIdSum += (theFogTypePeriods.at(i).second * actualPeriodLength);
      fogPeriodCount += actualPeriodLength;
      encounteredFogTypes.insert(
          make_pair(theFogTypePeriods.at(i).second, theFogTypePeriods.at(i).second));
    }

    if (longestFogPeriodIndex >= 0)
    {
      WeatherPeriod longestPeriod(theFogTypePeriods.at(longestFogPeriodIndex).first);
      float longestPeriodLength(
          longestPeriod.localEndTime().DifferenceInHours(longestPeriod.localStartTime()));

      // if the longest period is more than 70% of the forecast period, use it
      if (longestPeriodLength / forecastPeriodLength > 0.70)
      {
        theResultPeriod = getActualFogPeriod(
            theForecastPeriod, theFogTypePeriods.at(longestFogPeriodIndex).first, fogPeriodOk);
        theFogTypeId = theFogTypePeriods.at(longestFogPeriodIndex).second;
        return true;
      }
      else
      {
        WeatherPeriod firstPeriod(getActualFogPeriod(
            theForecastPeriod, theFogTypePeriods.at(firstPeriodIndex).first, fogPeriodOk));
        WeatherPeriod lastPeriod(getActualFogPeriod(
            theForecastPeriod, theFogTypePeriods.at(lastPeriodIndex).first, fogPeriodOk));
        theResultPeriod = WeatherPeriod(firstPeriod.localStartTime(), lastPeriod.localEndTime());
        float periodIdAverage(static_cast<float>(fogIdSum) / static_cast<float>(fogPeriodCount));

        // find the fog type that is closest to the average
        fog_type_id finalFogType = NO_FOG;
        float fogTypeGap = 10.0;
        for (unsigned int i = FOG; i < NO_FOG; i++)
        {
          if (encounteredFogTypes.find(i) != encounteredFogTypes.end())
          {
            if (finalFogType == NO_FOG)
            {
              finalFogType = static_cast<fog_type_id>(i);
              fogTypeGap = abs(periodIdAverage - i);
            }
            else
            {
              if (fogTypeGap > abs(periodIdAverage - static_cast<float>(i)))
              {
                finalFogType = static_cast<fog_type_id>(i);
                fogTypeGap = abs(periodIdAverage - i);
              }
            }
          }
        }
        theFogTypeId = finalFogType;
        return (theFogTypeId != NO_FOG);
      }
    }
  }
  return false;
}

Sentence FogForecast::constructFogSentence(const std::string& theDayPhasePhrase,
                                           const std::string& theAreaString,
                                           const std::string& theInPlacesString,
                                           const bool& thePossiblyDenseFlag) const
{
  Sentence sentence;

  bool dayPhaseExists(!theDayPhasePhrase.empty());
  bool placeExists(!theAreaString.empty());
  bool inPlacesPhraseExists(!theInPlacesString.empty());

  if (dayPhaseExists)
  {
    if (placeExists)
    {
      if (inPlacesPhraseExists)
      {
        if (thePossiblyDenseFlag)
          sentence << TIME_PLACE_INPLACES_FOG_DENSE_COMPOSITE_PHRASE;
        else
          sentence << TIME_PLACE_INPLACES_FOG_COMPOSITE_PHRASE;

        sentence << theDayPhasePhrase << theAreaString << theInPlacesString;
      }
      else
      {
        if (thePossiblyDenseFlag)
          sentence << TIME_PLACE_FOG_DENSE_COMPOSITE_PHRASE;
        else
          sentence << TIME_PLACE_FOG_COMPOSITE_PHRASE;

        sentence << theDayPhasePhrase << theAreaString;
      }
    }
    else
    {
      if (inPlacesPhraseExists)
      {
        if (thePossiblyDenseFlag)
          sentence << TIME_INPLACES_FOG_DENSE_COMPOSITE_PHRASE;
        else
          sentence << TIME_INPLACES_FOG_COMPOSITE_PHRASE;
        sentence << theDayPhasePhrase << theInPlacesString;
      }
      else
      {
        if (thePossiblyDenseFlag)
          sentence << TIME_FOG_DENSE_COMPOSITE_PHRASE;
        else
          sentence << TIME_FOG_COMPOSITE_PHRASE;
        sentence << theDayPhasePhrase;
      }
    }
  }
  else
  {
    if (placeExists)
    {
      if (inPlacesPhraseExists)
      {
        if (thePossiblyDenseFlag)
          sentence << PLACE_INPLACES_FOG_DENSE_COMPOSITE_PHRASE;
        else
          sentence << PLACE_INPLACES_FOG_COMPOSITE_PHRASE;
        sentence << theAreaString << theInPlacesString;
      }
      else
      {
        if (thePossiblyDenseFlag)
          sentence << INPLACES_FOG_DENSE_COMPOSITE_PHRASE;
        else
          sentence << INPLACES_FOG_COMPOSITE_PHRASE;
        sentence << theAreaString;
      }
    }
    else
    {
      if (inPlacesPhraseExists)
      {
        if (thePossiblyDenseFlag)
          sentence << INPLACES_FOG_DENSE_COMPOSITE_PHRASE;
        else
          sentence << INPLACES_FOG_COMPOSITE_PHRASE;
        sentence << theInPlacesString;
      }
      else
      {
        sentence << SUMUA_WORD;
        if (thePossiblyDenseFlag) sentence << Delimiter(",") << JOKA_VOI_OLLA_SAKEAA_PHRASE;
      }
    }
  }

  return sentence;
}

Sentence FogForecast::fogSentence(const WeatherPeriod& thePeriod,
                                  const fog_type_period_vector& theFogTypePeriods,
                                  const std::string& theAreaString) const
{
  Sentence sentence;
  WeatherPeriod fogPeriod(thePeriod);
  fog_type_id fogTypeId(NO_FOG);

  if (getFogPeriodAndId(thePeriod, theFogTypePeriods, fogPeriod, fogTypeId))
  {
    Sentence todayPhrase;

    if (thePeriod.localEndTime().DifferenceInHours(thePeriod.localStartTime()) > 24)
    {
      todayPhrase << PeriodPhraseFactory::create("today",
                                                 theParameters.theVariable,
                                                 theParameters.theForecastTime,
                                                 fogPeriod,
                                                 theParameters.theArea);
    }

    theParameters.theLog << todayPhrase;

    vector<std::string> theStringVector;

    bool specifyDay =
        get_period_length(theParameters.theForecastPeriod) > 24 && todayPhrase.size() > 0;

    std::string dayPhasePhrase;
    part_of_the_day_id id;
    get_time_phrase_large(
        fogPeriod, specifyDay, theParameters.theVariable, dayPhasePhrase, false, id);

    WeatherHistory& thePhraseHistory = const_cast<WeatherArea&>(theParameters.theArea).history();

    if (dayPhasePhrase == thePhraseHistory.latestDayPhasePhrase)
      dayPhasePhrase = "";
    else
      thePhraseHistory.updateDayPhasePhrase(dayPhasePhrase);

    switch (fogTypeId)
    {
      case FOG:
        sentence << constructFogSentence(dayPhasePhrase, theAreaString, "", false);
        break;
      case FOG_POSSIBLY_DENSE:

        sentence << constructFogSentence(dayPhasePhrase, theAreaString, "", true);
        break;
      case FOG_IN_SOME_PLACES:
        sentence << constructFogSentence(dayPhasePhrase, theAreaString, PAIKOIN_WORD, false);
        break;
      case FOG_IN_SOME_PLACES_POSSIBLY_DENSE:

        sentence << constructFogSentence(dayPhasePhrase, theAreaString, PAIKOIN_WORD, true);
        break;
      case FOG_IN_MANY_PLACES:
        sentence << constructFogSentence(dayPhasePhrase, theAreaString, MONIN_PAIKOIN_WORD, false);
        break;
      case FOG_IN_MANY_PLACES_POSSIBLY_DENSE:
        sentence << constructFogSentence(dayPhasePhrase, theAreaString, MONIN_PAIKOIN_WORD, true);
        break;
      default:
        break;
    };
  }

  return sentence;
}

Sentence FogForecast::fogSentence(const WeatherPeriod& thePeriod) const
{
  Sentence sentence;

  if (theParameters.theArea.isMarine() || theParameters.theArea.isIsland())
  {
    sentence << fogSentence(thePeriod, theInlandFogType, EMPTY_STRING);
  }
  else
  {
    if (theParameters.theForecastArea & FULL_AREA)
    {
      float coastalFogAvgExtent(getMean(theCoastalFog, thePeriod));
      float inlandFogAvgExtent(getMean(theInlandFog, thePeriod));

      if (coastalFogAvgExtent >= IN_SOME_PLACES_LOWER_LIMIT_FOG &&
          inlandFogAvgExtent >= IN_SOME_PLACES_LOWER_LIMIT_FOG)
      {
        // ARE 31.10.2011: if fog exisis on both areas report the whole area together
        sentence << fogSentence(thePeriod, theFullAreaFogType, EMPTY_STRING);
      }
      else if (coastalFogAvgExtent >= IN_SOME_PLACES_LOWER_LIMIT_FOG &&
               inlandFogAvgExtent < IN_SOME_PLACES_LOWER_LIMIT_FOG)
      {
        sentence << fogSentence(thePeriod, theCoastalFogType, COAST_PHRASE);
      }
      else if (coastalFogAvgExtent < IN_SOME_PLACES_LOWER_LIMIT_FOG &&
               inlandFogAvgExtent >= IN_SOME_PLACES_LOWER_LIMIT_FOG)
      {
        sentence << fogSentence(thePeriod, theInlandFogType, INLAND_PHRASE);
      }
    }
    else if (theParameters.theForecastArea & INLAND_AREA)
    {
      sentence << fogSentence(thePeriod, theInlandFogType, EMPTY_STRING);
    }
    else if (theParameters.theForecastArea & COASTAL_AREA)
    {
      sentence << fogSentence(thePeriod, theCoastalFogType, EMPTY_STRING);
    }
  }

  return sentence;
}

Sentence FogForecast::areaSpecificSentence(const WeatherPeriod& thePeriod) const
{
  Sentence sentence;

  // If the area contains both coast and inland, we don't ude area specific sentence

  if (!(theParameters.theForecastArea & FULL_AREA))
  {
    WeatherResult northEastShare(kFloatMissing, 0);
    WeatherResult southEastShare(kFloatMissing, 0);
    WeatherResult southWestShare(kFloatMissing, 0);
    WeatherResult northWestShare(kFloatMissing, 0);
    RangeAcceptor acceptor;
    acceptor.lowerLimit(kTModerateFog);
    acceptor.upperLimit(kTDenseFog);

    AreaTools::getArealDistribution(theParameters.theSources,
                                    Fog,
                                    theParameters.theArea,
                                    thePeriod,
                                    acceptor,
                                    northEastShare,
                                    southEastShare,
                                    southWestShare,
                                    northWestShare);

    float north = northEastShare.value() + northWestShare.value();
    float south = southEastShare.value() + southWestShare.value();
    float east = northEastShare.value() + southEastShare.value();
    float west = northWestShare.value() + southWestShare.value();

    area_specific_sentence_id sentenceId = get_area_specific_sentence_id(north,
                                                                         south,
                                                                         east,
                                                                         west,
                                                                         northEastShare.value(),
                                                                         southEastShare.value(),
                                                                         southWestShare.value(),
                                                                         northWestShare.value(),
                                                                         false);

    Rect areaRect(theParameters.theArea);
    NFmiMercatorArea mercatorArea(areaRect.getBottomLeft(), areaRect.getTopRight());
    float areaHeightWidthRatio =
        mercatorArea.WorldRect().Height() / mercatorArea.WorldRect().Width();

    Sentence areaSpecificSentence;
    areaSpecificSentence << area_specific_sentence(north,
                                                   south,
                                                   east,
                                                   west,
                                                   northEastShare.value(),
                                                   southEastShare.value(),
                                                   southWestShare.value(),
                                                   northWestShare.value(),
                                                   false);

    // If the area is too cigar-shaped, we can use only north-south/east-west specifier
    if ((areaHeightWidthRatio >= 0.6 && areaHeightWidthRatio <= 1.5) ||
        (areaHeightWidthRatio < 0.6 &&
         (sentenceId == ALUEEN_ITAOSASSA || sentenceId == ALUEEN_LANSIOSASSA ||
          sentenceId == ENIMMAKSEEN_ALUEEN_ITAOSASSA ||
          sentenceId == ENIMMAKSEEN_ALUEEN_LANSIOSASSA)) ||
        (areaHeightWidthRatio > 1.5 &&
         (sentenceId == ALUEEN_POHJOISOSASSA || sentenceId == ALUEEN_ETELAOSASSA ||
          sentenceId == ENIMMAKSEEN_ALUEEN_POHJOISOSASSA ||
          sentenceId == ENIMMAKSEEN_ALUEEN_ETELAOSASSA)))
    {
      sentence << areaSpecificSentence;
    }
  }

  return sentence;
}
}
