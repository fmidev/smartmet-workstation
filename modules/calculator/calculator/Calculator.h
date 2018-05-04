// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::Calculator
 */
// ======================================================================

#ifndef TEXTGEN_CALCULATOR_H
#define TEXTGEN_CALCULATOR_H

#include <boost/shared_ptr.hpp>

namespace TextGen
{
class Acceptor;

class Calculator
{
 public:
  Calculator() {}
  virtual ~Calculator() {}
  virtual void operator()(float theValue) = 0;
  virtual float operator()() const = 0;
  virtual void acceptor(const Acceptor& theAcceptor) = 0;
  virtual boost::shared_ptr<Calculator> clone() const = 0;
  virtual void reset() = 0;

};  // class Calculator

}  // namespace TextGen

#endif  // TEXTGEN_CALCULATOR_H

// ======================================================================
