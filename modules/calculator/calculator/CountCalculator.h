// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::CountCalculator
 */
// ======================================================================

#ifndef TEXTGEN_COUNTCALCULATOR_H
#define TEXTGEN_COUNTCALCULATOR_H

#include "Calculator.h"
#include <boost/shared_ptr.hpp>

namespace TextGen
{
class CountCalculator : public Calculator
{
 public:
  CountCalculator();
  virtual ~CountCalculator() {}
  virtual void operator()(float theValue);
  virtual float operator()() const;
  virtual void acceptor(const Acceptor& theAcceptor);
  virtual boost::shared_ptr<Calculator> clone() const;
  virtual void reset();

  void condition(const Acceptor& theCondition);

 private:
  boost::shared_ptr<Acceptor> itsAcceptor;
  boost::shared_ptr<Acceptor> itsCondition;
  long itsCounter;
  long itsTotalCounter;

};  // class CountCalculator

}  // namespace TextGen

#endif  // TEXTGEN_COUNTCALCULATOR_H

// ======================================================================
