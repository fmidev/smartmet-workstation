// ======================================================================
/*!
 * \file
 * \brief Interface of namespace TextGen::UnitFactory
 */
// ======================================================================

#ifndef TEXTGEN_UNITFACTORY_H
#define TEXTGEN_UNITFACTORY_H

#include <boost/shared_ptr.hpp>

namespace TextGen
{
class Sentence;

enum Units
{
  DegreesCelsius,
  MetersPerSecond,
  Millimeters,
  Percent,
  HectoPascal,
  Meters,
};

namespace UnitFactory
{
boost::shared_ptr<Sentence> create(Units theUnit);
boost::shared_ptr<Sentence> create(Units theUnit, int value);
boost::shared_ptr<Sentence> create_unit(Units theUnit, int value, bool isInterval = false);

}  // namespace NumberFactory
}  // namespace TextGen

#endif  // TEXTGEN_UNITFACTORY_H

// ======================================================================
