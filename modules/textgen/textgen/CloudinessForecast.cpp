// ======================================================================
/*!
 * \file
 * \brief Implementation of CloudinessForecast class
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
#include "CloudinessForecast.h"

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

#define HUOMENNA_SISAMAASSA_PUOLIPILVISESTA_PILVISEEN_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] saa vaihtelee puolipilvisesta pilviseen"
#define SISAMAASSA_PUOLIPILVISESTA_PILVISEEN_COMPOSITE_PHRASE \
  "[sisamaassa] saa vaihtelee puolipilvisesta pilviseen"
#define HUOMENNA_PUOLIPILVISESTA_PILVISEEN_COMPOSITE_PHRASE \
  "[huomenna] saa vaihtelee puolipilvisesta pilviseen"

#define HUOMENNA_SISAMAASSA_PUOLIPILVISESTA_PILVISEEN_JA_POUTAINEN_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] saa vaihtelee puolipilvisesta pilviseen ja on poutainen"
#define SISAMAASSA_PUOLIPILVISESTA_PILVISEEN_JA_POUTAINEN_COMPOSITE_PHRASE \
  "[sisamaassa] saa vaihtelee puolipilvisesta pilviseen ja on poutainen"
#define HUOMENNA_PUOLIPILVISESTA_PILVISEEN_JA_POUTAINEN_COMPOSITE_PHRASE \
  "[huomenna] saa vaihtelee puolipilvisesta pilviseen ja on poutainen"

#define HUOMENNA_SISAMAASSA_SAA_ON_SELKEA_COMPOSITE_PHRASE "[huomenna] [sisamaassa] saa on [selkea]"
#define SISAMAASSA_SAA_ON_SELKEA_COMPOSITE_PHRASE "[sisamaassa] saa on [selkea]"
#define HUOMENNA_SAA_ON_SELKEA_COMPOSITE_PHRASE "[huomenna] saa on [selkea]"

#define HUOMENNA_SISAMAASSA_SAA_ON_SELKEA_JA_POUTAINEN_COMPOSITE_PHRASE \
  "[huomenna] [sisamaassa] saa on [selkea] ja poutainen"
#define SISAMAASSA_SAA_ON_SELKEA_JA_POUTAINEN_COMPOSITE_PHRASE \
  "[sisamaassa] saa on [selkea] ja poutainen"
#define HUOMENNA_SAA_ON_SELKEA_JA_POUTAINEN_COMPOSITE_PHRASE \
  "[huomenna] saa on [selkea] ja poutainen"
#define SAA_ON_SELKEA_JA_POUTAINEN_COMPOSITE_PHRASE "saa on [selkea] ja poutainen"

#define ILTAPAIVASTA_ALKAEN_PILVISTYVAA_COMPOSITE_PHRASE "[iltapaivasta alkaen] [pilvistyvaa]"

#define PUOLIPILVISESTA_PILVISEEN_JA_POUTAINEN_PHRASE \
  "saa vaihtelee puolipilvisesta pilviseen ja on poutainen"

std::ostream& operator<<(std::ostream& theOutput,
                         const CloudinessDataItemData& theCloudinessDataItemData)
{
  string cloudinessIdStr(cloudiness_string(theCloudinessDataItemData.theId));

  string weatherEventIdStr = weather_event_string(theCloudinessDataItemData.theWeatherEventId);

  theOutput << "    " << cloudinessIdStr << ": ";
  theOutput << "min=" << theCloudinessDataItemData.theMin << " ";
  theOutput << "mean=" << theCloudinessDataItemData.theMean << " ";
  theOutput << "max=" << theCloudinessDataItemData.theMax << " ";
  theOutput << "std.dev=" << theCloudinessDataItemData.theStandardDeviation << endl;
  theOutput << "    weather event: " << weatherEventIdStr << endl;
  theOutput << "    pearson coefficient: " << theCloudinessDataItemData.thePearsonCoefficient
            << endl;

  return theOutput;
}

std::ostream& operator<<(std::ostream& theOutput, const CloudinessDataItem& theCloudinessDataItem)
{
  if (theCloudinessDataItem.theCoastalData)
  {
    theOutput << "  Coastal" << endl;
    theOutput << *theCloudinessDataItem.theCoastalData;
  }
  if (theCloudinessDataItem.theInlandData)
  {
    theOutput << "  Inland" << endl;
    theOutput << *theCloudinessDataItem.theInlandData;
  }
  if (theCloudinessDataItem.theFullData)
  {
    theOutput << "  Full area" << endl;
    theOutput << *theCloudinessDataItem.theFullData;
  }
  return theOutput;
}

bool puolipilvisesta_pilviseen(const cloudiness_id& theCloudinessId1,
                               const cloudiness_id& theCloudinessId2)
{
  if (theCloudinessId1 != MISSING_CLOUDINESS_ID && theCloudinessId1 == PUOLIPILVINEN_JA_PILVINEN &&
      theCloudinessId2 == PUOLIPILVINEN_JA_PILVINEN)
    return true;

  return false;
}

Sentence cloudiness_sentence(const cloudiness_id& theCloudinessId, const bool& theShortForm)
{
  Sentence sentence;
  Sentence cloudinessSentence;

  switch (theCloudinessId)
  {
    case PUOLIPILVINEN_JA_PILVINEN:
      cloudinessSentence << VAIHTELEE_PUOLIPILVISESTA_PILVISEEN_PHRASE;
      break;
    case SELKEA:
      cloudinessSentence << SELKEA_WORD;
      break;
    case MELKO_SELKEA:
      cloudinessSentence << MELKO_SELKEA_PHRASE;
      break;
    case PUOLIPILVINEN:
      cloudinessSentence << PUOLIPILVINEN_WORD;
      break;
    case VERRATTAIN_PILVINEN:
      cloudinessSentence << VERRATTAIN_PILVINEN_PHRASE;
      break;
    case PILVINEN:
      cloudinessSentence << PILVINEN_WORD;
      break;
    default:
      break;
  }

  if (cloudinessSentence.size() > 0 && !theShortForm)
  {
    if (theCloudinessId == PUOLIPILVINEN_JA_PILVINEN)
    {
      sentence << SAA_WORD;
    }
    else
    {
      sentence << SAA_WORD << ON_WORD;
    }
  }

  sentence << cloudinessSentence;

  return sentence;
}

Sentence cloudiness_sentence(const cloudiness_id& theCloudinessId,
                             const bool& thePoutainenFlag,
                             const Sentence& thePeriodPhrase,
                             const std::string& theAreaString,
                             const bool& theShortForm)
{
  Sentence sentence;
  Sentence cloudinessSentence;
  bool periodPhraseEmpty(thePeriodPhrase.size() == 0);
  bool areaPhraseEmpty(theAreaString.size() == 0 || theAreaString.compare(EMPTY_STRING) == 0);

  if (thePeriodPhrase.size() == 0 && theAreaString.compare(EMPTY_STRING) == 0)
  {
    if (thePoutainenFlag)
    {
      if (theCloudinessId == PUOLIPILVINEN_JA_PILVINEN)
        sentence << PUOLIPILVISESTA_PILVISEEN_JA_POUTAINEN_PHRASE;
      else if (theCloudinessId != SELKEA)
        sentence << SAA_ON_SELKEA_JA_POUTAINEN_COMPOSITE_PHRASE
                 << cloudiness_string(theCloudinessId);
      else
        sentence << cloudiness_sentence(theCloudinessId, theShortForm);
    }
    else
    {
      sentence << cloudiness_sentence(theCloudinessId, theShortForm);
    }

    return sentence;
  }

  cloudinessSentence << cloudiness_sentence(theCloudinessId, true);

  if (theShortForm)
  {
    sentence << cloudinessSentence;
  }
  else
  {
    if (theCloudinessId == PUOLIPILVINEN_JA_PILVINEN)
    {
      if ((periodPhraseEmpty && !areaPhraseEmpty) || (!periodPhraseEmpty && areaPhraseEmpty))
      {
        if (periodPhraseEmpty)
        {
          if (thePoutainenFlag)
            sentence << SISAMAASSA_PUOLIPILVISESTA_PILVISEEN_JA_POUTAINEN_COMPOSITE_PHRASE
                     << theAreaString;
          else
            sentence << SISAMAASSA_PUOLIPILVISESTA_PILVISEEN_COMPOSITE_PHRASE << theAreaString;
        }
        else
        {
          if (thePoutainenFlag)
            sentence << HUOMENNA_PUOLIPILVISESTA_PILVISEEN_JA_POUTAINEN_COMPOSITE_PHRASE
                     << thePeriodPhrase;
          else
            sentence << HUOMENNA_PUOLIPILVISESTA_PILVISEEN_COMPOSITE_PHRASE << thePeriodPhrase;
        }
      }
      else
      {
        if (thePoutainenFlag)
          sentence << HUOMENNA_SISAMAASSA_PUOLIPILVISESTA_PILVISEEN_JA_POUTAINEN_COMPOSITE_PHRASE
                   << thePeriodPhrase << theAreaString;
        else
          sentence << HUOMENNA_SISAMAASSA_PUOLIPILVISESTA_PILVISEEN_COMPOSITE_PHRASE
                   << thePeriodPhrase << theAreaString;
      }
    }
    else
    {
      if ((periodPhraseEmpty && !areaPhraseEmpty) || (!periodPhraseEmpty && areaPhraseEmpty))
      {
        if (periodPhraseEmpty)
        {
          if (thePoutainenFlag && theCloudinessId != SELKEA)

            sentence << SISAMAASSA_SAA_ON_SELKEA_JA_POUTAINEN_COMPOSITE_PHRASE << theAreaString
                     << cloudinessSentence;
          else
            sentence << SISAMAASSA_SAA_ON_SELKEA_COMPOSITE_PHRASE << theAreaString
                     << cloudinessSentence;
        }
        else
        {
          if (thePoutainenFlag && theCloudinessId != SELKEA)
            sentence << HUOMENNA_SAA_ON_SELKEA_JA_POUTAINEN_COMPOSITE_PHRASE << thePeriodPhrase
                     << cloudinessSentence;
          else
            sentence << HUOMENNA_SAA_ON_SELKEA_COMPOSITE_PHRASE << thePeriodPhrase
                     << cloudinessSentence;
        }
      }
      else
      {
        if (thePoutainenFlag && theCloudinessId != SELKEA)
          sentence << HUOMENNA_SISAMAASSA_SAA_ON_SELKEA_JA_POUTAINEN_COMPOSITE_PHRASE
                   << thePeriodPhrase << theAreaString << cloudinessSentence;
        else
          sentence << HUOMENNA_SISAMAASSA_SAA_ON_SELKEA_COMPOSITE_PHRASE << thePeriodPhrase
                   << theAreaString << cloudinessSentence;
      }
    }
  }
  return sentence;
}

const char* cloudiness_string(const cloudiness_id& theCloudinessId)
{
  const char* retval = "";

  switch (theCloudinessId)
  {
    case SELKEA:
      retval = SELKEA_WORD;
      break;
    case MELKO_SELKEA:
      retval = MELKO_SELKEA_PHRASE;
      break;
    case PUOLIPILVINEN:
      retval = PUOLIPILVINEN_WORD;
      break;
    case VERRATTAIN_PILVINEN:
      retval = VERRATTAIN_PILVINEN_PHRASE;
      break;
    case PILVINEN:
      retval = PILVINEN_WORD;
      break;
    case PUOLIPILVINEN_JA_PILVINEN:
      retval = VAIHTELEE_PUOLIPILVISESTA_PILVISEEN_PHRASE;
      break;
    default:
      retval = "missing cloudiness id";
      break;
  }

  return retval;
}

CloudinessForecast::CloudinessForecast(wf_story_params& parameters)
    : theParameters(parameters), theCoastalData(0), theInlandData(0), theFullData(0)

{
  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    theFullData = ((*theParameters.theCompleteData[FULL_AREA])[CLOUDINESS_DATA]);
  if (theParameters.theForecastArea & COASTAL_AREA)
    theCoastalData = ((*theParameters.theCompleteData[COASTAL_AREA])[CLOUDINESS_DATA]);
  if (theParameters.theForecastArea & INLAND_AREA)
    theInlandData = ((*theParameters.theCompleteData[INLAND_AREA])[CLOUDINESS_DATA]);

  findOutCloudinessPeriods();
  joinPeriods();
  findOutCloudinessWeatherEvents();
}

Sentence CloudinessForecast::cloudinessChangeSentence(const WeatherPeriod& thePeriod) const
{
  Sentence sentence;

  const weather_event_id_vector& cloudinessWeatherEvents =
      ((theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
           ? theCloudinessWeatherEventsFull
           : ((theParameters.theForecastArea & INLAND_AREA) ? theCloudinessWeatherEventsInland
                                                            : theCloudinessWeatherEventsCoastal));

  for (unsigned int i = 0; i < cloudinessWeatherEvents.size(); i++)
  {
    TextGenPosixTime weatherEventTimestamp(cloudinessWeatherEvents.at(i).first);

    if (!(weatherEventTimestamp >= thePeriod.localStartTime() &&
          weatherEventTimestamp <= thePeriod.localEndTime()))
    {
      continue;
    }

    weather_event_id trid(cloudinessWeatherEvents.at(i).second);
    std::string timePhrase(get_time_phrase(weatherEventTimestamp, theParameters.theVariable, true));
    if (timePhrase.empty()) timePhrase = EMPTY_STRING;

    sentence << ILTAPAIVASTA_ALKAEN_PILVISTYVAA_COMPOSITE_PHRASE << timePhrase
             << (trid == PILVISTYY ? PILVISTYVAA_WORD : SELKENEVAA_WORD);
  }

  return sentence;
}

float CloudinessForecast::getMeanCloudiness(
    const WeatherPeriod& theWeatherPeriod,
    const weather_result_data_item_vector& theDataVector) const
{
  float cloudinessSum(0.0);
  unsigned int count = 0;
  for (unsigned int i = 0; i < theDataVector.size(); i++)
  {
    if (theDataVector[i]->thePeriod.localStartTime() >= theWeatherPeriod.localStartTime() &&
        theDataVector[i]->thePeriod.localStartTime() <= theWeatherPeriod.localEndTime() &&
        theDataVector[i]->thePeriod.localEndTime() >= theWeatherPeriod.localStartTime() &&
        theDataVector[i]->thePeriod.localEndTime() <= theWeatherPeriod.localEndTime())
    {
      cloudinessSum += theDataVector[i]->theResult.value();
      count++;
    }
  }
  return (count == 0 ? 0.0 : (cloudinessSum / count));
}

bool CloudinessForecast::separateCoastInlandCloudiness(const WeatherPeriod& theWeatherPeriod) const
{
  if (theParameters.theCoastalAndInlandTogetherFlag == true) return false;

  weather_result_data_item_vector theInterestingDataCoastal;
  weather_result_data_item_vector theInterestingDataInland;

  get_sub_time_series(theWeatherPeriod, *theCoastalData, theInterestingDataCoastal);

  float mean_coastal = get_mean(theInterestingDataCoastal);

  get_sub_time_series(theWeatherPeriod, *theInlandData, theInterestingDataInland);

  float mean_inland = get_mean(theInterestingDataInland);

  bool retval = ((mean_inland <= 5.0 && mean_coastal >= 70.0) ||
                 (mean_inland >= 70.0 && mean_coastal <= 5.0));

  return retval;
}

bool CloudinessForecast::separateWeatherPeriodCloudiness(
    const WeatherPeriod& theWeatherPeriod1,
    const WeatherPeriod& theWeatherPeriod2,
    const weather_result_data_item_vector& theCloudinessData) const
{
  weather_result_data_item_vector theInterestingDataPeriod1;
  weather_result_data_item_vector theInterestingDataPeriod2;

  get_sub_time_series(theWeatherPeriod1, theCloudinessData, theInterestingDataPeriod1);

  cloudiness_id cloudinessId1 = getCloudinessId(get_mean(theInterestingDataPeriod1));

  get_sub_time_series(theWeatherPeriod2, theCloudinessData, theInterestingDataPeriod2);

  cloudiness_id cloudinessId2 = getCloudinessId(get_mean(theInterestingDataPeriod2));

  // difference must be at least two grades
  return (abs(cloudinessId1 - cloudinessId2) >= 2);
}

void CloudinessForecast::findOutCloudinessWeatherEvents(
    const weather_result_data_item_vector* theData,
    weather_event_id_vector& theCloudinessWeatherEvents)
{
  for (unsigned int i = 3; i < theData->size() - 3; i++)
  {
    WeatherPeriod firstHalfPeriod(theData->at(0)->thePeriod.localStartTime(),
                                  theData->at(i)->thePeriod.localEndTime());
    WeatherPeriod secondHalfPeriod(theData->at(i + 1)->thePeriod.localStartTime(),
                                   theData->at(theData->size() - 1)->thePeriod.localEndTime());

    weather_result_data_item_vector theFirstHalfData;
    weather_result_data_item_vector theSecondHalfData;

    get_sub_time_series(firstHalfPeriod, *theData, theFirstHalfData);

    get_sub_time_series(secondHalfPeriod, *theData, theSecondHalfData);

    float meanCloudinessInTheFirstHalf = get_mean(theFirstHalfData);
    float meanCloudinessInTheSecondHalf = get_mean(theSecondHalfData);
    weather_event_id weatherEventId(MISSING_WEATHER_EVENT);
    unsigned int changeIndex(0);

    if (meanCloudinessInTheFirstHalf >= PILVISTYVAA_UPPER_LIMIT &&
        meanCloudinessInTheSecondHalf <= PILVISTYVAA_LOWER_LIMIT)
    {
      bool selkeneeReally = true;
      // check that cloudiness stays under the limit for the rest of the forecast period
      for (unsigned int k = i + 1; k < theData->size(); k++)
        if (theData->at(k)->theResult.value() > PILVISTYVAA_LOWER_LIMIT)
        {
          selkeneeReally = false;
          break;
        }
      if (!selkeneeReally) continue;

      changeIndex = i + 1;
      while (changeIndex >= 1 &&
             theData->at(changeIndex - 1)->theResult.value() <= PILVISTYVAA_LOWER_LIMIT)
        changeIndex--;

      weatherEventId = SELKENEE;
    }
    else if (meanCloudinessInTheFirstHalf <= PILVISTYVAA_LOWER_LIMIT &&
             meanCloudinessInTheSecondHalf >= PILVISTYVAA_UPPER_LIMIT)
    {
      bool pilvistyyReally = true;
      // check that cloudiness stays above the limit for the rest of the forecast period
      for (unsigned int k = i + 1; k < theData->size(); k++)
        if (theData->at(k)->theResult.value() < PILVISTYVAA_UPPER_LIMIT)
        {
          pilvistyyReally = false;
          break;
        }
      if (!pilvistyyReally) continue;

      changeIndex = i + 1;
      while (changeIndex >= 1 &&
             theData->at(changeIndex - 1)->theResult.value() >= PILVISTYVAA_UPPER_LIMIT)
        changeIndex--;

      weatherEventId = PILVISTYY;
    }

    if (weatherEventId != MISSING_WEATHER_EVENT)
    {
      theCloudinessWeatherEvents.push_back(
          make_pair(theData->at(changeIndex)->thePeriod.localStartTime(), weatherEventId));
      // Note: only one event (pilvistyy/selkenee) during the period.
      // The Original plan was that several events are allowed
      break;
    }
  }
}

void CloudinessForecast::findOutCloudinessWeatherEvents()
{
  if (theCoastalData)
    findOutCloudinessWeatherEvents(theCoastalData, theCloudinessWeatherEventsCoastal);
  if (theInlandData)
    findOutCloudinessWeatherEvents(theInlandData, theCloudinessWeatherEventsInland);
  if (theFullData) findOutCloudinessWeatherEvents(theFullData, theCloudinessWeatherEventsFull);
}

void CloudinessForecast::findOutCloudinessPeriods(const weather_result_data_item_vector* theData,
                                                  cloudiness_period_vector& theCloudinessPeriods)
{
  if (theData)
  {
    TextGenPosixTime previousStartTime;
    TextGenPosixTime previousEndTime;
    cloudiness_id previousCloudinessId(MISSING_CLOUDINESS_ID);
    for (unsigned int i = 0; i < theData->size(); i++)
    {
      if (i == 0)
      {
        previousStartTime = theData->at(i)->thePeriod.localStartTime();
        previousEndTime = theData->at(i)->thePeriod.localEndTime();
        previousCloudinessId = getCloudinessId(theData->at(i)->theResult.value());
      }
      else
      {
        if (previousCloudinessId != getCloudinessId(theData->at(i)->theResult.value()))
        {
          pair<WeatherPeriod, cloudiness_id> item =
              make_pair(WeatherPeriod(previousStartTime, previousEndTime), previousCloudinessId);
          theCloudinessPeriods.push_back(item);
          previousStartTime = theData->at(i)->thePeriod.localStartTime();
          previousEndTime = theData->at(i)->thePeriod.localEndTime();
          previousCloudinessId = getCloudinessId(theData->at(i)->theResult.value());
        }
        else if (i == theData->size() - 1)
        {
          pair<WeatherPeriod, cloudiness_id> item =
              make_pair(WeatherPeriod(previousStartTime, theData->at(i)->thePeriod.localEndTime()),
                        previousCloudinessId);
          theCloudinessPeriods.push_back(item);
        }
        else
        {
          previousEndTime = theData->at(i)->thePeriod.localEndTime();
        }
      }
    }
  }
}

void CloudinessForecast::findOutCloudinessPeriods()
{
  findOutCloudinessPeriods(theCoastalData, theCloudinessPeriodsCoastal);
  findOutCloudinessPeriods(theInlandData, theCloudinessPeriodsInland);
  findOutCloudinessPeriods(theFullData, theCloudinessPeriodsFull);
}

void CloudinessForecast::joinPuolipilvisestaPilviseen(
    const weather_result_data_item_vector* theData,
    vector<int>& theCloudinessPuolipilvisestaPilviseen) const
{
  if (!theData) return;

  int index = -1;
  for (unsigned int i = 0; i < theData->size(); i++)
  {
    if (getCloudinessId(theData->at(i)->theResult.value()) >= PUOLIPILVINEN &&
        getCloudinessId(theData->at(i)->theResult.value()) <= PILVINEN && i != theData->size() - 1)
    {
      if (index == -1) index = i;
    }
    else if (getCloudinessId(theData->at(i)->theResult.value()) == SELKEA ||
             getCloudinessId(theData->at(i)->theResult.value()) == MELKO_SELKEA ||
             getCloudinessId(theData->at(i)->theResult.value()) == PILVINEN ||
             i == theData->size() - 1)
    {
      if (index != -1 && i - 1 - index > 1)
      {
        for (int k = index; k < static_cast<int>(i); k++)
        {
          WeatherPeriod period(theData->at(k)->thePeriod.localStartTime(),
                               theData->at(k)->thePeriod.localEndTime());

          theCloudinessPuolipilvisestaPilviseen.push_back(k);
        }
      }
      index = -1;
    }
  }
}

void CloudinessForecast::joinPeriods(const weather_result_data_item_vector* theCloudinessDataSource,
                                     const cloudiness_period_vector& theCloudinessPeriodsSource,
                                     cloudiness_period_vector& theCloudinessPeriodsDestination)
{
  if (theCloudinessPeriodsSource.empty()) return;

  vector<int> theCloudinessPuolipilvisestaPilviseen;

  int periodStartIndex = 0;
  cloudiness_id clidPrevious(theCloudinessPeriodsSource.at(0).second);

  for (unsigned int i = 1; i < theCloudinessPeriodsSource.size(); i++)
  {
    bool lastPeriod = (i == theCloudinessPeriodsSource.size() - 1);
    cloudiness_id clidCurrent = theCloudinessPeriodsSource.at(i).second;

    if (abs(clidPrevious - clidCurrent) >= 2 || lastPeriod)
    {
      // check if "puolipilvisesta pilviseen"
      /*
      if((clidPrevious == PUOLIPILVINEN && clidCurrent == PILVINEN) ||
         (clidPrevious == PILVINEN && clidCurrent == PUOLIPILVINEN))
        {
        }
      */

      TextGenPosixTime startTime(
          theCloudinessPeriodsSource.at(periodStartIndex).first.localStartTime());
      TextGenPosixTime endTime(
          theCloudinessPeriodsSource.at(lastPeriod ? i : i - 1).first.localEndTime());

      weather_result_data_item_vector thePeriodCloudiness;

      get_sub_time_series(
          WeatherPeriod(startTime, endTime), *theCloudinessDataSource, thePeriodCloudiness);

      float min, max, mean, stddev;

      get_min_max(thePeriodCloudiness, min, max);
      mean = get_mean(thePeriodCloudiness);
      stddev = get_standard_deviation(thePeriodCloudiness);

      cloudiness_id actual_clid = getCloudinessId(min, mean, max, stddev);

      pair<WeatherPeriod, cloudiness_id> item =
          make_pair(WeatherPeriod(startTime, endTime), actual_clid);

      theCloudinessPeriodsDestination.push_back(item);

      clidPrevious = clidCurrent;
      periodStartIndex = i;
    }
  }
}

void CloudinessForecast::joinPeriods()
{
  joinPeriods(theCoastalData, theCloudinessPeriodsCoastal, theCloudinessPeriodsCoastalJoined);
  joinPeriods(theInlandData, theCloudinessPeriodsInland, theCloudinessPeriodsInlandJoined);
  joinPeriods(theFullData, theCloudinessPeriodsFull, theCloudinessPeriodsFullJoined);
}

void CloudinessForecast::printOutCloudinessData(std::ostream& theOutput) const
{
  theOutput << "** CLOUDINESS DATA **" << endl;
  if (theCoastalData)
  {
    theOutput << "Coastal cloudiness: " << endl;
    printOutCloudinessData(theOutput, theCoastalData);
  }
  if (theInlandData)
  {
    theOutput << "Inland cloudiness: " << endl;
    printOutCloudinessData(theOutput, theInlandData);
  }
  if (theFullData)
  {
    theOutput << "Full area cloudiness: " << endl;
    printOutCloudinessData(theOutput, theFullData);
  }
}

void CloudinessForecast::printOutCloudinessData(
    std::ostream& theOutput, const weather_result_data_item_vector* theDataVector) const
{
  for (unsigned int i = 0; i < theDataVector->size(); i++)
  {
    theOutput << theDataVector->at(i)->thePeriod.localStartTime() << "..."
              << theDataVector->at(i)->thePeriod.localEndTime() << ": "
              << theDataVector->at(i)->theResult.value() << endl;
  }
}

void CloudinessForecast::printOutCloudinessPeriods(
    std::ostream& theOutput, const cloudiness_period_vector& theCloudinessPeriods) const
{
  for (unsigned int i = 0; i < theCloudinessPeriods.size(); i++)
  {
    WeatherPeriod period(theCloudinessPeriods.at(i).first.localStartTime(),
                         theCloudinessPeriods.at(i).first.localEndTime());
    cloudiness_id clid(theCloudinessPeriods.at(i).second);
    theOutput << period.localStartTime() << "..." << period.localEndTime() << ": "
              << cloudiness_string(clid) << endl;
  }
}

void CloudinessForecast::printOutCloudinessWeatherEvents(std::ostream& theOutput) const
{
  theOutput << "** CLOUDINESS WEATHER EVENTS **" << endl;
  bool isWeatherEvents = false;
  if (!theCloudinessWeatherEventsCoastal.empty())
  {
    theOutput << "Coastal weather events: " << endl;
    print_out_weather_event_vector(theOutput, theCloudinessWeatherEventsCoastal);
    isWeatherEvents = true;
  }
  if (!theCloudinessWeatherEventsInland.empty())
  {
    theOutput << "Inland weather events: " << endl;
    print_out_weather_event_vector(theOutput, theCloudinessWeatherEventsInland);
    isWeatherEvents = true;
  }
  if (!theCloudinessWeatherEventsFull.empty())
  {
    theOutput << "Full area weather events: " << endl;
    print_out_weather_event_vector(theOutput, theCloudinessWeatherEventsFull);
    isWeatherEvents = true;
  }

  if (!isWeatherEvents) theOutput << "No weather events!" << endl;
}

void CloudinessForecast::printOutCloudinessPeriods(std::ostream& theOutput) const
{
  theOutput << "** CLOUDINESS PERIODS **" << endl;

  if (theCoastalData)
  {
    theOutput << "Coastal cloudiness: " << endl;
    printOutCloudinessPeriods(theOutput, theCloudinessPeriodsCoastal);
    theOutput << "Coastal cloudiness joined: " << endl;
    printOutCloudinessPeriods(theOutput, theCloudinessPeriodsCoastalJoined);
    vector<int> theCloudinessPuolipilvisestaPilviseen;
    joinPuolipilvisestaPilviseen(theCoastalData, theCloudinessPuolipilvisestaPilviseen);
  }
  if (theInlandData)
  {
    theOutput << "Inland cloudiness: " << endl;
    printOutCloudinessPeriods(theOutput, theCloudinessPeriodsInland);
    theOutput << "Inland cloudiness joined: " << endl;
    printOutCloudinessPeriods(theOutput, theCloudinessPeriodsInlandJoined);
    vector<int> theCloudinessPuolipilvisestaPilviseen;
    joinPuolipilvisestaPilviseen(theInlandData, theCloudinessPuolipilvisestaPilviseen);
  }
  if (theFullData)
  {
    theOutput << "Full area cloudiness: " << endl;
    printOutCloudinessPeriods(theOutput, theCloudinessPeriodsFull);
    theOutput << "Full area cloudiness joined: " << endl;
    printOutCloudinessPeriods(theOutput, theCloudinessPeriodsFullJoined);
    vector<int> theCloudinessPuolipilvisestaPilviseen;
    joinPuolipilvisestaPilviseen(theFullData, theCloudinessPuolipilvisestaPilviseen);
  }
}

void CloudinessForecast::getWeatherPeriodCloudiness(
    const WeatherPeriod& thePeriod,
    const cloudiness_period_vector& theSourceCloudinessPeriods,
    cloudiness_period_vector& theWeatherPeriodCloudiness) const
{
  for (unsigned int i = 0; i < theSourceCloudinessPeriods.size(); i++)
  {
    if (thePeriod.localStartTime() >= theSourceCloudinessPeriods.at(i).first.localStartTime() &&
        thePeriod.localEndTime() <= theSourceCloudinessPeriods.at(i).first.localEndTime())
    {
      TextGenPosixTime startTime(thePeriod.localStartTime());
      TextGenPosixTime endTime(thePeriod.localEndTime());
      cloudiness_id clid(theSourceCloudinessPeriods.at(i).second);
      pair<WeatherPeriod, cloudiness_id> item = make_pair(WeatherPeriod(startTime, endTime), clid);
      theWeatherPeriodCloudiness.push_back(item);
    }
    else if (thePeriod.localStartTime() >=
                 theSourceCloudinessPeriods.at(i).first.localStartTime() &&
             thePeriod.localStartTime() < theSourceCloudinessPeriods.at(i).first.localEndTime() &&
             thePeriod.localEndTime() > theSourceCloudinessPeriods.at(i).first.localEndTime())
    {
      TextGenPosixTime startTime(thePeriod.localStartTime());
      TextGenPosixTime endTime(theSourceCloudinessPeriods.at(i).first.localEndTime());
      cloudiness_id clid(theSourceCloudinessPeriods.at(i).second);
      pair<WeatherPeriod, cloudiness_id> item = make_pair(WeatherPeriod(startTime, endTime), clid);
      theWeatherPeriodCloudiness.push_back(item);
    }
    else if (thePeriod.localStartTime() < theSourceCloudinessPeriods.at(i).first.localStartTime() &&
             thePeriod.localEndTime() > theSourceCloudinessPeriods.at(i).first.localStartTime() &&
             thePeriod.localEndTime() <= theSourceCloudinessPeriods.at(i).first.localEndTime())
    {
      TextGenPosixTime startTime(theSourceCloudinessPeriods.at(i).first.localStartTime());
      TextGenPosixTime endTime(thePeriod.localEndTime());
      cloudiness_id clid(theSourceCloudinessPeriods.at(i).second);
      pair<WeatherPeriod, cloudiness_id> item = make_pair(WeatherPeriod(startTime, endTime), clid);
      theWeatherPeriodCloudiness.push_back(item);
    }
    else if (thePeriod.localStartTime() < theSourceCloudinessPeriods.at(i).first.localStartTime() &&
             thePeriod.localEndTime() > theSourceCloudinessPeriods.at(i).first.localEndTime())
    {
      TextGenPosixTime startTime(theSourceCloudinessPeriods.at(i).first.localStartTime());
      TextGenPosixTime endTime(theSourceCloudinessPeriods.at(i).first.localEndTime());
      cloudiness_id clid(theSourceCloudinessPeriods.at(i).second);
      pair<WeatherPeriod, cloudiness_id> item = make_pair(WeatherPeriod(startTime, endTime), clid);
      theWeatherPeriodCloudiness.push_back(item);
    }
  }
}

Sentence CloudinessForecast::cloudinessSentence(const WeatherPeriod& thePeriod,
                                                const bool& theShortForm) const
{
  Sentence sentence;

  Sentence cloudinessSentence;

  cloudiness_id coastalCloudinessId = getCloudinessId(thePeriod, theCoastalData);
  cloudiness_id inlandCloudinessId = getCloudinessId(thePeriod, theInlandData);
  cloudiness_id fullAreaCloudinessId = getCloudinessId(thePeriod, theFullData);

  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
  {
    if (separateCoastInlandCloudiness(thePeriod))
    {
      cloudinessSentence << COAST_PHRASE;
      cloudinessSentence << cloudiness_sentence(coastalCloudinessId, theShortForm);
      cloudinessSentence << Delimiter(COMMA_PUNCTUATION_MARK);
      cloudinessSentence << INLAND_PHRASE;
      cloudinessSentence << cloudiness_sentence(inlandCloudinessId, theShortForm);
    }
    else
    {
      cloudinessSentence << cloudiness_sentence(fullAreaCloudinessId, theShortForm);
    }
  }
  else if (theParameters.theForecastArea & INLAND_AREA)
  {
    cloudinessSentence << cloudiness_sentence(inlandCloudinessId, theShortForm);
  }
  else if (theParameters.theForecastArea & COASTAL_AREA)
  {
    cloudinessSentence << cloudiness_sentence(coastalCloudinessId, theShortForm);
  }

  if (cloudinessSentence.size() > 0)
  {
    sentence << cloudinessSentence;
  }

  /*
  if(sentence.size() > 0 && !(theParameters.theForecastArea & FULL_AREA))
    {
          cloudiness_id clid = (theParameters.theForecastArea & INLAND_AREA ? inlandCloudinessId :
  coastalCloudinessId);
          if(clid == VERRATTAIN_PILVINEN || clid == PILVINEN)
            sentence << areaSpecificSentence(thePeriod);
    }
  */

  return sentence;
}

Sentence CloudinessForecast::cloudinessSentence(const WeatherPeriod& thePeriod,
                                                const bool& thePoutainenFlag,
                                                const Sentence& thePeriodPhrase,
                                                const bool& theShortForm) const
{
  Sentence sentence;

  Sentence cloudinessSentence;

  cloudiness_id coastalCloudinessId = getCloudinessId(thePeriod, theCoastalData);
  cloudiness_id inlandCloudinessId = getCloudinessId(thePeriod, theInlandData);
  cloudiness_id fullAreaCloudinessId = getCloudinessId(thePeriod, theFullData);

  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
  {
    if (separateCoastInlandCloudiness(thePeriod))
    {
      cloudinessSentence << COAST_PHRASE;
      cloudinessSentence << cloudiness_sentence(
          coastalCloudinessId, thePoutainenFlag, thePeriodPhrase, COAST_PHRASE, theShortForm);
      cloudinessSentence << Delimiter(COMMA_PUNCTUATION_MARK);
      cloudinessSentence << INLAND_PHRASE;
      cloudinessSentence << cloudiness_sentence(
          inlandCloudinessId, thePoutainenFlag, thePeriodPhrase, INLAND_PHRASE, theShortForm);
    }
    else
    {
      cloudinessSentence << cloudiness_sentence(
          fullAreaCloudinessId, thePoutainenFlag, thePeriodPhrase, EMPTY_STRING, theShortForm);
    }
  }
  else if (theParameters.theForecastArea & INLAND_AREA)
  {
    cloudinessSentence << cloudiness_sentence(
        inlandCloudinessId, thePoutainenFlag, thePeriodPhrase, EMPTY_STRING, theShortForm);
  }
  else if (theParameters.theForecastArea & COASTAL_AREA)
  {
    cloudinessSentence << cloudiness_sentence(
        coastalCloudinessId, thePoutainenFlag, thePeriodPhrase, EMPTY_STRING, theShortForm);
  }

  if (cloudinessSentence.size() > 0)
  {
    sentence << cloudinessSentence;
  }

  /*
  if(sentence.size() > 0 && !(theParameters.theForecastArea & FULL_AREA))
    {
          cloudiness_id clid = (theParameters.theForecastArea & INLAND_AREA ? inlandCloudinessId :
  coastalCloudinessId);
          if(clid == VERRATTAIN_PILVINEN || clid == PILVINEN)
            sentence << areaSpecificSentence(thePeriod);
    }
  */

  return sentence;
}

cloudiness_id CloudinessForecast::getCloudinessPeriodId(
    const TextGenPosixTime& theObservationTime,
    const cloudiness_period_vector& theCloudinessPeriodVector) const
{
  for (unsigned int i = 0; i < theCloudinessPeriodVector.size(); i++)
  {
    if (theObservationTime >= theCloudinessPeriodVector.at(i).first.localStartTime() &&
        theObservationTime <= theCloudinessPeriodVector.at(i).first.localEndTime())
      return theCloudinessPeriodVector.at(i).second;
  }

  return MISSING_CLOUDINESS_ID;
}

cloudiness_id CloudinessForecast::getCloudinessId(const WeatherPeriod& thePeriod) const
{
  const weather_result_data_item_vector* theCloudinessDataVector = 0;

  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    theCloudinessDataVector = theFullData;
  else if (theParameters.theForecastArea & INLAND_AREA)
    theCloudinessDataVector = theInlandData;
  else if (theParameters.theForecastArea & COASTAL_AREA)
    theCloudinessDataVector = theCoastalData;

  return getCloudinessId(thePeriod, theCloudinessDataVector);
}

cloudiness_id CloudinessForecast::getCloudinessId(
    const WeatherPeriod& thePeriod, const weather_result_data_item_vector* theCloudinessData) const
{
  if (!theCloudinessData) return MISSING_CLOUDINESS_ID;

  weather_result_data_item_vector thePeriodCloudiness;

  get_sub_time_series(thePeriod, *theCloudinessData, thePeriodCloudiness);

  float min, max, mean, stddev;
  get_min_max(thePeriodCloudiness, min, max);
  mean = get_mean(thePeriodCloudiness);
  stddev = get_standard_deviation(thePeriodCloudiness);
  cloudiness_id clid = getCloudinessId(min, mean, max, stddev);

  return clid;
}

void CloudinessForecast::getWeatherEventIdVector(
    weather_event_id_vector& theCloudinessWeatherEvents) const
{
  const weather_event_id_vector* vectorToClone = 0;

  if (theParameters.theForecastArea & INLAND_AREA && theParameters.theForecastArea & COASTAL_AREA)
    vectorToClone = &theCloudinessWeatherEventsFull;
  else if (theParameters.theForecastArea & COASTAL_AREA)
    vectorToClone = &theCloudinessWeatherEventsCoastal;
  else if (theParameters.theForecastArea & INLAND_AREA)
    vectorToClone = &theCloudinessWeatherEventsInland;

  if (vectorToClone) theCloudinessWeatherEvents = *vectorToClone;
}

Sentence CloudinessForecast::areaSpecificSentence(const WeatherPeriod& thePeriod) const
{
  Sentence sentence;

  // TODO: tsekkaa, etta aluuen toisella puolella on selkeaa
  /*
  WeatherResult northEastShare(kFloatMissing, 0);
  WeatherResult southEastShare(kFloatMissing, 0);
  WeatherResult southWestShare(kFloatMissing, 0);
  WeatherResult northWestShare(kFloatMissing, 0);

  RangeAcceptor cloudinesslimits;
  cloudinesslimits.lowerLimit(VERRATTAIN_PILVISTA_LOWER_LIMIT);
  AreaTools::getArealDistribution(theParameters.theSources,
                                                                  Cloudiness,
                                                                  theParameters.theArea,
                                                                  thePeriod,
                                                                  cloudinesslimits,
                                                                  northEastShare,
                                                                  southEastShare,
                                                                  southWestShare,
                                                                  northWestShare);

  float north = northEastShare.value() + northWestShare.value();
  float south = southEastShare.value() + southWestShare.value();
  float east = northEastShare.value() + southEastShare.value();
  float west = northWestShare.value() + southWestShare.value();

  sentence << area_specific_sentence(north,
                                                                     south,
                                                                     east,
                                                                     west,
                                                                     northEastShare.value(),
                                                                     southEastShare.value(),
                                                                     southWestShare.value(),
                                                                     northWestShare.value(),
                                                                     false);

  */
  return sentence;
}

cloudiness_id CloudinessForecast::getCloudinessId(const float& theMin,
                                                  const float& theMean,
                                                  const float& theMax,
                                                  const float& theStandardDeviation) const
{
  cloudiness_id id(MISSING_CLOUDINESS_ID);

  if (theMean == -1) return id;

  if (theMin > theParameters.theAlmostClearSkyUpperLimit &&
      theMin <= theParameters.thePartlyCloudySkyUpperLimit &&
      theMax > theParameters.theMostlyCloudySkyUpperLimit)
  {
    id = PUOLIPILVINEN_JA_PILVINEN;
  }
  else if (theMean <= theParameters.theClearSkyUpperLimit)
  {
    id = SELKEA;
  }
  else if (theMean <= theParameters.theAlmostClearSkyUpperLimit)
  {
    id = MELKO_SELKEA;
  }
  else if (theMean <= theParameters.thePartlyCloudySkyUpperLimit)
  {
    id = PUOLIPILVINEN;
  }
  else if (theMean <= theParameters.theMostlyCloudySkyUpperLimit)
  {
    id = VERRATTAIN_PILVINEN;
  }
  else
  {
    id = PILVINEN;
  }

  return id;
}

cloudiness_id CloudinessForecast::getCloudinessId(const float& theCloudiness) const
{
  cloudiness_id id(MISSING_CLOUDINESS_ID);

  if (theCloudiness < 0) return id;

  if (theCloudiness <= theParameters.theClearSkyUpperLimit)
  {
    id = SELKEA;
  }
  else if (theCloudiness <= theParameters.theAlmostClearSkyUpperLimit)
  {
    id = MELKO_SELKEA;
  }
  else if (theCloudiness <= theParameters.thePartlyCloudySkyUpperLimit)
  {
    id = PUOLIPILVINEN;
  }
  else if (theCloudiness <= theParameters.theMostlyCloudySkyUpperLimit)
  {
    id = VERRATTAIN_PILVINEN;
  }
  else
  {
    id = PILVINEN;
  }

  return id;
}
}
