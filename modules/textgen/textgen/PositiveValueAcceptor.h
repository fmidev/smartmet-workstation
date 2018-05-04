// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::PositiveValueAcceptor
 */
// ======================================================================

#ifndef TEXTGEN_POSITIVEVALUEACCEPTOR_H
#define TEXTGEN_POSITIVEVALUEACCEPTOR_H

#include <calculator/Acceptor.h>

namespace TextGen
{
class PositiveValueAcceptor : public Acceptor
{
 public:
  virtual ~PositiveValueAcceptor() {}
  virtual bool accept(float theValue) const;
  virtual Acceptor* clone() const;

};  // class PositiveValueAcceptor
}  // namespace TextGen

#endif  // TEXTGEN_POSITIVEVALUEACCEPTOR_H

// ======================================================================
