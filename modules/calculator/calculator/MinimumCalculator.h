// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::MinimumCalculator
 */
// ======================================================================

#ifndef TEXTGEN_MINIMUMCALCULATOR_H
#define TEXTGEN_MINIMUMCALCULATOR_H

#include "Calculator.h"
#include <boost/shared_ptr.hpp>

namespace TextGen
{
class MinimumCalculator : public Calculator
{
 public:
  MinimumCalculator();
  virtual ~MinimumCalculator() {}
  virtual void operator()(float theValue);
  virtual float operator()() const;
  virtual void acceptor(const Acceptor& theAcceptor);
  virtual boost::shared_ptr<Calculator> clone() const;
  virtual void reset();

 private:
  boost::shared_ptr<Acceptor> itsAcceptor;
  long itsCounter;
  float itsMinimum;

};  // class MinimumCalculator

}  // namespace TextGen

#endif  // TEXTGEN_MINIMUMCALCULATOR_H

// ======================================================================
