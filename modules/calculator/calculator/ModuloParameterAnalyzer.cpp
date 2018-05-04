// ======================================================================
/*!
 * \file
 * \brief Implementation of class TextGen::ModuloParameterAnalyzer
 */
// ======================================================================
/*!
 * \class TextGen::ModuloParameterAnalyzer
 *
 * \brief Modulo parameter analysis functions
 *
 */
// ======================================================================

#include "ModuloParameterAnalyzer.h"
#include "AnalysisSources.h"
#include "RegularFunctionAnalyzer.h"
#include "WeatherResult.h"

using namespace std;
using namespace boost;

namespace TextGen
{
// ----------------------------------------------------------------------
/*
 * \brief Constructor
 *
 * \param theVariable The variable name, usually textgen::parametername
 * \param theParameter The parameter name
 * \param theModulo The modulo value
 */
// ----------------------------------------------------------------------

ModuloParameterAnalyzer::ModuloParameterAnalyzer(const string& theVariable,
                                                 const string& theParameter,
                                                 int theModulo)
    : itsVariable(theVariable), itsParameter(theParameter), itsModulo(theModulo)
{
}

// ----------------------------------------------------------------------
/*!
 * \brief Analyze a regular parameter modulo some value in an area
 *
 * \param theSources Analysis sources
 * \param theDataType The source type to be used
 * \param theAreaFunction The area function to analyze
 * \param theTimeFunction The time function to analyze
 * \param theSubTimeFunction The subtime function to analyze
 * \param theArea The area
 * \param thePeriods The time period generator
 * \param theAreaAcceptor The acceptor for data in area integrals
 * \param theTimeAcceptor The acceptor for data in time integrals
 * \param theTester The optional acceptor for Percentage calculations
 */
// ----------------------------------------------------------------------

WeatherResult ModuloParameterAnalyzer::analyze(const AnalysisSources& theSources,
                                               const WeatherDataType& theDataType,
                                               const WeatherFunction& theAreaFunction,
                                               const WeatherFunction& theTimeFunction,
                                               const WeatherFunction& theSubTimeFunction,
                                               const WeatherArea& theArea,
                                               const WeatherPeriodGenerator& thePeriods,
                                               const Acceptor& theAreaAcceptor,
                                               const Acceptor& theTimeAcceptor,
                                               const Acceptor& theTester) const
{
  auto_ptr<RegularFunctionAnalyzer> analyzer(
      new RegularFunctionAnalyzer(theAreaFunction, theTimeFunction, theSubTimeFunction));

  analyzer->modulo(itsModulo);

  return analyzer->analyze(theSources,
                           theDataType,
                           theArea,
                           thePeriods,
                           theAreaAcceptor,
                           theTimeAcceptor,
                           theTester,
                           itsVariable,
                           itsParameter);
}

}  // namespace TextGen

// ======================================================================
