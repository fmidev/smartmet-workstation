// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::ModStandardDeviationCalculator
 */
// ======================================================================

#ifndef TEXTGEN_MODSTANDARDDEVIATIONCALCULATOR_H
#define TEXTGEN_MODSTANDARDDEVIATIONCALCULATOR_H

#include "Calculator.h"
#include <boost/shared_ptr.hpp>

namespace TextGen
{
class ModStandardDeviationCalculator : public Calculator
{
 public:
  ModStandardDeviationCalculator(int theModulo);
  virtual ~ModStandardDeviationCalculator() {}
  virtual void operator()(float theValue);
  virtual float operator()() const;
  virtual void acceptor(const Acceptor& theAcceptor);
  virtual boost::shared_ptr<Calculator> clone() const;
  virtual void reset();

 private:
  ModStandardDeviationCalculator();

  boost::shared_ptr<Acceptor> itsAcceptor;
  const int itsModulo;

  long itsCounter;
  double itsSum;
  double itsSquaredSum;
  double itsPreviousDirection;

};  // class ModStandardDeviationCalculator

}  // namespace TextGen

#endif  // TEXTGEN_MODSTANDARDDEVIATIONCALCULATOR_H

// ======================================================================
