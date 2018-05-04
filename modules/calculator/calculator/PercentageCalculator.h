// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::PercentageCalculator
 */
// ======================================================================

#ifndef TEXTGEN_PERCENTAGECALCULATOR_H
#define TEXTGEN_PERCENTAGECALCULATOR_H

#include "Calculator.h"
#include <boost/shared_ptr.hpp>

namespace TextGen
{
class PercentageCalculator : public Calculator
{
 public:
  PercentageCalculator();
  PercentageCalculator(const PercentageCalculator& theOther);
  virtual ~PercentageCalculator() {}
  virtual void operator()(float theValue);
  virtual float operator()() const;
  virtual void acceptor(const Acceptor& theAcceptor);
  virtual boost::shared_ptr<Calculator> clone() const;
  virtual void reset();

  void condition(const Acceptor& theCondition);

 private:
  Acceptor* itsAcceptor;
  Acceptor* itsCondition;
  long itsCounter;
  long itsTotalCounter;

};  // class PercentageCalculator

}  // namespace TextGen

#endif  // TEXTGEN_PERCENTAGECALCULATOR_H

// ======================================================================
