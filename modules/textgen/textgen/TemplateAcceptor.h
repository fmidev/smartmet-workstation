// ======================================================================
/*!
 * \file
 * \brief Interface of class TextGen::TemplateAcceptor
 */
// ======================================================================

#ifndef TEXTGEN_TEMPLATEACCEPTOR_H
#define TEXTGEN_TEMPLATEACCEPTOR_H

#include <calculator/Acceptor.h>

namespace TextGen
{
template <class T>
class TemplateAcceptor : public Acceptor
{
 public:
  virtual ~TemplateAcceptor() {}
  TemplateAcceptor(const T& theFunctor) : itsFunctor(theFunctor) {}
  virtual bool accept(float theValue) const { return itsFunctor(theValue); }
  virtual Acceptor* clone() const { return new TemplateAcceptor(*this); }
 private:
  TemplateAcceptor(void);

  T itsFunctor;

};  // class TemplateAcceptor

typedef TemplateAcceptor<bool (*)(float)> FunctionAcceptor;

}  // namespace TextGen

#endif  // TEXTGEN_TEMPLATEACCEPTOR_H

// ======================================================================
