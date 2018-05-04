// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::MaximumCalculator
 */
// ======================================================================

#ifndef TEXTGEN_MAXIMUMCALCULATOR_H
#define TEXTGEN_MAXIMUMCALCULATOR_H

#include "Calculator.h"
#include <boost/shared_ptr.hpp>

namespace TextGen
{
class MaximumCalculator : public Calculator
{
 public:
  MaximumCalculator();
  virtual ~MaximumCalculator() {}
  virtual void operator()(float theValue);
  virtual float operator()() const;
  virtual void acceptor(const Acceptor& theAcceptor);
  virtual boost::shared_ptr<Calculator> clone() const;
  virtual void reset();

 private:
  boost::shared_ptr<Acceptor> itsAcceptor;
  long itsCounter;
  float itsMaximum;

};  // class MaximumCalculator

}  // namespace TextGen

#endif  // TEXTGEN_MAXIMUMCALCULATOR_H

// ======================================================================
