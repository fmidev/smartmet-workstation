// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::HeaderFactory
 */
// ======================================================================
/*!
 * \namespace TextGen::HeaderFactory
 *
 * \brief Producers for the headers of various weather periods
 *
 * The namespace consists of functions which generate a header
 * text of desired type for the given weather period.
 *
 * \see page_aikavalit
 *
 */
// ======================================================================

#include "HeaderFactory.h"
#include "Header.h"
#include "Integer.h"
#include "IntegerRange.h"
#include "LocationPhrase.h"
#include "MessageLogger.h"
#include <calculator/Settings.h>
#include <calculator/TextGenError.h>
#include <calculator/WeatherArea.h>
#include <calculator/WeatherPeriod.h>
#include "WeekdayTools.h"

#include <boost/lexical_cast.hpp>

using namespace TextGen;
using namespace std;
using namespace boost;

// ======================================================================
//				IMPLEMENTATION HIDING FUNCTIONS
// ======================================================================

namespace
{
// ----------------------------------------------------------------------
/*!
 * \brief Return empty header
 *
 * \param thePeriod The weather period
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_none(const WeatherPeriod& thePeriod, const string& theVariable)
{
  MessageLogger log("header_none");
  using namespace TextGen;
  Header header;
  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return header of type "Odotettavissa tiistai-iltaan asti"
 *
 * Note that the ending time must be either 06 or 18 local time,
 * otherwise an exception is thrown.
 *
 * \param thePeriod The weather period
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_until(const WeatherPeriod& thePeriod, const string& theVariable)
{
  MessageLogger log("header_until");
  using namespace TextGen;
  Header header;

  header << "odotettavissa" << WeekdayTools::until_weekday_time(thePeriod.localEndTime()) << "asti";

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return header of type "Odotettavissa tiistai-illasta torstaiaamuun asti"
 *
 * Note that the start and end times must be either 06 or 18 local time,
 * otherwise an exception is thrown.
 *
 * \param thePeriod The weather period
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_from_until(const WeatherPeriod& thePeriod, const string& theVariable)
{
  MessageLogger log("header_from_until");
  using namespace TextGen;
  Header header;

  header << "odotettavissa" << WeekdayTools::from_weekday_time(thePeriod.localStartTime())
         << WeekdayTools::until_weekday_time(thePeriod.localEndTime());

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return header of type "Odotettavissa seuraavan viiden vuorokauden aikana"
 *
 * The period start time must be 06 or 18. The period length must be
 * a multiple of 24.
 *
 * \param thePeriod The weather period
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_several_days(const WeatherPeriod& thePeriod, const string& theVariable)
{
  MessageLogger log("header_several_days");
  using namespace TextGen;
  Header header;

  const TextGenPosixTime& startTime = thePeriod.localStartTime();
  const TextGenPosixTime& endTime = thePeriod.localEndTime();
  const long diff = endTime.DifferenceInHours(startTime);
  const long days = diff / 24;

  if (diff % 24 != 0) throw TextGenError("HeaderFactory:: several_days must be N*24 hours long");
  header << WeekdayTools::from_weekday_time(thePeriod.localStartTime()) << "alkavan"
         << lexical_cast<string>(days) + "-vuorokauden saa";

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return report header for specific area
 *
 * \param theArea The named area
 * \param thePeriod The time period (only start time is relevant)
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_report_area(const WeatherArea& theArea,
                                   const WeatherPeriod& thePeriod,
                                   const string& theVariable)
{
  MessageLogger log("header_report_area");
  using namespace TextGen;

  Header header;

  if (!theArea.isNamed())
    throw TextGenError("Cannot generate report_area title for an unnamed point");

  const int starthour = thePeriod.localStartTime().GetHour();

  header << "saaennuste" << LocationPhrase(theArea.name() + ":lle")
         << WeekdayTools::on_weekday_time(thePeriod.localStartTime()) << TextGen::Integer(starthour)
         << "o'clock";

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return report header for location name
 *
 * \param theArea The area (must be named)
 * \param thePeriod The time period (only start time is relevant)
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_report_location(const WeatherArea& theArea,
                                       const WeatherPeriod& thePeriod,
                                       const string& theVariable)
{
  MessageLogger log("header_report_location");
  using namespace TextGen;

  Header header;

  if (!theArea.isNamed())
    throw TextGenError("Cannot generate report_location title for an unnamed point");

  header << LocationPhrase(theArea.name());

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return report header for forecast time
 *
 * \param theArea The area (must not be a point)
 * \param thePeriod The time period (only start time is relevant)
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_report_time(const WeatherArea& theArea,
                                   const WeatherPeriod& thePeriod,
                                   const string& theVariable)
{
  MessageLogger log("header_report_area");
  using namespace TextGen;

  Header header;

  const int starthour = thePeriod.localStartTime().GetHour();

  header << "saaennuste" << WeekdayTools::on_weekday_time(thePeriod.localStartTime())
         << TextGen::Integer(starthour) << "o'clock";

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return header of type "Odotettavissa maanantaina aamulla"
 *
 * \param thePeriod The weather period
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_morning(const WeatherPeriod& thePeriod, const string& theVariable)
{
  MessageLogger log("header_morning");
  using namespace TextGen;
  Header header;

  const bool weekdays = Settings::optional_bool(theVariable + "::weekdays", false);

  if (!weekdays)
  {
    header << "odotettavissa"
           << "aamulla";
  }
  else
  {
    header << "odotettavissa" << WeekdayTools::on_weekday_morning(thePeriod.localStartTime());
  }

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return header of type "Odotettavissa maanantaina aamupaivalla"
 *
 * \param thePeriod The weather period
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_forenoon(const WeatherPeriod& thePeriod, const string& theVariable)
{
  MessageLogger log("header_forenoon");
  using namespace TextGen;
  Header header;

  const bool weekdays = Settings::optional_bool(theVariable + "::weekdays", false);

  if (!weekdays)
  {
    header << "odotettavissa"
           << "aamupaivalla";
  }
  else
  {
    header << "odotettavissa" << WeekdayTools::on_weekday_forenoon(thePeriod.localStartTime());
  }

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return header of type "Odotettavissa maanantaina iltapaivalla"
 *
 * \param thePeriod The weather period
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_afternoon(const WeatherPeriod& thePeriod, const string& theVariable)
{
  MessageLogger log("header_afternoon");
  using namespace TextGen;
  Header header;

  const bool weekdays = Settings::optional_bool(theVariable + "::weekdays", false);

  if (!weekdays)
  {
    header << "odotettavissa"
           << "iltapaivalla";
  }
  else
  {
    header << "odotettavissa" << WeekdayTools::on_weekday_afternoon(thePeriod.localStartTime());
  }

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return header of type "Odotettavissa maanantaina illalla"
 *
 * \param thePeriod The weather period
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_evening(const WeatherPeriod& thePeriod, const string& theVariable)
{
  MessageLogger log("header_evening");
  using namespace TextGen;
  Header header;

  const bool weekdays = Settings::optional_bool(theVariable + "::weekdays", false);

  if (!weekdays)
  {
    header << "odotettavissa"
           << "illalla";
  }
  else
  {
    header << "odotettavissa" << WeekdayTools::on_weekday_evening(thePeriod.localStartTime());
  }

  log << header;
  return header;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return header of type "Odotettavissa maanantaina kello 10-12"
 *
 * \param thePeriod The weather period
 * \param theVariable The variable for extra settings
 * \return The header
 */
// ----------------------------------------------------------------------

TextGen::Header header_clock_range(const WeatherPeriod& thePeriod, const string& theVariable)
{
  MessageLogger log("header_clock_range");
  using namespace TextGen;
  Header header;

  const bool weekdays = Settings::optional_bool(theVariable + "::weekdays", false);
  const int starthour = thePeriod.localStartTime().GetHour();
  const int endhour = thePeriod.localEndTime().GetHour();

  if (!weekdays)
  {
    header << "odotettavissa"
           << "kello" << IntegerRange(starthour, endhour, "-") << "o'clock";
  }
  else
  {
    header << "odotettavissa" << WeekdayTools::on_weekday(thePeriod.localStartTime()) << "kello"
           << IntegerRange(starthour, endhour, "-") << "o'clock";
  }

  log << header;
  return header;
}

}  // namespace anonymous

// ======================================================================
//				PUBLIC INTERFACE
// ======================================================================

namespace TextGen
{
namespace HeaderFactory
{
// ----------------------------------------------------------------------
/*!
 * \brief Produce headers of desired type
 *
 * Throws if the type is unknown.
 *
 * \see page_aikavalit
 *
 * \param theArea The area concerned
 * \param thePeriod The relevant weather period
 * \param theVariable The variable
 * \return The generated header
 */
// ----------------------------------------------------------------------

Header create(const WeatherArea& theArea,
              const WeatherPeriod& thePeriod,
              const std::string& theVariable)
{
  MessageLogger log("HeaderFactory::create");

  const string type = Settings::require_string(theVariable + "::type");

  if (type == "none") return header_none(thePeriod, theVariable);
  if (type == "until") return header_until(thePeriod, theVariable);
  if (type == "from_until") return header_from_until(thePeriod, theVariable);
  if (type == "several_days") return header_several_days(thePeriod, theVariable);
  if (type == "report_area") return header_report_area(theArea, thePeriod, theVariable);
  if (type == "report_time") return header_report_time(theArea, thePeriod, theVariable);
  if (type == "report_location") return header_report_location(theArea, thePeriod, theVariable);
  if (type == "morning") return header_morning(thePeriod, theVariable);
  if (type == "forenoon") return header_forenoon(thePeriod, theVariable);
  if (type == "afternoon") return header_afternoon(thePeriod, theVariable);
  if (type == "evening") return header_evening(thePeriod, theVariable);
  if (type == "clock_range") return header_clock_range(thePeriod, theVariable);

  throw TextGenError("HeaderFactory does not recognize header type " + type);
}

}  // namespace HeaderFactory
}  // namespace TextGen

// ======================================================================
