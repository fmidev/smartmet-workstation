// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::AndAcceptor
 */
// ======================================================================

#ifndef TEXTGEN_ANDACCEPTOR_H
#define TEXTGEN_ANDACCEPTOR_H

#include <calculator/Acceptor.h>
#include <functional>

namespace TextGen
{
class AndAcceptor : public Acceptor
{
 public:
  AndAcceptor(const AndAcceptor& theOther);
  AndAcceptor(const Acceptor& theLhs, const Acceptor& theRhs);
  virtual ~AndAcceptor()
  {
    delete itsLhs;
    delete itsRhs;
  }
  virtual bool accept(float theValue) const;
  virtual Acceptor* clone() const;

 private:
  AndAcceptor();

  Acceptor* itsLhs;
  Acceptor* itsRhs;

};  // class AndAcceptor
}  // namespace TextGen

#endif  // TEXTGEN_ANDACCEPTOR_H

// ======================================================================
