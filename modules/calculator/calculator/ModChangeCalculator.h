// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::ModChangeCalculator
 */
// ======================================================================

#ifndef TEXTGEN_MODCHANGECALCULATOR_H
#define TEXTGEN_MODCHANGECALCULATOR_H

#include "Calculator.h"
#include <boost/shared_ptr.hpp>

namespace TextGen
{
class ModChangeCalculator : public Calculator
{
 public:
  ModChangeCalculator(int theModulo);
  virtual ~ModChangeCalculator() {}
  virtual void operator()(float theValue);
  virtual float operator()() const;
  virtual void acceptor(const Acceptor& theAcceptor);
  virtual boost::shared_ptr<Calculator> clone() const;
  virtual void reset();

 private:
  ModChangeCalculator();

  boost::shared_ptr<Acceptor> itsAcceptor;
  const int itsModulo;

  long itsCounter;
  double itsCumulativeChange;
  double itsLastValue;

};  // class ModChangeCalculator

}  // namespace TextGen

#endif  // TEXTGEN_MODCHANGECALCULATOR_H

// ======================================================================
