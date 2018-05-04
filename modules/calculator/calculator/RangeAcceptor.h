// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::RangeAcceptor
 */
// ======================================================================

#ifndef TEXTGEN_RANGEACCEPTOR_H
#define TEXTGEN_RANGEACCEPTOR_H

#include "Acceptor.h"

namespace TextGen
{
class RangeAcceptor : public Acceptor
{
 public:
  virtual ~RangeAcceptor() {}
  virtual bool accept(float theValue) const;
  virtual Acceptor* clone() const;

  RangeAcceptor();
  void lowerLimit(float theLimit);
  void upperLimit(float theLimit);

 private:
  float itsLoLimit;
  float itsHiLimit;

};  // class RangeAcceptor
}  // namespace TextGen

#endif  // TEXTGEN_RANGEACCEPTOR_H

// ======================================================================
