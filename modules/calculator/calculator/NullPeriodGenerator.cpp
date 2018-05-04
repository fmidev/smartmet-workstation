// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::NullPeriodGenerator
 */
// ======================================================================
/*!
 * \class TextGen::NullPeriodGenerator
 *
 * \brief Generates a sequence of periods
 *
 * This class always returns the main period as is.
 */
// ----------------------------------------------------------------------

// boost included laitettava ennen newbase:n NFmiGlobals-includea,
// muuten MSVC:ss‰ min max m‰‰rittelyt jo tehty

#include <boost/lexical_cast.hpp>

#include "NullPeriodGenerator.h"
#include "TextGenError.h"

using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*!
 * \brief Constructor
 *
 * \param theMainPeriod The period to iterate
 */
// ----------------------------------------------------------------------

NullPeriodGenerator::NullPeriodGenerator(const WeatherPeriod& theMainPeriod)
    : itsMainPeriod(theMainPeriod)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Test if the period is undivided
 *
 * \return Always true, null period is the original one
 */
// ----------------------------------------------------------------------

bool NullPeriodGenerator::undivided() const { return true; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the number of subperiods
 *
 * \return The number of subperiods (always 1)
 */
// ----------------------------------------------------------------------

NullPeriodGenerator::size_type NullPeriodGenerator::size() const { return 1; }
// ----------------------------------------------------------------------
/*!
 * \brief Return the minimal period covered by the generator
 *
 * \return The minimal period
 */
// ----------------------------------------------------------------------

WeatherPeriod NullPeriodGenerator::period() const { return itsMainPeriod; }
// ----------------------------------------------------------------------
/*!
 *†\brief Return the desired subperiod
 *
 * Throws if anything but the first period is requested, since by
 * definition there is always exactly 1 subperiod.
 *
 * \param thePeriod The index of the subperiod
 * \return The subperiod
 */
// ----------------------------------------------------------------------

WeatherPeriod NullPeriodGenerator::period(size_type thePeriod) const
{
  if (thePeriod == 1) return itsMainPeriod;

  const string msg =
      ("NullPeriodGenerator cannot return period " + lexical_cast<string>(thePeriod));
  throw TextGenError(msg);
}

}  // namespace TextGen

// ======================================================================
