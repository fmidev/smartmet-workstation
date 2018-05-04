// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::MeanCalculator
 */
// ======================================================================

#ifndef TEXTGEN_MEANCALCULATOR_H
#define TEXTGEN_MEANCALCULATOR_H

#include "Calculator.h"
#include <boost/shared_ptr.hpp>

namespace TextGen
{
class MeanCalculator : public Calculator
{
 public:
  MeanCalculator();
  virtual ~MeanCalculator() {}
  virtual void operator()(float theValue);
  virtual float operator()() const;
  virtual void acceptor(const Acceptor& theAcceptor);
  virtual boost::shared_ptr<Calculator> clone() const;
  virtual void reset();

 private:
  boost::shared_ptr<Acceptor> itsAcceptor;
  long itsCounter;
  double itsSum;

};  // class MeanCalculator

}  // namespace TextGen

#endif  // TEXTGEN_MEANCALCULATOR_H

// ======================================================================
