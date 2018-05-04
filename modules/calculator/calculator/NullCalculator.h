// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::NullCalculator
 */
// ======================================================================

#ifndef TEXTGEN_NULLCALCULATOR_H
#define TEXTGEN_NULLCALCULATOR_H

#include "Calculator.h"

namespace TextGen
{
class Acceptor;

class NullCalculator : public Calculator
{
 public:
  virtual ~NullCalculator() {}
  virtual void operator()(float theValue);
  virtual float operator()() const;
  virtual void acceptor(const Acceptor& theAcceptor);
  virtual boost::shared_ptr<Calculator> clone() const;
  virtual void reset();

};  // class Calculator

}  // namespace TextGen

#endif  // TEXTGEN_NULLCALCULATOR_H

// ======================================================================
