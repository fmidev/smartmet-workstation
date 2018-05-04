// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::StandardDeviationCalculator
 */
// ======================================================================

#ifndef TEXTGEN_STANDARDDEVIATIONCALCULATOR_H
#define TEXTGEN_STANDARDDEVIATIONCALCULATOR_H

#include "Calculator.h"
#include "Acceptor.h"
#include <boost/shared_ptr.hpp>

namespace TextGen
{
class StandardDeviationCalculator : public Calculator
{
 public:
  StandardDeviationCalculator();
  virtual ~StandardDeviationCalculator() {}
  virtual void operator()(float theValue);
  virtual float operator()() const;
  virtual void acceptor(const Acceptor& theAcceptor);
  virtual boost::shared_ptr<Calculator> clone() const;
  virtual void reset();

 private:
  boost::shared_ptr<Acceptor> itsAcceptor;
  long itsCounter;
  double itsSum;
  double itsSquaredSum;

};  // class StandardDeviationCalculator

}  // namespace TextGen

#endif  // TEXTGEN_STANDARDDEVIATIONCALCULATOR_H

// ======================================================================
