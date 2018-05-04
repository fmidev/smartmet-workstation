// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::DefaultAcceptor
 */
// ======================================================================

#ifndef TEXTGEN_DEFAULTACCEPTOR_H
#define TEXTGEN_DEFAULTACCEPTOR_H

#include "Acceptor.h"

namespace TextGen
{
class DefaultAcceptor : public Acceptor
{
 public:
  virtual ~DefaultAcceptor() {}
  virtual bool accept(float theValue) const;
  virtual Acceptor* clone() const;

};  // class DefaultAcceptor
}  // namespace TextGen

#endif  // TEXTGEN_DEFAULTACCEPTOR_H

// ======================================================================
