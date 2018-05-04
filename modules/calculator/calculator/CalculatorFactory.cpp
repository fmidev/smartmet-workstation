// ======================================================================
/*!
 * \file
 * \brief Implementation of namespace TextGen::CalculatorFactory
 */
// ======================================================================
/*!
 * \namespace TextGen::CalculatorFactory
 *
 * \brief Creating TextGen::Calculator objects
 *
 */
// ======================================================================

#include "CalculatorFactory.h"
#include "ChangeCalculator.h"
#include "CountCalculator.h"
#include "MaximumCalculator.h"
#include "MeanCalculator.h"
#include "MinimumCalculator.h"
#include "MedianCalculator.h"
#include "ModChangeCalculator.h"
#include "ModMeanCalculator.h"
#include "ModStandardDeviationCalculator.h"
#include "ModTrendCalculator.h"
#include "NullCalculator.h"
#include "PercentageCalculator.h"
#include "RangeAcceptor.h"
#include "StandardDeviationCalculator.h"
#include "SumCalculator.h"
#include "TrendCalculator.h"
#include "TextGenError.h"

#include <boost/lexical_cast.hpp>

#include <string>

using namespace boost;
using namespace std;

namespace TextGen
{
namespace CalculatorFactory
{
// ----------------------------------------------------------------------
/*!
 * \brief Create an calculator suitable for the given WeatherFunction
 *
 * Throws if there is no suitable data modifier.
 *
 * \param theFunction The weather function
 * \return The data modifier
 */
// ----------------------------------------------------------------------

Calculator* create(WeatherFunction theFunction)
{
  switch (theFunction)
  {
    case Mean:
      return new MeanCalculator;
    case Maximum:
      return new MaximumCalculator;
    case Minimum:
      return new MinimumCalculator;
    case Median:
      return new MedianCalculator;
    case Sum:
      return new SumCalculator;
    case Percentage:
      return new PercentageCalculator;
    case Count:
      return new CountCalculator;
    case Trend:
      return new TrendCalculator;
    case Change:
      return new ChangeCalculator;
    case NullFunction:
      return new NullCalculator;
    case StandardDeviation:
      return new StandardDeviationCalculator;
  }

  throw TextGenError("CalculatorFactory failed to recognize the given function" +
                     lexical_cast<string>(static_cast<int>(theFunction)));
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a modular calculator suitable for the given WeatherFunction
 *
 * Throws if there is no suitable data modifier.
 *
 * \param theFunction The weather function
 * \param theModulo The modulo value
 * \return The data modifier
 */
// ----------------------------------------------------------------------

Calculator* create(WeatherFunction theFunction, int theModulo)
{
  switch (theFunction)
  {
    case Mean:
      return new ModMeanCalculator(theModulo);
    case StandardDeviation:
      return new ModStandardDeviationCalculator(theModulo);
    case Change:
      return new ModChangeCalculator(theModulo);
    case Trend:
      return new ModTrendCalculator(theModulo);

    case Percentage:
      return new PercentageCalculator;
    case Count:
      return new CountCalculator;
    case NullFunction:
      return new NullCalculator;

    case Median:
      throw TextGenError("CalculatorFactory cannot create modular Median analyzer");
    case Maximum:
      throw TextGenError("CalculatorFactory cannot create modular Maximum analyzer");
    case Minimum:
      throw TextGenError("CalculatorFactory cannot create modular Minimum analyzer");
    case Sum:
      throw TextGenError("CalculatorFactory cannot create modular Sum analyzer");
  }

  throw TextGenError("CalculatorFactory failed to recognize the given function" +
                     lexical_cast<string>(static_cast<int>(theFunction)));
}

// ----------------------------------------------------------------------
/*!
 * \brief Create an calculator suitable for the given WeatherFunction
 *
 * Throws if there is no suitable data modifier.
 *
 * \param theFunction The weather function
 * \param theTester The tester for Percentage calculations
 * \return The data modifier
 */
// ----------------------------------------------------------------------

Calculator* create(WeatherFunction theFunction, const Acceptor& theTester)
{
  switch (theFunction)
  {
    case Mean:
    case Maximum:
    case Minimum:
    case Median:
    case StandardDeviation:
    case Sum:
    case Trend:
    case Change:
    case NullFunction:
      return create(theFunction);
    case Percentage:
    {
      PercentageCalculator* tmp = new PercentageCalculator;
      tmp->condition(theTester);
      return tmp;
    }
    case Count:
    {
      CountCalculator* tmp = new CountCalculator;
      tmp->condition(theTester);
      return tmp;
    }
  }

  throw TextGenError("CalculatorFactory failed to recognize the given function" +
                     lexical_cast<string>(static_cast<int>(theFunction)));
}

// ----------------------------------------------------------------------
/*!
 * \brief Create a modular calculator suitable for the given WeatherFunction
 *
 * Throws if there is no suitable data modifier.
 *
 * \param theFunction The weather function
 * \param theTester The tester for Percentage calculations
 * \param theModulo The modulo value
 * \return The data modifier
 */
// ----------------------------------------------------------------------

Calculator* create(WeatherFunction theFunction, const Acceptor& theTester, int theModulo)
{
  switch (theFunction)
  {
    case Mean:
    case Maximum:
    case Minimum:
    case Median:
    case StandardDeviation:
    case Sum:
    case Trend:
    case Change:
    case NullFunction:
      return create(theFunction, theModulo);
    case Percentage:
    {
      PercentageCalculator* tmp = new PercentageCalculator;
      tmp->condition(theTester);
      return tmp;
    }
    case Count:
    {
      CountCalculator* tmp = new CountCalculator;
      tmp->condition(theTester);
      return tmp;
    }
  }

  throw TextGenError("CalculatorFactory failed to recognize the given function" +
                     lexical_cast<string>(static_cast<int>(theFunction)));
}

}  // namespace CalculatorFactory
}  // namespace TextGen

// ======================================================================
