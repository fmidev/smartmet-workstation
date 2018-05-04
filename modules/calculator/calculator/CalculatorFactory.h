// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::CalculatorFactory
 */
// ======================================================================

#ifndef TEXTGEN_CALCULATORFACTORY_H
#define TEXTGEN_CALCULATORFACTORY_H

#include "WeatherFunction.h"

#include "Calculator.h"

namespace TextGen
{
class Acceptor;

namespace CalculatorFactory
{
Calculator* create(WeatherFunction theFunction);

Calculator* create(WeatherFunction theFunction, int theModulo);

Calculator* create(WeatherFunction theFunction, const Acceptor& theTester);

Calculator* create(WeatherFunction theFunction, const Acceptor& theTester, int theModulo);

}  // namespace CalculatorFactory
}  // namespace TextGen

#endif  // TEXTGEN_CALCULATORFACTORY_H

// ======================================================================
