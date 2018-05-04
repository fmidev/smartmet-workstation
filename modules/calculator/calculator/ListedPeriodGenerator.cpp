// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::ListedPeriodGenerator
 */
// ======================================================================
/*!
 * \class TextGen::ListedPeriodGenerator
 *
 * \brief Generates a sequence of periods
 *
 * The class simply returns the periods fed to it in the same
 * order.
 */
// ----------------------------------------------------------------------

#include "ListedPeriodGenerator.h"
#include "Settings.h"
#include "TextGenError.h"

using namespace std;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theMainPeriod The period to iterate
 */
// ----------------------------------------------------------------------

ListedPeriodGenerator::ListedPeriodGenerator(const WeatherPeriod& theMainPeriod)
    : itsMainPeriod(theMainPeriod)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period is undivided
 *
 * \return Always false, listed period is never in principle the original
 */
// ----------------------------------------------------------------------

bool ListedPeriodGenerator::undivided() const { return false; }
// ----------------------------------------------------------------------
/*!
 * \brief Add a new period to be processed
 */
// ----------------------------------------------------------------------

void ListedPeriodGenerator::add(const WeatherPeriod& thePeriod) { itsPeriods.push_back(thePeriod); }
// ----------------------------------------------------------------------
/*!
 * \brief Return the number of subperiods
 *
 * \return The number of subperiods
 */
// ----------------------------------------------------------------------

ListedPeriodGenerator::size_type ListedPeriodGenerator::size() const { return itsPeriods.size(); }
// ----------------------------------------------------------------------
/*!
 * \brief Return the minimal period covered by the generator
 *
 * \return The minimal period
 */
// ----------------------------------------------------------------------

WeatherPeriod ListedPeriodGenerator::period() const { return itsMainPeriod; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the desired subperiod
 *
 * Throws if there is no such subperiod
 *
 * \param thePeriod The index of the subperiod
 * \return The subperiod
 */
// ----------------------------------------------------------------------

WeatherPeriod ListedPeriodGenerator::period(size_type thePeriod) const
{
  if (thePeriod < 1 || thePeriod > itsPeriods.size())
    throw TextGen::TextGenError("ListedPeriodGenerator::period(): invalid argument");
  return itsPeriods[thePeriod - 1];
}

}  // namespace TextGen

// ======================================================================
