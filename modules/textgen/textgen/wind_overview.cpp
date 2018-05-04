#include "WindStory.h"
#include "MessageLogger.h"
#include "Paragraph.h"
#include "WindStoryTools.h"
#include <calculator/GridForecaster.h>
#include <calculator/ParameterAnalyzer.h>
#include "PositiveValueAcceptor.h"
#include <calculator/WeatherResult.h>
#include "Sentence.h"
#include <calculator/Settings.h>
#include "WeatherForecast.h"
#include "WindForecast.h"
#include <calculator/RangeAcceptor.h>
#include "SubMaskExtractor.h"

#include <bitset>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <algorithm>

#include <macgyver/StringConversion.h>
#include <boost/date_time/gregorian/gregorian.hpp>    //include all types plus i/o
#include <boost/date_time/posix_time/posix_time.hpp>  //include all types plus i/o
#include <boost/foreach.hpp>

using namespace boost;
using namespace TextGen;
using namespace TextGen::WindStoryTools;
using namespace std;

namespace TextGen
{
const WindDataItemUnit& get_data_item(const wo_story_params& storyParams,
                                      const TextGenPosixTime& theTime)
{
  unsigned int retIndex = 0;

  WeatherArea::Type areaType(storyParams.theArea.type());
  for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
  {
    const WindDataItemUnit& dataItem = (*storyParams.theWindDataVector[i])(areaType);

    if (dataItem.thePeriod.localStartTime() == theTime)
    {
      retIndex = i;
      break;
    }
  }

  return (*storyParams.theWindDataVector[retIndex])(areaType);
}

float calculate_weighted_wind_speed(const wo_story_params& storyParams,
                                    float topWind,
                                    float medianWind)
{
  // weights are read from config file
  double share = (storyParams.theWeakTopWind ? storyParams.theWindCalcTopShareWeak
                                             : storyParams.theWindCalcTopShare);
  float topWindWeight = (share / 100.0);
  float medianWindWeight = 1.0 - topWindWeight;

  return ((topWind * topWindWeight) + (medianWind * medianWindWeight));
}

float calculate_weighted_wind_speed(const wo_story_params& storyParams,
                                    const WindDataItemUnit& dataItem)
{
  return calculate_weighted_wind_speed(
      storyParams, dataItem.theWindSpeedTop.value(), dataItem.theWindSpeedMedian.value());
}

bool wind_event_period_sort(const WindEventPeriodDataItem* first,
                            const WindEventPeriodDataItem* second)
{
  return (first->thePeriod.localStartTime() < second->thePeriod.localStartTime());
}

bool wind_direction_item_sort(pair<float, WeatherResult> firstItem,
                              pair<float, WeatherResult> secondItem)
{
  return firstItem.second.value() > secondItem.second.value();
}

std::string get_area_type_string(const WeatherArea::Type& theAreaType)
{
  std::string retval;

  switch (theAreaType)
  {
    case WeatherArea::Full:
      retval = "full";
      break;
    case WeatherArea::Land:
      retval = "land";
      break;
    case WeatherArea::Coast:
      retval = "coast";
      break;
    case WeatherArea::Inland:
      retval = "inland";
      break;
    case WeatherArea::Northern:
      retval = "northern";
      break;
    case WeatherArea::Southern:
      retval = "southern";
      break;
    case WeatherArea::Eastern:
      retval = "eastern";
      break;
    case WeatherArea::Western:
      retval = "western";
      break;
    default:
      break;
  }

  return retval;
}

std::string get_area_name_string(const WeatherArea& theArea)
{
  std::string retval(theArea.isNamed() ? theArea.name() : "");
  retval += "_";
  retval += get_area_type_string(theArea.type());

  return retval;
}

std::ostream& operator<<(std::ostream& theOutput, const WeatherPeriod& period)
{
  theOutput << period.localStartTime() << "..." << period.localEndTime();

  return theOutput;
}

std::string get_direction_abbreviation(
    float direction, WindStoryTools::CompassType compass_type = sixteen_directions)
{
  direction = abs(direction);

  while (direction > 360.0)
    direction = -360.0;

  if (compass_type == sixteen_directions)
  {
    if (direction >= 348.75 || direction < 11.25)
      return "N";
    else if (direction >= 11.25 && direction < 33.75)
      return "n-ne";
    else if (direction >= 33.75 && direction < 56.25)
      return "NE";
    else if (direction >= 56.25 && direction < 78.75)
      return "ne-e";
    else if (direction >= 78.75 && direction < 101.25)
      return "E";
    else if (direction >= 101.25 && direction < 123.75)
      return "e-se";
    else if (direction >= 123.75 && direction < 146.25)
      return "SE";
    else if (direction >= 146.25 && direction < 168.75)
      return "se-s";
    else if (direction >= 168.75 && direction < 191.25)
      return "S";
    else if (direction >= 191.25 && direction < 213.75)
      return "s-sw";
    else if (direction >= 213.75 && direction < 236.25)
      return "SW";
    else if (direction >= 236.25 && direction < 258.75)
      return "sw-w";
    else if (direction >= 258.75 && direction < 281.25)
      return "W";
    else if (direction >= 281.25 && direction < 303.75)
      return "w-nw";
    else if (direction >= 303.75 && direction < 326.25)
      return "NW";
    else
      return "nw-n";
  }
  else
  {
    if (direction >= 337.50 || direction < 22.50)
      return "N";
    else if (direction >= 22.50 && direction < 67.50)
      return "NE";
    else if (direction >= 67.50 && direction < 112.50)
      return "E";
    else if (direction >= 112.50 && direction < 157.50)
      return "SE";
    else if (direction >= 157.50 && direction < 202.50)
      return "S";
    else if (direction >= 202.50 && direction < 247.50)
      return "SW";
    else if (direction >= 247.50 && direction < 292.50)
      return "W";
    else
      return "NW";
  }
}

void printDataItem(std::ostream& theOutput,
                   const WindDataItemUnit& theWindDataItem,
                   double theWindDirectionMinSpeed)
{
  theOutput << theWindDataItem.thePeriod.localStartTime() << " ... "
            << theWindDataItem.thePeriod.localEndTime() << ": min: " << fixed << setprecision(4)
            << theWindDataItem.theWindSpeedMin.value() << "; maximi: " << fixed << setprecision(4)
            << theWindDataItem.theWindSpeedMax.value() << "; tasoitettu maximi: " << fixed
            << setprecision(4) << theWindDataItem.theEqualizedMaxWind.value()
            << "; mediaani: " << fixed << setprecision(4)
            << theWindDataItem.theWindSpeedMedian.value() << "; tasoitettu mediaani: " << fixed
            << setprecision(4) << theWindDataItem.theEqualizedMedianWind.value()
            << "; ka,k-hajonta: (" << fixed << setprecision(4)
            << theWindDataItem.theWindSpeedMean.value() << ", " << fixed << setprecision(4)
            << theWindDataItem.theWindSpeedMean.error() << ")"
            << "; huipputuuli: " << fixed << setprecision(4)
            << theWindDataItem.theWindSpeedTop.value() << "; tasoitettu huipputuuli: " << fixed
            << setprecision(4) << theWindDataItem.theEqualizedTopWind.value()
            << "; laskennallinen tuuli: " << fixed << setprecision(4)
            << theWindDataItem.theWindSpeedCalc.value() << "; tasoitettu laskenn: " << fixed
            << setprecision(4) << theWindDataItem.theEqualizedCalcWind.value()
            << "; suunta: " << fixed << setprecision(4)
            << theWindDataItem.theCorrectedWindDirection.value() << "; suunnan k-hajonta: " << fixed
            << setprecision(4) << theWindDataItem.theCorrectedWindDirection.error()
            << "; tasoitettu suunta: " << fixed << setprecision(4)
            << theWindDataItem.theEqualizedWindDirection.value() << " ("
            << wind_direction_string(wind_direction_id(theWindDataItem.theEqualizedWindDirection,
                                                       theWindDataItem.theEqualizedTopWind,
                                                       "",
                                                       theWindDirectionMinSpeed))
            << ") " << theWindDataItem.theEqualizedWindDirection.value() << ", "
            << theWindDataItem.theEqualizedWindDirection.error() << ", "
            << theWindDataItem.theEqualizedTopWind.value() << "; puuska: " << fixed
            << setprecision(4) << theWindDataItem.theGustSpeed.value();
}

std::ostream& operator<<(std::ostream& theOutput,
                         const WindSpeedPeriodDataItem& theWindSpeedPeriodDataItem)
{
  theOutput << theWindSpeedPeriodDataItem.thePeriod.localStartTime() << " ... "
            << theWindSpeedPeriodDataItem.thePeriod.localEndTime() << ": "
            << wind_speed_string(theWindSpeedPeriodDataItem.theWindSpeedId) << endl;

  return theOutput;
}

std::ostream& operator<<(std::ostream& theOutput,
                         const WindDirectionPeriodDataItem& theWindDirectionPeriodDataItem)
{
  theOutput << theWindDirectionPeriodDataItem.thePeriod.localStartTime() << " ... "
            << theWindDirectionPeriodDataItem.thePeriod.localEndTime() << ": "
            << wind_direction_string(theWindDirectionPeriodDataItem.theWindDirection) << endl;

  return theOutput;
}

std::ostream& operator<<(std::ostream& theOutput,
                         const WindEventPeriodDataItem& theWindEventPeriodDataItem)
{
  theOutput << endl
            << "thePeriod: " << theWindEventPeriodDataItem.thePeriod << endl
            << "theWindSpeedChangePeriod: " << theWindEventPeriodDataItem.theWindSpeedChangePeriod
            << endl
            << "theWindEvent: " << get_wind_event_string(theWindEventPeriodDataItem.theWindEvent)
            << endl
            << endl;

  return theOutput;
}

void print_wiki_table(const WeatherArea& theArea,
                      const string& theVar,
                      const wind_data_item_vector& theWindDataItemVector)
{
  std::string filename("./" + get_area_name_string(theArea) + "_rawdata.txt");

  ofstream output_file(filename.c_str(), ios::out);

  if (output_file.fail())
  {
    throw std::runtime_error("wind_overview failed to open '" + filename + "' for writing");
  }

  output_file << "|| aika || min || max || ka, k-hajonta || maksimituuli || puuska || suunta || "
                 "suunnan k-hajonta || fraasi ||"
              << endl;

  for (unsigned int i = 0; i < theWindDataItemVector.size(); i++)
  {
    const WindDataItemUnit& theWindDataItem = (*theWindDataItemVector[i])(theArea.type());
    output_file << "| " << theWindDataItem.thePeriod.localEndTime() << " | " << fixed
                << setprecision(4) << theWindDataItem.theWindSpeedMin.value() << " | " << fixed
                << setprecision(4) << theWindDataItem.theWindSpeedMax.value() << " | (" << fixed
                << setprecision(4) << theWindDataItem.theWindSpeedMean.value() << ", " << fixed
                << setprecision(4) << theWindDataItem.theWindSpeedMean.error() << ")"
                << " | " << fixed << setprecision(4) << theWindDataItem.theWindSpeedTop.value()
                << " | " << fixed << setprecision(4) << theWindDataItem.theGustSpeed.value()
                << " | " << fixed << setprecision(4)
                << theWindDataItem.theCorrectedWindDirection.value() << " | " << fixed
                << setprecision(4) << theWindDataItem.theWindDirection.error() << " | ";

    output_file << directed16_speed_string(theWindDataItem.theWindSpeedMean,
                                           theWindDataItem.theCorrectedWindDirection,
                                           theVar)
                << " |" << endl;
  }
}

void print_csv_table(const WeatherArea& theArea,
                     const std::string& fileIdentifierString,
                     const std::string& theVar,
                     const wind_data_item_vector& theWindDataItemVector,
                     const vector<unsigned int>& theIndexVector,
                     const bitset<14>& theColumnSelectionBitset)
{
  std::string filename("./" + get_area_name_string(theArea) + fileIdentifierString + ".csv");

  ofstream output_file(filename.c_str(), ios::out);

  if (output_file.fail())
  {
    throw std::runtime_error("wind_overview failed to open '" + filename + "' for writing");
  }

  const char* column_names[] = {"aika",
                                ", min",
                                ", max",
                                ", mediaani",
                                ", tasoitettu mediaani",
                                ", ka",
                                ", nopeuden k-hajonta",
                                ", puuska",
                                ", maksimituuli",
                                ", tasoitettu maksimituuli",
                                ", suunta",
                                ", tasoitettu suunta",
                                ", suunnan k-hajonta",
                                ", fraasi"};

  const unsigned int number_of_columns = 14;
  for (unsigned int i = 0; i < number_of_columns; i++)
    if (theColumnSelectionBitset.test(number_of_columns - i - 1)) output_file << column_names[i];
  output_file << endl;

  for (unsigned int i = 0; i < theIndexVector.size(); i++)
  {
    const unsigned int& index = theIndexVector[i];

    const WindDataItemUnit& theWindDataItem = (*theWindDataItemVector[index])(theArea.type());
    if (theColumnSelectionBitset.test(13)) output_file << theWindDataItem.thePeriod.localEndTime();
    if (theColumnSelectionBitset.test(12))
    {
      output_file << ", " << fixed << setprecision(4) << theWindDataItem.theWindSpeedMin.value();
    }
    if (theColumnSelectionBitset.test(11))
    {
      output_file << ", " << fixed << setprecision(4) << theWindDataItem.theWindSpeedMax.value();
    }
    if (theColumnSelectionBitset.test(10))
    {
      output_file << ", " << fixed << setprecision(4) << theWindDataItem.theWindSpeedMedian.value();
    }
    if (theColumnSelectionBitset.test(9))
    {
      output_file << ", " << fixed << setprecision(4)
                  << theWindDataItem.theEqualizedMedianWind.value();
    }
    if (theColumnSelectionBitset.test(8))
    {
      output_file << ", " << fixed << setprecision(4) << theWindDataItem.theWindSpeedMean.value();
    }
    if (theColumnSelectionBitset.test(7))
    {
      output_file << ", " << fixed << setprecision(4) << theWindDataItem.theWindSpeedMean.error();
    }
    if (theColumnSelectionBitset.test(6))
    {
      output_file << ", " << fixed << setprecision(4) << theWindDataItem.theGustSpeed.value();
    }
    if (theColumnSelectionBitset.test(5))
    {
      output_file << ", " << fixed << setprecision(4) << theWindDataItem.theWindSpeedTop.value();
    }
    if (theColumnSelectionBitset.test(4))
    {
      output_file << ", " << fixed << setprecision(4)
                  << theWindDataItem.theEqualizedTopWind.value();
    }
    if (theColumnSelectionBitset.test(3))
    {
      output_file << ", " << fixed << setprecision(4)
                  << theWindDataItem.theCorrectedWindDirection.value();
    }
    if (theColumnSelectionBitset.test(2))
    {
      output_file << ", " << fixed << setprecision(4)
                  << theWindDataItem.theEqualizedWindDirection.value();
    }
    if (theColumnSelectionBitset.test(1))
    {
      output_file << ", " << fixed << setprecision(4) << theWindDataItem.theWindDirection.error();
    }
    if (theColumnSelectionBitset.test(0))
    {
      output_file
          << ", "
          << directed16_speed_string(
                 theWindDataItem.theWindSpeedMean,
                 theWindDataItem.theCorrectedWindDirection,  // theWindDataItem.theWindDirection,
                 theVar);
    }
    output_file << endl;
  }
}

void print_windspeed_distribution(const WeatherArea& theArea,
                                  const std::string& fileIdentifierString,
                                  const std::string& theVar,
                                  const wind_data_item_vector& theWindDataItemVector,
                                  const vector<unsigned int>& theIndexVector)
{
  std::string filename("./" + get_area_name_string(theArea) + fileIdentifierString + ".csv");

  ofstream output_file(filename.c_str(), ios::out);

  if (output_file.fail())
  {
    throw std::runtime_error("wind_overview failed to open '" + filename + "' for writing");
  }

  const WindDataItemUnit& firstWindDataItem = (*theWindDataItemVector[0])(theArea.type());

  unsigned int numberOfWindSpeedCategories = firstWindDataItem.theWindSpeedDistribution.size();

  for (unsigned int i = 0; i < numberOfWindSpeedCategories; i++)
  {
    if (i == numberOfWindSpeedCategories - 1)
      output_file << ", > " << i + 1 << " m/s";
    else
      output_file << ", " << i + 1 << " m/s";
  }
  output_file << endl;

  for (unsigned int i = 0; i < theIndexVector.size(); i++)
  {
    const unsigned int& index = theIndexVector[i];

    const WindDataItemUnit& theWindDataItem = (*theWindDataItemVector[index])(theArea.type());

    output_file << theWindDataItem.thePeriod.localEndTime();

    for (unsigned int k = 0; k < numberOfWindSpeedCategories; k++)
    {
      output_file << ", ";
      output_file << fixed << setprecision(2) << theWindDataItem.getWindSpeedShare(k, k + 1);
    }
    output_file << endl;
  }
}

void print_winddirection_distribution(const WeatherArea& theArea,
                                      const std::string& fileIdentifierString,
                                      wo_story_params& storyParams)
{
  std::string filename("./" + get_area_name_string(storyParams.theArea) + fileIdentifierString +
                       ".csv");

  ofstream output_file(filename.c_str(), ios::out);

  if (output_file.fail())
  {
    throw std::runtime_error("wind_overview failed to open '" + filename + "' for writing");
  }

  const vector<unsigned int>& theIndexVector =
      storyParams.originalWindDataIndexes(storyParams.theArea.type());

  for (unsigned int i = POHJOINEN; i <= POHJOINEN_LUODE; i++)
  {
    output_file << ", ";
    output_file << wind_direction_string(static_cast<WindDirectionId>(i));
  }
  output_file << ", ";
  output_file << "keskihajonta";
  output_file << endl;

  for (unsigned int i = 0; i < theIndexVector.size(); i++)
  {
    const unsigned int& index = theIndexVector[i];

    const WindDataItemUnit& theWindDataItem =
        (*storyParams.theWindDataVector[index])(theArea.type());

    output_file << theWindDataItem.thePeriod.localEndTime();

    for (unsigned int i = POHJOINEN; i <= POHJOINEN_LUODE; i++)
    {
      output_file << ", ";
      output_file << fixed << setprecision(2)
                  << theWindDataItem.getWindDirectionShare(static_cast<WindDirectionId>(i),
                                                           storyParams.theWindDirectionMinSpeed);
    }
    output_file << ", ";
    output_file << fixed << setprecision(2) << theWindDataItem.theWindDirection.error();
    output_file << endl;
  }
}

void print_html_table(const WeatherArea::Type& theAreaType,
                      const std::string& fileIdentifierString,
                      const std::string& theVar,
                      const wind_data_item_vector& theWindDataItemVector,
                      const vector<unsigned int>& theIndexVector,
                      const bitset<14>& theColumnSelectionBitset)
{
  std::string filename("./" + get_area_type_string(theAreaType) + fileIdentifierString + ".html");

  ofstream output_file(filename.c_str(), ios::out);

  if (output_file.fail())
  {
    throw std::runtime_error("wind_overview failed to open '" + filename + "' for writing");
  }

  output_file << "<h1>" << get_area_type_string(theAreaType) << "</h1>" << endl;
  output_file << "<table border=\"1\">" << endl;

  const char* column_names[] = {"aika",
                                "min",
                                "max",
                                "mediaani",
                                "tasoitettu mediaani",
                                "ka",
                                "nopeuden k-hajonta",
                                "puuska",
                                "maksimituuli",
                                "tasoitettu maksimituuli",
                                "suunta",
                                "tasoitettu suunta",
                                "suunnan k-hajonta",
                                "fraasi"};

  output_file << "<tr>" << endl;
  const unsigned int number_of_columns = 14;
  for (unsigned int i = 0; i < number_of_columns; i++)
    if (theColumnSelectionBitset.test(number_of_columns - i - 1))
    {
      output_file << "<td>";
      output_file << column_names[i];
      output_file << "</td>";
    }
  output_file << "</tr>" << endl;

  for (unsigned int i = 0; i < theIndexVector.size(); i++)
  {
    output_file << "<tr>" << endl;
    const unsigned int& index = theIndexVector[i];

    const WindDataItemUnit& theWindDataItem = (*theWindDataItemVector[index])(theAreaType);
    if (theColumnSelectionBitset.test(13))
    {
      output_file << "<td style=\"white-space: nowrap;\">";
      output_file << theWindDataItem.thePeriod.localEndTime();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(12))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theWindSpeedMin.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(11))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theWindSpeedMax.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(10))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theWindSpeedMedian.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(9))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theEqualizedMedianWind.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(8))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theWindSpeedMean.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(7))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theWindSpeedMean.error();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(6))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theGustSpeed.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(5))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theWindSpeedTop.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(4))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theEqualizedTopWind.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(3))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theCorrectedWindDirection.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(2))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theEqualizedWindDirection.value();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(1))
    {
      output_file << "<td>";
      output_file << fixed << setprecision(2) << theWindDataItem.theWindDirection.error();
      output_file << "</td>" << endl;
    }
    if (theColumnSelectionBitset.test(0))
    {
      output_file << "<td style=\"white-space: nowrap;\">";
      output_file << directed16_speed_string(
          theWindDataItem.theWindSpeedMean, theWindDataItem.theCorrectedWindDirection, theVar);
      output_file << "</td>" << endl;
    }
    output_file << "</tr>" << endl;
  }  // for

  output_file << "</table>";
}

void save_raw_data(wo_story_params& storyParams, const string& id_str = "_original")
{
  bitset<14> columnSelectionBitset;

  // aika == 13
  // min, == 12
  // max == 11
  // mediaani == 10
  // tasoitettu mediaani == 9
  // ka == 8
  // nopeuden k-hajonta == 7
  // puuska == 6
  // maksimituuli == 5
  // tasoitettu maksimituuli == 4
  // suunta == 3
  // tasoitettu suunta == 2
  // suunnan k-hajonta == 1
  // fraasi == 0

  for (unsigned int i = 0; i < storyParams.theWeatherAreas.size(); i++)
  {
    const WeatherArea& weatherArea = storyParams.theWeatherAreas[i];

    columnSelectionBitset.set();
    print_csv_table(weatherArea,
                    id_str,
                    storyParams.theVar,
                    storyParams.theWindDataVector,
                    storyParams.originalWindDataIndexes(storyParams.theArea.type()),
                    columnSelectionBitset);

    print_windspeed_distribution(weatherArea,
                                 "_windspeed_distribution",
                                 storyParams.theVar,
                                 storyParams.theWindDataVector,
                                 storyParams.originalWindDataIndexes(storyParams.theArea.type()));

    print_winddirection_distribution(weatherArea, "_winddirection_distribution", storyParams);
  }
}

std::string get_csv_data(wo_story_params& storyParams, const std::string& param)
{
  std::stringstream csv_data;
  if (param.empty())
    csv_data << std::endl
             << "time,min,max,eq-max,median,eq-median,top-wind,eq-top-wind,wind-calc,gust,"
                "direction,direction-sdev,eq-direction"
             << std::endl;
  else if (param == "windspeed")
    csv_data << std::endl << "time,median,eq-median,top-wind,eq-top-wind,wind-calc" << std::endl;
  else if (param == "winddirection")
    csv_data << std::endl << "time,direction,eq-direction" << std::endl;

  WeatherArea::Type areaType(storyParams.theWeatherAreas[0].type());

  for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
  {
    const WindDataItemUnit& windDataItem = (*storyParams.theWindDataVector[i])(areaType);

    csv_data << windDataItem.thePeriod.localStartTime();
    if (param.empty())
    {
      csv_data << ", " << fixed << setprecision(2) << windDataItem.theWindSpeedMin.value() << ", "
               << fixed << setprecision(2) << windDataItem.theWindSpeedMax.value() << ", " << fixed
               << setprecision(2) << windDataItem.theEqualizedMaxWind.value() << ", " << fixed
               << setprecision(2) << windDataItem.theWindSpeedMedian.value() << ", " << fixed
               << setprecision(2) << windDataItem.theEqualizedMedianWind.value() << ", " << fixed
               << setprecision(2) << windDataItem.theWindSpeedMean.value() << ", " << fixed
               << setprecision(2) << windDataItem.theWindSpeedMean.error() << ", " << fixed
               << setprecision(2) << windDataItem.theWindSpeedTop.value() << ", " << fixed
               << setprecision(2) << windDataItem.theEqualizedTopWind.value() << ", " << fixed
               << setprecision(2) << windDataItem.theEqualizedCalcWind.value() << ", " << fixed
               << setprecision(2) << windDataItem.theGustSpeed.value() << ", " << fixed
               << setprecision(2) << windDataItem.theCorrectedWindDirection.value() << ", " << fixed
               << setprecision(2) << windDataItem.theCorrectedWindDirection.error() << ", " << fixed
               << setprecision(2) << windDataItem.theEqualizedWindDirection.value();
    }
    else if (param == "windspeed")
    {
      csv_data << ", " << fixed << setprecision(2) << windDataItem.theWindSpeedMedian.value()
               << ", " << fixed << setprecision(2) << windDataItem.theEqualizedMedianWind.value()
               << ", " << fixed << setprecision(2) << windDataItem.theWindSpeedTop.value() << ", "
               << fixed << setprecision(2) << windDataItem.theEqualizedTopWind.value() << ", "
               << fixed << setprecision(2) << windDataItem.theEqualizedCalcWind.value();
    }
    else if (param == "winddirection")
    {
      csv_data << ", " << fixed << setprecision(2) << windDataItem.theCorrectedWindDirection.value()
               << ", " << fixed << setprecision(2)
               << windDataItem.theEqualizedWindDirection.value();
    }

    csv_data << std::endl;
  }

  return csv_data.str();
}

std::string get_html_rawdata(wo_story_params& storyParams)
{
  std::stringstream html_data;

  html_data << "<table border=\"1\">" << endl;

  html_data << std::endl
            << "<tr>" << std::endl
            << "<td>time</td><td>min</td><td>max</td><td>eq-max</td><td>median</td><td "
               "BGCOLOR=lightgreen>eq-median</td><td>mean</td><td>sdev</td><td>top-wind</td><td "
               "BGCOLOR=lightgreen>eq-top-wind</td><td "
               "BGCOLOR=gold>wind-calc</td><td>gust</td><td>direction</td><td>direction-sdev</"
               "td><td BGCOLOR=lightblue>eq-direction</td>"
            << std::endl
            << "</tr>" << std::endl;

  WeatherArea::Type areaType(storyParams.theWeatherAreas[0].type());

  for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
  {
    const WindDataItemUnit& windDataItem = (*storyParams.theWindDataVector[i])(areaType);

    html_data
        << "<tr>"
        << "<td>" << windDataItem.thePeriod.localStartTime() << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theWindSpeedMin.value() << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theWindSpeedMax.value() << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theEqualizedMaxWind.value() << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theWindSpeedMedian.value() << "</td>"
        << "<td BGCOLOR=lightgreen>" << fixed << setprecision(2)
        << windDataItem.theEqualizedMedianWind.value() << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theWindSpeedMean.value() << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theWindSpeedMean.error() << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theWindSpeedTop.value() << "</td>"
        << "<td BGCOLOR=lightgreen>" << fixed << setprecision(2)
        << windDataItem.theEqualizedTopWind.value() << "</td>"
        << "<td BGCOLOR=gold>" << fixed << setprecision(2) << windDataItem.theWindSpeedCalc.value()
        << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theGustSpeed.value() << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theCorrectedWindDirection.value()
        << "</td>"
        << "<td>" << fixed << setprecision(2) << windDataItem.theCorrectedWindDirection.error()
        << "</td>"
        << "<td BGCOLOR=lightblue>" << fixed << setprecision(2)
        << windDataItem.theEqualizedWindDirection.value() << "</td>" << std::endl
        << "</tr>" << std::endl;
  }

  html_data << "</table>" << std::endl;

  return html_data.str();
}

std::string get_html_winddirection_distribution(
    wo_story_params& storyParams, WindStoryTools::CompassType compass_type = sixteen_directions)
{
  std::stringstream html_data;

  html_data << "<h5>Wind direction distribution - "
            << (compass_type == sixteen_directions ? "16-compass" : "8-compass") << "</h5>" << endl;
  html_data << "<font face=\"Serif\" size=\"2\" color=\"darkblue\">" << endl;
  html_data << "<p>" << endl;
  if (compass_type == sixteen_directions)
  {
    html_data << "N=348.75-11.25 / n-ne=11.25-33.75 / NE=33.75-56.25 / "
                 "ne-e=56.25-78.75 / "
                 "E=78.75-101.25 / e-se=101.25-123.75 / SE=123.75-146.25 / "
                 "se-s=146.25-168.75</br>"
              << endl;
    html_data << "S=168.75-191.25 / s-sw=191.25-213.75 / SW=213.75-236.25 / "
                 "sw-w=236.25-258.75 / "
                 "W=258.75-281.25 / w-nw=281.25-303.75 / NW=303.75-326.25 / "
                 "nw-n=326.25-348.75</br>"
              << endl;
  }
  else
  {
    html_data << "N=337.50-22.50 / NE=22.20-67.50 / E=67.50-112.50 SE=112.50-157.50 / "
                 "S=157.50-202.50</br>"
              << endl;
    html_data << "SW=202.50-247.50 W=247.50-292.50 NW=292.50-337.50</br>" << endl;
  }

  html_data << "</p>" << endl;
  html_data << "</font>" << endl;
  html_data << "<table border=\"1\">" << endl;

  if (compass_type == sixteen_directions)
  {
    html_data << std::endl
              << "<tr>" << std::endl
              << "<td>time</td><td>N</td><td>n-ne</td><td>NE</td><td>ne-e</td><td>E</"
                 "td><td>e-se</td><td>SE</td><td>se-s</td><td>S</td><td>s-sw</td><td>SW</"
                 "td><td>sw-w</td><td>W</td><td>w-nw</td><td>NW</"
                 "td><td>nw-n</td><td>mean</td><td>sdev</td>"
              << std::endl
              << "</tr>" << std::endl;
  }
  else
  {
    html_data << std::endl
              << "<tr>" << std::endl
              << "<td>time</td><td>N</td><td>NE</td><td>E</td><td>SE</td><td>S</td><td>SW</"
                 "td><td>W</td><td>NW</td><td>mean</td><td>sdev</td>"
              << std::endl
              << "</tr>" << std::endl;
  }

  const wind_data_item_vector& theWindDataItemVector(storyParams.theWindDataVector);
  const vector<unsigned int>& theIndexVector(
      storyParams.originalWindDataIndexes(storyParams.theArea.type()));

  for (unsigned int i = 0; i < theIndexVector.size(); i++)
  {
    const unsigned int& index = theIndexVector[i];

    const WindDataItemUnit& theWindDataItem =
        (*theWindDataItemVector[index])(storyParams.theArea.type());

    html_data << "<tr>" << std::endl
              << "<td>" << theWindDataItem.thePeriod.localEndTime() << "</td>";

    for (unsigned int i = POHJOINEN; i <= POHJOINEN_LUODE;
         i += (compass_type == sixteen_directions ? 1 : 2))
    {
      double share = theWindDataItem.getWindDirectionShare(
          static_cast<WindDirectionId>(i), storyParams.theWindDirectionMinSpeed, compass_type);
      std::string cell_effect("<td>");
      if (share > 50.0)
        cell_effect = "<td BGCOLOR=\"#FF9A9A\">";
      else if (share > 0.0)
        cell_effect = "<td BGCOLOR=lightgreen>";
      html_data << cell_effect << fixed << setprecision(2) << share << "</td>";
    }

    float direction(theWindDataItem.theCorrectedWindDirection.value());
    html_data << "<td>" << fixed << setprecision(2) << direction << "("
              << get_direction_abbreviation(direction, compass_type) << ")</td>" << std::endl
              << "<td>" << fixed << setprecision(2) << theWindDataItem.theWindDirection.error()
              << "</td>" << std::endl
              << "</tr>" << std::endl;
  }

  html_data << "</table>" << std::endl;

  return html_data.str();
}

std::string get_html_windspeed_distribution(wo_story_params& storyParams, std::string type)
{
  if (storyParams.theWindDataVector.size() == 0) return "";

  std::stringstream html_data;

  html_data << "<h5>Wind speed distribution - " + type + " wind </h5>" << endl;
  html_data << "<table border=\"1\">" << endl;

  const wind_data_item_vector& theWindDataItemVector(storyParams.theWindDataVector);
  const vector<unsigned int>& theIndexVector(
      storyParams.originalWindDataIndexes(storyParams.theArea.type()));

  const WindDataItemUnit& firstWindDataItem =
      (*theWindDataItemVector[0])(storyParams.theArea.type());
  unsigned int numberOfWindSpeedCategories =
      (type == "mean" ? firstWindDataItem.theWindSpeedDistribution.size()
                      : firstWindDataItem.theWindSpeedDistributionTop.size());

  // show only categories where wind exists plus couple of additional columns beyond
  unsigned int start_index(33), end_index(0);
  for (unsigned int i = 0; i < theIndexVector.size(); i++)
  {
    const unsigned int& index = theIndexVector[i];
    const WindDataItemUnit& theWindDataItem =
        (*theWindDataItemVector[index])(storyParams.theArea.type());

    for (float k = 0; k < numberOfWindSpeedCategories; k++)
    {
      if ((type == "mean"
               ? theWindDataItem.getWindSpeedShare((k == 0 ? 0.0 : k - 0.5),
                                                   (k == 0 ? 0.5 : k + 0.5)) > 0.0
               : theWindDataItem.getTopWindSpeedShare((k == 0 ? 0.0 : k - 0.5),
                                                      (k == 0 ? 0.5 : k + 0.5)) > 0.0))
      {
        if (k < start_index) start_index = k;
        if (k > end_index) end_index = k;
      }
    }
  }

  start_index = (start_index <= 2 ? 0 : start_index - 2);
  end_index = (end_index + 3 >= 33 ? 33 : end_index + 3);

  for (unsigned int i = 0; i < theIndexVector.size(); i++)
  {
    const unsigned int& index = theIndexVector[i];
    const WindDataItemUnit& theWindDataItem =
        (*theWindDataItemVector[index])(storyParams.theArea.type());

    // otsikkorivi
    if (i == 0)
    {
      html_data << std::endl << "<tr>" << std::endl << "<td>time</td>";
      for (unsigned int k = start_index; k < end_index; k++)
        html_data << "<td>" << k << "m/s</td>";
      html_data << std::endl << "</tr>" << std::endl;
    }

    // datarivit
    html_data << std::endl
              << "<tr>" << std::endl
              << "<td>" << theWindDataItem.thePeriod.localEndTime() << "</td>";

    for (unsigned int k = start_index; k < end_index; k++)
    {
      double share = (type == "mean"
                          ? theWindDataItem.getWindSpeedShare((k == 0 ? 0.0 : k - 0.5),
                                                              (k == 0 ? 0.5 : k + 0.5))
                          : theWindDataItem.getTopWindSpeedShare((k == 0 ? 0.0 : k - 0.5),
                                                                 (k == 0 ? 0.5 : k + 0.5)));
      std::string cell_effect("<td>");
      if (share > 10.0)
        cell_effect = "<td BGCOLOR=\"#FF9A9A\">";
      else if (share > 0.0)
        cell_effect = "<td BGCOLOR=lightgreen>";

      html_data << cell_effect << fixed << setprecision(2) << share << "</td>";
    }

    html_data << std::endl << "</tr>" << std::endl;
  }

  html_data << "</table>" << std::endl;

  return html_data.str();
}

std::string get_js_code(unsigned int js_id, bool addExternalScripts, unsigned int peakWindSpeed)
{
  std::stringstream js_code;

  if (addExternalScripts)
    js_code << std::endl
            << "<script src=\"http://code.jquery.com/jquery-1.12.0.min.js\"></script>" << std::endl
            << "<script src=\"https://code.highcharts.com/highcharts.js\"></script>" << std::endl
            << "<script src=\"https://code.highcharts.com/modules/data.js\"></script>" << std::endl
            << "<script src=\"https://code.highcharts.com/modules/exporting.js\"></script>"
            << std::endl;

  js_code << "<script>"
             "$(function () {"
             "$('#ws_container"
          << js_id << "').highcharts({yAxis: { title: {text: 'm/s' }, min:0, max:" << peakWindSpeed
          << ", tickInterval: 2.5},"
             "title: {"
             "text: 'Wind speed'"
             "},"
             "data: {"
             "csv: document.getElementById('csv"
          << js_id << "').innerHTML"
                      "},"
                      "plotOptions: {"
                      "series: {"
                      "marker: {"
                      "enabled: false"
                      "}"
                      "}"
                      "},"
                      "series: [{"
                      "lineWidth: 1"
                      "}, {"
                      "type: 'line',"
                      "color: '#c4392d',"
                      "negativeColor: '#5679c4',"
                      "fillOpacity: 0.5"
                      "}]"
                      "});"
                      "});"
                      "$(function () {"
                      "$('#wd_container"
          << js_id + 1
          << "').highcharts({yAxis: { title: {text: 'degrees' }, min:0, max:360, tickInterval: 45},"
             "title: {"
             "text: 'Wind direction'"
             "},"
             "data: {"
             "csv: document.getElementById('csv"
          << js_id + 1 << "').innerHTML"
                          "},"
                          "plotOptions: {"
                          "series: {"
                          "marker: {"
                          "enabled: false"
                          "}"
                          "}"
                          "},"
                          "series: [{"
                          "lineWidth: 1"
                          "}, {"
                          "type: 'line',"
                          "color: '#c4392d',"
                          "negativeColor: '#5679c4',"
                          "fillOpacity: 0.5"
                          "}]"
                          "});"
                          "});"
                          "</script>"
          << std::endl;

  return js_code.str();
}

std::string get_js_data(wo_story_params& storyParams, const std::string& param, unsigned int js_id)
{
  std::stringstream js_data;

  js_data << "<div id=\""
          << std::string(param.empty() || param == "windspeed" ? "ws_container" : "wd_container")
          << js_id << "\" style=\"min-width: 310px; height: 400px; margin: 0 auto\"></div>"
          << std::endl
          << std::endl
          << "<pre id=\"csv" << js_id << "\" style=\"display:none\">" << std::endl
          << get_csv_data(storyParams, param) << std::endl
          << "</pre>" << std::endl;

  return js_data.str();
}

void generate_csv_file(wo_story_params& storyParams)
{
  for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
  {
    std::stringstream csv_data;
    csv_data << std::endl
             << "time,min,max,eq-max,median,eq-median,mean,sdev,top-wind,eq-top-wind,direction,"
                "direction-sdev,eq-direction,gust"
             << std::endl;

    WeatherArea::Type areaType(storyParams.theWeatherAreas[k].type());

    for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
    {
      const WindDataItemUnit& windDataItem = (*storyParams.theWindDataVector[i])(areaType);

      csv_data << windDataItem.thePeriod.localStartTime() << ", " << fixed << setprecision(4)
               << windDataItem.theWindSpeedMin.value() << ", " << fixed << setprecision(4)
               << windDataItem.theWindSpeedMax.value() << ", " << fixed << setprecision(4)
               << windDataItem.theEqualizedMaxWind.value() << ", " << fixed << setprecision(4)
               << windDataItem.theWindSpeedMedian.value() << ", " << fixed << setprecision(4)
               << windDataItem.theEqualizedMedianWind.value() << ", " << fixed << setprecision(4)
               << windDataItem.theWindSpeedMean.value() << ", " << fixed << setprecision(4)
               << windDataItem.theWindSpeedMean.error() << ", " << fixed << setprecision(4)
               << windDataItem.theWindSpeedTop.value() << ", " << fixed << setprecision(4)
               << windDataItem.theEqualizedTopWind.value() << ", " << fixed << setprecision(4)
               << windDataItem.theCorrectedWindDirection.value() << ", " << fixed << setprecision(4)
               << windDataItem.theCorrectedWindDirection.error() << ", " << fixed << setprecision(4)
               << windDataItem.theEqualizedWindDirection.value() << ", " << fixed << setprecision(4)
               << windDataItem.theGustSpeed.value() << std::endl;
    }

    std::string csv_filename =
        Settings::require_string("qdtext::outdir") + "html/" + storyParams.theArea.name() + ".csv";
    ofstream output_file(csv_filename.c_str(), ios::out);

    if (output_file.fail())
    {
      throw std::runtime_error("wind_overview failed to open '" + csv_filename + "' for writing");
    }
    output_file << csv_data.str();
  }
}

void log_raw_data(wo_story_params& storyParams)
{
  for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
  {
    WeatherArea::Type areaType(storyParams.theWeatherAreas[k].type());
    std::string areaIdentifier(get_area_type_string(areaType));

    storyParams.theLog << "*********** RAW DATA (" << areaIdentifier << ") ***********" << endl;

    for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
    {
      const WindDataItemUnit& windDataItem = (*storyParams.theWindDataVector[i])(areaType);
      WindDirectionId windDirectionId = wind_direction_id(windDataItem.theCorrectedWindDirection,
                                                          windDataItem.theEqualizedTopWind,
                                                          storyParams.theVar,
                                                          storyParams.theWindDirectionMinSpeed);
      std::stringstream ss;
      printDataItem(ss, windDataItem, storyParams.theWindDirectionMinSpeed);
      storyParams.theLog << ss.str();

      storyParams.theLog << "; fraasi: " << wind_direction_string(windDirectionId) << endl;
    }
  }
}

void log_windirection_distribution(wo_story_params& storyParams)
{
  std::string areaIdentifier(get_area_type_string(storyParams.theArea.type()));
  const wind_data_item_vector& theWindDataItemVector(storyParams.theWindDataVector);
  const vector<unsigned int>& theIndexVector(
      storyParams.originalWindDataIndexes(storyParams.theArea.type()));

  stringstream ss;
  ss << endl
     << std::setw(75) << std::setfill('*') << " WINDDIRECTION DISTRIBUTION "
     << storyParams.theArea.name() << " " << std::setw(50) << std::setfill('*') << " " << endl;

  ss << std::setw(31) << std::setfill(' ') << std::right << "POH" << std::setw(12)
     << std::setfill(' ') << std::right << "KOI" << std::setw(12) << std::setfill(' ') << std::right
     << "ITÄ" << std::setw(12) << std::setfill(' ') << std::right << "KAA" << std::setw(12)
     << std::setfill(' ') << std::right << "ETE" << std::setw(12) << std::setfill(' ') << std::right
     << "LOU" << std::setw(12) << std::setfill(' ') << std::right << "LÄN" << std::setw(12)
     << std::setfill(' ') << std::right << "LUO" << std::setw(12) << std::setfill(' ') << std::right
     << "hajonta" << endl;

  for (unsigned int i = 0; i < theIndexVector.size(); i++)
  {
    const unsigned int& index = theIndexVector[i];

    const WindDataItemUnit& theWindDataItem =
        (*theWindDataItemVector[index])(storyParams.theArea.type());

    ss << theWindDataItem.thePeriod.localEndTime();

    for (unsigned int i = POHJOINEN; i <= POHJOINEN_LUODE; i++)
    {
      ss << std::setw(i == POHJOINEN ? 12 : 6) << std::setfill(' ') << std::fixed << std::right
         << setprecision(2)
         << theWindDataItem.getWindDirectionShare(static_cast<WindDirectionId>(i),
                                                  storyParams.theWindDirectionMinSpeed);
    }

    ss << std::setw(6) << std::setfill(' ') << std::fixed << std::right << setprecision(2)
       << theWindDataItem.theWindDirection.error() << endl;
  }

  storyParams.theLog << ss.str() << endl;
}

void log_wind_data_vector(wo_story_params& storyParams,
                          const vector<unsigned int>& indexVector,
                          WeatherArea::Type areaType)
{
  for (unsigned int i = 0; i < indexVector.size(); i++)
  {
    unsigned int index = indexVector[i];
    const WindDataItemUnit& windDataItem = (*storyParams.theWindDataVector[index])(areaType);
    std::stringstream ss;
    printDataItem(ss, windDataItem, storyParams.theWindDirectionMinSpeed);
    storyParams.theLog << ss.str() << " " << std::endl;
  }
}

void log_equalized_wind_speed_data_vector(wo_story_params& storyParams)
{
  for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
  {
    WeatherArea::Type areaType(storyParams.theWeatherAreas[k].type());
    std::string areaIdentifier(get_area_type_string(areaType));

    storyParams.theLog << "*********** EQUALIZED MEDIAN WIND SPEED DATA (" << areaIdentifier
                       << ") ***********" << endl;

    const vector<unsigned int>& indexVector = storyParams.equalizedWSIndexesMedian(areaType);

    log_wind_data_vector(storyParams, indexVector, areaType);
  }

  for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
  {
    WeatherArea::Type areaType(storyParams.theWeatherAreas[k].type());
    std::string areaIdentifier(get_area_type_string(areaType));

    storyParams.theLog << "*********** EQUALIZED MAXIMUM WIND SPEED DATA (" << areaIdentifier
                       << ") ***********" << endl;

    vector<unsigned int>& indexVector = storyParams.equalizedWSIndexesMaxWind(areaType);

    log_wind_data_vector(storyParams, indexVector, areaType);
  }

  for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
  {
    WeatherArea::Type areaType(storyParams.theWeatherAreas[k].type());
    std::string areaIdentifier(get_area_type_string(areaType));

    storyParams.theLog << "*********** EQUALIZED TOP WIND SPEED DATA (" << areaIdentifier
                       << ") ***********" << endl;

    vector<unsigned int>& indexVector = storyParams.equalizedWSIndexesTopWind(areaType);

    log_wind_data_vector(storyParams, indexVector, areaType);
  }
}

void log_equalized_wind_direction_data_vector(wo_story_params& storyParams)
{
  for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
  {
    WeatherArea::Type areaType(storyParams.theWeatherAreas[k].type());
    std::string areaIdentifier(get_area_type_string(areaType));

    storyParams.theLog << "*********** EQUALIZED WIND DIRECTION DATA (" << areaIdentifier
                       << ") ***********" << endl;

    vector<unsigned int>& indexVector = storyParams.equalizedWDIndexes(areaType);

    log_wind_data_vector(storyParams, indexVector, areaType);
  }
}

void log_wind_speed_periods(wo_story_params& storyParams)
{
  storyParams.theLog << "*********** WIND SPEED PERIODS ***********" << endl;
  for (unsigned int i = 0; i < storyParams.theWindSpeedVector.size(); i++)
  {
    storyParams.theLog << *(storyParams.theWindSpeedVector[i]);
  }
}

void log_wind_direction_periods(wo_story_params& storyParams)
{
  storyParams.theLog << "*********** WIND DIRECTION PERIODS ***********" << endl;
  for (unsigned int i = 0; i < storyParams.theWindDirectionVector.size(); i++)
  {
    storyParams.theLog << *(storyParams.theWindDirectionVector[i]);
  }
}

void log_wind_event_periods(wo_story_params& storyParams)
{
  storyParams.theLog << "*********** WIND EVENT PERIODS ***********" << endl;

  wind_event_period_data_item_vector windEventPeriodVector;
  windEventPeriodVector.insert(windEventPeriodVector.end(),
                               storyParams.theWindSpeedEventPeriodVector.begin(),
                               storyParams.theWindSpeedEventPeriodVector.end());

  std::sort(windEventPeriodVector.begin(), windEventPeriodVector.end(), wind_event_period_sort);

  for (unsigned int i = 0; i < windEventPeriodVector.size(); i++)
  {
    WindEventId windEventId = windEventPeriodVector[i]->theWindEvent;

    storyParams.theLog << windEventPeriodVector[i]->thePeriod.localStartTime() << "..."
                       << windEventPeriodVector[i]->thePeriod.localEndTime() << ": "
                       << get_wind_event_string(windEventId) << " ";

    if (windEventId == TUULI_HEIKKENEE || windEventId == TUULI_VOIMISTUU ||
        windEventId == TUULI_TYYNTYY || windEventId == MISSING_WIND_SPEED_EVENT)
    {
      float maxWindBeg =
          windEventPeriodVector[i]->thePeriodBeginDataItem.theEqualizedTopWind.value();
      float maxWindEnd = windEventPeriodVector[i]->thePeriodEndDataItem.theEqualizedTopWind.value();
      float medianWindBeg =
          windEventPeriodVector[i]->thePeriodBeginDataItem.theEqualizedMedianWind.value();
      float medianWindEnd =
          windEventPeriodVector[i]->thePeriodEndDataItem.theEqualizedMedianWind.value();

      storyParams.theLog << ": " << fixed << setprecision(2) << medianWindBeg << "..." << fixed
                         << setprecision(2) << maxWindBeg << " -> " << fixed << setprecision(2)
                         << medianWindEnd << "..." << fixed << setprecision(2) << maxWindEnd
                         << endl;
    }
    else if (windEventId == TUULI_KAANTYY || windEventId == TUULI_MUUTTUU_VAIHTELEVAKSI ||
             windEventId == MISSING_WIND_DIRECTION_EVENT)
    {
      WeatherResult directionBeg(
          windEventPeriodVector[i]->thePeriodBeginDataItem.theEqualizedWindDirection);
      WeatherResult directionEnd(
          windEventPeriodVector[i]->thePeriodEndDataItem.theEqualizedWindDirection);
      WeatherResult speedBeg(windEventPeriodVector[i]->thePeriodBeginDataItem.theEqualizedTopWind);
      WeatherResult speedEnd(windEventPeriodVector[i]->thePeriodEndDataItem.theEqualizedTopWind);

      WindDirectionId directionIdBeg = wind_direction_id(
          directionBeg, speedBeg, storyParams.theVar, storyParams.theWindDirectionMinSpeed);
      WindDirectionId directionIdEnd = wind_direction_id(
          directionEnd, speedEnd, storyParams.theVar, storyParams.theWindDirectionMinSpeed);

      storyParams.theLog << ": " << wind_direction_string(directionIdBeg) << "->"
                         << wind_direction_string(directionIdEnd) << endl;
    }
    else
    {
      storyParams.theLog << " " << endl;
    }
  }
}

void allocate_data_structures(wo_story_params& storyParams)
{
  TextGenPosixTime periodStartTime = storyParams.theDataPeriod.localStartTime();

  // also the last hour is included
  while (periodStartTime <= storyParams.theDataPeriod.localEndTime())
  {
    WeatherPeriod weatherPeriod(periodStartTime, periodStartTime);
    WeatherResult minWind(kFloatMissing, kFloatMissing);
    WeatherResult meanWind(kFloatMissing, kFloatMissing);
    WeatherResult medianWind(kFloatMissing, kFloatMissing);
    WeatherResult maxWind(kFloatMissing, kFloatMissing);
    WeatherResult maximumWind(kFloatMissing, kFloatMissing);
    WeatherResult windDirection(kFloatMissing, kFloatMissing);
    WeatherResult gustSpeed(kFloatMissing, kFloatMissing);

    WindDataItemsByArea* dataItemsByArea = new WindDataItemsByArea();

    for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
    {
      WeatherArea::Type type = storyParams.theWeatherAreas[k].type();

      dataItemsByArea->addItem(weatherPeriod,
                               minWind,
                               maxWind,
                               meanWind,
                               medianWind,
                               maximumWind,
                               windDirection,
                               gustSpeed,
                               type);
    }

    storyParams.theWindDataVector.push_back(dataItemsByArea);

    periodStartTime.ChangeByHours(1);
  }

  for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
  {
    WeatherArea::Type type = storyParams.theWeatherAreas[k].type();

    storyParams.indexes.insert(make_pair(type, new index_vectors()));
  }
}

void deallocate_data_structures(wo_story_params& storyParams)
{
  for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
  {
    delete storyParams.theWindDataVector[i];
  }
  storyParams.theWindDataVector.clear();

  for (unsigned int i = 0; i < storyParams.theWindSpeedVector.size(); i++)
  {
    delete storyParams.theWindSpeedVector[i];
  }
  storyParams.theWindSpeedVector.clear();

  for (unsigned int i = 0; i < storyParams.theWindDirectionVector.size(); i++)
  {
    delete storyParams.theWindDirectionVector[i];
  }
  storyParams.theWindDirectionVector.clear();

  for (unsigned int i = 0; i < storyParams.theWindSpeedEventPeriodVector.size(); i++)
  {
    delete storyParams.theWindSpeedEventPeriodVector[i];
  }
  storyParams.theWindSpeedEventPeriodVector.clear();

  for (unsigned int i = 0; i < storyParams.theWeatherAreas.size(); i++)
  {
    delete storyParams.indexes[storyParams.theWeatherAreas[i].type()];
  }
  storyParams.indexes.clear();
}

void populate_windspeed_distribution_time_series(
    const AnalysisSources& theSources,
    const WeatherArea& theArea,
    const WeatherPeriod& thePeriod,
    const string& theVar,
    vector<pair<float, WeatherResult> >& theWindSpeedDistribution,
    vector<pair<float, WeatherResult> >& theWindSpeedDistributionTop)
{
  GridForecaster forecaster;

  float ws_lower_limit(0.0);
  float ws_upper_limit(0.5);

  while (ws_lower_limit < HIRMUMYRSKY_LOWER_LIMIT)
  {
    RangeAcceptor acceptor;
    acceptor.lowerLimit(ws_lower_limit);
    if (ws_lower_limit < HIRMUMYRSKY_LOWER_LIMIT + 1) acceptor.upperLimit(ws_upper_limit - 0.0001);

    WeatherResult share = forecaster.analyze(theVar + "::fake::tyyni::share",
                                             theSources,
                                             WindSpeed,
                                             Mean,
                                             Percentage,
                                             theArea,
                                             thePeriod,
                                             DefaultAcceptor(),
                                             DefaultAcceptor(),
                                             acceptor);

    pair<float, WeatherResult> shareItem(ws_lower_limit, share);
    theWindSpeedDistribution.push_back(shareItem);

    share = forecaster.analyze(theVar + "::fake::tyyni::share",
                               theSources,
                               MaximumWind,
                               Mean,
                               Percentage,
                               theArea,
                               thePeriod,
                               DefaultAcceptor(),
                               DefaultAcceptor(),
                               acceptor);

    pair<float, WeatherResult> shareItemTop(ws_lower_limit, share);
    theWindSpeedDistributionTop.push_back(shareItemTop);

    ws_lower_limit += (ws_lower_limit == 0.0 ? 0.5 : 1.0);
    ws_upper_limit += 1.0;
  }
}

bool populate_time_series(wo_story_params& storyParams)
{
  GridForecaster forecaster;

  for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
  {
    for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
    {
      const WeatherArea& weatherArea = storyParams.theWeatherAreas[k];
      WeatherArea::Type areaType(weatherArea.type());

      WindDataItemUnit& dataItem = (storyParams.theWindDataVector[i])->getDataItem(areaType);

      dataItem.theWindSpeedMin =
          forecaster.analyze(storyParams.theVar + "::fake::wind::speed::minimum",
                             storyParams.theSources,
                             WindSpeed,
                             Minimum,
                             Mean,
                             weatherArea,
                             dataItem.thePeriod);

      if (dataItem.theWindSpeedMin.value() == kFloatMissing)
      {
        storyParams.theLog << "Minimum wind speed value missing, cannot continue!" << endl;
        return false;
      }

      dataItem.theWindSpeedMax =
          forecaster.analyze(storyParams.theVar + "::fake::wind::speed::maximum",
                             storyParams.theSources,
                             WindSpeed,
                             Maximum,
                             Mean,
                             weatherArea,
                             dataItem.thePeriod);

      if (dataItem.theWindSpeedMax.value() == kFloatMissing)
      {
        storyParams.theLog << "Maximum wind speed value missing, cannot continue!" << endl;
        return false;
      }

      dataItem.theEqualizedMaxWind = dataItem.theWindSpeedMax;

      dataItem.theWindSpeedMean =
          forecaster.analyze(storyParams.theVar + "::fake::wind::speed::mean",
                             storyParams.theSources,
                             WindSpeed,
                             Mean,
                             Mean,
                             weatherArea,
                             dataItem.thePeriod);

      if (dataItem.theWindSpeedMean.value() == kFloatMissing)
      {
        storyParams.theLog << "Mean wind speed value missing, cannot continue!" << endl;
        return false;
      }

      dataItem.theWindSpeedMedian =
          forecaster.analyze(storyParams.theVar + "::fake::wind::medianwind",
                             storyParams.theSources,
                             WindSpeed,
                             Median,
                             Mean,
                             weatherArea,
                             dataItem.thePeriod);

      if (dataItem.theWindSpeedMedian.value() == kFloatMissing)
      {
        storyParams.theLog << "Median wind speed value missing, cannot continue!" << endl;
        return false;
      }

      dataItem.theEqualizedMedianWind = dataItem.theWindSpeedMedian;

      dataItem.theWindSpeedTop =
          forecaster.analyze(storyParams.theVar + "::fake::wind::maximumwind",
                             storyParams.theSources,
                             MaximumWind,
                             Maximum,
                             Mean,
                             weatherArea,
                             dataItem.thePeriod);

      // ARE 2012-04-26: if MaximumWind parameter is missing, use WindSpeed maximum + 1.0 instead
      if (dataItem.theWindSpeedTop.value() == kFloatMissing)
      {
        dataItem.theWindSpeedTop =
            WeatherResult(dataItem.theWindSpeedMax.value() + 1.0, dataItem.theWindSpeedMax.error());
      }

      dataItem.theEqualizedTopWind = dataItem.theWindSpeedTop;

      dataItem.theWindDirection = forecaster.analyze(storyParams.theVar + "::fake::wind:direction",
                                                     storyParams.theSources,
                                                     WindDirection,
                                                     Mean,
                                                     Mean,
                                                     weatherArea,
                                                     dataItem.thePeriod);

      if (dataItem.theWindDirection.value() == kFloatMissing)
      {
        storyParams.theLog << "Wind direction value missing, cannot continue!" << endl;
        return false;
      }

      dataItem.theCorrectedWindDirection = dataItem.theWindDirection;
      dataItem.theEqualizedWindDirection = dataItem.theWindDirection;

      dataItem.theGustSpeed = forecaster.analyze(storyParams.theVar + "::fake::gust::speed",
                                                 storyParams.theSources,
                                                 GustSpeed,
                                                 Maximum,
                                                 Mean,
                                                 weatherArea,
                                                 dataItem.thePeriod);

      if (dataItem.theGustSpeed.value() == kFloatMissing)
      {
        storyParams.theLog << "Gust speed value missing, cannot continue!" << endl;
        return false;
      }

      populate_windspeed_distribution_time_series(storyParams.theSources,
                                                  weatherArea,
                                                  dataItem.thePeriod,
                                                  storyParams.theVar,
                                                  dataItem.theWindSpeedDistribution,
                                                  dataItem.theWindSpeedDistributionTop);

      populate_winddirection_distribution_time_series(
          storyParams.theSources,
          weatherArea,
          dataItem.thePeriod,
          storyParams.theVar,
          dataItem.theWindDirectionDistribution16,
          WindStoryTools::CompassType::sixteen_directions);

      populate_winddirection_distribution_time_series(
          storyParams.theSources,
          weatherArea,
          dataItem.thePeriod,
          storyParams.theVar,
          dataItem.theWindDirectionDistribution8,
          WindStoryTools::CompassType::eight_directions);

      WeatherResult correctedDirection =
          WindStoryTools::mode_wind_direction(storyParams.theSources,
                                              weatherArea,
                                              dataItem.thePeriod,
                                              dataItem.theWindSpeedMedian,
                                              dataItem.theWindSpeedTop,
                                              storyParams.theVar);

      dataItem.theCorrectedWindDirection = correctedDirection;
      dataItem.theEqualizedWindDirection = correctedDirection;

      storyParams.originalWindDataIndexes(areaType).push_back(i);
      storyParams.equalizedWSIndexesMedian(areaType).push_back(i);
      storyParams.equalizedWSIndexesMaxWind(areaType).push_back(i);
      storyParams.equalizedWSIndexesTopWind(areaType).push_back(i);
      storyParams.equalizedWSIndexesCalcWind(areaType).push_back(i);
      storyParams.equalizedWDIndexes(areaType).push_back(i);
    }
  }

  // check if wind is weak during whole period
  storyParams.theWeakTopWind = true;
  unsigned int total_counter = 0;
  unsigned int counter = 0;

  for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
  {
    for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
    {
      const WeatherArea& weatherArea = storyParams.theWeatherAreas[k];
      WeatherArea::Type areaType(weatherArea.type());

      WindDataItemUnit& dataItem = (storyParams.theWindDataVector[i])->getDataItem(areaType);
      if (dataItem.theWindSpeedTop.value() >= storyParams.theWindSpeedWarningThreshold) counter++;
      total_counter++;
    }
  }
  // if more than 10% above 'storyParams.theWindSpeedWarningThreshold' m/s wind is not weak
  double topWindAboveWeakShare =
      ((static_cast<float>(counter) / static_cast<float>(total_counter)) * 100.0);
  if (topWindAboveWeakShare > 10.0) storyParams.theWeakTopWind = false;

  storyParams.theLog << "Top wind is " << (storyParams.theWeakTopWind ? "weak" : "NOT weak")
                     << " at period " << storyParams.theForecastPeriod << ", " << fixed
                     << setprecision(2) << topWindAboveWeakShare << "% is above "
                     << storyParams.theWindSpeedWarningThreshold << " m/s" << std::endl;

  // get calculated wind speed
  for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
  {
    for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
    {
      const WeatherArea& weatherArea = storyParams.theWeatherAreas[k];
      WeatherArea::Type areaType(weatherArea.type());

      WindDataItemUnit& dataItem = (storyParams.theWindDataVector[i])->getDataItem(areaType);

      // calculated wind speed
      dataItem.theWindSpeedCalc = WeatherResult(
          calculate_weighted_wind_speed(
              storyParams, dataItem.theWindSpeedTop.value(), dataItem.theWindSpeedMedian.value()),
          0.0);
      dataItem.theEqualizedCalcWind = dataItem.theWindSpeedCalc;
    }
  }
  return true;
}

void find_out_wind_speed_periods(wo_story_params& storyParams)

{
  unsigned int equalizedDataIndex;
  WeatherArea::Type areaType(storyParams.theArea.type());

  vector<unsigned int>& equalizedWSIndexesMedian = storyParams.equalizedWSIndexesMedian(areaType);

  if (equalizedWSIndexesMedian.size() == 0)
    return;
  else if (equalizedWSIndexesMedian.size() == 1)
  {
    equalizedDataIndex = equalizedWSIndexesMedian[0];
    const WindDataItemUnit& dataItem =
        (*storyParams.theWindDataVector[equalizedDataIndex])(areaType);
    storyParams.theWindSpeedVector.push_back(new WindSpeedPeriodDataItem(
        dataItem.thePeriod, wind_speed_id(dataItem.theEqualizedMedianWind)));
    return;
  }

  unsigned int periodStartEqualizedDataIndex = equalizedWSIndexesMedian[0];
  const WindDataItemUnit& dataItemFirst =
      (*storyParams.theWindDataVector[periodStartEqualizedDataIndex])(areaType);

  WindSpeedId previous_wind_speed_id(wind_speed_id(dataItemFirst.theEqualizedMedianWind));

  for (unsigned int i = 1; i < equalizedWSIndexesMedian.size(); i++)
  {
    equalizedDataIndex = equalizedWSIndexesMedian[i];
    const WindDataItemUnit& dataItemCurrent =
        (*storyParams.theWindDataVector[equalizedDataIndex])(areaType);

    WindSpeedId current_wind_speed_id(wind_speed_id(dataItemCurrent.theEqualizedMedianWind));

    if (current_wind_speed_id != previous_wind_speed_id)
    {
      const WindDataItemUnit& dataItemPrevious =
          (*storyParams.theWindDataVector[periodStartEqualizedDataIndex])(areaType);

      TextGenPosixTime periodStartTime(dataItemPrevious.thePeriod.localStartTime());
      TextGenPosixTime periodEndTime(dataItemCurrent.thePeriod.localStartTime());
      periodEndTime.ChangeByHours(-1);

      WeatherPeriod windSpeedPeriod(periodStartTime, periodEndTime);

      storyParams.theWindSpeedVector.push_back(
          new WindSpeedPeriodDataItem(windSpeedPeriod, previous_wind_speed_id));
      periodStartEqualizedDataIndex = equalizedDataIndex;
      previous_wind_speed_id = current_wind_speed_id;
    }
  }
  const WindDataItemUnit& dataItemBeforeLast =
      (*storyParams.theWindDataVector[periodStartEqualizedDataIndex])(areaType);
  const WindDataItemUnit& dataItemLast =
      (*storyParams.theWindDataVector[storyParams.theWindDataVector.size() - 1])(areaType);

  WeatherPeriod windSpeedPeriod(dataItemBeforeLast.thePeriod.localStartTime(),
                                dataItemLast.thePeriod.localEndTime());

  storyParams.theWindSpeedVector.push_back(
      new WindSpeedPeriodDataItem(windSpeedPeriod, previous_wind_speed_id));
}

void find_out_wind_direction_periods(wo_story_params& storyParams)
{
  WeatherArea::Type areaType(storyParams.theArea.type());

  const wind_data_item_vector& dataVector = storyParams.theWindDataVector;

  if (dataVector.size() == 0)
    return;
  else if (dataVector.size() == 1)
  {
    const WindDataItemUnit& dataItem = (*dataVector[0])(areaType);

    storyParams.theWindDirectionVector.push_back(
        new WindDirectionPeriodDataItem(dataItem.thePeriod,
                                        wind_direction_id(dataItem.theEqualizedWindDirection,
                                                          dataItem.theEqualizedTopWind,
                                                          storyParams.theVar,
                                                          storyParams.theWindDirectionMinSpeed)));
    return;
  }

  const WindDataItemUnit& dataItemFirst = (*dataVector[0])(areaType);
  WindDirectionId previous_wind_direction_id(
      wind_direction_id(dataItemFirst.theEqualizedWindDirection,
                        dataItemFirst.theEqualizedTopWind,
                        storyParams.theVar,
                        storyParams.theWindDirectionMinSpeed));

  TextGenPosixTime periodStartTime(dataItemFirst.thePeriod.localStartTime());
  TextGenPosixTime periodEndTime(dataItemFirst.thePeriod.localStartTime());

  for (unsigned int i = 1; i < dataVector.size(); i++)
  {
    const WindDataItemUnit& dataItemCurrent = (*dataVector[i])(areaType);

    WindDirectionId current_wind_direction_id(
        wind_direction_id(dataItemCurrent.theEqualizedWindDirection,
                          dataItemCurrent.theEqualizedTopWind,
                          storyParams.theVar,
                          storyParams.theWindDirectionMinSpeed));

    if (current_wind_direction_id != previous_wind_direction_id)
    {
      if (i < dataVector.size() - 1)
      {
        const WindDataItemUnit& dataItemPrevious = (*dataVector[i - 1])(areaType);
        const WindDataItemUnit& dataItemNext = (*dataVector[i + 1])(areaType);
        if (wind_direction_id(dataItemPrevious.theEqualizedWindDirection,
                              dataItemPrevious.theEqualizedTopWind,
                              storyParams.theVar,
                              storyParams.theWindDirectionMinSpeed) ==
            wind_direction_id(dataItemNext.theEqualizedWindDirection,
                              dataItemNext.theEqualizedTopWind,
                              storyParams.theVar,
                              storyParams.theWindDirectionMinSpeed))
        {
          continue;
        }
      }

      periodEndTime = dataItemCurrent.thePeriod.localStartTime();
      periodEndTime.ChangeByHours(-1);

      WeatherPeriod windDirectionPeriod(periodStartTime, periodEndTime);

      storyParams.theWindDirectionVector.push_back(
          new WindDirectionPeriodDataItem(windDirectionPeriod, previous_wind_direction_id));
      previous_wind_direction_id = current_wind_direction_id;

      periodStartTime = periodEndTime;
      periodStartTime.ChangeByHours(1);
    }
  }
  if (periodEndTime != dataItemFirst.thePeriod.localStartTime())
  {
    periodEndTime.ChangeByHours(1);
  }

  const WindDataItemUnit& dataItemLast =
      (*dataVector[storyParams.theWindDataVector.size() - 1])(areaType);

  WeatherPeriod windDirectionLastPeriod(periodEndTime, dataItemLast.thePeriod.localEndTime());

  storyParams.theWindDirectionVector.push_back(
      new WindDirectionPeriodDataItem(windDirectionLastPeriod, previous_wind_direction_id));
}

WindEventId get_wind_speed_event(float windSpeedAtStart,
                                 float windSpeedAtEnd,
                                 double windSpeedThreshold)
{
  // round the wind speed to nearest integer
  double difference = (windSpeedAtEnd - windSpeedAtStart);

  if (abs(difference) <= windSpeedThreshold)
    return MISSING_WIND_SPEED_EVENT;
  else if (difference < 0.0 && windSpeedAtEnd >= 0.0 && windSpeedAtEnd < 0.5)
    return TUULI_TYYNTYY;
  else if (difference < 0.0)
    return TUULI_HEIKKENEE;
  else
    return TUULI_VOIMISTUU;
}

WindEventId get_wind_direction_event(const WeatherResult& windDirection1,
                                     const WeatherResult& windDirection2,
                                     const WeatherResult& maximumWind1,
                                     const WeatherResult& maximumWind2,
                                     const string& var,
                                     double windDirectionThreshold,
                                     double theWindDirectionMinSpeed)
{
  WindDirectionId directionId1 =
      wind_direction_id(windDirection1, maximumWind1, var, theWindDirectionMinSpeed);
  WindDirectionId directionId2 =
      wind_direction_id(windDirection2, maximumWind2, var, theWindDirectionMinSpeed);

  if (directionId1 != VAIHTELEVA && directionId2 == VAIHTELEVA)
    return TUULI_MUUTTUU_VAIHTELEVAKSI;
  else if (directionId1 == directionId2)
    return MISSING_WIND_DIRECTION_EVENT;
  else if (directionId1 == VAIHTELEVA)
  {
    return TUULI_KAANTYY;
  }

  bool windDirectionDifferEnough =
      wind_direction_differ_enough(windDirection1, windDirection2, windDirectionThreshold);

  return (windDirectionDifferEnough && directionId1 != directionId2 ? TUULI_KAANTYY
                                                                    : MISSING_WIND_DIRECTION_EVENT);
}

WindEventId get_wind_direction_event(const WindEventPeriodDataItem& windEventPeriodDataItem,
                                     const string& var,
                                     double windDirectionThreshold,
                                     double theWindDirectionMinSpeed)

{
  const WeatherResult& windDirection1(
      windEventPeriodDataItem.thePeriodBeginDataItem.theEqualizedWindDirection);
  const WeatherResult& windDirection2(
      windEventPeriodDataItem.thePeriodEndDataItem.theEqualizedWindDirection);
  const WeatherResult& maximumWind1(
      windEventPeriodDataItem.thePeriodBeginDataItem.theEqualizedTopWind);
  const WeatherResult& maximumWind2(
      windEventPeriodDataItem.thePeriodEndDataItem.theEqualizedTopWind);

  return get_wind_direction_event(windDirection1,
                                  windDirection2,
                                  maximumWind1,
                                  maximumWind2,
                                  var,
                                  windDirectionThreshold,
                                  theWindDirectionMinSpeed);
}

// find out wind events: strengthening wind, weakening wind,
// or missing wind speed event:
// fill up the storyParams.theWindSpeedEventPeriodVector
void find_out_wind_speed_event_periods(wo_story_params& storyParams)
{
  WeatherArea::Type areaType(storyParams.theArea.type());

  const vector<unsigned int>& theEqualizedIndexes =
      storyParams.equalizedWSIndexesCalcWind(areaType);

  if (theEqualizedIndexes.size() == 0)
  {
    return;
  }
  else if (theEqualizedIndexes.size() == 1)
  {
    unsigned int dataIndex = theEqualizedIndexes[0];
    const WindDataItemUnit& dataItem = (*storyParams.theWindDataVector[dataIndex])(areaType);

    storyParams.theWindSpeedEventPeriodVector.push_back(new WindEventPeriodDataItem(
        dataItem.thePeriod, MISSING_WIND_SPEED_EVENT, dataItem, dataItem));
    return;
  }

  for (unsigned int i = 1; i < theEqualizedIndexes.size(); i++)
  {
    unsigned int periodBeginDataIndex = theEqualizedIndexes[i - 1];
    unsigned int periodEndDataIndex = theEqualizedIndexes[i];

    // find the data item for the period start and end
    const WindDataItemUnit& dataItemPeriodBegin =
        (*storyParams.theWindDataVector[periodBeginDataIndex])(areaType);
    const WindDataItemUnit& dataItemPeriodEnd =
        (*storyParams.theWindDataVector[periodEndDataIndex])(areaType);

    // define the event period
    WeatherPeriod windEventPeriod(dataItemPeriodBegin.thePeriod.localStartTime(),
                                  dataItemPeriodEnd.thePeriod.localStartTime());

    // find out wind speed event; for that we need speed at the beginnig and the end

    float begSpeed = dataItemPeriodBegin.theEqualizedCalcWind.value();
    float endSpeed = dataItemPeriodEnd.theEqualizedCalcWind.value();

    // at first use small threshold value, in the end
    // when periods has been (possibly) merged, check again with actual threshold
    WindEventId currentPeriodWindEvent = get_wind_speed_event(begSpeed, endSpeed, 0.001);

    bool firstRound(i == 1);
    WindEventPeriodDataItem* previousEventPeriod =
        (firstRound ? NULL : storyParams.theWindSpeedEventPeriodVector
                                 [storyParams.theWindSpeedEventPeriodVector.size() - 1]);

    if (firstRound || (previousEventPeriod &&
                       previousEventPeriod->theWindEvent !=
                           currentPeriodWindEvent))  // first round or different event
    {
      // add the original period
      storyParams.theWindSpeedEventPeriodVector.push_back(new WindEventPeriodDataItem(
          windEventPeriod, currentPeriodWindEvent, dataItemPeriodBegin, dataItemPeriodEnd));

      continue;
    }
    else
    {
      WeatherPeriod mergedPeriod(previousEventPeriod->thePeriod.localStartTime(),
                                 windEventPeriod.localEndTime());

      WindEventPeriodDataItem* newEventPeriod =
          new WindEventPeriodDataItem(mergedPeriod,
                                      currentPeriodWindEvent,
                                      previousEventPeriod->thePeriodBeginDataItem,
                                      dataItemPeriodEnd);

      delete previousEventPeriod;

      storyParams.theWindSpeedEventPeriodVector.erase(
          storyParams.theWindSpeedEventPeriodVector.begin() +
          storyParams.theWindSpeedEventPeriodVector.size() - 1);

      storyParams.theWindSpeedEventPeriodVector.push_back(newEventPeriod);
    }
  }

  // iterate through and check against actual threshold value that wind speed differ enough
  // if it doesnt, set event as missing
  BOOST_FOREACH (WindEventPeriodDataItem* windEventPeriodDataItem,
                 storyParams.theWindSpeedEventPeriodVector)
  {
    if (!wind_speed_differ_enough(storyParams, windEventPeriodDataItem->thePeriod))
    {
      windEventPeriodDataItem->theWindEvent = MISSING_WIND_SPEED_EVENT;
    }
  }
}

void get_calculated_max_min(const wo_story_params& storyParams,
                            const WindEventPeriodDataItem& dataItem,
                            float& max,
                            float& min)
{
  float begSpeed = calculate_weighted_wind_speed(storyParams, dataItem.thePeriodBeginDataItem);
  float endSpeed = calculate_weighted_wind_speed(storyParams, dataItem.thePeriodEndDataItem);

  if (begSpeed > max) max = begSpeed;
  if (endSpeed > max) max = endSpeed;
  if (begSpeed < min) min = begSpeed;
  if (endSpeed < min) min = endSpeed;
}

// iterate merged event periods and remove short (<= 6h) missing period if it is between
// strenghtening/weakening period
wind_event_period_data_item_vector remove_short_missing_periods(
    wo_story_params& storyParams, const wind_event_period_data_item_vector& eventPeriodVector)
{
  if (eventPeriodVector.size() <= 2) return eventPeriodVector;

  wind_event_period_data_item_vector cleanedEventPeriods;

  for (unsigned int i = 0; i < eventPeriodVector.size(); i++)
  {
    WindEventPeriodDataItem* currentDataItem = eventPeriodVector[i];

    if (i >= eventPeriodVector.size() - 2)
    {
      cleanedEventPeriods.push_back(currentDataItem);
      continue;
    }

    WindEventPeriodDataItem* nextDataItem = eventPeriodVector[i + 1];
    WindEventPeriodDataItem* afterNextDataItem = eventPeriodVector[i + 2];

    if (nextDataItem->theWindEvent == MISSING_WIND_SPEED_EVENT &&
        currentDataItem->theWindEvent == afterNextDataItem->theWindEvent)
    {
      if (get_period_length(nextDataItem->thePeriod) <= 6)
      {
        // merge the three event periods
        WeatherPeriod newPeriod(currentDataItem->thePeriod.localStartTime(),
                                afterNextDataItem->thePeriod.localEndTime());

        float begSpeed =
            calculate_weighted_wind_speed(storyParams, currentDataItem->thePeriodBeginDataItem);
        float endSpeed =
            calculate_weighted_wind_speed(storyParams, afterNextDataItem->thePeriodEndDataItem);
        WindEventId newWindEvent =
            get_wind_speed_event(begSpeed, endSpeed, storyParams.theWindSpeedThreshold);

        WindEventPeriodDataItem* newDataItem =
            new WindEventPeriodDataItem(newPeriod,
                                        newWindEvent,
                                        currentDataItem->thePeriodBeginDataItem,
                                        afterNextDataItem->thePeriodEndDataItem);
        cleanedEventPeriods.push_back(newDataItem);
        i += 2;
      }
      else
      {
        afterNextDataItem->theSuccessiveEventFlag = true;
        cleanedEventPeriods.push_back(currentDataItem);
      }
    }
    else
    {
      cleanedEventPeriods.push_back(currentDataItem);
    }
  }

  return cleanedEventPeriods;
}

typedef std::pair<float, WeatherPeriod> SpeedDifferencePeriod;
bool comp_sdp(const SpeedDifferencePeriod& sdp1, const SpeedDifferencePeriod& sdp2)
{
  return sdp1.first > sdp2.first;
}

// examine MISSING_WIND_SPEED_EVENT periods, if there are strenghtening/weakening periods inside
// separate them
wind_event_period_data_item_vector examine_merged_missing_event_period(
    wo_story_params& storyParams, const WindEventPeriodDataItem& dataItem)
{
  wind_event_period_data_item_vector ret;

  std::vector<SpeedDifferencePeriod> speedDifferenceVector;
  unsigned int periodLength = get_period_length(dataItem.thePeriod);
  for (unsigned int j = 0; j < periodLength - 1; j++)
  {
    TextGenPosixTime startTime = dataItem.thePeriod.localStartTime();
    startTime.ChangeByHours(j);
    for (unsigned int k = j + 1; k < periodLength; k++)
    {
      TextGenPosixTime endTime = startTime;
      endTime.ChangeByHours(k - j);
      const WindDataItemUnit& begDataItem = get_data_item(storyParams, startTime);
      const WindDataItemUnit& endDataItem = get_data_item(storyParams, endTime);
      float begSpeed = calculate_weighted_wind_speed(storyParams, begDataItem);
      float endSpeed = calculate_weighted_wind_speed(storyParams, endDataItem);

      speedDifferenceVector.push_back(std::pair<float, WeatherPeriod>(
          fabs(begSpeed - endSpeed), WeatherPeriod(startTime, endTime)));
    }
  }

  // biggest difference in the beginnig
  std::sort(speedDifferenceVector.begin(), speedDifferenceVector.end(), comp_sdp);

  // remove overlapping periods
  for (unsigned int i = 0; i < speedDifferenceVector.size() - 1; i++)
    for (unsigned int j = i + 1; j < speedDifferenceVector.size(); j++)
    {
      if (speedDifferenceVector[i].second.localEndTime().GetYear() == 1970 ||
          speedDifferenceVector[j].second.localEndTime().GetYear() == 1970)
        continue;

      WeatherPeriod intersection =
          intersecting_period(speedDifferenceVector[i].second, speedDifferenceVector[j].second);

      if (intersection.localStartTime().GetYear() != 1970)
        speedDifferenceVector[j].second =
            WeatherPeriod(TextGenPosixTime(1970, 1, 1),
                          TextGenPosixTime(1970, 1, 1));  // set period length to zero
    }

  for (unsigned int i = 0; i < speedDifferenceVector.size(); i++)
  {
    const SpeedDifferencePeriod& sdp = speedDifferenceVector[i];

    if (get_period_length(sdp.second) == 0 || sdp.first < storyParams.theWindSpeedThreshold)
      continue;

    // if calculated wind speed between beginning and end differs more than configuration parameter
    // allows, split the period
    if (sdp.first >= storyParams.theWindSpeedThreshold)
    {
      WeatherPeriod newPeriod(sdp.second.localStartTime(), sdp.second.localEndTime());

      // if the new period starts after original period start time
      // add the remaining period to the beginning
      int hoursBeforeNewPeriod =
          newPeriod.localStartTime().DifferenceInHours(dataItem.thePeriod.localStartTime());

      if (hoursBeforeNewPeriod > 1)
      {
        TextGenPosixTime beforeNewEndTime = newPeriod.localStartTime();
        beforeNewEndTime.ChangeByHours(-1);
        WeatherPeriod beforeNewPeriod(dataItem.thePeriod.localStartTime(), beforeNewEndTime);
        // add MISSING_WIND_SPEED_EVENT period in the beginning
        const WindDataItemUnit& begDataItem =
            get_data_item(storyParams, dataItem.thePeriod.localStartTime());
        const WindDataItemUnit& endDataItem = get_data_item(storyParams, beforeNewEndTime);
        float begSpeed = calculate_weighted_wind_speed(storyParams, begDataItem);
        float endSpeed = calculate_weighted_wind_speed(storyParams, endDataItem);
        WindEventPeriodDataItem* newDataItem = new WindEventPeriodDataItem(
            beforeNewPeriod,
            get_wind_speed_event(begSpeed, endSpeed, storyParams.theWindSpeedThreshold),
            begDataItem,
            endDataItem);
        ret.push_back(newDataItem);
      }

      // add the new period
      const WindDataItemUnit& begDataItem = get_data_item(storyParams, sdp.second.localStartTime());
      const WindDataItemUnit& endDataItem = get_data_item(storyParams, sdp.second.localEndTime());

      float begSpeed = calculate_weighted_wind_speed(storyParams, begDataItem);
      float endSpeed = calculate_weighted_wind_speed(storyParams, endDataItem);

      WindEventPeriodDataItem* newDataItem = new WindEventPeriodDataItem(
          sdp.second,
          get_wind_speed_event(begSpeed, endSpeed, storyParams.theWindSpeedThreshold),
          begDataItem,
          endDataItem);
      ret.push_back(newDataItem);

      // if the new period ends before original period end time
      // add the remaining period to the end
      int hoursAfterNewPeriod =
          dataItem.thePeriod.localEndTime().DifferenceInHours(newPeriod.localEndTime());

      if (hoursAfterNewPeriod > 1)
      {
        TextGenPosixTime afterNewStartTime = newPeriod.localEndTime();
        afterNewStartTime.ChangeByHours(1);
        WeatherPeriod afterNewPeriod(afterNewStartTime, dataItem.thePeriod.localEndTime());
        // add MISSING_WIND_SPEED_EVENT period in the end
        const WindDataItemUnit& begDataItem = get_data_item(storyParams, afterNewStartTime);
        const WindDataItemUnit& endDataItem =
            get_data_item(storyParams, dataItem.thePeriod.localEndTime());
        float begSpeed = calculate_weighted_wind_speed(storyParams, begDataItem);
        float endSpeed = calculate_weighted_wind_speed(storyParams, endDataItem);
        WindEventPeriodDataItem* newDataItem = new WindEventPeriodDataItem(
            afterNewPeriod,
            get_wind_speed_event(begSpeed, endSpeed, storyParams.theWindSpeedThreshold),
            begDataItem,
            endDataItem);
        ret.push_back(newDataItem);
      }
    }
  }

  return ret;
}

void merge_missing_wind_speed_event_periods2(wo_story_params& storyParams)
{
  wind_event_period_data_item_vector mergedEventPeriods;

  // 1) merge all successive missing periods
  for (unsigned int i = 0; i < storyParams.theWindSpeedEventPeriodVector.size(); i++)
  {
    WindEventPeriodDataItem* currentDataItem = storyParams.theWindSpeedEventPeriodVector[i];

    if (currentDataItem->theWindEvent != MISSING_WIND_SPEED_EVENT ||
        i == storyParams.theWindSpeedEventPeriodVector.size() - 1)
    {
      mergedEventPeriods.push_back(currentDataItem);
      continue;
    }

    unsigned int lastMissingIndex = 0;
    for (unsigned int k = i + 1; k < storyParams.theWindSpeedEventPeriodVector.size(); k++)
    {
      if (storyParams.theWindSpeedEventPeriodVector[k]->theWindEvent == MISSING_WIND_SPEED_EVENT)
      {
        lastMissingIndex = k;
        continue;
      }
      break;
    }

    if (lastMissingIndex == 0)
    {
      mergedEventPeriods.push_back(currentDataItem);
      continue;
    }

    WindEventPeriodDataItem* nextDataItem =
        storyParams.theWindSpeedEventPeriodVector[lastMissingIndex];

    // merge periods
    WeatherPeriod newPeriod(currentDataItem->thePeriod.localStartTime(),
                            nextDataItem->thePeriod.localEndTime());
    WindEventPeriodDataItem* newDataItem =
        new WindEventPeriodDataItem(newPeriod,
                                    MISSING_WIND_SPEED_EVENT,
                                    currentDataItem->thePeriodBeginDataItem,
                                    nextDataItem->thePeriodEndDataItem);
    mergedEventPeriods.push_back(newDataItem);

    i = lastMissingIndex;
  }
  // iterate merged event periods and remove short (<= 6h) missing period if it is between
  // strenghtening/weakening period
  mergedEventPeriods = remove_short_missing_periods(storyParams, mergedEventPeriods);

  wind_event_period_data_item_vector cleanedMissingEventPeriods;

  // 2) examine the merged missing period if there are weakening/strenghtening periods inside
  for (unsigned int i = 0; i < mergedEventPeriods.size(); i++)
  {
    WindEventPeriodDataItem* dataItem = mergedEventPeriods[i];

    if (dataItem->theWindEvent != MISSING_WIND_SPEED_EVENT ||
        get_period_length(dataItem->thePeriod) < 4)
    {
      cleanedMissingEventPeriods.push_back(mergedEventPeriods[i]);
      continue;
    }

    wind_event_period_data_item_vector retVector =
        examine_merged_missing_event_period(storyParams, *(mergedEventPeriods[i]));

    if (retVector.size() > 0)
      cleanedMissingEventPeriods.insert(
          cleanedMissingEventPeriods.end(), retVector.begin(), retVector.end());
    else
      cleanedMissingEventPeriods.push_back(mergedEventPeriods[i]);
  }
  storyParams.theWindSpeedEventPeriodVector.clear();

  BOOST_FOREACH (WindEventPeriodDataItem* eventPeriod, cleanedMissingEventPeriods)
  {
    storyParams.theWindSpeedEventPeriodVector.push_back(eventPeriod);
  }
}

void merge_missing_wind_speed_event_periods(wo_story_params& storyParams)
{
  WindEventPeriodDataItem* currentDataItem = 0;

  wind_event_period_data_item_vector mergedEventPeriods;

  size_t eventPeriodVectorSize = storyParams.theWindSpeedEventPeriodVector.size();

  // iterate event periods
  for (unsigned int i = 0; i < eventPeriodVectorSize; i++)
  {
    currentDataItem = storyParams.theWindSpeedEventPeriodVector[i];

    WindEventPeriodDataItem* maxSpeedDataItem = currentDataItem;
    WindEventPeriodDataItem* nextDataItemAfterMax =
        i < eventPeriodVectorSize - 1 ? storyParams.theWindSpeedEventPeriodVector[i + 1] : 0;
    WindEventPeriodDataItem* minSpeedDataItem = currentDataItem;
    WindEventPeriodDataItem* nextDataItemAfterMin =
        i < eventPeriodVectorSize - 1 ? storyParams.theWindSpeedEventPeriodVector[i + 1] : 0;
    // handle only successive missing wind speed events
    if (currentDataItem->theWindEvent != MISSING_WIND_SPEED_EVENT)
    {
      mergedEventPeriods.push_back(currentDataItem);
      continue;
    }

    float previousMax = -kFloatMissing;
    float previousMin = kFloatMissing;

    get_calculated_max_min(storyParams, *currentDataItem, previousMax, previousMin);

    WindEventPeriodDataItem* nextDataItem = 0;
    for (unsigned int k = i + 1; k < eventPeriodVectorSize; k++)
    {
      if (storyParams.theWindSpeedEventPeriodVector[k]->theWindEvent == MISSING_WIND_SPEED_EVENT)
      {
        nextDataItem = storyParams.theWindSpeedEventPeriodVector[k];

        i = k;

        float max = previousMax;
        float min = previousMin;

        get_calculated_max_min(storyParams, *nextDataItem, max, min);

        // strongest/weakest wind can be before period end
        if (max > previousMax)
        {
          previousMax = max;
          maxSpeedDataItem = nextDataItem;
          if (k < eventPeriodVectorSize - 1)
            nextDataItemAfterMax = storyParams.theWindSpeedEventPeriodVector[k + 1];
        }
        if (min < previousMin)
        {
          previousMin = min;
          minSpeedDataItem = nextDataItem;
          nextDataItemAfterMin =
              k < eventPeriodVectorSize - 1 ? storyParams.theWindSpeedEventPeriodVector[k + 1] : 0;
        }
      }
      else
        break;
    }

    if (!nextDataItem)
    {
      mergedEventPeriods.push_back(currentDataItem);
      continue;
    }

    // merge periods
    WeatherPeriod newPeriod(currentDataItem->thePeriod.localStartTime(),
                            nextDataItem->thePeriod.localEndTime());

    float begSpeed =
        calculate_weighted_wind_speed(storyParams, currentDataItem->thePeriodBeginDataItem);
    float endSpeed = calculate_weighted_wind_speed(storyParams, nextDataItem->thePeriodEndDataItem);

    WindEventId newWindEvent =
        get_wind_speed_event(begSpeed, endSpeed, storyParams.theWindSpeedThreshold);

    // even if wind speed strenghtens/weakens on merged period, there can be MISSING period in the
    // beginning
    bool modifyPeriod = false;
    if (newWindEvent == TUULI_HEIKKENEE && maxSpeedDataItem != nextDataItem)
    {
      modifyPeriod = true;
    }
    else if (newWindEvent == TUULI_VOIMISTUU && minSpeedDataItem != nextDataItem)
    {
      modifyPeriod = true;
    }

    if (modifyPeriod)
    {
      bool tuuliHeikkenee = (newWindEvent == TUULI_HEIKKENEE);

      WindEventPeriodDataItem* dataItem = (tuuliHeikkenee ? minSpeedDataItem : maxSpeedDataItem);

      newPeriod = WeatherPeriod(currentDataItem->thePeriod.localStartTime(),
                                dataItem->thePeriod.localEndTime());

      endSpeed = calculate_weighted_wind_speed(storyParams, dataItem->thePeriodEndDataItem);

      newWindEvent = get_wind_speed_event(begSpeed, endSpeed, storyParams.theWindSpeedThreshold);

      WindEventPeriodDataItem* newDataItem =
          new WindEventPeriodDataItem(newPeriod,
                                      newWindEvent,
                                      currentDataItem->thePeriodBeginDataItem,
                                      dataItem->thePeriodEndDataItem);

      mergedEventPeriods.push_back(newDataItem);

      if (nextDataItemAfterMin &&
          nextDataItemAfterMin->thePeriod.localStartTime() < newPeriod.localEndTime())
        nextDataItemAfterMin = 0;
      if (nextDataItemAfterMax &&
          nextDataItemAfterMax->thePeriod.localStartTime() < newPeriod.localEndTime())
        nextDataItemAfterMax = 0;

      currentDataItem = (tuuliHeikkenee ? nextDataItemAfterMin : nextDataItemAfterMax);

      if (currentDataItem)
      {
        newPeriod = WeatherPeriod(currentDataItem->thePeriod.localStartTime(),
                                  nextDataItem->thePeriod.localEndTime());

        begSpeed =
            calculate_weighted_wind_speed(storyParams, currentDataItem->thePeriodBeginDataItem);
        endSpeed = calculate_weighted_wind_speed(storyParams, nextDataItem->thePeriodEndDataItem);
        newWindEvent = get_wind_speed_event(begSpeed, endSpeed, storyParams.theWindSpeedThreshold);
        WindEventPeriodDataItem* newDataItem =
            new WindEventPeriodDataItem(newPeriod,
                                        newWindEvent,
                                        currentDataItem->thePeriodBeginDataItem,
                                        nextDataItem->thePeriodEndDataItem);
        mergedEventPeriods.push_back(newDataItem);
      }
    }
    else
    {
      WindEventPeriodDataItem* newDataItem =
          new WindEventPeriodDataItem(newPeriod,
                                      newWindEvent,
                                      currentDataItem->thePeriodBeginDataItem,
                                      nextDataItem->thePeriodEndDataItem);
      mergedEventPeriods.push_back(newDataItem);
    }
  }

  wind_event_period_data_item_vector cleanedEventPeriods =
      remove_short_missing_periods(storyParams, mergedEventPeriods);

  mergedEventPeriods.clear();

  BOOST_FOREACH (WindEventPeriodDataItem* p, cleanedEventPeriods)
  {
    if (p->theWindEvent != MISSING_WIND_SPEED_EVENT)
    {
      mergedEventPeriods.push_back(p);
      continue;
    }

    bool missingPeriodSplit = false;
    // check missing period once more if it contains weakening/strenghtening period in the end
    float endSpeed = calculate_weighted_wind_speed(storyParams, p->thePeriodEndDataItem);

    WeatherArea::Type areaType(storyParams.theArea.type());
    for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
    {
      const WindDataItemUnit& dataItem = (*storyParams.theWindDataVector[i])(areaType);

      if (dataItem.thePeriod.localStartTime() > p->thePeriod.localStartTime() &&
          dataItem.thePeriod.localStartTime() < p->thePeriod.localEndTime())
      {
        float begSpeed = calculate_weighted_wind_speed(storyParams, dataItem);
        WindEventId newWindEvent =
            get_wind_speed_event(begSpeed, endSpeed, storyParams.theWindSpeedThreshold);

        if (newWindEvent != MISSING_WIND_SPEED_EVENT)
        {
          // split missing period into two
          WeatherPeriod newPeriod1(p->thePeriod.localStartTime(),
                                   dataItem.thePeriod.localStartTime());
          WeatherPeriod newPeriod2(dataItem.thePeriod.localStartTime(),
                                   p->thePeriod.localEndTime());
          WindEventPeriodDataItem* newDataItem1 = new WindEventPeriodDataItem(
              newPeriod1, p->theWindEvent, p->thePeriodBeginDataItem, dataItem);
          WindEventPeriodDataItem* newDataItem2 = new WindEventPeriodDataItem(
              newPeriod2, newWindEvent, dataItem, p->thePeriodEndDataItem);
          mergedEventPeriods.push_back(newDataItem1);
          mergedEventPeriods.push_back(newDataItem2);
          missingPeriodSplit = true;
          storyParams.theLog << "Missing wind speed event period " << p->thePeriod
                             << " split into two:\n" << newPeriod1 << " -> "
                             << get_wind_event_string(p->theWindEvent) << " and " << newPeriod2
                             << " -> " << get_wind_event_string(newWindEvent) << std::endl;
          break;
        }
      }
    }

    if (!missingPeriodSplit) mergedEventPeriods.push_back(p);
  }

  cleanedEventPeriods = remove_short_missing_periods(storyParams, mergedEventPeriods);

  // in the end merge successive strenghtening/weakening periods if there are any
  size_t vectorOriginalSize = cleanedEventPeriods.size();
  size_t removedElements = 0;
  for (size_t i = vectorOriginalSize - 1; i > 0; i--)
  {
    WindEventPeriodDataItem* prevItem = cleanedEventPeriods[i - 1];
    WindEventPeriodDataItem* currentItem = cleanedEventPeriods[i];
    if (currentItem->theWindEvent == prevItem->theWindEvent)
    {
      WindEventPeriodDataItem* newWindEventPeriodDataItem =
          new WindEventPeriodDataItem(WeatherPeriod(prevItem->thePeriod.localStartTime(),
                                                    currentItem->thePeriod.localEndTime()),
                                      prevItem->theWindEvent,
                                      prevItem->thePeriodBeginDataItem,
                                      currentItem->thePeriodEndDataItem);
      delete cleanedEventPeriods[i];
      cleanedEventPeriods[i] = 0;
      delete cleanedEventPeriods[i - 1];
      cleanedEventPeriods[i - 1] = newWindEventPeriodDataItem;
      removedElements++;
    }
  }
  storyParams.theWindSpeedEventPeriodVector.clear();

  BOOST_FOREACH (WindEventPeriodDataItem* eventPeriod, cleanedEventPeriods)
  {
    if (eventPeriod) storyParams.theWindSpeedEventPeriodVector.push_back(eventPeriod);
  }

  mergedEventPeriods.clear();
  cleanedEventPeriods.clear();
}

void find_out_wind_event_periods(wo_story_params& storyParams)
{
  find_out_wind_speed_event_periods(storyParams);

  merge_missing_wind_speed_event_periods2(storyParams);

  if (storyParams.theWindSpeedEventPeriodVector.size() > 1)
    std::sort(storyParams.theWindSpeedEventPeriodVector.begin(),
              storyParams.theWindSpeedEventPeriodVector.end(),
              wind_event_period_sort);
}

double distance_from_line(const NFmiPoint& point,
                          const NFmiPoint& lineBeg,
                          const NFmiPoint& lineEnd)
{
  /*
  double side1Len =
      sqrt(pow(fabs(point.X() - lineEnd.X()), 2) + pow(fabs(point.Y() - lineEnd.Y()), 2));
  double side2Len =
      sqrt(pow(fabs(point.X() - lineBeg.X()), 2) + pow(fabs(point.Y() - lineBeg.Y()), 2));
  double baseLen =
      sqrt(pow(fabs(lineEnd.X() - lineBeg.X()), 2) + pow(fabs(lineEnd.Y() - lineBeg.Y()), 2));
  double s = (side1Len + side2Len + baseLen) / 2.0;
  double A = sqrt(s * (s - side1Len) * (s - side2Len) * (s - baseLen));
  //	double distance = (0.5*baseLen == 0 ? 0 : (A/(0.5*baseLen)));
  double distance = (A / (0.5 * baseLen));

  return distance;
  */

  double slope = (lineEnd.Y() - lineBeg.Y()) / (lineEnd.X() - lineBeg.X());

  double yvalue = lineBeg.Y() + (slope * (point.X() - lineBeg.X()));

  double deviation = abs(point.Y() - yvalue);

  return deviation;
}

enum class StatValueType
{
  MIN,
  MAX,
  TOP,
  MEDIAN,
  CALC
};

bool add_local_min_max_values(vector<unsigned int>& eqIndexVector,
                              wo_story_params& storyParams,
                              StatValueType statType)
{
  // include the local maximum and minimum values that were possibly removed
  // in the previous step
  WeatherArea::Type areaType(storyParams.theArea.type());
  vector<unsigned int> indexesToAdd;
  for (unsigned int i = 1; i < eqIndexVector.size(); i++)
  {
    unsigned int currentIndex = eqIndexVector[i];
    unsigned int previousIndex = eqIndexVector[i - 1];
    const WindDataItemUnit& previousItem =
        (*storyParams.theWindDataVector[previousIndex])(areaType);
    const WindDataItemUnit& currentItem = (*storyParams.theWindDataVector[currentIndex])(areaType);

    if (currentIndex == previousIndex + 1) continue;

    float localMax = UINT_MAX;
    float localMin = UINT_MAX;
    switch (statType)
    {
      case StatValueType::MAX:
      {
        localMax = (previousItem.theWindSpeedMax.value() > currentItem.theWindSpeedMax.value()
                        ? previousItem.theWindSpeedMax.value()
                        : currentItem.theWindSpeedMax.value());
        localMin = (previousItem.theWindSpeedMax.value() < currentItem.theWindSpeedMax.value()
                        ? previousItem.theWindSpeedMax.value()
                        : currentItem.theWindSpeedMax.value());
      }
      break;
      case StatValueType::MIN:
      {
        localMax = (previousItem.theWindSpeedMin.value() > currentItem.theWindSpeedMin.value()
                        ? previousItem.theWindSpeedMin.value()
                        : currentItem.theWindSpeedMin.value());
        localMin = (previousItem.theWindSpeedMin.value() < currentItem.theWindSpeedMin.value()
                        ? previousItem.theWindSpeedMin.value()
                        : currentItem.theWindSpeedMin.value());
      }
      break;
      case StatValueType::TOP:
      {
        localMax = (previousItem.theWindSpeedTop.value() > currentItem.theWindSpeedTop.value()
                        ? previousItem.theWindSpeedTop.value()
                        : currentItem.theWindSpeedTop.value());
        localMin = (previousItem.theWindSpeedTop.value() < currentItem.theWindSpeedTop.value()
                        ? previousItem.theWindSpeedTop.value()
                        : currentItem.theWindSpeedTop.value());
      }
      break;
      case StatValueType::MEDIAN:
      {
        localMax = (previousItem.theWindSpeedMedian.value() > currentItem.theWindSpeedMedian.value()
                        ? previousItem.theWindSpeedMedian.value()
                        : currentItem.theWindSpeedMedian.value());
        localMin = (previousItem.theWindSpeedMedian.value() < currentItem.theWindSpeedMedian.value()
                        ? previousItem.theWindSpeedMedian.value()
                        : currentItem.theWindSpeedMedian.value());
      }
      break;
      case StatValueType::CALC:
      {
        localMax = (previousItem.theWindSpeedCalc.value() > currentItem.theWindSpeedCalc.value()
                        ? previousItem.theWindSpeedCalc.value()
                        : currentItem.theWindSpeedCalc.value());
        localMin = (previousItem.theWindSpeedCalc.value() < currentItem.theWindSpeedCalc.value()
                        ? previousItem.theWindSpeedCalc.value()
                        : currentItem.theWindSpeedCalc.value());
      }
      break;
    };

    unsigned localMaxIndex = UINT_MAX;
    unsigned localMinIndex = UINT_MAX;

    for (unsigned int k = previousIndex + 1; k < currentIndex; k++)
    {
      const WindDataItemUnit& itemK = ((*storyParams.theWindDataVector[k])(areaType));
      float currentValue = 0.0;
      switch (statType)
      {
        case StatValueType::MAX:
          currentValue = itemK.theWindSpeedMax.value();
          break;
        case StatValueType::MIN:
          currentValue = itemK.theWindSpeedMin.value();
          break;
        case StatValueType::TOP:
          currentValue = itemK.theWindSpeedTop.value();
          break;
        case StatValueType::MEDIAN:
          currentValue = itemK.theWindSpeedMedian.value();
          break;
        case StatValueType::CALC:
          currentValue = itemK.theWindSpeedCalc.value();
          break;
      };

      if (currentValue > localMax)
      {
        localMax = currentValue;
        localMaxIndex = k;
      }
      else if (currentValue < localMin)
      {
        localMin = currentValue;
        localMinIndex = k;
      }
    }

    if (localMaxIndex != UINT_MAX)
    {
      indexesToAdd.push_back(localMaxIndex);
    }

    if (localMinIndex != UINT_MAX)
    {
      indexesToAdd.push_back(localMinIndex);
    }
  }
  eqIndexVector.insert(eqIndexVector.begin(), indexesToAdd.begin(), indexesToAdd.end());
  sort(eqIndexVector.begin(), eqIndexVector.end());

  return indexesToAdd.size() > 0;
}

void calculate_equalized_wind_speed_indexes_for_median_wind(wo_story_params& storyParams)
{
  for (unsigned int j = 0; j < storyParams.theWeatherAreas.size(); j++)
  {
    const WeatherArea& weatherArea = storyParams.theWeatherAreas[j];

    unsigned int index1, index2, index3;
    WeatherArea::Type areaType(weatherArea.type());  // storyParams.theArea.type());

    vector<unsigned int>& eqIndexVector = storyParams.equalizedWSIndexesMedian(areaType);

    while (1)
    {
      double minError = UINT_MAX;
      unsigned int minErrorIndex = UINT_MAX;

      for (unsigned int i = 0; i < eqIndexVector.size() - 2; i++)
      {
        index1 = eqIndexVector[i];
        index2 = eqIndexVector[i + 1];
        index3 = eqIndexVector[i + 2];

        const WindDataItemUnit& dataItemIndex1 = (*storyParams.theWindDataVector[index1])(areaType);
        const WindDataItemUnit& dataItemIndex2 = (*storyParams.theWindDataVector[index2])(areaType);
        const WindDataItemUnit& dataItemIndex3 = (*storyParams.theWindDataVector[index3])(areaType);

        double lineBegX = index1;
        double lineBegY = dataItemIndex1.theWindSpeedMedian.value();
        double lineEndX = index3;
        double lineEndY = dataItemIndex3.theWindSpeedMedian.value();
        double coordX = index2;
        double coordY = dataItemIndex2.theWindSpeedMedian.value();
        NFmiPoint point(coordX, coordY);
        NFmiPoint lineBegPoint(lineBegX, lineBegY);
        NFmiPoint lineEndPoint(lineEndX, lineEndY);
        double deviation_from_line = distance_from_line(point, lineBegPoint, lineEndPoint);

        if (deviation_from_line < minError)
        {
          minError = deviation_from_line;
          minErrorIndex = i + 1;
        }
      }

      if (minError > storyParams.theWindSpeedMaxError)
      {
        break;
      }
      // remove the point with minimum error
      eqIndexVector.erase(eqIndexVector.begin() + minErrorIndex);
    }  // while

    bool valuesToAdd = true;
    do
    {
      valuesToAdd = add_local_min_max_values(eqIndexVector, storyParams, StatValueType::MEDIAN);
    } while (valuesToAdd);

    // re-calculate equalized values for the removed points
    for (unsigned int i = 1; i < eqIndexVector.size(); i++)
    {
      unsigned int currentIndex = eqIndexVector[i];
      unsigned int previousIndex = eqIndexVector[i - 1];

      const WindDataItemUnit& previousItem =
          (*storyParams.theWindDataVector[previousIndex])(areaType);
      const WindDataItemUnit& currentItem =
          (*storyParams.theWindDataVector[currentIndex])(areaType);

      if (currentIndex == previousIndex + 1) continue;

      float oppositeLen =
          currentItem.theWindSpeedMedian.value() - previousItem.theWindSpeedMedian.value();
      float adjacentLen = currentIndex - previousIndex;
      float slope = oppositeLen / adjacentLen;

      for (unsigned int k = previousIndex + 1; k < currentIndex; k++)
      {
        float yValue = (slope * (k - previousIndex)) + previousItem.theWindSpeedMedian.value();
        WindDataItemUnit& item = (storyParams.theWindDataVector[k])->getDataItem(areaType);
        item.theEqualizedMedianWind = WeatherResult(yValue, 0.0);
      }
    }
  }
}

void calculate_equalized_wind_speed_indexes_for_maximum_wind(wo_story_params& storyParams,
                                                             bool topWind = true)
{
  for (unsigned int j = 0; j < storyParams.theWeatherAreas.size(); j++)
  {
    const WeatherArea& weatherArea = storyParams.theWeatherAreas[j];

    unsigned int index1, index2, index3;
    WeatherArea::Type areaType(weatherArea.type());

    vector<unsigned int>& eqIndexVector =
        (topWind ? storyParams.equalizedWSIndexesTopWind(areaType)
                 : storyParams.equalizedWSIndexesMaxWind(areaType));

    while (1)
    {
      double minError = UINT_MAX;
      unsigned int minErrorIndex = UINT_MAX;

      for (unsigned int i = 0; i < eqIndexVector.size() - 2; i++)
      {
        index1 = eqIndexVector[i];
        index2 = eqIndexVector[i + 1];
        index3 = eqIndexVector[i + 2];

        const WindDataItemUnit& dataItemIndex1 = (*storyParams.theWindDataVector[index1])(areaType);
        const WindDataItemUnit& dataItemIndex2 = (*storyParams.theWindDataVector[index2])(areaType);
        const WindDataItemUnit& dataItemIndex3 = (*storyParams.theWindDataVector[index3])(areaType);

        double lineBegX = index1;
        double lineBegY = (topWind ? dataItemIndex1.theWindSpeedTop.value()
                                   : dataItemIndex1.theWindSpeedMax.value());
        double lineEndX = index3;
        double lineEndY = (topWind ? dataItemIndex3.theWindSpeedTop.value()
                                   : dataItemIndex3.theWindSpeedMax.value());
        double coordX = index2;
        double coordY = (topWind ? dataItemIndex2.theWindSpeedTop.value()
                                 : dataItemIndex2.theWindSpeedMax.value());
        NFmiPoint point(coordX, coordY);
        NFmiPoint lineBegPoint(lineBegX, lineBegY);
        NFmiPoint lineEndPoint(lineEndX, lineEndY);
        double deviation_from_line = distance_from_line(point, lineBegPoint, lineEndPoint);

        if (deviation_from_line < minError)
        {
          minError = deviation_from_line;
          minErrorIndex = i + 1;
        }
      }

      if (minError > storyParams.theWindSpeedMaxError)
      {
        break;
      }
      // remove the point with minimum error
      eqIndexVector.erase(eqIndexVector.begin() + minErrorIndex);
    }  // while

    bool valuesToAdd = true;
    do
    {
      valuesToAdd = add_local_min_max_values(
          eqIndexVector, storyParams, (topWind ? StatValueType::TOP : StatValueType::MAX));
    } while (valuesToAdd);

    // re-calculate equalized values for the removed points
    for (unsigned int i = 1; i < eqIndexVector.size(); i++)
    {
      unsigned int currentIndex = eqIndexVector[i];
      unsigned int previousIndex = eqIndexVector[i - 1];
      const WindDataItemUnit& previousItem =
          (*storyParams.theWindDataVector[previousIndex])(areaType);
      const WindDataItemUnit& currentItem =
          (*storyParams.theWindDataVector[currentIndex])(areaType);

      if (currentIndex == previousIndex + 1) continue;

      float oppositeLen =
          (topWind ? (currentItem.theWindSpeedTop.value() - previousItem.theWindSpeedTop.value())
                   : (currentItem.theWindSpeedMax.value() - previousItem.theWindSpeedMax.value()));
      float adjacentLen = currentIndex - previousIndex;
      float slope = oppositeLen / adjacentLen;

      for (unsigned int k = previousIndex + 1; k < currentIndex; k++)
      {
        float yValue =
            (slope * (k - previousIndex)) +
            (topWind ? previousItem.theWindSpeedTop.value() : previousItem.theWindSpeedMax.value());
        WindDataItemUnit& item = (storyParams.theWindDataVector[k])->getDataItem(areaType);
        if (topWind)
          item.theEqualizedTopWind = WeatherResult(yValue, 0.0);
        else
          item.theEqualizedMaxWind = WeatherResult(yValue, 0.0);
      }
    }
  }
}

void calculate_equalized_wind_speed_indexes_for_calc_wind(wo_story_params& storyParams)
{
  // get claculated wind speed
  for (unsigned int k = 0; k < storyParams.theWeatherAreas.size(); k++)
  {
    const WeatherArea& weatherArea = storyParams.theWeatherAreas[k];
    WeatherArea::Type areaType(weatherArea.type());

    //	add equalized indexes of top and median wind first to set to get unique indexes
    std::set<unsigned int> index_set;

    BOOST_FOREACH (unsigned int ind, storyParams.equalizedWSIndexesTopWind(areaType))
      index_set.insert(ind);

    BOOST_FOREACH (unsigned int ind, storyParams.equalizedWSIndexesMedian(areaType))
      index_set.insert(ind);

    // clear equalized calculated wind indexes and then insert indexes from set
    vector<unsigned int>* eqCalcWindIndexVector =
        &(storyParams.equalizedWSIndexesCalcWind(areaType));
    eqCalcWindIndexVector->clear();

    BOOST_FOREACH (unsigned int ind, index_set)
      eqCalcWindIndexVector->push_back(ind);

    // recalculate equalized calculated wind speed
    for (unsigned int i = 0; i < storyParams.theWindDataVector.size(); i++)
    {
      WindDataItemUnit& dataItem = (storyParams.theWindDataVector[i])->getDataItem(areaType);
      dataItem.theEqualizedCalcWind =
          WeatherResult(calculate_weighted_wind_speed(storyParams,
                                                      dataItem.theEqualizedTopWind.value(),
                                                      dataItem.theEqualizedMedianWind.value()),
                        0.0);
    }
  }
}

void calculate_equalized_wind_direction_indexes(wo_story_params& storyParams)
{
  for (unsigned int j = 0; j < storyParams.theWeatherAreas.size(); j++)
  {
    const WeatherArea& weatherArea = storyParams.theWeatherAreas[j];

    unsigned int index1, index2, index3;
    WeatherArea::Type areaType(weatherArea.type());

    vector<unsigned int>& eqIndexVector = storyParams.equalizedWDIndexes(areaType);

    while (1)
    {
      double minError(UINT_MAX);
      unsigned int minErrorIndex(UINT_MAX);
      WindDirectionId directionIdIndex1(MISSING_WIND_DIRECTION_ID);
      WindDirectionId directionIdIndex2(MISSING_WIND_DIRECTION_ID);
      WindDirectionId directionIdIndex3(MISSING_WIND_DIRECTION_ID);

      for (unsigned int i = 0; i < eqIndexVector.size() - 2; i++)
      {
        index1 = eqIndexVector[i];
        index2 = eqIndexVector[i + 1];
        index3 = eqIndexVector[i + 2];

        const WindDataItemUnit& dataItemIndex1 = (*storyParams.theWindDataVector[index1])(areaType);
        const WindDataItemUnit& dataItemIndex2 = (*storyParams.theWindDataVector[index2])(areaType);
        const WindDataItemUnit& dataItemIndex3 = (*storyParams.theWindDataVector[index3])(areaType);

        directionIdIndex1 = wind_direction_id(dataItemIndex1.theEqualizedWindDirection,
                                              dataItemIndex1.theEqualizedTopWind,
                                              storyParams.theVar,
                                              storyParams.theWindDirectionMinSpeed);
        directionIdIndex2 = wind_direction_id(dataItemIndex2.theEqualizedWindDirection,
                                              dataItemIndex3.theEqualizedTopWind,
                                              storyParams.theVar,
                                              storyParams.theWindDirectionMinSpeed);
        directionIdIndex3 = wind_direction_id(dataItemIndex3.theEqualizedWindDirection,
                                              dataItemIndex3.theEqualizedTopWind,
                                              storyParams.theVar,
                                              storyParams.theWindDirectionMinSpeed);

        // dont remove variable wind
        if (directionIdIndex1 == VAIHTELEVA || directionIdIndex2 == VAIHTELEVA ||
            directionIdIndex3 == VAIHTELEVA)
        {
          continue;
        }

        double lineBegX = index1;
        double lineBegY = dataItemIndex1.theEqualizedWindDirection.value();
        double lineEndX = index3;
        double lineEndY = dataItemIndex3.theEqualizedWindDirection.value();
        double coordX = index2;
        double coordY = dataItemIndex2.theEqualizedWindDirection.value();

        // if wind changes more than 180 degrees in three steps, dont try to smoothen
        if (abs(coordY - lineBegY) > 180.0 || abs(coordY - lineEndY) > 180.0 ||
            abs(lineBegY - lineEndY) > 180.0)
        {
          continue;
        }

        NFmiPoint point(coordX, coordY);
        NFmiPoint lineBegPoint(lineBegX, lineBegY);
        NFmiPoint lineEndPoint(lineEndX, lineEndY);
        double deviation_from_line = distance_from_line(point, lineBegPoint, lineEndPoint);

        if (deviation_from_line < minError)
        {
          minError = deviation_from_line;
          minErrorIndex = i + 1;
        }
      }

      if (minError > storyParams.theWindDirectionMaxError)
      {
        break;
      }

      unsigned int remove_index = eqIndexVector[minErrorIndex];

      const WindDataItemUnit& iii = (*storyParams.theWindDataVector[remove_index])(areaType);
      WeatherPeriod per(iii.thePeriod);
      WeatherResult res(iii.theEqualizedWindDirection);

      eqIndexVector.erase(eqIndexVector.begin() + minErrorIndex);

    }  // for (unsigned int i = 0; i < eqIndexVector.size() - 2; i++)

    // re-calculate equalized values for the removed points
    for (unsigned int i = 1; i < eqIndexVector.size(); i++)
    {
      unsigned int currentIndex = eqIndexVector[i];
      unsigned int previousIndex = eqIndexVector[i - 1];
      const WindDataItemUnit& previousItem =
          (*storyParams.theWindDataVector[previousIndex])(areaType);
      const WindDataItemUnit& currentItem =
          (*storyParams.theWindDataVector[currentIndex])(areaType);

      if (currentIndex == previousIndex + 1) continue;

      float oppositeLen = currentItem.theEqualizedWindDirection.value() -
                          previousItem.theEqualizedWindDirection.value();
      float adjacentLen = currentIndex - previousIndex;
      float slope = oppositeLen / adjacentLen;

      for (unsigned int k = previousIndex + 1; k < currentIndex; k++)
      {
        float yValue =
            (slope * (k - previousIndex)) + previousItem.theEqualizedWindDirection.value();

        WindDataItemUnit& item = (storyParams.theWindDataVector[k])->getDataItem(areaType);
        item.theEqualizedWindDirection = WeatherResult(yValue, item.theWindDirection.error());
      }
    }

    unsigned int startIndex(eqIndexVector[0]);
    unsigned int endIndex(eqIndexVector[eqIndexVector.size() - 1]);

    for (unsigned int i = startIndex + 1; i < endIndex - 1; i++)
    {
      WindDataItemUnit& item1 = (storyParams.theWindDataVector[i - 1])->getDataItem(areaType);
      WindDataItemUnit& item2 = (storyParams.theWindDataVector[i])->getDataItem(areaType);
      WindDataItemUnit& item3 = (storyParams.theWindDataVector[i + 1])->getDataItem(areaType);

      WindDirectionId directionId1(wind_direction_id(item1.theEqualizedWindDirection,
                                                     item1.theEqualizedTopWind,
                                                     storyParams.theVar,
                                                     storyParams.theWindDirectionMinSpeed));
      WindDirectionId directionId2(wind_direction_id(item2.theEqualizedWindDirection,
                                                     item2.theEqualizedTopWind,
                                                     storyParams.theVar,
                                                     storyParams.theWindDirectionMinSpeed));
      WindDirectionId directionId3(wind_direction_id(item3.theEqualizedWindDirection,
                                                     item3.theEqualizedTopWind,
                                                     storyParams.theVar,
                                                     storyParams.theWindDirectionMinSpeed));
      if (directionId1 != VAIHTELEVA && directionId2 == VAIHTELEVA && directionId3 != VAIHTELEVA)
      {
        item2.theEqualizedWindDirection =
            WeatherResult(item2.theEqualizedWindDirection.value(), 40.0);
        item2.theCorrectedWindDirection =
            WeatherResult(item2.theEqualizedWindDirection.value(), 40.0);
      }
    }
  }
}

void calculate_equalized_data(wo_story_params& storyParams)
{
  WeatherArea::Type areaType(storyParams.theArea.type());
  // first calculate the indexes
  if (storyParams.equalizedWSIndexesMedian(areaType).size() > 3)
    calculate_equalized_wind_speed_indexes_for_median_wind(storyParams);

  if (storyParams.equalizedWSIndexesTopWind(areaType).size() > 3)
    calculate_equalized_wind_speed_indexes_for_maximum_wind(storyParams, true);

  if (storyParams.equalizedWSIndexesMaxWind(areaType).size() > 3)
    calculate_equalized_wind_speed_indexes_for_maximum_wind(storyParams, false);

  if (storyParams.equalizedWDIndexes(areaType).size() > 3)
    calculate_equalized_wind_direction_indexes(storyParams);

  if (storyParams.equalizedWSIndexesCalcWind(areaType).size() > 3)
    calculate_equalized_wind_speed_indexes_for_calc_wind(storyParams);
}

void read_configuration_params(wo_story_params& storyParams)
{
  double windSpeedMaxError =
      Settings::optional_double(storyParams.theVar + "::max_error_wind_speed", 2.0);
  double windDirectionMaxError =
      Settings::optional_double(storyParams.theVar + "::max_error_wind_direction", 10.0);
  double windSpeedThreshold =
      Settings::optional_double(storyParams.theVar + "::wind_speed_threshold", 3.0);
  double windSpeedWarningThreshold =
      Settings::optional_double(storyParams.theVar + "::wind_speed_warning_threshold", 11.0);
  double windDirectionThreshold =
      Settings::optional_double(storyParams.theVar + "::wind_direction_threshold", 25.0);
  double windCalcTopShare =
      Settings::optional_double(storyParams.theVar + "::wind_calc_top_share", 80.0);
  double windCalcTopShareWeak =
      Settings::optional_double(storyParams.theVar + "::wind_calc_top_share_weak", 80.0);
  double windSpeedTopCoverage =
      Settings::optional_double(storyParams.theVar + "::wind_speed_top_coverage", 98.0);
  double gustyWindTopWindDifference =
      Settings::optional_double(storyParams.theVar + "::gusty_wind_max_wind_difference", 5.0);
  string rangeSeparator = Settings::optional_string(storyParams.theVar + "::rangeseparator", "-");
  unsigned int minIntervalSize =
      Settings::optional_int(storyParams.theVar + "::wind_speed_interval_min_size", 2);
  unsigned int maxIntervalSize =
      Settings::optional_int(storyParams.theVar + "::wind_speed_interval_max_size", 5);
  double windDirectionMinSpeed =
      Settings::optional_double(storyParams.theVar + "::wind_direction_min_speed", 6.5);

  storyParams.theWindSpeedMaxError = windSpeedMaxError;
  storyParams.theWindDirectionMaxError = windDirectionMaxError;
  storyParams.theWindSpeedThreshold = windSpeedThreshold;
  storyParams.theWindSpeedWarningThreshold = windSpeedWarningThreshold;
  storyParams.theWindDirectionThreshold = windDirectionThreshold;
  storyParams.theWindCalcTopShare = windCalcTopShare;
  storyParams.theWindCalcTopShareWeak = windCalcTopShareWeak;
  storyParams.theWindSpeedTopCoverage = windSpeedTopCoverage;
  storyParams.theWindDirectionMinSpeed = windDirectionMinSpeed;
  storyParams.theGustyWindTopWindDifference = gustyWindTopWindDifference;
  storyParams.theRangeSeparator = rangeSeparator;
  storyParams.theMinIntervalSize = minIntervalSize;
  storyParams.theMaxIntervalSize = maxIntervalSize;
  storyParams.theContextualMaxIntervalSize = maxIntervalSize;

  storyParams.theWeatherAreas.push_back(storyParams.theArea);

  std::string split_section_name("textgen::split_the_area::" +
                                 get_area_name_string(storyParams.theArea));
  std::string split_method_name(split_section_name + "::method");

  if (Settings::isset(split_method_name))
  {
    WeatherArea northernArea(storyParams.theArea);
    WeatherArea southernArea(storyParams.theArea);
    WeatherArea easternArea(storyParams.theArea);
    WeatherArea westernArea(storyParams.theArea);
    northernArea.type(WeatherArea::Northern);
    southernArea.type(WeatherArea::Southern);
    easternArea.type(WeatherArea::Eastern);
    westernArea.type(WeatherArea::Western);

    std::string split_method = Settings::require_string(split_method_name);
    if (split_method.compare("vertical") == 0)
    {
      storyParams.theWeatherAreas.push_back(easternArea);
      storyParams.theWeatherAreas.push_back(westernArea);
      storyParams.theSplitMethod = VERTICAL;
    }
    else if (split_method.compare("horizontal") == 0)
    {
      storyParams.theWeatherAreas.push_back(southernArea);
      storyParams.theWeatherAreas.push_back(northernArea);
      storyParams.theSplitMethod = HORIZONTAL;
    }
  }
}

float get_wind_direction_share(
    const vector<pair<float, WeatherResult> >& theWindDirectionDistribution,
    WindDirectionId windDirectionId,
    double theWindDirectionMinSpeed)
{
  float retval(0.0);

  for (unsigned int i = 0; i < theWindDirectionDistribution.size(); i++)
  {
    WeatherResult directionVar(theWindDirectionDistribution[i].first + 1.0, 0.0);
    WeatherResult speedVar(WeatherResult(5.0, 0.0));
    WindDirectionId directionId =
        wind_direction_id(directionVar, speedVar, "", theWindDirectionMinSpeed);
    if (directionId == windDirectionId) retval += theWindDirectionDistribution[i].second.value();
  }

  return retval;
}

float get_wind_direction_share(const wo_story_params& theParameters,
                               const WeatherPeriod& period,
                               const WindDirectionId& windDirectionId,
                               WindStoryTools::CompassType compass_type /*= sixteen_directions*/)
{
  float sumShare(0.0);
  unsigned int counter(0);

  for (unsigned int i = 0; i < theParameters.theWindDataVector.size(); i++)
  {
    WindDataItemUnit& item =
        theParameters.theWindDataVector[i]->getDataItem(theParameters.theArea.type());
    if (is_inside(item.thePeriod.localStartTime(), period))
    {
      sumShare += get_wind_direction_share(
          (compass_type == sixteen_directions ? item.theWindDirectionDistribution16
                                              : item.theWindDirectionDistribution8),
          windDirectionId,
          theParameters.theWindDirectionMinSpeed);
      counter++;
    }
  }

  return (counter == 0 ? 0.0 : sumShare / counter);
}

float WindDataItemUnit::getWindSpeedShare(float theLowerLimit, float theUpperLimit) const
{
  float retval = 0.0;

  for (unsigned int i = 0; i < theWindSpeedDistribution.size(); i++)
  {
    if (theWindSpeedDistribution[i].first >= theLowerLimit &&
        theWindSpeedDistribution[i].first < theUpperLimit)
      retval += theWindSpeedDistribution[i].second.value();
  }

  return retval;
}

float WindDataItemUnit::getTopWindSpeedShare(float theLowerLimit, float theUpperLimit) const
{
  float retval = 0.0;

  for (unsigned int i = 0; i < theWindSpeedDistributionTop.size(); i++)
  {
    if (theWindSpeedDistributionTop[i].first >= theLowerLimit &&
        theWindSpeedDistributionTop[i].first < theUpperLimit)
      retval += theWindSpeedDistributionTop[i].second.value();
  }

  return retval;
}

float WindDataItemUnit::getWindDirectionShare(
    WindDirectionId windDirectionId,
    double theWindDirectionMinSpeed,
    WindStoryTools::CompassType compass_type /* = sixteen_directions*/) const
{
  return get_wind_direction_share(
      (compass_type == sixteen_directions ? theWindDirectionDistribution16
                                          : theWindDirectionDistribution8),
      windDirectionId,
      theWindDirectionMinSpeed);
}

Paragraph WindStory::overview() const
{
  MessageLogger logger("WeatherStory::wind_overview");

  std::string areaName("");
  if (itsArea.isNamed())
  {
    areaName = itsArea.name();
    logger << "** " << areaName << " **" << endl;
  }

  Paragraph paragraph;

  wo_story_params storyParams(itsVar, itsArea, itsPeriod, itsForecastTime, itsSources, logger);

  // read the configuration parameters
  read_configuration_params(storyParams);

  // allocate data structures for hourly data
  allocate_data_structures(storyParams);

  // populate the data structures with the relevant data
  if (populate_time_series(storyParams))
  {
    // equalize the data
    calculate_equalized_data(storyParams);

    // find out wind event periods:
    // event periods are used to produce the story
    find_out_wind_event_periods(storyParams);

#ifndef NDEBUG
    log_raw_data(storyParams);

    // find out the wind speed periods (for logging purposes)
    find_out_wind_speed_periods(storyParams);
    // find out the wind direction periods of 16-direction compass (for logging purposes)
    find_out_wind_direction_periods(storyParams);

    // log functions
    // save_raw_data(storyParams);
    log_windirection_distribution(storyParams);
    log_raw_data(storyParams);
    log_equalized_wind_speed_data_vector(storyParams);
    log_equalized_wind_direction_data_vector(storyParams);
    log_wind_speed_periods(storyParams);
    log_wind_direction_periods(storyParams);
    log_wind_event_periods(storyParams);
#else
    log_wind_speed_periods(storyParams);
    log_wind_direction_periods(storyParams);
    log_raw_data(storyParams);
#endif
    WindForecast windForecast(storyParams);

    paragraph << windForecast.getWindStory(itsPeriod);

    static unsigned int js_id(1);

    if (Settings::optional_bool("qdtext::append_graph", false))
    {
      std::string html_string(Settings::optional_string("html__append", ""));
      html_string += get_js_code(js_id,
                                 html_string.empty(),
                                 ceil(get_top_wind(storyParams.theForecastPeriod, storyParams)));

      html_string += "</br><br>\n";
      html_string += "<hr size=\"3\" color=\"black\">\n";
      html_string += "<h5>";
      boost::gregorian::date startDate(
          boost::gregorian::from_undelimited_string(itsPeriod.localStartTime().ToStr(kYYYYMMDD)));
      html_string += boost::gregorian::to_simple_string(startDate);
      html_string += (itsPeriod.localStartTime().GetHour() < 10 ? " 0" : " ");
      html_string += Fmi::to_string(itsPeriod.localStartTime().GetHour());
      html_string += (itsPeriod.localStartTime().GetMin() < 10 ? ":0" : ":");
      html_string += Fmi::to_string(itsPeriod.localStartTime().GetMin());
      html_string += " - ";
      boost::gregorian::date endDate(
          boost::gregorian::from_undelimited_string(itsPeriod.localEndTime().ToStr(kYYYYMMDD)));
      html_string += boost::gregorian::to_simple_string(endDate);
      html_string += (itsPeriod.localEndTime().GetHour() < 10 ? " 0" : " ");
      html_string += Fmi::to_string(itsPeriod.localEndTime().GetHour());
      html_string += (itsPeriod.localEndTime().GetMin() < 10 ? ":0" : ":");
      html_string += Fmi::to_string(itsPeriod.localEndTime().GetMin());
      html_string += "</h5>";

      Settings::set("html__append", html_string);
    }

    if (Settings::optional_bool("qdtext::append_graph", false))
    {
      std::string html_string(Settings::optional_string("html__append", ""));
      html_string += get_js_data(storyParams, "windspeed", js_id);
      js_id++;
      html_string += get_js_data(storyParams, "winddirection", js_id);
      js_id++;
      Settings::set("html__append", html_string);
    }

    if (Settings::optional_bool("qdtext::append_rawdata", false))
    {
      std::string html_string(Settings::optional_string("html__append", ""));
      html_string += get_html_rawdata(storyParams);
      Settings::set("html__append", html_string);
    }
    if (Settings::optional_bool("qdtext::append_windspeed_distribution", false))
    {
      std::string html_string(Settings::optional_string("html__append", ""));
      html_string += get_html_windspeed_distribution(storyParams, "mean");
      html_string += get_html_windspeed_distribution(storyParams, "top");
      Settings::set("html__append", html_string);
    }
    if (Settings::optional_bool("qdtext::append_winddirection_distribution", false))
    {
      std::string html_string(Settings::optional_string("html__append", ""));
      html_string += get_html_winddirection_distribution(storyParams, sixteen_directions);
      html_string += get_html_winddirection_distribution(storyParams, eight_directions);
      Settings::set("html__append", html_string);
    }
  }

  deallocate_data_structures(storyParams);

  // logger << paragraph;

  return paragraph;
}
}  // namespace TextGen

// ======================================================================
