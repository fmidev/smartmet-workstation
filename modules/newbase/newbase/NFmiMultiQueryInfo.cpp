// ======================================================================
/*!
 * \file NFmiMultiQueryInfo.cpp
 * \brief Implementation of class NFmiMultiQueryInfo
 */
// ======================================================================
/*!
 * \class NFmiMultiQueryInfo
 *
 * \brief Interface to retrieve values from multiple query data files
 *
 * Handles multiple query data files
 *
 */
// ======================================================================

#ifdef _MSC_VER
#pragma warning(disable : 4512)  // poistaa MSVC++2008 kääntäjän ikävän näköisen boost:ista tulevan
                                 // varoituksen
#endif

#include "NFmiMultiQueryInfo.h"
#include "NFmiCombinedParam.h"
#include "NFmiFileSystem.h"
#include "NFmiMetTime.h"
#include "NFmiQueryData.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>

#include <cassert>
#include <utility>

// ----------------------------------------------------------------------
/*!
 * \brief Find the data with the newest origin time
 */
// ----------------------------------------------------------------------

std::size_t find_newest_data(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfos)
{
  assert(!theInfos.empty());

  std::size_t best = 0;
  for (std::size_t i = 1; i < theInfos.size(); i++)
  {
    if (theInfos[i]->OriginTime() > theInfos[best]->OriginTime()) best = i;
  }
  return best;
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from a file name
 *
 * If the path refers to a directory, all querydata in the directory
 * is included if possible.
 */
// ----------------------------------------------------------------------

NFmiMultiQueryInfo::NFmiMultiQueryInfo(const std::string &thePath)
    : itsDatas(), itsInfos(), itsMultiIndexes(), itsMultiTimeIndex(kUnsignedLongMissing)
{
  std::list<std::string> files;

  if (!NFmiFileSystem::DirectoryExists(thePath))
  {
    if (!NFmiFileSystem::FileReadable(thePath))
      throw std::runtime_error("File '" + thePath + "' is not readable");

    files.push_back(thePath);
  }
  else
  {
    // Construct from multiple querydatas
    std::list<std::string> dirfiles = NFmiFileSystem::DirectoryFiles(thePath);

    BOOST_FOREACH (const std::string &name, dirfiles)
    {
      if (name.empty() || name[0] == '.') continue;

      if (boost::iends_with(name, ".sqd") || boost::iends_with(name, ".fqd") ||
          boost::iends_with(name, ".sqd.gz") || boost::iends_with(name, ".fqd.gz") ||
          boost::iends_with(name, ".sqd.bz2") || boost::iends_with(name, ".fqd.bz2"))
      {
        std::string filename = thePath + '/' + name;
        if (NFmiFileSystem::FileSize(filename) != 0) files.push_back(filename);
      }
    }
  }

  Init(files);
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from a list of files
 */
// ----------------------------------------------------------------------

NFmiMultiQueryInfo::NFmiMultiQueryInfo(const std::list<std::string> &theFiles)
    : itsDatas(), itsInfos(), itsMultiIndexes(), itsMultiTimeIndex(kUnsignedLongMissing)
{
  Init(theFiles);
}

// ----------------------------------------------------------------------
/*!
 * \brief Construct from multiple NFmiFastQueryInfo objects
 */
// ----------------------------------------------------------------------

NFmiMultiQueryInfo::NFmiMultiQueryInfo(std::vector<boost::shared_ptr<NFmiFastQueryInfo> > &theInfos)
    : itsDatas()  // will remain empty
      ,
      itsInfos(theInfos),
      itsMultiIndexes(),
      itsMultiTimeIndex(kUnsignedLongMissing)
{
  if (theInfos.size() == 0)
    throw std::runtime_error("Cannot construct NFmiMultiQueryInfo from zero NFmiFastQueryInfos");

  Init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize from a list of files
 */
// ----------------------------------------------------------------------

void NFmiMultiQueryInfo::Init(const std::list<std::string> &theFiles)
{
  BOOST_FOREACH (const std::string &filename, theFiles)
  {
    boost::shared_ptr<NFmiQueryData> qd(new NFmiQueryData(filename));
    itsDatas.push_back(qd);
    boost::shared_ptr<NFmiFastQueryInfo> qi(new NFmiFastQueryInfo(qd.get()));
    itsInfos.push_back(qi);

    itsFileModificationTimes.push_back(NFmiFileSystem::FileModificationTime(filename));
  }

  Init();
}

// ----------------------------------------------------------------------
/*!
 * \brief Initialize the rest once NFmiFastQueryInfo objects are initialized
 */
// ----------------------------------------------------------------------

void NFmiMultiQueryInfo::Init()
{
  // Establish the info which refers to data with the newest origin time

  std::size_t ref_index = find_newest_data(itsInfos);

  // Use it as a reference for acceptable parameters etc

  NFmiFastQueryInfo::operator=(*itsInfos[ref_index]);

  // Now collect all unique times from data which has acceptable descriptors.
  // For each unique time choose the data with the latest origin time, and
  // store the respective index into the query infos and the time index
  // inside that data. Note that if there are multiple equal origintimes
  // we must compare the modification time of the respective files.
  // Otherwise we may miss fixes to model data etc.

  typedef std::set<MultiIndex> MultiIndexSet;
  MultiIndexSet time_index;

  for (std::size_t i = 0; i < itsInfos.size(); i++)
  {
    // Comparing the indices first avoids a descriptor comparison when the reference data is
    // encountered
    bool acceptable = (i == ref_index || (ParamDescriptor() == itsInfos[i]->ParamDescriptor() &&
                                          HPlaceDescriptor() == itsInfos[i]->HPlaceDescriptor() &&
                                          VPlaceDescriptor() == itsInfos[i]->VPlaceDescriptor()));

    if (acceptable)
    {
      // Try to insert all times into the set of unique times
      NFmiFastQueryInfo &qi = *itsInfos[i];
      for (qi.ResetTime(); qi.NextTime();)
      {
        const NFmiMetTime &t = qi.ValidTime();
        MultiIndex idx(t, i, qi.TimeIndex());

        auto res = time_index.insert(idx);

        // Resolve conflict when multiple datas provide the same valid time
        if (res.second == false)
        {
          std::size_t old_index = res.first->info_index;

          bool replace_data = false;

          if (itsInfos[i]->OriginTime() > itsInfos[old_index]->OriginTime())
            replace_data = true;
          else if (itsInfos[i]->OriginTime() == itsInfos[old_index]->OriginTime())
          {
            if (!itsFileModificationTimes.empty())  // may not be available
              if (itsFileModificationTimes[i] > itsFileModificationTimes[old_index])
                replace_data = true;
          }

          if (replace_data)
          {
            time_index.erase(res.first);
            time_index.insert(idx);
          }
        }
      }
    }
  }

  // time -> index set correspondance is now established, initialize the data structures accordingly

  // int j = 0;
  BOOST_FOREACH (const auto &idx, time_index)
  {
    itsMultiIndexes.push_back(idx);
  }
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the origin time
 */
// ----------------------------------------------------------------------

const NFmiMetTime &NFmiMultiQueryInfo::OriginTime() const
{
  // Origin time for selected time
  if (itsMultiTimeIndex < itsMultiIndexes.size())
    return itsInfos.at(itsMultiIndexes[itsMultiTimeIndex].info_index)->OriginTime();

  // Otherwise the best guess is the last origin time
  auto last_idx = itsMultiIndexes.size() - 1;
  return itsInfos.at(itsMultiIndexes[last_idx].info_index)->OriginTime();
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the time index
 */
// ----------------------------------------------------------------------

unsigned long NFmiMultiQueryInfo::TimeIndex() const { return itsMultiTimeIndex; }
// ----------------------------------------------------------------------
/*!
 * \brief Set the time index
 */
// ----------------------------------------------------------------------

bool NFmiMultiQueryInfo::TimeIndex(unsigned long theIndex)
{
  if (theIndex >= itsMultiIndexes.size()) return false;

  itsMultiTimeIndex = theIndex;

  const MultiIndex &idx = itsMultiIndexes[itsMultiTimeIndex];

  itsRefRawData = const_cast<NFmiRawData *>(itsInfos[idx.info_index]->RefRawData());
  itsRefQueryData = const_cast<NFmiQueryData *>(itsInfos[idx.info_index]->RefQueryData());

  itsTimeIndex = idx.time_index;

  itsLocLevTimSize = itsInfos[idx.info_index]->itsLocLevTimSize;
  itsLevTimSize = itsInfos[idx.info_index]->itsLevTimSize;
  itsTimeSize = itsInfos[idx.info_index]->itsTimeSize;

  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Return the current valid time
 */
// ----------------------------------------------------------------------

const NFmiMetTime &NFmiMultiQueryInfo::ValidTime() const
{
  if (itsMultiTimeIndex < itsMultiIndexes.size())
    return itsMultiIndexes[itsMultiTimeIndex].valid_time;

  throw std::runtime_error("Trying to access valid time for time index -1");
}

// ----------------------------------------------------------------------
/*!
 * \brief Reset time iterator
 */
// ----------------------------------------------------------------------

void NFmiMultiQueryInfo::ResetTime() { itsMultiTimeIndex = kUnsignedLongMissing; }
// ----------------------------------------------------------------------
/*!
 * \return Next valid time
 */
// ----------------------------------------------------------------------

bool NFmiMultiQueryInfo::NextTime()
{
  if (itsMultiTimeIndex == kUnsignedLongMissing)
  {
    TimeIndex(0);
    return true;
  }

  if (++itsMultiTimeIndex >= itsMultiIndexes.size())
  {
    itsMultiTimeIndex = kUnsignedLongMissing;
    return false;
  }

  TimeIndex(itsMultiTimeIndex);
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \return Last valid time
 */
// ----------------------------------------------------------------------

bool NFmiMultiQueryInfo::LastTime() { return TimeIndex(itsMultiIndexes.size() - 1); }
// ----------------------------------------------------------------------
/*!
 * \return First valid time
 */
// ----------------------------------------------------------------------

bool NFmiMultiQueryInfo::FirstTime() { return TimeIndex(0); }
// ----------------------------------------------------------------------
/*!
 * \return Previous valid time
 */
// ----------------------------------------------------------------------

bool NFmiMultiQueryInfo::PreviousTime()
{
  if (itsMultiTimeIndex == kUnsignedLongMissing || itsMultiTimeIndex == 0)
  {
    itsMultiTimeIndex = kUnsignedLongMissing;
    return false;
  }

  TimeIndex(itsMultiTimeIndex - 1);
  return true;
}

// ----------------------------------------------------------------------
/*!
 * \brief Set the desired time
 */
// ----------------------------------------------------------------------

bool NFmiMultiQueryInfo::Time(const NFmiMetTime &theTime)
{
  std::vector<MultiIndex>::const_iterator pos =
      std::lower_bound(itsMultiIndexes.begin(), itsMultiIndexes.end(), theTime);

  if (pos != itsMultiIndexes.end() && pos->valid_time == theTime)
    return TimeIndex(pos - itsMultiIndexes.begin());

  itsMultiTimeIndex = kUnsignedLongMissing;
  return false;
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the given time is inside the time period defined by the data
 */
// ----------------------------------------------------------------------

bool NFmiMultiQueryInfo::IsInside(const NFmiMetTime &theTime) const
{
  return (itsMultiIndexes[0].valid_time <= theTime &&
          itsMultiIndexes[itsMultiIndexes.size() - 1].valid_time >= theTime);
}

// ----------------------------------------------------------------------
/*!
 * \brief Find the nearest time in the given direction
 */
// ----------------------------------------------------------------------

bool NFmiMultiQueryInfo::TimeToNearestStep(const NFmiMetTime &theTime,
                                           FmiDirection theDirection,
                                           long theTimeRangeInMinutes)
{
  std::vector<MultiIndex>::const_iterator pos =
      std::lower_bound(itsMultiIndexes.begin(), itsMultiIndexes.end(), theTime);

  // Algorithm from NFmiTimeList::FindNearestTime

  if (pos != itsMultiIndexes.end())
  {
    if (theDirection == kBackward && theTime < pos->valid_time)
    {
      if (pos != itsMultiIndexes.begin())
      {
        --pos;
        if (pos ==
            itsMultiIndexes
                .end())  // en tiedä toimiiko siten, että jos tekee startissa --, menee ohi vectorin
          ++pos;
      }
    }
    else if (theDirection == kCenter)
    {
      int index = pos - itsMultiIndexes.begin();
      if (index > 0)
      {
        double diff1 = theTime.DifferenceInMinutes(itsMultiIndexes[index].valid_time);
        double diff2 = theTime.DifferenceInMinutes(itsMultiIndexes[index - 1].valid_time);
        if (fabs(diff2) < fabs(diff1)) --pos;
      }
    }
    // HUOM! else eli muuten tai theDirection == kForward vaihtoehto puuttuu!!!!

    int indexFinal = pos - itsMultiIndexes.begin();
    if (indexFinal != -1 && theTimeRangeInMinutes != kLongMissing)
    {
      double diffFinal = theTime.DifferenceInMinutes(itsMultiIndexes[indexFinal].valid_time);
      if (fabs(diffFinal) > theTimeRangeInMinutes) return false;
    }
    TimeIndex(indexFinal);
    return true;
  }
  if (theTimeRangeInMinutes == kLongMissing)
  {
    TimeIndex(itsMultiIndexes.size() - 1);
    return true;
  }

  // tässä pitäisi olla vielä else haara, joka tarkistaa kelpaako aika jos se on listan
  // ulkopuolöella, mutta tarpeeksi lähellä
  return false;
}

// ----------------------------------------------------------------------
/*!
 * Calculates interpolated value for the requested time.
 *
 * \param  theTime the time
 * \param  theMaxMinuteRange
 *
 * \return The interpolated value.
 *
 */
// ----------------------------------------------------------------------

float NFmiMultiQueryInfo::InterpolatedValue(const NFmiMetTime &theTime, int theMaxMinuteRange)
{
  std::size_t oldTimeIndex = itsMultiTimeIndex;

  MultiIndexes::const_iterator pos =
      std::lower_bound(itsMultiIndexes.begin(), itsMultiIndexes.end(), theTime);

  float value = InterpolatedValue(theTime, pos, theMaxMinuteRange);

  TimeIndex(oldTimeIndex);

  return value;
}

float NFmiMultiQueryInfo::InterpolatedValue(const NFmiMetTime &theTime,
                                            const MultiIndexes::const_iterator &thePos,
                                            int theMaxMinuteRange)
{
  if (thePos == itsMultiIndexes.end()) return kFloatMissing;

  if (thePos->valid_time == theTime)
  {
    TimeIndex(thePos - itsMultiIndexes.begin());
    return FloatValue();
  }

  // Interpolate if possible

  unsigned long idx2 = thePos - itsMultiIndexes.begin();
  if (idx2 == 0) return kFloatMissing;

  unsigned long idx1 = idx2 - 1;

  // Find previous value to use in interpolation

  float value1 = kFloatMissing;
  while (true)
  {
    TimeIndex(idx1);
    value1 = FloatValue();
    if ((value1 != kFloatMissing && value1 != kTCombinedWeatherFloatMissing) || idx1 == 0) break;
    --idx1;
  }

  const NFmiMetTime &time1 = itsMultiIndexes[idx1].valid_time;

  if (theMaxMinuteRange != 0 && abs(theTime.DifferenceInMinutes(time1)) > theMaxMinuteRange)
    return kFloatMissing;

  // Find next value to use in interpolation

  float value2 = kFloatMissing;
  while (true)
  {
    TimeIndex(idx2);
    value2 = FloatValue();
    if ((value2 != kFloatMissing && value2 != kTCombinedWeatherFloatMissing) ||
        idx2 == itsMultiIndexes.size() - 1)
      break;
    ++idx2;
  }

  const NFmiMetTime &time2 = itsMultiIndexes[idx2].valid_time;

  if (theMaxMinuteRange != 0 && abs(theTime.DifferenceInMinutes(time2)) > theMaxMinuteRange)
    return kFloatMissing;

  // Interpolate.

  switch (Param().GetParamIdent())
  {
    case kFmiWindDirection:
    {
      Param(kFmiWindSpeedMS);  // asetetaan parametriksi väliaikaisesti tuulennopeus
      TimeIndex(idx1);  // otetaan 1. tuulen nopeus samalta ajalta kuin vastaava tuulen suunta
      float ws1 = FloatValue();
      TimeIndex(idx2);  // otetaan 2. tuulen nopeus samalta ajalta kuin vastaava tuulen suunta
      float ws2 = FloatValue();
      Param(kFmiWindDirection);  // palautetaan tuulensuunta takaisin parametriksi
      NFmiInterpolation::WindInterpolator windInterpolator;
      float offset1 = CalcTimeOffsetToLastTime(theTime, time1, time2);
      windInterpolator.operator()(ws1, value1, offset1);
      windInterpolator.operator()(ws2, value2, (1 - offset1));
      return static_cast<float>(windInterpolator.Direction());
    }
    default:
    {
      return Interpolate(Param(), theTime, time1, time2, value1, value2);
    }
  }
}

// ======================================================================
