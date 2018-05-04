#include <iostream>
#include <string>

#include <calculator/WeatherPeriod.h>
#include <calculator/WeatherArea.h>
#include "MessageLogger.h"
#include "Sentence.h"
#include "Paragraph.h"
#include "Delimiter.h"
#include <calculator/Settings.h>
#include "PeriodPhraseFactory.h"

#include "PrecipitationForecast.h"
#include "CloudinessForecast.h"
#include "FogForecast.h"
#include "ThunderForecast.h"
#include "WeatherForecastStory.h"

#define USE_FROM_SPECIFIER true
#define DONT_USE_FROM_SPECIFIER false
#define USE_SHORT_FORM true
#define DONT_USE_SHORT_FORM false
#define USE_TIME_SPECIFIER true
#define DONT_USE_TIME_SPECIFIER false

namespace TextGen
{
using namespace boost;
using namespace std;
using namespace TextGen;
using namespace Settings;

std::ostream& operator<<(std::ostream& theOutput, const WeatherForecastStoryItem& theStoryItem)
{
  theOutput << theStoryItem.getPeriod().localStartTime() << "..."
            << theStoryItem.getPeriod().localEndTime() << " ";
  theOutput << story_part_id_string(theStoryItem.getStoryPartId()) << " - ";
  theOutput << (theStoryItem.isIncluded() ? "included" : "exluded") << endl;

  return theOutput;
}

WeatherForecastStory::WeatherForecastStory(const std::string& var,
                                           const WeatherPeriod& forecastPeriod,
                                           const WeatherArea& weatherArea,
                                           const unsigned short& forecastArea,
                                           const TextGenPosixTime& theForecastTime,
                                           PrecipitationForecast& precipitationForecast,
                                           const CloudinessForecast& cloudinessForecast,
                                           const FogForecast& fogForecast,
                                           const ThunderForecast& thunderForecast,
                                           MessageLogger& logger)
    : theVar(var),
      theForecastPeriod(forecastPeriod),
      theWeatherArea(weatherArea),
      theForecastArea(forecastArea),
      theForecastTime(theForecastTime),
      thePrecipitationForecast(precipitationForecast),
      theCloudinessForecast(cloudinessForecast),
      theFogForecast(fogForecast),
      theThunderForecast(thunderForecast),
      theLogger(logger),
      theStorySize(0),
      theShortTimePrecipitationReportedFlag(false),
      theReportTimePhraseFlag(false),
      theCloudinessReportedFlag(false)
{
  addPrecipitationStoryItems();
  addCloudinessStoryItems();
  mergePeriodsWhenFeasible();

  bool specifyPartOfTheDayFlag =
      Settings::optional_bool(theVar + "::specify_part_of_the_day", true);
  int storyItemCounter(0);
  bool moreThanOnePrecipitationForms(false);
  for (unsigned int i = 0; i < theStoryItemVector.size(); i++)
  {
    // when specifyPartOfTheDayFlag is false it means
    // that the period is short and part of the day should not be expressed
    if (!specifyPartOfTheDayFlag)
    {
      // ARE 14.4.2011: checking theIncludeInTheStoryFlag
      if (theStoryItemVector[i]->theIncludeInTheStoryFlag == true &&
          theStoryItemVector[i]->getSentence().size() > 0)
        storyItemCounter++;
    }

    // check wheather more than one precipitation form exists during the forecast period
    if (!moreThanOnePrecipitationForms &&
        theStoryItemVector[i]->theStoryPartId == PRECIPITATION_STORY_PART &&
        theStoryItemVector[i]->theIncludeInTheStoryFlag == true)
    {
      precipitation_form_id precipitationForm = thePrecipitationForecast.getPrecipitationForm(
          theStoryItemVector[i]->thePeriod, theForecastArea);

      moreThanOnePrecipitationForms = !PrecipitationForecast::singleForm(precipitationForm);
    }
  }
  // if more than one item exists, use the phrases "aluksi", "myöhemmin" when the period is short
  theReportTimePhraseFlag = storyItemCounter > 1;
  thePrecipitationForecast.setSinglePrecipitationFormFlag(!moreThanOnePrecipitationForms);
}

WeatherForecastStory::~WeatherForecastStory() { theStoryItemVector.clear(); }
Paragraph WeatherForecastStory::getWeatherForecastStory()
{
  Paragraph paragraph;

  theShortTimePrecipitationReportedFlag = false;
  theCloudinessReportedFlag = false;
  theStorySize = 0;

  thePrecipitationForecast.setDryPeriodTautologyFlag(false);

  for (unsigned int i = 0; i < theStoryItemVector.size(); i++)
  {
    Sentence storyItemSentence;
    storyItemSentence << theStoryItemVector[i]->getSentence();
    if (storyItemSentence.size() > 0)
    {
      theStorySize += storyItemSentence.size();

      paragraph << storyItemSentence;

      // additional sentences: currently only in precipitation story: like "iltapäivästä alkaen sade
      // voi olla runsasta"
      for (unsigned int k = 0; k < theStoryItemVector[i]->numberOfAdditionalSentences(); k++)
        paragraph << theStoryItemVector[i]->getAdditionalSentence(k);

      // possible fog sentence after
      if (theStoryItemVector[i]->theStoryPartId == CLOUDINESS_STORY_PART)
      {
        paragraph << theFogForecast.fogSentence(theStoryItemVector[i]->thePeriod);
      }
    }
    // reset the today phrase when day changes
    if (i > 0 &&
        theStoryItemVector[i - 1]->thePeriod.localEndTime().GetJulianDay() !=
            theStoryItemVector[i]->thePeriod.localEndTime().GetJulianDay() &&
        get_period_length(theForecastPeriod) > 24)
      const_cast<WeatherHistory&>(theWeatherArea.history())
          .updateTimePhrase("", "", TextGenPosixTime(1970, 1, 1));
  }

  // ARE 19.04.2012: yletv for kaakkois-suomi, empty story was created
  // if story is empty take the whole forecast period and tell story of it
  if (theStorySize == 0)
  {
    Sentence periodPhrase;
    std::vector<Sentence> theAdditionalSentences;
    paragraph << thePrecipitationForecast.precipitationSentence(
        theForecastPeriod, periodPhrase, theAdditionalSentences);
    for (unsigned int i = 0; i < theAdditionalSentences.size(); i++)
      paragraph << theAdditionalSentences[i];
  }

  return paragraph;
}

void WeatherForecastStory::addPrecipitationStoryItems()
{
  vector<WeatherPeriod> precipitationPeriods;

  thePrecipitationForecast.getPrecipitationPeriods(theForecastPeriod, precipitationPeriods);

  PrecipitationForecastStoryItem* previousPrItem = 0;
  WeatherForecastStoryItem* missingStoryItem = 0;
  for (unsigned int i = 0; i < precipitationPeriods.size(); i++)
  {
    float intensity(
        thePrecipitationForecast.getMeanIntensity(precipitationPeriods[i], theForecastArea));
    float extent(
        thePrecipitationForecast.getPrecipitationExtent(precipitationPeriods[i], theForecastArea));
    unsigned int form(
        thePrecipitationForecast.getPrecipitationForm(precipitationPeriods[i], theForecastArea));
    precipitation_type type(
        thePrecipitationForecast.getPrecipitationType(precipitationPeriods[i], theForecastArea));

    bool thunder(
        thePrecipitationForecast.thunderExists(precipitationPeriods[i], theForecastArea, theVar));

    if (get_period_length(precipitationPeriods[i]) <= 1 && extent < 10) continue;

    PrecipitationForecastStoryItem* item =
        new PrecipitationForecastStoryItem(*this,
                                           precipitationPeriods[i],
                                           PRECIPITATION_STORY_PART,
                                           intensity,
                                           extent,
                                           form,
                                           type,
                                           thunder);

    TextGenPosixTime startTimeFull;
    TextGenPosixTime endTimeFull;
    thePrecipitationForecast.getPrecipitationPeriod(
        precipitationPeriods[i].localStartTime(), startTimeFull, endTimeFull);
    // This parameter shows if the precipitation starts
    // before forecast period or continues after
    int precipitationFullDuration = endTimeFull.DifferenceInHours(startTimeFull);
    item->theFullDuration = precipitationFullDuration;

    if (previousPrItem != 0)
    {
      TextGenPosixTime startTime(previousPrItem->thePeriod.localEndTime());
      TextGenPosixTime endTime(precipitationPeriods[i].localStartTime());
      if (endTime.DifferenceInHours(startTime) > 1)
      {
        startTime.ChangeByHours(1);
        endTime.ChangeByHours(-1);

        // placeholder for some other story item between precipitation periods
        missingStoryItem = new WeatherForecastStoryItem(
            *this, WeatherPeriod(startTime, endTime), MISSING_STORY_PART);

        theStoryItemVector.push_back(missingStoryItem);
      }
    }
    // precipitation story item
    theStoryItemVector.push_back(item);

    previousPrItem = item;
  }

  if (theStoryItemVector.size() == 0)
  {
    WeatherPeriod cloudinessPeriod(theForecastPeriod);
    CloudinessForecastStoryItem* item =
        new CloudinessForecastStoryItem(*this,
                                        cloudinessPeriod,
                                        CLOUDINESS_STORY_PART,
                                        theCloudinessForecast.getCloudinessId(cloudinessPeriod),
                                        0,
                                        0);

    item->theReportAboutDryWeatherFlag = true;

    // cloudiness story item
    theStoryItemVector.push_back(item);
  }
  else
  {
    // check if first period is missing
    TextGenPosixTime firstPeriodStartTime(theForecastPeriod.localStartTime());
    TextGenPosixTime firstPeriodEndTime(theStoryItemVector[0]->thePeriod.localStartTime());

    WeatherPeriod firstPeriod(theForecastPeriod.localStartTime(),
                              theStoryItemVector[0]->thePeriod.localStartTime());

    if (firstPeriodEndTime.DifferenceInHours(firstPeriodStartTime) > 0)
    {
      firstPeriodEndTime.ChangeByHours(-1);
      WeatherPeriod firstPeriod(firstPeriodStartTime, firstPeriodEndTime);
      missingStoryItem = new WeatherForecastStoryItem(*this, firstPeriod, MISSING_STORY_PART);
      theStoryItemVector.insert(theStoryItemVector.begin(), missingStoryItem);
    }

    TextGenPosixTime lastPeriodStartTime(
        theStoryItemVector[theStoryItemVector.size() - 1]->thePeriod.localEndTime());
    TextGenPosixTime lastPeriodEndTime(theForecastPeriod.localEndTime());
    // chek if the last period is missing
    if (lastPeriodEndTime.DifferenceInHours(lastPeriodStartTime) > 0)
    {
      lastPeriodStartTime.ChangeByHours(1);
      WeatherPeriod lastPeriod(lastPeriodStartTime, lastPeriodEndTime);

      missingStoryItem = new WeatherForecastStoryItem(*this, lastPeriod, MISSING_STORY_PART);
      theStoryItemVector.push_back(missingStoryItem);
    }

    bool emptyPeriodsFound = true;
    while (emptyPeriodsFound)
    {
      emptyPeriodsFound = false;
      // place placeholder in the missing slots
      for (unsigned int i = 1; i < theStoryItemVector.size(); i++)
      {
        TextGenPosixTime middlePeriodStartTime(theStoryItemVector[i - 1]->thePeriod.localEndTime());
        TextGenPosixTime middlePeriodEndTime(theStoryItemVector[i]->thePeriod.localStartTime());

        if (middlePeriodEndTime.DifferenceInHours(middlePeriodStartTime) > 1)
        {
          WeatherPeriod middlePeriod(theStoryItemVector[i - 1]->thePeriod.localEndTime(),
                                     theStoryItemVector[i]->thePeriod.localStartTime());

          middlePeriodStartTime.ChangeByHours(1);
          middlePeriodEndTime.ChangeByHours(-1);
          missingStoryItem = new WeatherForecastStoryItem(
              *this, WeatherPeriod(middlePeriodStartTime, middlePeriodEndTime), MISSING_STORY_PART);
          theStoryItemVector.insert(theStoryItemVector.begin() + i, missingStoryItem);
          emptyPeriodsFound = true;
          break;
        }
      }
    }
  }
}

void WeatherForecastStory::addCloudinessStoryItems()
{  // replace the missing story items with the cloudiness story part
  PrecipitationForecastStoryItem* previousPrecipitationStoryItem = 0;
  for (unsigned int i = 0; i < theStoryItemVector.size(); i++)
  {
    if (theStoryItemVector[i]->theStoryPartId == MISSING_STORY_PART)
    {
      WeatherForecastStoryItem* placeholder = theStoryItemVector[i];

      CloudinessForecastStoryItem* cloudinessStoryItem = new CloudinessForecastStoryItem(
          *this,
          placeholder->thePeriod,
          CLOUDINESS_STORY_PART,
          theCloudinessForecast.getCloudinessId(placeholder->thePeriod),
          previousPrecipitationStoryItem,
          0);

      if (!previousPrecipitationStoryItem)
      {
        cloudinessStoryItem->theReportAboutDryWeatherFlag = true;
      }

      cloudinessStoryItem->thePreviousPrecipitationStoryItem = previousPrecipitationStoryItem;

      if (i < theStoryItemVector.size() - 1 &&
          theStoryItemVector[i + 1]->theStoryPartId == PRECIPITATION_STORY_PART)
        cloudinessStoryItem->theNextPrecipitationStoryItem =
            static_cast<PrecipitationForecastStoryItem*>(theStoryItemVector[i + 1]);

      theStoryItemVector[i] = cloudinessStoryItem;

      delete placeholder;
    }
    else if (theStoryItemVector[i]->theStoryPartId == PRECIPITATION_STORY_PART)
    {
      previousPrecipitationStoryItem =
          static_cast<PrecipitationForecastStoryItem*>(theStoryItemVector[i]);
    }

    // one hour period in the beginning is ignored
    /*
    if(i == 0)
      {
            if(theStoryItemVector[i]->storyItemPeriodLength() <= 1 && theStoryItemVector.size() > 1)
              {
                    theStoryItemVector[i]->theIncludeInTheStoryFlag = false;
              }
      }
    */
  }

  // short cloudiness period in the end after precipitation period is not reported
  /*
  int storyItemCount = theStoryItemVector.size();
  if(storyItemCount > 1)
    {
          if(theStoryItemVector[storyItemCount-1]->theStoryPartId == CLOUDINESS_STORY_PART &&
             theStoryItemVector[storyItemCount-1]->storyItemPeriodLength() <= 1 &&
             theStoryItemVector[storyItemCount-2]->theStoryPartId == PRECIPITATION_STORY_PART &&
             theStoryItemVector[storyItemCount-2]->theIncludeInTheStoryFlag == true)
            {
                  theStoryItemVector[storyItemCount-1]->theIncludeInTheStoryFlag = false;
            }
    }
  */
}

void WeatherForecastStory::mergePrecipitationPeriodsWhenFeasible()
{
  PrecipitationForecastStoryItem* previousPrecipitationStoryItem = 0;
  PrecipitationForecastStoryItem* currentPrecipitationStoryItem = 0;
  for (unsigned int i = 0; i < theStoryItemVector.size(); i++)
  {
    if (theStoryItemVector[i]->theStoryPartId == PRECIPITATION_STORY_PART)
    {
      currentPrecipitationStoryItem =
          static_cast<PrecipitationForecastStoryItem*>(theStoryItemVector[i]);
      if (previousPrecipitationStoryItem)
      {
        if (currentPrecipitationStoryItem->thePeriod.localStartTime().DifferenceInHours(
                previousPrecipitationStoryItem->thePeriod.localEndTime()) <= 2)
          previousPrecipitationStoryItem->theReportPoutaantuuFlag = false;

        // if the type is different don't merge, except when thunder exists on both periods
        if (previousPrecipitationStoryItem->theType != currentPrecipitationStoryItem->theType &&
            !(previousPrecipitationStoryItem->theThunder &&
              currentPrecipitationStoryItem->theThunder))
        {
          // if the dry period between precpitation periods is short don't mention it
          if (theStoryItemVector[i - 1] != previousPrecipitationStoryItem &&
              theStoryItemVector[i - 1]->storyItemPeriodLength() <= 1)
          {
            theStoryItemVector[i - 1]->theIncludeInTheStoryFlag = false;
          }
          previousPrecipitationStoryItem = currentPrecipitationStoryItem;
          continue;
        }

        TextGenPosixTime gapPeriodStartTime(
            previousPrecipitationStoryItem->thePeriod.localEndTime());
        gapPeriodStartTime.ChangeByHours(+1);
        TextGenPosixTime gapPeriodEndTime(
            currentPrecipitationStoryItem->thePeriod.localStartTime());
        if (gapPeriodStartTime < gapPeriodEndTime) gapPeriodEndTime.ChangeByHours(-1);
        WeatherPeriod gapPeriod(gapPeriodStartTime, gapPeriodEndTime);

        // merge periods
        if (get_period_length(gapPeriod) <= 3 &&
            (get_period_length(gapPeriod) <=
             currentPrecipitationStoryItem->storyItemPeriodLength() +
                 previousPrecipitationStoryItem->storyItemPeriodLength() + 2))
        {
          // merge two weak precipitation periods
          previousPrecipitationStoryItem->thePeriodToMergeWith = currentPrecipitationStoryItem;
          currentPrecipitationStoryItem->thePeriodToMergeTo = previousPrecipitationStoryItem;
          if (theStoryItemVector[i - 1]->theStoryPartId == CLOUDINESS_STORY_PART)
            theStoryItemVector[i - 1]->theIncludeInTheStoryFlag = false;
        }
      }
      previousPrecipitationStoryItem = currentPrecipitationStoryItem;
    }
  }

  int storyItemCount = theStoryItemVector.size();
  // short precipitation period in the beginning is ignored
  if (storyItemCount > 1)
  {
    WeatherPeriod storyItemPeriod(theStoryItemVector[0]->getStoryItemPeriod());

    if (theStoryItemVector[0]->theStoryPartId == PRECIPITATION_STORY_PART &&
        theStoryItemVector[1]->theStoryPartId == CLOUDINESS_STORY_PART &&
        theStoryItemVector[0]->getPeriodLength() <= 1 &&
        theStoryItemVector[0]->thePeriodToMergeWith == 0)
    {
      theStoryItemVector[0]->theIncludeInTheStoryFlag = false;
    }
    else if (theStoryItemVector[0]->theStoryPartId == CLOUDINESS_STORY_PART &&
             theStoryItemVector[1]->theStoryPartId == PRECIPITATION_STORY_PART &&
             theStoryItemVector[0]->getPeriodLength() <= 1 &&
             theStoryItemVector[0]->thePeriodToMergeWith == 0 &&
             static_cast<PrecipitationForecastStoryItem*>(theStoryItemVector[1])->theIntensity <
                 0.1)
    /* &&
           static_cast<PrecipitationForecastStoryItem*>(theStoryItemVector[1])->theExtent < 40)*/
    {
      theStoryItemVector[0]->theIncludeInTheStoryFlag = false;
    }

    // short precipitation period in the end after coudiness period is not reported
    if (theStoryItemVector[storyItemCount - 1]->theStoryPartId == PRECIPITATION_STORY_PART &&
        theStoryItemVector[storyItemCount - 1]->getPeriodLength() <= 1 &&
        theStoryItemVector[storyItemCount - 1]->thePeriodToMergeTo == 0 &&
        theStoryItemVector[storyItemCount - 2]->theStoryPartId == CLOUDINESS_STORY_PART &&
        theStoryItemVector[storyItemCount - 2]->theIncludeInTheStoryFlag == true)
    {
      theStoryItemVector[storyItemCount - 1]->theIncludeInTheStoryFlag = false;
    }
  }
}

void WeatherForecastStory::mergeCloudinessPeriodsWhenFeasible()
{
  CloudinessForecastStoryItem* previousCloudinessStoryItem = 0;
  CloudinessForecastStoryItem* currentCloudinessStoryItem = 0;
  for (unsigned int i = 0; i < theStoryItemVector.size(); i++)
  {
    if (theStoryItemVector[i]->theStoryPartId == CLOUDINESS_STORY_PART &&
        theStoryItemVector[i]->theIncludeInTheStoryFlag == true)
    {
      currentCloudinessStoryItem = static_cast<CloudinessForecastStoryItem*>(theStoryItemVector[i]);
      if (previousCloudinessStoryItem)
      {
        TextGenPosixTime gapPeriodStartTime(previousCloudinessStoryItem->thePeriod.localEndTime());
        gapPeriodStartTime.ChangeByHours(+1);
        TextGenPosixTime gapPeriodEndTime(currentCloudinessStoryItem->thePeriod.localStartTime());
        if (gapPeriodStartTime < gapPeriodEndTime) gapPeriodEndTime.ChangeByHours(-1);
        WeatherPeriod gapPeriod(gapPeriodStartTime, gapPeriodEndTime);

        // merge periods if the precipitation period in between is short and
        // intensity is weak and precipitation area is small
        if (get_period_length(gapPeriod) <= 1 &&
            currentCloudinessStoryItem->thePreviousPrecipitationStoryItem->theIntensity <= 0.1)
        /* &&
               currentCloudinessStoryItem->thePreviousPrecipitationStoryItem->theExtent <= 40)*/
        {
          // merge two cloudiness periods
          previousCloudinessStoryItem->thePeriodToMergeWith = currentCloudinessStoryItem;
          currentCloudinessStoryItem->thePeriodToMergeTo = previousCloudinessStoryItem;
          currentCloudinessStoryItem->thePreviousPrecipitationStoryItem->theIncludeInTheStoryFlag =
              false;
        }
      }
      previousCloudinessStoryItem = currentCloudinessStoryItem;
    }
  }

  int storyItemCount = theStoryItemVector.size();
  // short cloudiness period in the beginning is ignored
  if (storyItemCount > 1)
  {
    WeatherPeriod storyItemPeriod(theStoryItemVector[0]->getStoryItemPeriod());

    if (theStoryItemVector[0]->theStoryPartId == CLOUDINESS_STORY_PART &&
        theStoryItemVector[1]->theStoryPartId == PRECIPITATION_STORY_PART &&
        theStoryItemVector[0]->getPeriodLength() <= 1 &&
        theStoryItemVector[0]->thePeriodToMergeWith == 0)
    {
      theStoryItemVector[0]->theIncludeInTheStoryFlag = false;
    }
    else if (theStoryItemVector[0]->theStoryPartId == PRECIPITATION_STORY_PART &&
             theStoryItemVector[1]->theStoryPartId == CLOUDINESS_STORY_PART &&
             theStoryItemVector[0]->getPeriodLength() <= 1 &&
             theStoryItemVector[0]->thePeriodToMergeWith == 0 &&
             static_cast<PrecipitationForecastStoryItem*>(theStoryItemVector[0])->theIntensity <
                 0.1)
    /* &&
       static_cast<PrecipitationForecastStoryItem*>(theStoryItemVector[0])->theExtent < 40)*/
    {
      theStoryItemVector[0]->theIncludeInTheStoryFlag = false;
    }

    // short cloudiness period in the end after precipitation period is not reported
    if (theStoryItemVector[storyItemCount - 1]->theStoryPartId == CLOUDINESS_STORY_PART &&
        theStoryItemVector[storyItemCount - 1]->getPeriodLength() <= 1 &&
        theStoryItemVector[storyItemCount - 1]->thePeriodToMergeTo == 0 &&
        theStoryItemVector[storyItemCount - 2]->theStoryPartId == PRECIPITATION_STORY_PART &&
        theStoryItemVector[storyItemCount - 2]->theIncludeInTheStoryFlag == true)
    {
      theStoryItemVector[storyItemCount - 1]->theIncludeInTheStoryFlag = false;
    }
  }
}

void WeatherForecastStory::mergePeriodsWhenFeasible()
{
  mergePrecipitationPeriodsWhenFeasible();
  mergeCloudinessPeriodsWhenFeasible();
}

Sentence WeatherForecastStory::getTimePhrase()
{
  Sentence sentence;

  if (theReportTimePhraseFlag)
  {
    if (theStorySize == 0)
      sentence << ALUKSI_WORD;
    else
      sentence << MYOHEMMIN_WORD;
  }

  return sentence;
}

void WeatherForecastStory::logTheStoryItems() const
{
  theLogger << "******** STORY ITEMS ********" << endl;
  for (unsigned int i = 0; i < theStoryItemVector.size(); i++)
  {
    theLogger << *theStoryItemVector[i];
  }
}

WeatherForecastStoryItem::WeatherForecastStoryItem(WeatherForecastStory& weatherForecastStory,
                                                   const WeatherPeriod& period,
                                                   const story_part_id& storyPartId)
    : theWeatherForecastStory(weatherForecastStory),
      thePeriod(period),
      theStoryPartId(storyPartId),
      theIncludeInTheStoryFlag(true),
      thePeriodToMergeWith(0),
      thePeriodToMergeTo(0)
{
}

Sentence WeatherForecastStoryItem::getSentence()
{
  Sentence sentence;

  theAdditionalSentences.clear();

  if (theIncludeInTheStoryFlag)
  {
    sentence << getStoryItemSentence();
  }

  return sentence;
}

Sentence WeatherForecastStoryItem::getAdditionalSentence(const unsigned int& index) const
{
  if (index >= theAdditionalSentences.size())
    return Sentence();
  else
    return theAdditionalSentences[index];
}

WeatherPeriod WeatherForecastStoryItem::getStoryItemPeriod() const
{
  if (thePeriodToMergeWith)
  {
    WeatherPeriod period(thePeriod.localStartTime(),
                         thePeriodToMergeWith->getStoryItemPeriod().localEndTime());
    return period;
  }
  else
  {
    return thePeriod;
  }
}

unsigned int WeatherForecastStoryItem::getPeriodLength()
{
  return thePeriod.localEndTime().DifferenceInHours(thePeriod.localStartTime());
}

int WeatherForecastStoryItem::forecastPeriodLength() const
{
  return get_period_length(theWeatherForecastStory.theForecastPeriod);
}

int WeatherForecastStoryItem::storyItemPeriodLength() const
{
  return get_period_length(getStoryItemPeriod());
}

Sentence WeatherForecastStoryItem::getTodayVectorSentence(const vector<Sentence*>& todayVector,
                                                          const unsigned int& theBegIndex,
                                                          const unsigned int& theEndIndex)
{
  Sentence sentence;

  for (unsigned int i = theBegIndex; i <= theEndIndex; i++)
  {
    if (sentence.size() > 0) sentence << JA_WORD;
    sentence << *(todayVector[i]);
  }
  return sentence;
}

// special treatment, because 06:00 can be aamuyö and morning, depends weather the period starts or
// ends
std::string WeatherForecastStoryItem::checkForAamuyoAndAamuPhrase(
    const bool& theFromSpecifier, const WeatherPeriod& thePhrasePeriod)
{
  std::string retValue("");

  // 6:00 in the morning or in the evening
  if (is_inside(thePhrasePeriod.localStartTime(), AAMU) &&
      is_inside(thePhrasePeriod.localEndTime(), AAMU) &&
      is_inside(thePhrasePeriod.localStartTime(), AAMUYO) &&
      is_inside(thePhrasePeriod.localEndTime(), AAMUYO))
  {
    if (thePhrasePeriod.localStartTime() ==
        theWeatherForecastStory.theForecastPeriod.localStartTime())
    {
      retValue = (theFromSpecifier ? AAMUSTA_ALKAEN_PHRASE : AAMULLA_WORD);
    }
    else
    {
      retValue = (theFromSpecifier ? AAMUYOSTA_ALKAEN_PHRASE : AAMUYOLLA_WORD);
    }
  }

  return retValue;
}

Sentence WeatherForecastStoryItem::getPeriodPhrase(
    const bool& theFromSpecifier,
    const WeatherPeriod* thePhrasePeriod /*= 0*/,
    const bool& theStoryUnderConstructionEmpty /*= true*/)
{
  Sentence sentence;

  if (theWeatherForecastStory.theStorySize == 0 && theStoryUnderConstructionEmpty) return sentence;

  WeatherPeriod phrasePeriod(thePhrasePeriod == 0 ? getStoryItemPeriod() : *thePhrasePeriod);
  bool specifyDay = false;

  if (forecastPeriodLength() > 24 &&
      theWeatherForecastStory.theForecastTime.GetJulianDay() !=
          phrasePeriod.localStartTime().GetJulianDay() &&
      abs(theWeatherForecastStory.theForecastTime.DifferenceInHours(
          phrasePeriod.localStartTime())) >= 21)
  {
    Sentence todaySentence;
    todaySentence << PeriodPhraseFactory::create("today",
                                                 theWeatherForecastStory.theVar,
                                                 theWeatherForecastStory.theForecastTime,
                                                 phrasePeriod,
                                                 theWeatherForecastStory.theWeatherArea);
    if (todaySentence.size() > 0) specifyDay = true;
  }
  std::string day_phase_phrase("");

  day_phase_phrase = checkForAamuyoAndAamuPhrase(theFromSpecifier, phrasePeriod);
  WeatherHistory& thePhraseHistory =
      const_cast<WeatherArea&>(theWeatherForecastStory.theWeatherArea).history();

  if (!day_phase_phrase.empty())
  {
    if (day_phase_phrase != thePhraseHistory.latestDayPhasePhrase)
    {
      thePhraseHistory.updateDayPhasePhrase(day_phase_phrase);
    }
    sentence << day_phase_phrase;
  }

  if (sentence.size() == 0)
  {
    part_of_the_day_id id;
    get_time_phrase_large(phrasePeriod,
                          specifyDay,
                          theWeatherForecastStory.theVar,
                          day_phase_phrase,
                          theFromSpecifier,
                          id);

    if (day_phase_phrase != thePhraseHistory.latestDayPhasePhrase)
    {
      thePhraseHistory.updateDayPhasePhrase(day_phase_phrase);
    }

    sentence << day_phase_phrase;
  }

  theWeatherForecastStory.theLogger << "PHRASE PERIOD: " << phrasePeriod.localStartTime() << "..."
                                    << phrasePeriod.localEndTime() << endl;
  theWeatherForecastStory.theLogger << "PHRASE: ";
  theWeatherForecastStory.theLogger << sentence;

  return sentence;
}

PrecipitationForecastStoryItem::PrecipitationForecastStoryItem(
    WeatherForecastStory& weatherForecastStory,
    const WeatherPeriod& period,
    const story_part_id& storyPartId,
    const float& intensity,
    const float& extent,
    const unsigned int& form,
    const precipitation_type& type,
    const bool& thunder)
    : WeatherForecastStoryItem(weatherForecastStory, period, storyPartId),
      theIntensity(intensity),
      theExtent(extent),
      theForm(form),
      theType(type),
      theThunder(thunder),
      theSadeJatkuuFlag(false),
      thePoutaantuuFlag(intensity > WEAK_PRECIPITATION_LIMIT_WATER),
      theReportPoutaantuuFlag(intensity > WEAK_PRECIPITATION_LIMIT_WATER),
      theFullDuration(period.localEndTime().DifferenceInHours(period.localStartTime()))
{
}

bool PrecipitationForecastStoryItem::weakPrecipitation() const
{
  return theIntensity <= WEAK_PRECIPITATION_LIMIT_WATER;
}

Sentence PrecipitationForecastStoryItem::getStoryItemSentence()
{
  Sentence sentence;

  // thePeriodToMergeWith handles the whole stuff
  if (thePeriodToMergeTo) return sentence;

  const PrecipitationForecast& prForecast = theWeatherForecastStory.thePrecipitationForecast;
  WeatherPeriod forecastPeriod = theWeatherForecastStory.theForecastPeriod;
  WeatherPeriod storyItemPeriod(getStoryItemPeriod());
  Sentence thePeriodPhrase;

  if (storyItemPeriodLength() >= 6)
  {
    if (storyItemPeriod.localStartTime() != forecastPeriod.localStartTime())
    {
      if (storyItemPeriod.localStartTime() > forecastPeriod.localStartTime())
        thePeriodPhrase << getPeriodPhrase(USE_FROM_SPECIFIER);
      if (thePeriodPhrase.size() == 0) thePeriodPhrase << theWeatherForecastStory.getTimePhrase();

      sentence << prForecast.precipitationChangeSentence(
          storyItemPeriod, thePeriodPhrase, SADE_ALKAA, theAdditionalSentences);
    }
    else
    {
      if (storyItemPeriod.localStartTime() > forecastPeriod.localStartTime())
        thePeriodPhrase << getPeriodPhrase(DONT_USE_FROM_SPECIFIER, &storyItemPeriod);
      if (thePeriodPhrase.size() == 0) thePeriodPhrase << theWeatherForecastStory.getTimePhrase();

      sentence << prForecast.precipitationSentence(
          storyItemPeriod, thePeriodPhrase, theAdditionalSentences);
    }

    if (storyItemPeriod.localEndTime() != forecastPeriod.localEndTime() && theReportPoutaantuuFlag)
    {
      WeatherPeriod poutaantuuPeriod(storyItemPeriod.localEndTime(),
                                     storyItemPeriod.localEndTime());
      thePeriodPhrase << getPeriodPhrase(
          DONT_USE_FROM_SPECIFIER, &poutaantuuPeriod, sentence.size() == 0);
      theWeatherForecastStory.theLogger << thePeriodPhrase;
      if (thePeriodPhrase.size() == 0) thePeriodPhrase << theWeatherForecastStory.getTimePhrase();

      if (sentence.size() > 0) sentence << Delimiter(",");
      sentence << prForecast.precipitationChangeSentence(
          storyItemPeriod, thePeriodPhrase, POUTAANTUU, theAdditionalSentences);
    }
    theWeatherForecastStory.theShortTimePrecipitationReportedFlag = false;
  }
  else
  {
    if (thePeriod.localStartTime() > forecastPeriod.localStartTime())
      thePeriodPhrase << getPeriodPhrase(DONT_USE_FROM_SPECIFIER, &storyItemPeriod);
    if (thePeriodPhrase.size() == 0) thePeriodPhrase << theWeatherForecastStory.getTimePhrase();

    if (prForecast.shortTermPrecipitationExists(thePeriod))
    {
      sentence << prForecast.shortTermPrecipitationSentence(thePeriod, thePeriodPhrase);
      theWeatherForecastStory.theShortTimePrecipitationReportedFlag = true;
    }
    else
    {
      sentence << prForecast.precipitationSentence(
          storyItemPeriod, thePeriodPhrase, theAdditionalSentences);
    }
  }
  return sentence;
}

CloudinessForecastStoryItem::CloudinessForecastStoryItem(
    WeatherForecastStory& weatherForecastStory,
    const WeatherPeriod& period,
    const story_part_id& storyPartId,
    const cloudiness_id& cloudinessId,
    PrecipitationForecastStoryItem* previousPrecipitationStoryItem,
    PrecipitationForecastStoryItem* nextPrecipitationStoryItem)
    : WeatherForecastStoryItem(weatherForecastStory, period, storyPartId),
      theCloudinessId(cloudinessId),
      thePreviousPrecipitationStoryItem(previousPrecipitationStoryItem),
      theNextPrecipitationStoryItem(nextPrecipitationStoryItem),
      theReportAboutDryWeatherFlag(true)

{
  if (thePreviousPrecipitationStoryItem && thePreviousPrecipitationStoryItem->isIncluded())
  {
    if (thePreviousPrecipitationStoryItem->thePoutaantuuFlag)
    {
      thePreviousPrecipitationStoryItem->theReportPoutaantuuFlag = false;
      theReportAboutDryWeatherFlag = false;
    }
  }
}

Sentence CloudinessForecastStoryItem::cloudinessChangeSentence()
{
  Sentence sentence;

  weather_event_id_vector cloudinessEvents;  // pilvistyy and selkenee

  theWeatherForecastStory.theCloudinessForecast.getWeatherEventIdVector(cloudinessEvents);

  for (unsigned int i = 0; i < cloudinessEvents.size(); i++)
  {
    TextGenPosixTime cloudinessEventTimestamp(cloudinessEvents.at(i).first);
    if (cloudinessEventTimestamp >= thePeriod.localStartTime() &&
        cloudinessEventTimestamp <= thePeriod.localEndTime())
    {
      sentence << theWeatherForecastStory.theCloudinessForecast.cloudinessChangeSentence(
          WeatherPeriod(cloudinessEventTimestamp, cloudinessEventTimestamp));

      theCloudinessChangeTimestamp = cloudinessEventTimestamp;

      theWeatherForecastStory.theLogger << "CLOUDINESS CHANGE: " << endl;
      theWeatherForecastStory.theLogger << cloudinessEventTimestamp;
      theWeatherForecastStory.theLogger << ": ";
      theWeatherForecastStory.theLogger << sentence;

      break;
    }
  }

  return sentence;
}

Sentence CloudinessForecastStoryItem::getStoryItemSentence()
{
  Sentence sentence;

  // thePeriodToMergeWith handles the whole stuff
  if (thePeriodToMergeTo) return sentence;

  WeatherPeriod storyItemPeriod(getStoryItemPeriod());
  // if the cloudiness period is max 2 hours and it is in the end of the forecast period and
  // the previous precipitation period is long > 6h -> don't report cloudiness
  if (storyItemPeriod.localEndTime() == theWeatherForecastStory.theForecastPeriod.localEndTime() &&
      storyItemPeriodLength() <= 2)
  {
    if (thePreviousPrecipitationStoryItem && thePreviousPrecipitationStoryItem->isIncluded() &&
        (thePreviousPrecipitationStoryItem->storyItemPeriodLength() >= 6 ||
         get_part_of_the_day_id_narrow(thePreviousPrecipitationStoryItem->getStoryItemPeriod()) ==
             get_part_of_the_day_id_narrow(getStoryItemPeriod())))
      return sentence;
  }

  const CloudinessForecast& clForecast = theWeatherForecastStory.theCloudinessForecast;
  const PrecipitationForecast& prForecast = theWeatherForecastStory.thePrecipitationForecast;

  theSentence.clear();
  theChangeSentence.clear();
  theShortFormSentence.clear();
  thePoutaantuuSentence.clear();

  theChangeSentence << cloudinessChangeSentence();

  if (thePreviousPrecipitationStoryItem && thePreviousPrecipitationStoryItem->isIncluded())
  {
    if (thePreviousPrecipitationStoryItem->thePoutaantuuFlag)
    {
      cloudiness_id cloudinessId = clForecast.getCloudinessId(getStoryItemPeriod());

      WeatherPeriod poutaantuuPeriod(
          thePreviousPrecipitationStoryItem->getStoryItemPeriod().localEndTime(),
          thePreviousPrecipitationStoryItem->getStoryItemPeriod().localEndTime());

      // ARE 22.02.2011: The missing period-phrase added
      Sentence thePeriodPhrase(getPeriodPhrase(USE_FROM_SPECIFIER, &poutaantuuPeriod));
      if (thePeriodPhrase.size() == 0) thePeriodPhrase << theWeatherForecastStory.getTimePhrase();
      thePoutaantuuSentence << prForecast.precipitationPoutaantuuAndCloudiness(thePeriodPhrase,
                                                                               cloudinessId);
      thePreviousPrecipitationStoryItem->theReportPoutaantuuFlag = false;
      theReportAboutDryWeatherFlag = false;
    }
  }

  if (!thePoutaantuuSentence.empty())
  {
    sentence << thePoutaantuuSentence;

    // ARE 10.03.2011: Jos sää on melko selkeä ei enää sanota selkenevää
    if (theChangeSentence.size() > 0 && clForecast.getCloudinessId(storyItemPeriod) > MELKO_SELKEA)
    {
      sentence << Delimiter(COMMA_PUNCTUATION_MARK);
      sentence << theChangeSentence;
    }
  }
  else
  {
    Sentence thePeriodPhrase;
    if (storyItemPeriod.localStartTime() >
        theWeatherForecastStory.theForecastPeriod.localStartTime())
    {
      if (storyItemPeriodLength() >= 6)
      {
        if (theWeatherForecastStory.theStorySize > 0)
          thePeriodPhrase << getPeriodPhrase(USE_FROM_SPECIFIER);
      }
      else
      {
        thePeriodPhrase << getPeriodPhrase(DONT_USE_FROM_SPECIFIER, &storyItemPeriod);
      }
    }
    if (thePeriodPhrase.size() == 0) thePeriodPhrase << theWeatherForecastStory.getTimePhrase();

    if (theChangeSentence.size() > 0)
    {
      WeatherPeriod clPeriod(storyItemPeriod.localStartTime(), theCloudinessChangeTimestamp);
      sentence << clForecast.cloudinessSentence(
          clPeriod,
          theReportAboutDryWeatherFlag && !prForecast.getDryPeriodTautologyFlag(),
          thePeriodPhrase,
          DONT_USE_SHORT_FORM);
    }
    else
    {
      sentence << clForecast.cloudinessSentence(
          storyItemPeriod,
          theReportAboutDryWeatherFlag && !prForecast.getDryPeriodTautologyFlag(),
          thePeriodPhrase,
          DONT_USE_SHORT_FORM);
    }
    prForecast.setDryPeriodTautologyFlag(theReportAboutDryWeatherFlag);

    // ARE 10.03.2011: Jos sää on melko selkeä ei enää sanota selkenevää
    if (theChangeSentence.size() > 0 &&
        clForecast.getCloudinessId(getStoryItemPeriod()) > MELKO_SELKEA)
    {
      sentence << Delimiter(COMMA_PUNCTUATION_MARK);
      sentence << theChangeSentence;
    }
  }
  theWeatherForecastStory.theCloudinessReportedFlag = true;

  return sentence;
}
}
