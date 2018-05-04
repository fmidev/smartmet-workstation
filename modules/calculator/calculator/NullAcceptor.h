// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::NullAcceptor
 */
// ======================================================================

#ifndef TEXTGEN_NULLACCEPTOR_H
#define TEXTGEN_NULLACCEPTOR_H

#include "Acceptor.h"

namespace TextGen
{
class NullAcceptor : public Acceptor
{
 public:
  virtual ~NullAcceptor() {}
  virtual bool accept(float theValue) const;
  virtual Acceptor* clone() const;

};  // class NullAcceptor
}  // namespace TextGen

#endif  // TEXTGEN_NULLACCEPTOR_H

// ======================================================================
